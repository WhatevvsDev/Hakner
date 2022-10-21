#include <cstdio>
#include <assert.h>

#include <DirectXMath.h>
#include "AppWindow.h"
#include "Renderer.h"
#include "Helper.h"
#include <SDL.h>

#include "Dependencies/ImGUI/imgui.h"
#include "Dependencies/ImGUI/imgui_impl_sdl.h"
#include "Dependencies/ImGUI/imgui_impl_sdlrenderer.h"

#ifdef main
#undef main
#endif main

int main()
{
	// QoL
	using namespace hakner;
	auto& windowData = AppWindow::State;

	LogAssert("Failed to verify DirectXMath CPU support", DirectX::XMVerifyCPUSupport());
	
	// ---------- Initialize Window and Renderer ----------
	AppWindow::Initialize();
	Graphics::Renderer::Initialize();

	SDL_SetRenderDrawColor(windowData->renderer,255,0,0,255);
	
	// ---------- Main Loop ----------
	SDL_Event e;
	
	while (!AppWindow::State->shouldClose) 
	{ 
		SDL_RenderClear(windowData->renderer);

		ImGui_ImplSDLRenderer_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

		// Handle SDL events
		while (SDL_PollEvent(&e)) 
		{ 
			ImGui_ImplSDL2_ProcessEvent(&e);

			switch(e.type)
			{
				case SDL_QUIT:
					AppWindow::State->shouldClose = true;
				break;
				case SDL_KEYDOWN:
				{
					Graphics::Renderer::KeyPress(e.key.keysym.scancode, true);
					if(e.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
						AppWindow::State->shouldClose = true;
					break;
				}
				case SDL_KEYUP:
					Graphics::Renderer::KeyPress(e.key.keysym.scancode, false);
					break;
				case SDL_MOUSEMOTION:
					Graphics::Renderer::MouseMove(e.motion.xrel, e.motion.yrel);
					break;
			}
		} 

		Graphics::Renderer::Update();

		Graphics::Renderer::Render();

		ImGui::Render();


		// ---------- Copy CPU backbuffer to GPU texture, and present that on-screen ----------
		SDL_UpdateTexture(windowData->texture, NULL, windowData->backBuffer, windowData->width * sizeof(uint32_t));
		SDL_RenderCopy(windowData->renderer, windowData->texture, NULL, NULL);

		ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
		SDL_RenderPresent(windowData->renderer);
	}

	AppWindow::Destroy();

	return 0;
}