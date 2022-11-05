#pragma once

#include <DirectXMath.h>
#include "SimpleMath.h"
#include "Helper.h"

using namespace DirectX::SimpleMath;

namespace hakner
{
	namespace Graphics
	{
		struct Ray
		{
			Ray(Vector3 aOrigin, Vector3 aDirection)
				: origin(aOrigin)
				, direction(aDirection)
			{
#ifdef _DEBUG
				if (aDirection.LengthSquared() != 1)
				{
					LogMsg(Log::Error, "Ray direction is not normalized! Probably should be. (Automatically normalized)");
					aDirection.Normalize();
				}
#endif
			}

			Vector3 origin;
			float min{ 0.0001f };
			Vector3 direction;
			float max{ INFINITY };

			inline Vector3 At(float t) { return origin + direction * t; };
		};

		// Nameless struct/union preventing.
#pragma warning( push )
#pragma warning( disable : 4201 )

		struct Color
		{
			union
			{
				unsigned char channel[4]{};
				struct
				{
					unsigned char r;
					unsigned char g;
					unsigned char b;
					unsigned char a;
				};
				uint32_t value;
			};
		};
#pragma warning( pop )

		inline Vector3 VectorMin(Vector3& a, Vector3& b)
		{
			return
			{
				fminf(a.x, b.x),
				fminf(a.y, b.y),
				fminf(a.z, b.z)
			};
		}

		inline Vector3 VectorMax(Vector3& a, Vector3& b)
		{
			return
			{
				fmaxf(a.x, b.x),
				fmaxf(a.y, b.y),
				fmaxf(a.z, b.z)
			};
		}

	}
}