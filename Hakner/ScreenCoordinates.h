#pragma once

namespace hakner
{
	namespace Graphics
	{
		namespace ScreenCoord
		{
			struct NDC;
			struct Pixel;
			struct UV;

			struct NDC
			{
				float x;
				float y;

				// -1.0 to 1.0 -> -1.0 to 1.0
				NDC(float xin, float yin);

				// 1920 x 1080 -> -1.0 to 1.0
				NDC(const Pixel& in);

				// 0.0 to 1.0 -> -1.0 to 1.0
				NDC(const UV& in);
			};

			struct Pixel
			{
				int x;
				int y;

				// 1920 x 1080 -> 1920 x 1080
				Pixel(int xin, int yin);

				// -1.0 to 1.0 -> 1920 x 1080
				Pixel(const NDC& in);

				// 0.0 to 1.0 -> 1920 x 1080
				Pixel(const UV& in);
			};

			struct UV
			{
				float u;
				float v;

				// 0.0 to 1.0 -> 0.0 to 1.0
				UV(float xin, float yin);

				// -1.0 to 1.0 -> 0.0 to 1.0
				UV(const NDC& in);

				// 1920 x 1080 -> 0.0 to 1.0
				UV(const Pixel& in);
			};
		}
	}
}