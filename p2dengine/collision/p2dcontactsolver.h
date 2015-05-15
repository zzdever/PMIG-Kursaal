#ifndef P2D_CONTACT_SOLVER_H
#define P2D_CONTACT_SOLVER_H

#include "../general/p2dmath.h"
#include "../collision/p2dcollision.h"
#include "../general/p2dcommonstructs.h"

class P2DContact;
class P2DBody;
class P2DStackMem;
struct P2DContactPositionConstraint;

struct P2DVelocityConstraintPoint
{
	P2DVec2 rA;
	P2DVec2 rB;
	float32 normalImpulse;
	float32 tangentImpulse;
	float32 normalMass;
	float32 tangentMass;
	float32 velocityBias;
};

struct P2DContactVelocityConstraint
{
	P2DVelocityConstraintPoint points[P2D_MAX_MANIFOLD_POINTS];
	P2DVec2 normal;
	P2DMat22 normalMass;
	P2DMat22 K;
	int32 indexA;
	int32 indexB;
	float32 invMassA, invMassB;
	float32 invIA, invIB;
	float32 friction;
	float32 restitution;
	float32 tangentSpeed;
	int32 pointCount;
	int32 contactIndex;
};

struct P2DContactSolverDef
{
	P2DTimeStep step;
	P2DContact** contacts;
	int32 count;
	P2DPosition* positions;
	P2DVelocity* velocities;
	P2DStackMem* allocator;
};

class P2DContactSolver
{
public:
	P2DContactSolver(P2DContactSolverDef* def);
	~P2DContactSolver();

	void InitializeVelocityConstraints();

	void WarmStart();
	void SolveVelocityConstraints();
	void StoreImpulses();

	bool SolvePositionConstraints();
	bool SolveTOIPositionConstraints(int32 toiIndexA, int32 toiIndexB);

	P2DTimeStep m_step;
	P2DPosition* m_positions;
	P2DVelocity* m_velocities;
	P2DStackMem* m_allocator;
	P2DContactPositionConstraint* m_positionConstraints;
	P2DContactVelocityConstraint* m_velocityConstraints;
	P2DContact** m_contacts;
	int m_count;
};

#endif

