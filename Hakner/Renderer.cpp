#include "Renderer.h"
#include "AppWindow.h"
#include "Math.h"
#include <vector>

namespace hakner
{

	struct HitData
	{
		Color color { 1, 0, 1 };
		Vector3 normal { 0, 0, 0 };
		int intersections { 0 };
	};


	struct Sphere
	{
		Sphere(Vector3 aPosition, Color aColor, float aRadius)
			: position(aPosition)
			, color(aColor)
			, radius(aRadius)
			, radiusradius(aRadius * aRadius)
		{

		}

		float GetRadius() { return radius; };
		float GetRadiusSquared() { return radiusradius; };
		void SetRadius(float aRadius) { radius = aRadius; radiusradius = aRadius * aRadius; };

		Vector3 position { 0, 0, 0 };
		Color color { 255, 0, 255, 0 };

	private:
		float radius { 1 };
		float radiusradius { 1 };
	};

	std::vector<Sphere> g_world;

	void Renderer::Initialize()
	{
		g_world.push_back({{0,0,-10}, {255,0,255,0}, 1.0f});
		g_world.push_back({{2,0,-10}, {255,0,255,0}, 1.0f});
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

		return { (unsigned char)vColor.x, (unsigned char)vColor.y, (unsigned char)vColor.z, 0};
	}

	Color VectorToColor(Vector3 v);
	Color VectorToColor(Vector4 v);

	// Assuming vector is 0.0 -> 1.0
	Color VectorToColor(Vector3 v) { return VectorToColor({v.x, v.y, v.z, 0.0f}); };
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


	void Intersection(Ray& ray, HitData& data)
	{
		auto& currentSphere = g_world[0];

		Vector3 oc = ray.origin - currentSphere.position;
		auto a = ray.direction.LengthSquared();
		auto half_b = oc.Dot(ray.direction);
		auto c = oc.LengthSquared() - currentSphere.GetRadiusSquared();
		auto discriminant = half_b*half_b - a*c;

		if (discriminant < 0)
		{
			data.color = Sky(ray);
			return;
		}

		auto t = (-half_b - sqrt(discriminant) ) / a;

		if(t < 0)
		{
			data.color = Sky(ray);
			return;
		}

		data.normal = (ray.At(t) - currentSphere.position);
		data.color = currentSphere.color;

	}

	void Raytrace(Ray ray, HitData& data)
	{ 
		Intersection(ray, data);
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

				HitData data;

				Raytrace(GenerateRay(x, y), data);

				Vector3 norm = data.normal * 0.5f;
				norm += Vector3(0.5f);

				bool showNormals = true;

				if(showNormals)
					surface[i] = VectorToColor(norm).value;
				else
					surface[i] = data.color.value;

			}
		}
	}
}