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
	using namespace hakner;
	auto& windowData = AppWindow::State;

	AppWindow::Initialize();

	LogAssert("Failed to verify DirectXMath CPU support", DirectX::XMVerifyCPUSupport());
	
	// Window event related
	SDL_Event e;
	bool quit = false; 

	// Clear color
	SDL_SetRenderDrawColor(windowData->renderer,255,0,0,255);
	
	while (!quit) 
	{ 
		SDL_RenderClear(windowData->renderer);

		// Handle SDL events
		while (SDL_PollEvent(&e)) 
		{ 
			if (e.type == SDL_QUIT) 
				quit = true; 
		} 

		Renderer::Update();

		Renderer::Render();

		// Copy CPU backbuffer to GPU texture, and present that on-screen
		SDL_UpdateTexture(windowData->texture, NULL, windowData->backBuffer, windowData->width * sizeof(uint32_t));
		SDL_RenderCopy(windowData->renderer, windowData->texture, NULL, NULL);
		SDL_RenderPresent(windowData->renderer);
	}

	AppWindow::Destroy();

	return 0;
}