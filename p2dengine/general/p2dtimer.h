#ifndef P2D_TIMER_H
#define P2D_TIMER_H

#include "p2dparams.h"

class P2DTimer
{
public:

	/// Constructor
	P2DTimer();

	/// Reset the timer.
	void Reset();

	/// Get the time since construction or the last reset.
	float32 GetMilliseconds() const;

private:

#if defined(_WIN32)
	float64 m_start;
	static float64 s_invFrequency;
#elif defined(__linux__) || defined (__APPLE__)
	unsigned long m_start_sec;
	unsigned long m_start_usec;
#endif
};

#endif
