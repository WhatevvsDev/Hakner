#pragma once
#include <SDL.h>
#include <string>

#include "Dependencies/ImGUI/imgui.h"

namespace hakner
{
	namespace AppWindow
	{
		inline struct WindowState
		{
			// Client Area
			int width = 800;
			int height = 800;
			int pixelCount = width * height;
			uint32_t* backBuffer { nullptr };
			
			// Window
			std::string title = "Hakner Raytracer";
			bool initialized { false };
			bool shouldClose { false };

			// SDL Related Variables
			SDL_Renderer* renderer { nullptr };
			SDL_Window* window { nullptr };
			SDL_Texture* texture { nullptr };
		} *State;

		void Initialize();
		void Destroy();
	}
}
