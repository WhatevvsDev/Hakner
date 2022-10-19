#include <cstdio>
#include <assert.h>

#include <DirectXMath.h>
#include "AppWindow.h"
#include "Renderer.h"
#include "Helper.h"
#include <SDL.h>

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
	bool quit = false; 
	
	while (!quit) 
	{ 
		SDL_RenderClear(windowData->renderer);

		// Handle SDL events
		while (SDL_PollEvent(&e)) 
		{ 
			quit = (e.type == SDL_QUIT); 

			switch(e.type)
			{
				case SDL_QUIT:
					quit = true;
				break;
				case SDL_KEYDOWN:
					Graphics::Renderer::KeyPress(e.key.keysym.scancode, true);
					break;
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

		// ---------- Copy CPU backbuffer to GPU texture, and present that on-screen ----------
		SDL_UpdateTexture(windowData->texture, NULL, windowData->backBuffer, windowData->width * sizeof(uint32_t));
		SDL_RenderCopy(windowData->renderer, windowData->texture, NULL, NULL);
		SDL_RenderPresent(windowData->renderer);
	}

	AppWindow::Destroy();

	return 0;
}