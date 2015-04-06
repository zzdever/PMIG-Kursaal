#ifndef P2D_MATH_H
#define P2D_MATH_H

#include <math.h>
#include <p2dengine/general/p2dparams.h>
#include <p2dengine/general/p2dmathvector.h>
#include <p2dengine/general/p2dmathmatrix.h>

/*
/// This is a approximate yet fast inverse square-root.
inline float32 P2DInvSqrt(float32 x)
{
	union
	{
		float32 x;
		int32 i;
	} convert;

	convert.x = x;
	float32 xhalf = 0.5f * x;
	convert.i = 0x5f3759df - (convert.i >> 1);
	x = convert.x;
	x = x * (1.5f - xhalf * x * x);
	return x;
}
*/


template <typename T>
inline T P2DAbs(T a)
{
	return a > T(0) ? a : -a;
}

inline P2DVec2 P2DAbs(const P2DVec2& a)
{
	return P2DVec2(P2DAbs(a.x), P2DAbs(a.y));
}

inline P2DMat22 P2DAbs(const P2DMat22& A)
{
    return P2DMat22(P2DAbs(A.ex), P2DAbs(A.ey));
}


template <typename T>
inline T P2DMin(T a, T b)
{
	return a < b ? a : b;
}

inline P2DVec2 P2DMin(const P2DVec2& a, const P2DVec2& b)
{
	return P2DVec2(P2DMin(a.x, b.x), P2DMin(a.y, b.y));
}

template <typename T>
inline T P2DMax(T a, T b)
{
	return a > b ? a : b;
}

inline P2DVec2 P2DMax(const P2DVec2& a, const P2DVec2& b)
{
	return P2DVec2(P2DMax(a.x, b.x), P2DMax(a.y, b.y));
}

template <typename T>
inline T P2DClamp(T a, T low, T high)
{
    return P2DMax(low, P2DMin(a, high));
}

inline P2DVec2 P2DClamp(const P2DVec2& a, const P2DVec2& low, const P2DVec2& high)
{
    return P2DMax(low, P2DMin(a, high));
}

template<typename T> inline void P2DSwap(T& a, T& b)
{
	T tmp = a;
	a = b;
	b = tmp;
}

/// "Next Largest Power of 2
/// Given a binary integer value x, the next largest power of 2 can be computed by a SWAR algorithm
/// that recursively "folds" the upper bits into the lower bits. This process yields a bit vector with
/// the same most significant 1 as x, but all 1's below it. Adding 1 to that value yields the next
/// largest power of 2. For a 32-bit value:"
inline uint32 P2DNextLargestPowerOfTwo(uint32 x)
{
	x |= (x >> 1);
	x |= (x >> 2);
	x |= (x >> 4);
	x |= (x >> 8);
	x |= (x >> 16);
	return x + 1;
}

inline bool P2DIsPowerOfTwo(uint32 x)
{
	bool result = x > 0 && (x & (x - 1)) == 0;
	return result;
}


#endif
