#include "p2dtimer.h"

#if defined(_WIN32)

float64 P2DTimer::s_invFrequency = 0.0f;

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

P2DTimer::P2DTimer()
{
	LARGE_INTEGER largeInteger;

	if (s_invFrequency == 0.0f)
	{
		QueryPerformanceFrequency(&largeInteger);
		s_invFrequency = float64(largeInteger.QuadPart);
		if (s_invFrequency > 0.0f)
		{
			s_invFrequency = 1000.0f / s_invFrequency;
		}
	}

	QueryPerformanceCounter(&largeInteger);
	m_start = float64(largeInteger.QuadPart);
}

void P2DTimer::Reset()
{
	LARGE_INTEGER largeInteger;
	QueryPerformanceCounter(&largeInteger);
	m_start = float64(largeInteger.QuadPart);
}

float32 P2DTimer::GetMilliseconds() const
{
	LARGE_INTEGER largeInteger;
	QueryPerformanceCounter(&largeInteger);
	float64 count = float64(largeInteger.QuadPart);
	float32 ms = float32(s_invFrequency * (count - m_start));
	return ms;
}

#elif defined(__linux__) || defined (__APPLE__)

#include <sys/time.h>

P2DTimer::P2DTimer()
{
    Reset();
}

void P2DTimer::Reset()
{
    timeval t;
    gettimeofday(&t, 0);
    m_start_sec = t.tv_sec;
    m_start_usec = t.tv_usec;
}

float32 P2DTimer::GetMilliseconds() const
{
    timeval t;
    gettimeofday(&t, 0);
    return 1000.0f * (t.tv_sec - m_start_sec) + 0.001f * (t.tv_usec - m_start_usec);
}

#else

// placeholders
P2DTimer::P2DTimer()
{}

void P2DTimer::Reset()
{}

float32 P2DTimer::GetMilliseconds() const
{
	return 0.0f;
}

#endif
