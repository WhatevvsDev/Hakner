#pragma once
#include <SDL.h>
#include <string>

namespace hakner
{
	namespace AppWindow
	{
		inline struct WindowState
		{
			// Client Area
			unsigned int width = 1280;
			unsigned int height = 720;
			unsigned int pixelCount = width * height;
			uint32_t* backBuffer { nullptr };
			
			// Window
			std::string title = "Hakner Raytracer";
			bool initialized { false };

			// SDL Related Variables
			SDL_Renderer* renderer { nullptr };
			SDL_Window* window { nullptr };
			SDL_Texture* texture { nullptr };
		} *State;

		void Initialize();
		void Destroy();
	}
}
