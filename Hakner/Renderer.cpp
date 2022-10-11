#include "Renderer.h"
#include "AppWindow.h"

namespace hakner
{
	void Renderer::Update()
	{

	}

	void Renderer::Render()
	{
		auto surface = AppWindow::State->backBuffer;
		auto& window = *AppWindow::State;

		for(int y = 0; y < window.height; y++)
		{
			for(int x = 0; x < window.width; x++)
			{
				int i = x + (y * window.width);

				int r = ((float)x / (float)window.width) * 255;
				int g = ((float)y / (float)window.height) * 255;

				surface[i] = (r << 16) + (g << 8);
			}
		}
	}
}