#include "Timer.h"
#include <assert.h>

namespace hakner
{
	// Starts timing
	void Timer::Start()
	{
		if (m_isTiming)
			return;

		m_start = Clock::now();
		m_delta = m_start;
		m_isTiming = true;
	}

	// Sets the "start" time of the timer to now
	void Timer::Reset()
	{
		m_start = Clock::now();
	}

	// Returns the time from start -> now in milliseconds
	float Timer::ToNow()
	{
		assert(m_isTiming && "Timer is queried, but has not started timing");

		Duration duration = Clock::now() - m_start;
		return duration.count();
	}

	// Returns the time from last delta call -> now in milliseconds
	float Timer::Delta()
	{
		assert(m_isTiming && "Timer is queried, but has not started timing");

		TimePoint now = Clock::now();
		Duration duration = now - m_delta;
		m_delta = now;
		return duration.count();
	}

	// Returns the time from last delta call -> now in milliseconds without reseting the delta 
	float Timer::PeekDelta()
	{
		assert(m_isTiming && "Timer is queried, but has not started timing");

		TimePoint now = Clock::now();
		Duration duration = now - m_delta;
		return duration.count();
	}
}