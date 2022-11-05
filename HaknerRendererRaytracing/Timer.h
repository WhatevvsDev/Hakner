#pragma once

#include <chrono>

namespace hakner
{
	class Timer
	{
		using Clock = std::chrono::high_resolution_clock;
		using TimePoint = Clock::time_point;
		using Duration = std::chrono::duration<float, std::milli>;

		bool m_isTiming{ false };
		TimePoint m_start;
		TimePoint m_delta;

	public:
		//Starts timing from now
		void Start();

		//Sets the "start" time of the timer to now
		void Reset();

		// Returns the time from start -> now in milliseconds
		float ToNow();

		// Returns the time from last delta call -> now in milliseconds
		float Delta();

		// Returns the time from last delta call -> now in milliseconds without reseting the delta timer
		float PeekDelta();
	};
}