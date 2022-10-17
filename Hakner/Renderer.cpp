#include "Renderer.h"
#include "AppWindow.h"
#include "Math.h"
#include "Sphere.h"
#include <vector>

namespace hakner
{
	namespace Graphics
	{
		std::vector<Sphere> g_world;
		bool showNormals = false;

		void Renderer::Initialize()
		{
			g_world.push_back({ {0,0,-10}, {255,0,255,0}, 1.0f });
			g_world.push_back({ {2,0,-10}, {255,0,255,0}, 0.3f });
		}

		void Renderer::Update()
		{

		}

		Ray GenerateRay(int x, int y)
		{
			x -= AppWindow::State->width / 2;
			y -= AppWindow::State->height / 2;

			float cameraFOV = 90.0f;
			float tanHalfAngle = tan(cameraFOV * 0.5f);

			float mul = tanHalfAngle / (float)AppWindow::State->width;
			Vector3 direction = (Vector3(x * mul, -y * mul, -1));
			direction.Normalize();

			return { Vector3(0.0f , 0.0f, 0.0f), direction };
		}

		Color Sky(Ray ray)
		{
			float t = 0.5f * (ray.direction.y + 1.0f);

			// Lerp between white and blueish for sky color
			Vector3 vColor = (1.0f - t) * Vector3(1.0f, 1.0f, 1.0f) + t * Vector3(0.5f, 0.7f, 1.0f);
			vColor *= 255;

			return { (unsigned char)vColor.x, (unsigned char)vColor.y, (unsigned char)vColor.z, 0 };
		}

		Color VectorToColor(Vector3 v);
		Color VectorToColor(Vector4 v);

		// Assuming vector is 0.0 -> 1.0
		Color VectorToColor(Vector3 v) { return VectorToColor({ v.x, v.y, v.z, 0.0f }); };
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


		void Intersection(Ray& ray, HitData& data)
		{
			for (auto& currentSphere : g_world)
			{
				Vector3 oc = ray.origin - currentSphere.position;
				auto a = ray.direction.LengthSquared();
				auto half_b = oc.Dot(ray.direction);
				auto c = oc.LengthSquared() - currentSphere.GetRadiusSquared();
				auto discriminant = half_b * half_b - a * c;

				if (discriminant < 0)
					continue;

				auto t = (-half_b - sqrt(discriminant)) / a;

				if (t < 0 || t > data.distance)
					continue;

				data.intersections++;
				data.normal = (ray.At(t) - currentSphere.position);

				// Color
				if (showNormals)
				{
					Vector3 norm = data.normal * 0.5f;
					norm += Vector3(0.5f);
					data.color = VectorToColor(norm);
				}
				else
				{
					data.color = currentSphere.color;
				}
			}

		}

		void Raytrace(Ray ray, HitData& data)
		{
			Intersection(ray, data);
		}

		void Renderer::Render()
		{
			auto surface = AppWindow::State->backBuffer;
			auto& window = *AppWindow::State;

			for (int y = 0; y < (int)window.height; y++)
			{
				for (int x = 0; x < (int)window.width; x++)
				{
					int i = x + (y * window.width);

					HitData data;
					Ray ray = GenerateRay(x, y);

					Raytrace(ray, data);

					if (data.intersections == 0)
						surface[i] = Sky(ray).value;
					else
						surface[i] = data.color.value;
				}
			}
		}
	}
}