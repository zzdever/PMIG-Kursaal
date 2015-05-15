#include "../collision/p2ddistance.h"
#include "p2disland.h"
#include "p2dbody.h"
#include "p2dfixture.h"
#include "p2dscenemanager.h"
#include "../collision/p2dcontact.h"
#include "../collision/p2dcontactsolver.h"
//#include "Joint.h"
#include "../general/p2dmem.h"
#include "../general/p2dtimer.h"

/*
2D Rotation

R = [cos(theta) -sin(theta)]
    [sin(theta) cos(theta) ]

thetaDot = omega

Let q1 = cos(theta), q2 = sin(theta).
R = [q1 -q2]
    [q2  q1]

q1Dot = -thetaDot * q2
q2Dot = thetaDot * q1

q1_new = q1_old - dt * w * q2
q2_new = q2_old + dt * w * q1
then normalize.

This might be faster than computing sin+cos.
However, we can compute sin+cos of the same angle fast.
*/

P2DIsland::P2DIsland(
	int32 bodyCapacity,
	int32 contactCapacity,
	int32 jointCapacity,
    P2DStackMem* allocator,
    P2DContactListener* listener)
{
	m_bodyCapacity = bodyCapacity;
	m_contactCapacity = contactCapacity;
	m_jointCapacity	 = jointCapacity;
	m_bodyCount = 0;
	m_contactCount = 0;
	m_jointCount = 0;

	m_allocator = allocator;
	m_listener = listener;

    m_bodies = (P2DBody**)m_allocator->Allocate(bodyCapacity * sizeof(P2DBody*));
    m_contacts = (P2DContact**)m_allocator->Allocate(contactCapacity	 * sizeof(P2DContact*));
    //m_joints = (P2DJoint**)m_allocator->Allocate(jointCapacity * sizeof(P2DJoint*));

    m_velocities = (P2DVelocity*)m_allocator->Allocate(m_bodyCapacity * sizeof(P2DVelocity));
    m_positions = (P2DPosition*)m_allocator->Allocate(m_bodyCapacity * sizeof(P2DPosition));
}

P2DIsland::~P2DIsland()
{
	// Warning: the order should reverse the constructor order.
	m_allocator->Free(m_positions);
	m_allocator->Free(m_velocities);
    //ying m_allocator->Free(m_joints);
	m_allocator->Free(m_contacts);
	m_allocator->Free(m_bodies);
}

void P2DIsland::Solve(P2DProfile* profile, const P2DTimeStep& step, const P2DVec2& gravity, bool allowSleep)
{
    P2DTimer timer;

	float32 h = step.dt;

	// Integrate velocities and apply damping. Initialize the body state.
	for (int32 i = 0; i < m_bodyCount; ++i)
	{
        P2DBody* b = m_bodies[i];

        P2DVec2 c = b->m_sweep.c;
		float32 a = b->m_sweep.a;
        P2DVec2 v = b->m_linearVelocity;
		float32 w = b->m_angularVelocity;

		// Store positions for continuous collision.
		b->m_sweep.c0 = b->m_sweep.c;
		b->m_sweep.a0 = b->m_sweep.a;

        if (b->m_type == P2D_DYNAMIC_BODY)
		{
			// Integrate velocities.
			v += h * (b->m_gravityScale * gravity + b->m_invMass * b->m_force);
			w += h * b->m_invI * b->m_torque;

			// Apply damping.
			// ODE: dv/dt + c * v = 0
			// Solution: v(t) = v0 * exp(-c * t)
			// Time step: v(t + dt) = v0 * exp(-c * (t + dt)) = v0 * exp(-c * t) * exp(-c * dt) = v * exp(-c * dt)
			// v2 = exp(-c * dt) * v1
			// Pade approximation:
			// v2 = v1 * 1 / (1 + c * dt)
			v *= 1.0f / (1.0f + h * b->m_linearDamping);
			w *= 1.0f / (1.0f + h * b->m_angularDamping);
		}

		m_positions[i].c = c;
		m_positions[i].a = a;
		m_velocities[i].v = v;
		m_velocities[i].w = w;
	}

	timer.Reset();

	// Solver data
    P2DSolverData solverData;
	solverData.step = step;
	solverData.positions = m_positions;
	solverData.velocities = m_velocities;

	// Initialize velocity constraints.
    P2DContactSolverDef contactSolverDef;
	contactSolverDef.step = step;
	contactSolverDef.contacts = m_contacts;
	contactSolverDef.count = m_contactCount;
	contactSolverDef.positions = m_positions;
	contactSolverDef.velocities = m_velocities;
	contactSolverDef.allocator = m_allocator;

    P2DContactSolver contactSolver(&contactSolverDef);
	contactSolver.InitializeVelocityConstraints();

	if (step.warmStarting)
	{
		contactSolver.WarmStart();
	}
	
    /* ying
	for (int32 i = 0; i < m_jointCount; ++i)
	{
		m_joints[i]->InitVelocityConstraints(solverData);
	}
    */

	profile->solveInit = timer.GetMilliseconds();

	// Solve velocity constraints
	timer.Reset();
	for (int32 i = 0; i < step.velocityIterations; ++i)
	{
        /* ying
		for (int32 j = 0; j < m_jointCount; ++j)
		{
			m_joints[j]->SolveVelocityConstraints(solverData);
		}
        */

		contactSolver.SolveVelocityConstraints();
	}

	// Store impulses for warm starting
	contactSolver.StoreImpulses();
	profile->solveVelocity = timer.GetMilliseconds();

	// Integrate positions
	for (int32 i = 0; i < m_bodyCount; ++i)
	{
        P2DVec2 c = m_positions[i].c;
		float32 a = m_positions[i].a;
        P2DVec2 v = m_velocities[i].v;
		float32 w = m_velocities[i].w;

		// Check for large velocities
        P2DVec2 translation = h * v;
        if (P2DVecDot(translation, translation) > P2D_MAX_TRANSLATION_SQUARED)
		{
            float32 ratio = P2D_MAX_TRANSLATION / translation.Length();
			v *= ratio;
		}

		float32 rotation = h * w;
        if (rotation * rotation > P2D_MAX_ROTATION_SQUARED)
		{
            float32 ratio = P2D_MAX_ROTATION / P2DAbs(rotation);
			w *= ratio;
		}

		// Integrate
		c += h * v;
		a += h * w;

		m_positions[i].c = c;
		m_positions[i].a = a;
		m_velocities[i].v = v;
		m_velocities[i].w = w;
	}

	// Solve position constraints
	timer.Reset();
	bool positionSolved = false;
	for (int32 i = 0; i < step.positionIterations; ++i)
	{
		bool contactsOkay = contactSolver.SolvePositionConstraints();

		bool jointsOkay = true;
        /* ying
		for (int32 i = 0; i < m_jointCount; ++i)
		{
			bool jointOkay = m_joints[i]->SolvePositionConstraints(solverData);
			jointsOkay = jointsOkay && jointOkay;
		}
        */

		if (contactsOkay && jointsOkay)
		{
			// Exit early if the position errors are small.
			positionSolved = true;
			break;
		}

	}

	// Copy state buffers back to the bodies
	for (int32 i = 0; i < m_bodyCount; ++i)
	{
        P2DBody* body = m_bodies[i];
		body->m_sweep.c = m_positions[i].c;
		body->m_sweep.a = m_positions[i].a;
		body->m_linearVelocity = m_velocities[i].v;
		body->m_angularVelocity = m_velocities[i].w;
		body->SynchronizeTransform();
	}

	profile->solvePosition = timer.GetMilliseconds();

	Report(contactSolver.m_velocityConstraints);

	if (allowSleep)
	{
        float32 minSleepTime = FLT_MAX;

        const float32 linTolSqr = P2D_LINEAR_SLEEP_TOLERANCE * P2D_LINEAR_SLEEP_TOLERANCE;
        const float32 angTolSqr = P2D_ANGULAR_SLEEP_TOLERANCE * P2D_ANGULAR_SLEEP_TOLERANCE;

		for (int32 i = 0; i < m_bodyCount; ++i)
		{
            P2DBody* b = m_bodies[i];
            if (b->GetType() == P2D_STATIC_BODY)
			{
				continue;
			}

            if ((b->m_flags & P2DBody::e_autoSleepFlag) == 0 ||
				b->m_angularVelocity * b->m_angularVelocity > angTolSqr ||
                P2DVecDot(b->m_linearVelocity, b->m_linearVelocity) > linTolSqr)
			{
				b->m_sleepTime = 0.0f;
				minSleepTime = 0.0f;
			}
			else
			{
				b->m_sleepTime += h;
                minSleepTime = P2DMin(minSleepTime, b->m_sleepTime);
			}
		}

        if (minSleepTime >= P2D_TIME_TO_SLEEP && positionSolved)
		{
			for (int32 i = 0; i < m_bodyCount; ++i)
			{
                P2DBody* b = m_bodies[i];
				b->SetAwake(false);
			}
		}
	}
}

void P2DIsland::SolveTOI(const P2DTimeStep& subStep, int32 toiIndexA, int32 toiIndexB)
{
    assert(toiIndexA < m_bodyCount);
    assert(toiIndexB < m_bodyCount);

	// Initialize the body state.
	for (int32 i = 0; i < m_bodyCount; ++i)
	{
        P2DBody* b = m_bodies[i];
		m_positions[i].c = b->m_sweep.c;
		m_positions[i].a = b->m_sweep.a;
		m_velocities[i].v = b->m_linearVelocity;
		m_velocities[i].w = b->m_angularVelocity;
	}

    P2DContactSolverDef contactSolverDef;
	contactSolverDef.contacts = m_contacts;
	contactSolverDef.count = m_contactCount;
	contactSolverDef.allocator = m_allocator;
	contactSolverDef.step = subStep;
	contactSolverDef.positions = m_positions;
	contactSolverDef.velocities = m_velocities;
    P2DContactSolver contactSolver(&contactSolverDef);

	// Solve position constraints.
	for (int32 i = 0; i < subStep.positionIterations; ++i)
	{
		bool contactsOkay = contactSolver.SolveTOIPositionConstraints(toiIndexA, toiIndexB);
		if (contactsOkay)
		{
			break;
		}
	}

#if 0
	// Is the new position really safe?
	for (int32 i = 0; i < m_contactCount; ++i)
	{
        P2DContact* c = m_contacts[i];
        P2DFixture* fA = c->GetFixtureA();
        P2DFixture* fB = c->GetFixtureB();

        P2DBody* bA = fA->GetBody();
        P2DBody* bB = fB->GetBody();

		int32 indexA = c->GetChildIndexA();
		int32 indexB = c->GetChildIndexB();

        P2DDistanceInput input;
		input.proxyA.Set(fA->GetShape(), indexA);
		input.proxyB.Set(fB->GetShape(), indexB);
		input.transformA = bA->GetTransform();
		input.transformB = bB->GetTransform();
		input.useRadii = false;

        P2DDistanceOutput output;
        P2DSimplexCache cache;
		cache.count = 0;
        P2DDistance(&output, &cache, &input);

		if (output.distance == 0 || cache.count == 3)
		{
			cache.count += 0;
		}
	}
#endif

	// Leap of faith to new safe state.
	m_bodies[toiIndexA]->m_sweep.c0 = m_positions[toiIndexA].c;
	m_bodies[toiIndexA]->m_sweep.a0 = m_positions[toiIndexA].a;
	m_bodies[toiIndexB]->m_sweep.c0 = m_positions[toiIndexB].c;
	m_bodies[toiIndexB]->m_sweep.a0 = m_positions[toiIndexB].a;

	// No warm starting is needed for TOI events because warm
	// starting impulses were applied in the discrete solver.
	contactSolver.InitializeVelocityConstraints();

	// Solve velocity constraints.
	for (int32 i = 0; i < subStep.velocityIterations; ++i)
	{
		contactSolver.SolveVelocityConstraints();
	}

	// Don't store the TOI contact forces for warm starting
	// because they can be quite large.

	float32 h = subStep.dt;

	// Integrate positions
	for (int32 i = 0; i < m_bodyCount; ++i)
	{
        P2DVec2 c = m_positions[i].c;
		float32 a = m_positions[i].a;
        P2DVec2 v = m_velocities[i].v;
		float32 w = m_velocities[i].w;

		// Check for large velocities
        P2DVec2 translation = h * v;
        if (P2DVecDot(translation, translation) > P2D_MAX_TRANSLATION_SQUARED)
		{
            float32 ratio = P2D_MAX_TRANSLATION / translation.Length();
			v *= ratio;
		}

		float32 rotation = h * w;
        if (rotation * rotation > P2D_MAX_ROTATION_SQUARED)
		{
            float32 ratio = P2D_MAX_ROTATION / P2DAbs(rotation);
			w *= ratio;
		}

		// Integrate
		c += h * v;
		a += h * w;

		m_positions[i].c = c;
		m_positions[i].a = a;
		m_velocities[i].v = v;
		m_velocities[i].w = w;

		// Sync bodies
        P2DBody* body = m_bodies[i];
		body->m_sweep.c = c;
		body->m_sweep.a = a;
		body->m_linearVelocity = v;
		body->m_angularVelocity = w;
		body->SynchronizeTransform();
	}

	Report(contactSolver.m_velocityConstraints);
}

void P2DIsland::Report(const P2DContactVelocityConstraint* constraints)
{
	if (m_listener == NULL)
	{
		return;
	}

	for (int32 i = 0; i < m_contactCount; ++i)
	{
        P2DContact* c = m_contacts[i];

        const P2DContactVelocityConstraint* vc = constraints + i;
		
        P2DContactImpulse impulse;
		impulse.count = vc->pointCount;
		for (int32 j = 0; j < vc->pointCount; ++j)
		{
			impulse.normalImpulses[j] = vc->points[j].normalImpulse;
			impulse.tangentImpulses[j] = vc->points[j].tangentImpulse;
		}

		m_listener->PostSolve(c, &impulse);
	}
}
