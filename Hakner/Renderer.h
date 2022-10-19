#pragma once

#include "Math.h"

namespace hakner
{
	namespace Graphics
	{
		struct HitData
		{
			Color color { 1, 0, 1 };
			Vector3 normal { 0, 0, 0 };
			Vector3 hitPosition { 0, 0, 0};
			int intersections { 0 };
			float distance { INFINITY };
		};

		namespace Renderer
		{
			void Initialize();

			void Update();
			void Render();
		}
	}
}