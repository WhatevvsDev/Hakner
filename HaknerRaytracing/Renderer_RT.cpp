#include "Renderer_RT.h"
#include "Math.h"
#include "Sphere.h"
#include "AccelerationStructure.h"

#include <vector>
#include <thread>
#include <mutex>
#include <atomic>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

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

		RenderTile* renderTiles{ nullptr };

		::std::thread* threads;
		::std::atomic<int> nextTile{ -1 };
		::std::mutex tileM;
		::std::condition_variable tileCV;

		BVHAS* accelStructure{ nullptr };

		// TODO: Replace this
		::std::vector<Sphere> g_world;
		bool showNormals = false;

		Vector3 activeRenderPosition;
		Matrix activeRenderMatrix;

		float secondTimer { 0 };
		int framesThisSecond { 0 };
		int framesPerSecond { 0 };

		// ---------- INITIALIZE ----------

		void Renderer::Initialize(RenderTarget& aRenderTarget)
		{
			renderTarget = aRenderTarget;

			// ---------- Load Assets ----------

			for (int z = 0; z < 100; z++)
			{
				for (int x = 0; x < 100; x++)
				{
					g_world.push_back({ { (float)x , 0, (float)z}, {255,255,0,0}, 0.4f });
				}
			}

			accelStructure = new BVHAS(g_world);

			// ---------- Initialize Threads and Render Tiles ----------
			int tileCountX = renderTarget.width / tileSize;
			int tileCountY = renderTarget.height / tileSize;

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
			Internal::exitingApplication = true;
			tileCV.notify_all();

			for (int i = 0; i < availableThreads; i++)
				threads[i].join();
		}

		// ---------- UPDATE ----------

		void Renderer::Update()
		{
			// ---------- Converted to seconds for ease of use in movement ---------- 
			float deltaTime = updateTimer.Delta() * 0.001f;

			secondTimer += deltaTime;
			if(secondTimer >= 1.0f)
			{
				secondTimer -= 1.0f;
				framesPerSecond = framesThisSecond;
				framesThisSecond = 0;
			}
			else
			{
				framesThisSecond++;
			}

			// ---------- Get move direction, align with camera, and update position ----------
			Vector3 moveVector = Vector3{ (float)moveHor, (float)moveVer, (float)moveWard };
			moveVector = Vector3::Transform(moveVector, Camera.GetRotationMatrix());

			moveVector = {0,deltaTime,deltaTime};

			Camera.position += moveVector * 20.0f * deltaTime;
		}

		// ---------- RENDERING ----------

		void SaveRenderToFile()
		{
			stbi_write_jpg("Render.jpg", Renderer::renderTarget.width, Renderer::renderTarget.height, 4, Renderer::renderTarget.backBuffer, 95);
		}

		Ray Renderer::GeneratePinholeRay(ScreenCoord::Pixel aPixel)
		{
			// ---------- Generate new Ray ----------
			// TODO: All of this only has to be generated once, unless resolution or camera "lens" type changes
			aPixel.x -= renderTarget.width / 2;
			aPixel.y -= renderTarget.height / 2;

			float tanHalfAngle = tan(Renderer::Camera.fieldOfView * 0.5f);

			float mul = tanHalfAngle / (float)renderTarget.width;
			Vector3 direction = (Vector3(aPixel.x * mul, -aPixel.y * mul, -1));
			direction.Normalize();

			// ---------- Transform ray to align with camera direction ----------
			direction = Vector3::Transform(direction, activeRenderMatrix);

			return { activeRenderPosition, direction };
		}

		Color Sky(Ray aRay)
		{
			float t = 0.5f * (aRay.direction.y + 1.0f);

			// Lerp between colors for sky color
			Vector3 vColor = (1.0f - t) * Vector3(1.0f, 0.8f, 0.65f) + t * Vector3(0.4f, 0.6f, 1.0f);
			vColor *= 255;

			return { (unsigned char)vColor.x, (unsigned char)vColor.y, (unsigned char)vColor.z, 0 };
		}

		// Assuming vector is 0.0 -> 1.0
		Color VectorToColor(Vector4 aVector)
		{
			aVector.x *= 255;
			aVector.y *= 255;
			aVector.z *= 255;
			aVector.w *= 255;

			unsigned char r = (unsigned char)aVector.x;
			unsigned char g = (unsigned char)aVector.y;
			unsigned char b = (unsigned char)aVector.z;
			unsigned char a = (unsigned char)aVector.w;

			return { r,g,b,a };
		}
		Color VectorToColor(Vector3 aVector) { return VectorToColor({ aVector.x, aVector.y, aVector.z, 0.0f }); };

		void IntersectWorld(Ray& aRay, HitData& aData)
		{
			accelStructure->IntersectBVH(aRay, aData);
		}

		void RaytraceThreadMain()
		{
			while (!Renderer::Internal::exitingApplication)
			{
				RaytraceTile();
			}
		}

		// Thread safe function
		void RaytraceTile()
		{
			// QoL
			auto surface = Renderer::renderTarget.backBuffer;

			// Fetch new tile
			int nextTileIndex = nextTile.fetch_sub(1);

			// If there are no tiles left, exit
			if (nextTileIndex < 0)
			{
				std::unique_lock<std::mutex> lock(tileM);
				tileCV.notify_all();
				tileCV.wait(lock, []() { return (nextTile.load() >= 0) || (Renderer::Internal::exitingApplication); });
				return;
			}

			RenderTile& currentTile = renderTiles[nextTileIndex];

			// Internal offset
			for (int iy = 0; iy < 8; iy++)
			for (int ix = 0; ix < 8; ix++)
			{
				int backBufferIndex = (currentTile.x + ix) + (currentTile.y + iy) * Renderer::renderTarget.width;
				Ray generatedRay = Renderer::GeneratePinholeRay(ScreenCoord::Pixel(currentTile.x + ix, currentTile.y + iy));
				HitData data;

				IntersectWorld(generatedRay, data);

				surface[backBufferIndex] = data.bvhColor.value;

				/*
				if (!data.intersections)
					surface[backBufferIndex] = Sky(generatedRay).value;
				else
					surface[backBufferIndex] = data.color.value;
				*/
			}
		}

		void Renderer::Render()
		{
			// ---------- Reseting threads for next frame ----------

			nextTile.store(tileCount - 1);
			tileCV.notify_all();

			std::unique_lock<std::mutex> lock(tileM);
			tileCV.wait(lock, []() { return (nextTile.load() < 0) || Internal::exitingApplication; });

			float deltaTime = renderTimer.Delta();

			if (renderToFile)
			{
				SaveRenderToFile();
				renderToFile = false;
			}

			activeRenderMatrix = Renderer::Camera.GetRotationMatrix();
			activeRenderPosition = Renderer::Camera.position;
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