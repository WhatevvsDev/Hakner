#include "Renderer.h"
#include "AppWindow.h"
#include "Math.h"

namespace hakner
{
	void Renderer::Update()
	{

	}

	Ray GenerateRay(int x, int y)
	{
		x -= AppWindow::State->width / 2;
		y -= AppWindow::State->height / 2;

		float cameraFOV = 90.0f;

		float tanHalfAngle = tan(cameraFOV * 0.5f);
		
		float aspectScale = (float)AppWindow::State->width;
		
		float mul = tanHalfAngle / aspectScale;
		Vector3 direction = (Vector3(x * mul, -y * mul, -1));
		direction.Normalize();

		return { Vector3(0.f), direction };
	}

	Color Sky(Ray ray)
	{
		float t = 0.5f * (ray.direction.y + 1.0f);

		// Lerp between white and blueish for sky color
		Vector3 vColor = (1.0f - t) * Vector3(1.0f, 1.0f, 1.0f) + t * Vector3(0.5f, 0.7f, 1.0f);
		vColor *= 255;

		return { (unsigned char)vColor.x, (unsigned char)vColor.y, (unsigned char)vColor.z, 0};
	}

	struct
	{
		Vector3 position {0, 0, 10};
		float radius { 1 };
	} Sphere;

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
		
		return {r,g,b,a};
	}


	Color Raytrace(Ray ray)
	{ 
		Vector3 oc = ray.origin - Sphere.position;
		float a = ray.direction.LengthSquared();
		float b = 2.0f * oc.Dot(ray.direction);
		auto c = oc.LengthSquared() - Sphere.radius * Sphere.radius;
		auto discriminant = b*b - 4*a*c;

		if (discriminant < 0)
			return Sky(ray);
		else
			return {255, 0, 0, 0};
	}

	void Renderer::Render()
	{
		auto surface = AppWindow::State->backBuffer;
		auto& window = *AppWindow::State;

		for(int y = 0; y < (int)window.height; y++)
		{
			for(int x = 0; x < (int)window.width; x++)
			{
				int i = x + (y * window.width);

				surface[i] = Raytrace(GenerateRay(x, y)).value;
			}
		}
	}
}