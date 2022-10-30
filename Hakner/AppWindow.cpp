#include "AppWindow.h"
#include <SDL.h>
#include <assert.h>
#include "Helper.h"

#include "Dependencies/ImGUI/imgui.h"
#include "Dependencies/ImGUI/imgui_impl_sdl.h"
#include "Dependencies/ImGUI/imgui_impl_sdlrenderer.h"

void hakner::AppWindow::Initialize()
{
	// Initialize SDL
	LogAssert("SDL failed to initalize!", (SDL_Init(SDL_INIT_VIDEO) == 0));
	// Create window
	State.window = SDL_CreateWindow(State.title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, State.width, State.height, SDL_WINDOW_SHOWN);
	LogAssert("SDL failed to create window!", State.window);

	SDL_ShowCursor(SDL_FALSE);
	SDL_SetRelativeMouseMode(SDL_TRUE);

	// Creating SDL
	State.backBuffer = new uint32_t[State.pixelCount];
	State.renderer = SDL_CreateRenderer(State.window, -1, SDL_RENDERER_ACCELERATED);
	
	// Format is ABGR8888 because of little endianness
	State.texture = SDL_CreateTexture(State.renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, State.width, State.height);
	LogMsg(Log::Debug, "SDL Intialized.");

	IMGUI_CHECKVERSION();
    ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	
	// Custom Font
	io.Fonts->AddFontFromFileTTF("C:/Hakner/Hakner/Assets/Roboto.ttf", 20.0f, nullptr);

    ImGui::StyleColorsDark();
	

    // Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForSDLRenderer(State.window, State.renderer);
    ImGui_ImplSDLRenderer_Init(State.renderer);

	State.initialized = true;
}

void hakner::AppWindow::Destroy()
{
	SDL_DestroyWindow(State.window);
	SDL_DestroyRenderer(State.renderer);
	SDL_Quit();

	delete[] State.backBuffer;
}
