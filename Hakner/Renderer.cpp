#include "Renderer.h"
#include "AppWindow.h"
#include "Math.h"
#include "Sphere.h"
#include "AccelerationStructure.h"

#include <vector>
#include <thread>
#include <mutex>
#include <atomic>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "Dependencies/ImGUI/imgui.h"

namespace hakner
{
	namespace Graphics
	{
		class RenderTile
		{
		public:
			int x = { 0 };
			int y = { 0 };

		public:
			RenderTile() = default;
			RenderTile(int x, int y) : x(x), y(y) {}
		};

		const int tileSize = 8;
		int tileCount = -1;
		int availableThreads = 0;

		RenderTile* renderTiles { nullptr };

		::std::thread* threads;
		::std::atomic<int> nextTile{ -1 };
		::std::atomic<bool> finishedRendering{ false };
		::std::mutex tileM;
		::std::condition_variable tileCV;

		BVHAS* accelStructure { nullptr };

		// TODO: Replace this
		::std::vector<Sphere> g_world;
		bool showNormals = false;

		Vector3 activeRenderPosition;
		Matrix activeRenderMatrix;

		// ---------- INITIALIZE ----------

		void Renderer::Initialize()
		{
			// ---------- Load Assets ----------

			for(int z = 0; z < 100; z++)
			{
				for(int x = 0; x < 100; x++)
				{
					g_world.push_back({ { (float)x, 0, (float)z }, {255,255,0,0}, 0.4f });
				}
			}

			accelStructure = new BVHAS(g_world);

			// ---------- Initialize Threads and Render Tiles ----------
			int tileCountX = AppWindow::State->width / tileSize;
			int tileCountY = AppWindow::State->height / tileSize;

			tileCount = tileCountX * tileCountY;

			renderTiles = new RenderTile[tileCount];

			for (int y = 0, i = 0; y < tileCountY; y++)
				for (int x = 0; x < tileCountX; x++, i++)
					renderTiles[i] = RenderTile(x * 8, y * 8);

			availableThreads = std::thread::hardware_concurrency();
			threads = new std::thread[availableThreads];

			for (int i = 0; i < availableThreads; i++)
			{
				threads[i] = std::thread(RaytraceThreadMain);
				printf("Created Thread %i \n", i);
			}

			// ---------- Start Profiling Timers ----------
			updateTimer.Start();
			renderTimer.Start();
		}

		void Renderer::Destroy()
		{
			for (int i = 0; i < availableThreads; i++)
				threads[i].join();
		}

		// ---------- INPUT ----------

		void Renderer::MouseMove(int aDeltaX, int aDeltaY)
		{
			Camera.AddYaw((float)-aDeltaX);
			Camera.AddPitch((float)-aDeltaY);
		}

		void Renderer::KeyPress(SDL_Scancode aKey, bool aPressed)
		{
			switch (aKey)
			{
			case SDL_SCANCODE_W:
				forward = aPressed;
				break;
			case SDL_SCANCODE_A:
				left = aPressed;
				break;
			case SDL_SCANCODE_S:
				backward = aPressed;
				break;
			case SDL_SCANCODE_D:
				right = aPressed;
				break;
			case SDL_SCANCODE_LSHIFT:
			case SDL_SCANCODE_RSHIFT:
				down = aPressed;
				break;
			case SDL_SCANCODE_P:
				if (!aPressed) // Released
					renderToFile = true;
				break;
			case SDL_SCANCODE_SPACE:
				up = aPressed;
				break;
			}

			moveHor = right - left;
			moveWard = backward - forward;
			moveVer = up - down;
		}

		// ---------- UPDATE ----------

		void Renderer::Update()
		{
			// ---------- Converted to seconds for ease of use in movement ---------- 
			float deltaTime = updateTimer.Delta() * 0.001f;

			// ---------- Get move direction, align with camera, and update position ----------
			Vector3 moveVector = Vector3{ (float)moveHor, (float)moveVer, (float)moveWard };
			moveVector = Vector3::Transform(moveVector, Camera.GetRotationMatrix());

			Camera.position += moveVector * 20.0f * deltaTime;
		}

		// ---------- RENDERING ----------

		void SaveRenderToFile()
		{
			stbi_write_jpg("Render.jpg", AppWindow::State->width, AppWindow::State->height, 4, AppWindow::State->backBuffer, 95);
		}

		Ray Renderer::GeneratePinholeRay(ScreenCoord::Pixel pixel)
		{
			// ---------- Generate new Ray ----------
			// TODO: All of this only has to be generated once, unless resolution or camera "lens" type changes
			pixel.x -= AppWindow::State->width / 2;
			pixel.y -= AppWindow::State->height / 2;

			float tanHalfAngle = tan(Renderer::Camera.fieldOfView * 0.5f);

			float mul = tanHalfAngle / (float)AppWindow::State->width;
			Vector3 direction = (Vector3(pixel.x * mul, -pixel.y * mul, -1));
			direction.Normalize();

			// ---------- Transform ray to align with camera direction ----------
			direction = Vector3::Transform(direction, activeRenderMatrix);

			return { activeRenderPosition, direction };
		}

		Color Sky(Ray ray)
		{
			float t = 0.5f * (ray.direction.y + 1.0f);

			// Lerp between colors for sky color
			Vector3 vColor = (1.0f - t) * Vector3(1.0f, 0.8f, 0.65f) + t * Vector3(0.4f, 0.6f, 1.0f);
			vColor *= 255;

			return { (unsigned char)vColor.x, (unsigned char)vColor.y, (unsigned char)vColor.z, 0 };
		}

		// Assuming vector is 0.0 -> 1.0
		Color VectorToColor(Vector4 v)
		{
			v.x *= 255;
			v.y *= 255;
			v.z *= 255;
			v.w *= 255;

			unsigned char r = (unsigned char)v.x;
			unsigned char g = (unsigned char)v.y;
			unsigned char b = (unsigned char)v.z;
			unsigned char a = (unsigned char)v.w;

			return { r,g,b,a };
		}
		Color VectorToColor(Vector3 v) { return VectorToColor({ v.x, v.y, v.z, 0.0f }); };

		void IntersectWorld(Ray& ray, HitData& data)
		{
			accelStructure->IntersectBVH(ray,data);
		}

		void RaytraceThreadMain()
		{
			while(!	AppWindow::State->shouldClose)
			{
				RaytraceTile();
			}
		}

		// Thread safe function
		void RaytraceTile()
		{
			// QoL
			auto surface = AppWindow::State->backBuffer;

			// Fetch new tile
			int nextTileIndex = nextTile.fetch_sub(1);

			// If there are no tiles left, exit
			if (nextTileIndex < 0)
			{
				std::unique_lock<std::mutex> lock(tileM);

				if(nextTileIndex == -(availableThreads + 1))
				{
					finishedRendering.store(true);
					tileCV.notify_all();
				}

				tileCV.wait(lock, [](){ return !finishedRendering.load(); });
				return;
			}

			RenderTile& currentTile = renderTiles[nextTileIndex];

			// Internal offset
			for (int iy = 0; iy < 8; iy++)
			for (int ix = 0; ix < 8; ix++)
			{
				int backBufferIndex = (currentTile.x + ix) + (currentTile.y + iy) * AppWindow::State->width;
				Ray generatedRay = Renderer::GeneratePinholeRay(ScreenCoord::Pixel(currentTile.x + ix, currentTile.y + iy));
				HitData data;

				IntersectWorld(generatedRay, data);

				//surface[backBufferIndex] = VectorToColor(Vector3(data.bvhIntersections / 16.0f, (data.intersections / 16.0f), 0.0f)).value;
				
				if (!data.intersections)
					surface[backBufferIndex] = Sky(generatedRay).value;
				else if (showNormals)
					surface[backBufferIndex] = data.color.value;
				else
					surface[backBufferIndex] = VectorToColor(data.normal * 0.5f + Vector3{ 0.5f }).value;
			}
		}

		void Renderer::Render()
		{
			// While not finished rendering
			while(!finishedRendering.load())
			{
				if(AppWindow::State->shouldClose)
					return;
			}

			float deltaTime = renderTimer.Delta();

			if (renderToFile)
			{
				SaveRenderToFile();
				renderToFile = false;
			}

			ImGui::SetNextWindowPos({ 0,0 });
			ImGui::SetNextWindowSize({ 200, 80 });
			ImGui::Begin("Metrics", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDecoration);

			ImGui::Text(" %.2fms per frame \n", deltaTime);

			ImGui::End();

			activeRenderMatrix = Renderer::Camera.GetRotationMatrix();
			activeRenderPosition = Renderer::Camera.position;

			finishedRendering.store(false);
			nextTile.store(tileCount - 1);
			tileCV.notify_all();
		}

		// ---------- CAMERA ----------

		void Renderer::CameraData::AddPitch(float aPitch)
		{
			dirtyRotationMatrix = true;
			pitch += aPitch;

			if (pitch > 85) pitch = 85;
			if (pitch < -85) pitch = -85;
		}

		void Renderer::CameraData::AddYaw(float aYaw)
		{
			dirtyRotationMatrix = true;
			yaw += aYaw;
		}

		void Renderer::CameraData::SetRotation(float aPitch, float aYaw)
		{
			dirtyRotationMatrix = true;
			pitch = aPitch;
			yaw = aYaw;
		}

		Matrix Renderer::CameraData::GetRotationMatrix()
		{
			if (dirtyRotationMatrix)
				CalculateRotationMatrix();

			return rotationMatrix;
		}

		void Renderer::CameraData::CalculateRotationMatrix()
		{
			rotationMatrix = Matrix::CreateRotationX(DirectX::XMConvertToRadians(Camera.pitch)) * Matrix::CreateRotationY(DirectX::XMConvertToRadians(Camera.yaw));
		}

	}
}