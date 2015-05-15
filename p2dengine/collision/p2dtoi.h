#ifndef P2D_TOI_H
#define P2D_TOI_H

#include "../general/p2dmath.h"
#include "p2ddistance.h"

/// Input parameters for P2DTimeOfImpact
struct P2DTOIInput
{
	P2DDistanceProxy proxyA;
	P2DDistanceProxy proxyB;
	P2DSweep sweepA;
	P2DSweep sweepB;
	float32 tMax;		// defines sweep interval [0, tMax]
};

// Output parameters for P2DTimeOfImpact.
struct P2DTOIOutput
{
	enum State
	{
		e_unknown,
		e_failed,
		e_overlapped,
		e_touching,
		e_separated
	};

	State state;
	float32 t;
};

/// Compute the upper bound on time before two shapes penetrate. Time is represented as
/// a fraction between [0,tMax]. This uses a swept separating axis and may miss some intermediate,
/// non-tunneling collision. If you change the time interval, you should call this function
/// again.
/// Note: use P2DDistance to compute the contact point and normal at the time of impact.
void P2DTimeOfImpact(P2DTOIOutput* output, const P2DTOIInput* input);

#endif
