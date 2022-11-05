#pragma once

namespace hakner
{
	namespace Graphics
	{
		namespace Renderer
		{
			// ---------- Main Renderer Functions ----------
			void Initialize() = delete;
			void Update();
			void Render();
			void Destroy();

			namespace Internal
			{
				inline bool exitingApplication { false };
			}
		}
	}
}