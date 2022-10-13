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
		float fov = 90.0f;

		float tanHalfAngle = tan(fov * 0.5f);

		float aspectScale = (float)AppWindow::State->width;
		float mod = tanHalfAngle / aspectScale;

		Vector3 direction = Vector3(x * mod, -y * mod, -1);

		direction.Normalize();

		return {Vector3(0.0f), direction};
	}

	Color Raytrace(Ray ray)
	{
		float t = 0.5f * (ray.direction.y + 1.0f);

		// Lerp between white and blueish for sky color
		Vector3 vColor = (1.0f - t) * Vector3(1.0f, 1.0f, 1.0f) + t * Vector3(0.5f, 0.7f, 1.0f);
		vColor *= 255;

		return { (unsigned char)vColor.x, (unsigned char)vColor.y, (unsigned char)vColor.z, 0};
	}

	void Renderer::Render()
	{
		auto surface = AppWindow::State->backBuffer;
		auto& window = *AppWindow::State;

		for(unsigned int y = 0; y < window.height; y++)
		{
			for(unsigned int x = 0; x < window.width; x++)
			{
				int i = x + (y * window.width);

				surface[i] = Raytrace(GenerateRay(x, y)).value;
			}
		}
	}
}