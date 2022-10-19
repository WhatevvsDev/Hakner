#pragma once

namespace hakner
{
	namespace Graphics
	{
		struct Sphere
		{
			Sphere(Vector3 aPosition, Color aColor, float aRadius)
				: position(aPosition)
				, color(aColor)
				, radius(aRadius)
				, radiusradius(aRadius* aRadius)
			{

			}

			float GetRadius() { return radius; };
			float GetRadiusSquared() { return radiusradius; };
			void SetRadius(float aRadius) { radius = aRadius; radiusradius = aRadius * aRadius; };

			Vector3 position{ 0, 0, 0 };
			Color color{ 255, 0, 255, 0 };

		private:
			float radius{ 1 };
			float radiusradius{ 1 };
		};
	}
}