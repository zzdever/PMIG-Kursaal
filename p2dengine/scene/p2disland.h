#ifndef P2D_ISLAND_H
#define P2D_ISLAND_H

#include "../general/p2dmath.h"
#include "../scene/p2dbody.h"
#include "../general/p2dcommonstructs.h"

class P2DContact;
//class P2DJoint;
class P2DStackMem;
class P2DContactListener;
struct P2DContactVelocityConstraint;
struct P2DProfile;

/// This is an internal class.
class P2DIsland
{
public:
    P2DIsland(int32 bodyCapacity, int32 contactCapacity, int32 jointCapacity,
            P2DStackMem* allocator, P2DContactListener* listener);
    ~P2DIsland();

	void Clear()
	{
		m_bodyCount = 0;
		m_contactCount = 0;
		m_jointCount = 0;
	}

    void Solve(P2DProfile* profile, const P2DTimeStep& step, const P2DVec2& gravity, bool allowSleep);

    void SolveTOI(const P2DTimeStep& subStep, int32 toiIndexA, int32 toiIndexB);

    void Add(P2DBody* body)
	{
        assert(m_bodyCount < m_bodyCapacity);
		body->m_islandIndex = m_bodyCount;
		m_bodies[m_bodyCount] = body;
		++m_bodyCount;
	}

    void Add(P2DContact* contact)
	{
        assert(m_contactCount < m_contactCapacity);
		m_contacts[m_contactCount++] = contact;
	}

    /*
    void Add(P2DJoint* joint)
	{
        assert(m_jointCount < m_jointCapacity);
		m_joints[m_jointCount++] = joint;
	}
    */

    void Report(const P2DContactVelocityConstraint* constraints);

    P2DStackMem* m_allocator;
    P2DContactListener* m_listener;

    P2DBody** m_bodies;
    P2DContact** m_contacts;
    //P2DJoint** m_joints;

    P2DPosition* m_positions;
    P2DVelocity* m_velocities;

	int32 m_bodyCount;
	int32 m_jointCount;
	int32 m_contactCount;

	int32 m_bodyCapacity;
	int32 m_contactCapacity;
	int32 m_jointCapacity;
};

#endif
