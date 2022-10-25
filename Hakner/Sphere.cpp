#include "Sphere.h"
#include "Renderer.h"
#include "Math.h"

namespace hakner
{
	namespace Graphics
	{
		void Sphere::Intersect(Ray& ray, HitData& data)
		{
			Vector3 oc = ray.origin - position;
			float half_b = oc.Dot(ray.direction);
			float c = oc.LengthSquared() - GetRadiusSquared();
			float discriminant = half_b * half_b - c;

			if (discriminant < 0)
				return;

			float sqrtd = sqrtf(discriminant);

			// Find the nearest root that lies in the acceptable range.
			float root = (-half_b - sqrtd);
			if (root < ray.min || ray.max < root)
			{
				root = (-half_b + sqrtd);
				if (root < ray.min || ray.max < root)
					return;
			}

			if (root > data.distance)
				return;

			data.distance = root;
			data.hitPosition = ray.At(data.distance);
			data.normal = (data.hitPosition - position);
			data.intersections++;
			data.color = color;
		}
	}
}