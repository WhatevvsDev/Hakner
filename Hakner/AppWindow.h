#pragma once
#include <SDL.h>
#include <string>

namespace hakner
{
	namespace AppWindow
	{
		inline struct WindowState
		{
			std::string title = "Hakner Raytracer";

			unsigned int width = 1280;
			unsigned int height = 720;
			unsigned int pixelCount = width * height;
			SDL_Renderer* renderer { nullptr };
			SDL_Window* window { nullptr };
			SDL_Texture* texture { nullptr };
			bool initialized { false };
			uint32_t* backBuffer { nullptr };
		} *State;

		void Initialize();
		void Destroy();
	}
}
