#pragma once

#include "Math.h"
#include "SDL.h"
#include "Timer.h"

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
			float distance { INFINITY };
		};

		namespace Renderer
		{
			// ---------- Main Functions ----------
			void Initialize();
			void Destroy();
			void Update();
			void Render();

			// ---------- Initialize Window and Renderer ----------
			void MouseMove(int aDeltaX, int aDeltaY);
			void KeyPress(SDL_Scancode aKey, bool aPressed);

			// ---------- Inputs ----------
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