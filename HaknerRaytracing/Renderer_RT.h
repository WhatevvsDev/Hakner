#pragma once
#include "Renderer.h"

#include "Math.h"
#include "Timer.h"
#include "ScreenCoordinates.h"

namespace hakner
{
	namespace Graphics
	{
		struct HitData
		{
			Color color { 1, 0, 1 };
			Vector3 normal { 0, 0, 0 };
			Vector3 hitPosition { 0, 0, 0};
			int intersections { 0 };
			int bvhIntersections { 0 };
			float distance { INFINITY };
			Color bvhColor;
		};

		// TODO: do this properly
		struct Sphere;

		void RaytraceThreadMain();
		void RaytraceTile();

		namespace Renderer
		{
			inline struct RenderTarget
			{
				int width { 0 };
				int height { 0 };
				uint32_t* backBuffer { nullptr };
			} renderTarget;

			// ---------- Main Functions ---------- 
			
			void Initialize();
			void Initialize(RenderTarget& aRenderTarget);

			// ---------- Ray Generation Functions ----------
			// TODO: implement these :)
			Ray GeneratePinholeRay(ScreenCoord::Pixel aPixel);
			Ray GenerateThinLensRay(ScreenCoord::UV aUV);
			Ray GeneratePaniniRay(ScreenCoord::Pixel aPixel);
			Ray GenerateFisheyeRay(ScreenCoord::Pixel aPixel);
			Ray GenerateLensletRay(ScreenCoord::UV aUV);
			Ray GenerateOctahedralRay(ScreenCoord::UV aUV);
			Ray GenerateCubeMapRay(ScreenCoord::Pixel aPixel);
			Ray GenerateOrthographicRay(ScreenCoord::NDC aNDC);
			Ray GenerateFibonacciSphereRay(ScreenCoord::UV aUV);

			// TODO: Replace this
			inline bool left = false;
			inline bool right = false;
			inline int moveHor = 0;
			inline bool forward = false;
			inline bool backward = false;
			inline int moveWard = 0;
			inline bool up = false;
			inline bool down = false;
			inline int moveVer = 0;

			// ---------- Camera ----------
			inline struct CameraData
			{
				Vector3 position{ 0, 0, 10};
				Vector3 lookAt { 0, 0, 0};
				float fieldOfView { 90.0f };

				void AddPitch(float aPitch);
				void AddYaw(float aYaw);
				void SetRotation(float aPitch, float aYaw);
				Matrix GetRotationMatrix();

				
			private:
				float pitch { 0 };
				float yaw { 0 };
				Matrix rotationMatrix;
				bool dirtyRotationMatrix { true };
				void CalculateRotationMatrix();
			} Camera;

			// ---------- Misc ----------
			inline Timer updateTimer;
			inline Timer renderTimer;
			inline bool renderToFile{ false };
		}
	}
}