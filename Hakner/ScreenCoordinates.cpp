#include "ScreenCoordinates.h"
#include "AppWindow.h"

namespace hakner
{
	namespace Graphics
	{
		namespace ScreenCoord
		{
			/* ---- NDC SCREEN POS ---- */

			// -1.0 to 1.0 -> -1.0 to 1.0
			NDC::NDC(float xin, float yin)
				: x(xin)
				, y(yin)
			{};

			// 1920 x 1080 -> -1.0 to 1.0
			NDC::NDC(const Pixel& in)
				: x(((static_cast<float>(in.x) / static_cast<float>(AppWindow::State.width)) * 2.0f) - 1.0f)
				, y(((static_cast<float>(in.y) / static_cast<float>(AppWindow::State.height)) * 2.0f) - 1.0f)
			{};

			// 0.0 to 1.0 -> -1.0 to 1.0
			NDC::NDC(const UV& in)
				: x((in.u * 2.0f) - 1.0f)
				, y((in.v * 2.0f) - 1.0f)
			{};

			/* ---- PIXEL SCREEN POS ---- */

			// 1920 x 1080 -> 1920 x 1080
			Pixel::Pixel(int xin, int yin)
				: x(xin)
				, y(yin)
			{};

			// -1.0 to 1.0 -> 1920 x 1080
			Pixel::Pixel(const NDC& in)
				: x((in.x* static_cast<float>(AppWindow::State.width)) + (static_cast<float>(AppWindow::State.width) * 0.5f))
				, y((in.y* static_cast<float>(AppWindow::State.height)) + (static_cast<float>(AppWindow::State.height) * 0.5f))
			{};

			// 0.0 to 1.0 -> 1920 x 1080
			Pixel::Pixel(const UV& in)
				: x(in.u* static_cast<float>(AppWindow::State.width))
				, y(in.v* static_cast<float>(AppWindow::State.height))
			{};

			/* ---- UV SCREEN POS ---- */

			// 0.0 to 1.0 -> 0.0 to 1.0
			UV::UV(float xin, float yin)
				: u(xin)
				, v(yin)
			{};

			// -1.0 to 1.0 -> 0.0 to 1.0
			UV::UV(const NDC& in)
				: u((in.x + 1.0f) * 0.5f)
				, v((in.y + 1.0f) * 0.5f)
			{};

			// 1920 x 1080 -> 0.0 to 1.0
			UV::UV(const Pixel& in)
				: u(in.x / static_cast<float>(AppWindow::State.width))
				, v(in.y / static_cast<float>(AppWindow::State.height))
			{};
		}
	}
}