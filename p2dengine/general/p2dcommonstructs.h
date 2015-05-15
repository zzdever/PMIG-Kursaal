#ifndef P2D_COMMON_STRUCTS_H
#define P2D_COMMON_STRUCTS_H

#include "p2dmath.h"

/// Profiling data. Times are in milliseconds.
struct P2DProfile
{
	float32 step;
	float32 collide;
	float32 solve;
	float32 solveInit;
	float32 solveVelocity;
	float32 solvePosition;
    float32 coarseCollision;
	float32 solveTOI;
};

/// This is an internal structure.
struct P2DTimeStep
{
	float32 dt;			// time step
	float32 inv_dt;		// inverse time step (0 if dt == 0).
	float32 dtRatio;	// dt * inv_dt0
	int32 velocityIterations;
	int32 positionIterations;
	bool warmStarting;
};

/// This is an internal structure.
struct P2DPosition
{
	P2DVec2 c;
	float32 a;
};

/// This is an internal structure.
struct P2DVelocity
{
	P2DVec2 v;
	float32 w;
};

/// Solver Data
struct P2DSolverData
{
	P2DTimeStep step;
	P2DPosition* positions;
	P2DVelocity* velocities;
};

#endif
