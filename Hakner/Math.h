#pragma once

#include <DirectXMath.h>
#include "SimpleMath.h"
#include "Helper.h"

using namespace DirectX::SimpleMath;

namespace hakner
{
	struct Ray
	{

		Ray(Vector3 aOrigin, Vector3 aDirection)
			: origin(aOrigin)
			, direction(aDirection)
		{
#ifdef _DEBUG
			if(aDirection.LengthSquared() != 1)
			{
				LogMsg(Log::Error, "Ray direction is not normalized! Probably should be. (Automatically normalized)");
				aDirection.Normalize();
			}
#endif
		}

		Vector3 origin;
		float min;
		Vector3 direction;
		float max;

		inline Vector3 At(float t) { return origin + direction * t; };
	};

	// Nameless struct/union preventing.
	#pragma warning( push )
	#pragma warning( disable : 4201 )

	struct Color
	{
		union
		{
			unsigned char channel[4] {};
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
}