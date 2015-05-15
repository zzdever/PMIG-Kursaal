#include "p2dscenemanager.h"

#include "p2dbody.h"
#include "p2dfixture.h"
#include "p2disland.h"
//#include "PulleyJoint.h"
#include "p2dcontactmanager.h"
#include "../collision/p2dcontactsolver.h"
#include "../collision/p2dcontact.h"
#include "../collision/p2dcoarsecollision.h"
//#include "CircleShape.h"
//#include "EdgeShape.h"
//#include "ChainShape.h"
#include "../objects/p2dpolygonobject.h"
#include "../collision/p2dtoi.h"
//#include "Draw.h"
#include "../general/p2dtimer.h"
#include "../general/p2dcommonstructs.h"
#include <new>

P2DScene::P2DScene(const P2DVec2& gravity)
{
	m_destructionListener = NULL;
    //ying g_debugDraw = NULL;

	m_bodyList = NULL;
    //ying m_jointList = NULL;

	m_bodyCount = 0;
	m_jointCount = 0;

	m_warmStarting = true;
	m_continuousPhysics = true;
	m_subStepping = false;

	m_stepComplete = true;

	m_allowSleep = true;
	m_gravity = gravity;

	m_flags = e_clearForces;

	m_inv_dt0 = 0.0f;

	m_contactManager.m_allocator = &m_blockAllocator;

    memset(&m_profile, 0, sizeof(P2DProfile));
}

P2DScene::~P2DScene()
{
    // Some shapes allocate using P2DAlloc.
    P2DBody* b = m_bodyList;
	while (b)
	{
        P2DBody* bNext = b->m_next;

        P2DFixture* f = b->m_fixtureList;
		while (f)
		{
            P2DFixture* fNext = f->m_next;
			f->m_proxyCount = 0;
			f->Destroy(&m_blockAllocator);
			f = fNext;
		}

		b = bNext;
	}
}

void P2DScene::SetDestructionListener(P2DDestructionListener* listener)
{
	m_destructionListener = listener;
}

void P2DScene::SetContactFilter(P2DContactFilter* filter)
{
	m_contactManager.m_contactFilter = filter;
}

void P2DScene::SetContactListener(P2DContactListener* listener)
{
	m_contactManager.m_contactListener = listener;
}

/*
void P2DScene::SetDebugDraw(P2DDraw* debugDraw)
{
	g_debugDraw = debugDraw;
}
*/

P2DBody* P2DScene::CreateBody(const P2DBodyDef* def)
{
    assert(IsLocked() == false);
	if (IsLocked())
	{
		return NULL;
	}

    void* mem = m_blockAllocator.Allocate(sizeof(P2DBody));
    P2DBody* b = new (mem) P2DBody(def, this);

	// Add to world doubly linked list.
	b->m_prev = NULL;
	b->m_next = m_bodyList;
	if (m_bodyList)
	{
		m_bodyList->m_prev = b;
	}
	m_bodyList = b;
	++m_bodyCount;

	return b;
}

void P2DScene::DestroyBody(P2DBody* b)
{
    assert(m_bodyCount > 0);
    assert(IsLocked() == false);
	if (IsLocked())
	{
		return;
	}

	// Delete the attached joints.
    /*
    P2DJointEdge* je = b->m_jointList;
	while (je)
	{
        P2DJointEdge* je0 = je;
		je = je->next;

		if (m_destructionListener)
		{
			m_destructionListener->SayGoodbye(je0->joint);
		}

		DestroyJoint(je0->joint);

		b->m_jointList = je;
	}
	b->m_jointList = NULL;
    */

	// Delete the attached contacts.
    P2DContactEdge* ce = b->m_contactList;
	while (ce)
	{
        P2DContactEdge* ce0 = ce;
		ce = ce->next;
		m_contactManager.Destroy(ce0->contact);
	}
	b->m_contactList = NULL;

	// Delete the attached fixtures. This destroys broad-phase proxies.
    P2DFixture* f = b->m_fixtureList;
	while (f)
	{
        P2DFixture* f0 = f;
		f = f->m_next;

		if (m_destructionListener)
		{
			m_destructionListener->SayGoodbye(f0);
		}

		f0->DestroyProxies(&m_contactManager.m_broadPhase);
		f0->Destroy(&m_blockAllocator);
        f0->~P2DFixture();
        m_blockAllocator.Free(f0, sizeof(P2DFixture));

		b->m_fixtureList = f;
		b->m_fixtureCount -= 1;
	}
	b->m_fixtureList = NULL;
	b->m_fixtureCount = 0;

	// Remove world body list.
	if (b->m_prev)
	{
		b->m_prev->m_next = b->m_next;
	}

	if (b->m_next)
	{
		b->m_next->m_prev = b->m_prev;
	}

	if (b == m_bodyList)
	{
		m_bodyList = b->m_next;
	}

	--m_bodyCount;
    b->~P2DBody();
    m_blockAllocator.Free(b, sizeof(P2DBody));
}

/*
P2DJoint* P2DScene::CreateJoint(const P2DJointDef* def)
{
    assert(IsLocked() == false);
	if (IsLocked())
	{
		return NULL;
	}

    P2DJoint* j = P2DJoint::Create(def, &m_blockAllocator);

	// Connect to the world list.
	j->m_prev = NULL;
	j->m_next = m_jointList;
	if (m_jointList)
	{
		m_jointList->m_prev = j;
	}
	m_jointList = j;
	++m_jointCount;

	// Connect to the bodies' doubly linked lists.
	j->m_edgeA.joint = j;
	j->m_edgeA.other = j->m_bodyB;
	j->m_edgeA.prev = NULL;
	j->m_edgeA.next = j->m_bodyA->m_jointList;
	if (j->m_bodyA->m_jointList) j->m_bodyA->m_jointList->prev = &j->m_edgeA;
	j->m_bodyA->m_jointList = &j->m_edgeA;

	j->m_edgeB.joint = j;
	j->m_edgeB.other = j->m_bodyA;
	j->m_edgeB.prev = NULL;
	j->m_edgeB.next = j->m_bodyB->m_jointList;
	if (j->m_bodyB->m_jointList) j->m_bodyB->m_jointList->prev = &j->m_edgeB;
	j->m_bodyB->m_jointList = &j->m_edgeB;

    P2DBody* bodyA = def->bodyA;
    P2DBody* bodyB = def->bodyB;

	// If the joint prevents collisions, then flag any contacts for filtering.
	if (def->collideConnected == false)
	{
        P2DContactEdge* edge = bodyB->GetContactList();
		while (edge)
		{
			if (edge->other == bodyA)
			{
				// Flag the contact for filtering at the next time step (where either
				// body is awake).
				edge->contact->FlagForFiltering();
			}

			edge = edge->next;
		}
	}

	// Note: creating a joint doesn't wake the bodies.

	return j;
}


void P2DScene::DestroyJoint(P2DJoint* j)
{
    assert(IsLocked() == false);
	if (IsLocked())
	{
		return;
	}

	bool collideConnected = j->m_collideConnected;

	// Remove from the doubly linked list.
	if (j->m_prev)
	{
		j->m_prev->m_next = j->m_next;
	}

	if (j->m_next)
	{
		j->m_next->m_prev = j->m_prev;
	}

	if (j == m_jointList)
	{
		m_jointList = j->m_next;
	}

	// Disconnect from island graph.
    P2DBody* bodyA = j->m_bodyA;
    P2DBody* bodyB = j->m_bodyB;

	// Wake up connected bodies.
	bodyA->SetAwake(true);
	bodyB->SetAwake(true);

	// Remove from body 1.
	if (j->m_edgeA.prev)
	{
		j->m_edgeA.prev->next = j->m_edgeA.next;
	}

	if (j->m_edgeA.next)
	{
		j->m_edgeA.next->prev = j->m_edgeA.prev;
	}

	if (&j->m_edgeA == bodyA->m_jointList)
	{
		bodyA->m_jointList = j->m_edgeA.next;
	}

	j->m_edgeA.prev = NULL;
	j->m_edgeA.next = NULL;

	// Remove from body 2
	if (j->m_edgeB.prev)
	{
		j->m_edgeB.prev->next = j->m_edgeB.next;
	}

	if (j->m_edgeB.next)
	{
		j->m_edgeB.next->prev = j->m_edgeB.prev;
	}

	if (&j->m_edgeB == bodyB->m_jointList)
	{
		bodyB->m_jointList = j->m_edgeB.next;
	}

	j->m_edgeB.prev = NULL;
	j->m_edgeB.next = NULL;

    P2DJoint::Destroy(j, &m_blockAllocator);

    assert(m_jointCount > 0);
	--m_jointCount;

	// If the joint prevents collisions, then flag any contacts for filtering.
	if (collideConnected == false)
	{
        P2DContactEdge* edge = bodyB->GetContactList();
		while (edge)
		{
			if (edge->other == bodyA)
			{
				// Flag the contact for filtering at the next time step (where either
				// body is awake).
				edge->contact->FlagForFiltering();
			}

			edge = edge->next;
		}
	}
}
*/

//
void P2DScene::SetAllowSleeping(bool flag)
{
	if (flag == m_allowSleep)
	{
		return;
	}

	m_allowSleep = flag;
	if (m_allowSleep == false)
	{
        for (P2DBody* b = m_bodyList; b; b = b->m_next)
		{
			b->SetAwake(true);
		}
	}
}

// Find islands, integrate and solve constraints, solve position constraints
void P2DScene::Solve(const P2DTimeStep& step)
{
	m_profile.solveInit = 0.0f;
	m_profile.solveVelocity = 0.0f;
	m_profile.solvePosition = 0.0f;

	// Size the island for the worst case.
    P2DIsland island(m_bodyCount,
					m_contactManager.m_contactCount,
					m_jointCount,
					&m_stackAllocator,
					m_contactManager.m_contactListener);

	// Clear all the island flags.
    for (P2DBody* b = m_bodyList; b; b = b->m_next)
	{
        b->m_flags &= ~P2DBody::e_islandFlag;
	}
    for (P2DContact* c = m_contactManager.m_contactList; c; c = c->m_next)
	{
        c->m_flags &= ~P2DContact::e_islandFlag;
	}
    /*
    for (P2DJoint* j = m_jointList; j; j = j->m_next)
	{
		j->m_islandFlag = false;
	}
    */

	// Build and simulate all awake islands.
	int32 stackSize = m_bodyCount;
    P2DBody** stack = (P2DBody**)m_stackAllocator.Allocate(stackSize * sizeof(P2DBody*));
    for (P2DBody* seed = m_bodyList; seed; seed = seed->m_next)
	{
        if (seed->m_flags & P2DBody::e_islandFlag)
		{
			continue;
		}

		if (seed->IsAwake() == false || seed->IsActive() == false)
		{
			continue;
		}

		// The seed can be dynamic or kinematic.
        if (seed->GetType() == P2D_STATIC_BODY)
		{
			continue;
		}

		// Reset island and stack.
		island.Clear();
		int32 stackCount = 0;
		stack[stackCount++] = seed;
        seed->m_flags |= P2DBody::e_islandFlag;

		// Perform a depth first search (DFS) on the constraint graph.
		while (stackCount > 0)
		{
			// Grab the next body off the stack and add it to the island.
            P2DBody* b = stack[--stackCount];
            assert(b->IsActive() == true);
			island.Add(b);

			// Make sure the body is awake.
			b->SetAwake(true);

			// To keep islands as small as possible, we don't
			// propagate islands across static bodies.
            if (b->GetType() == P2D_STATIC_BODY)
			{
				continue;
			}

			// Search all contacts connected to this body.
            for (P2DContactEdge* ce = b->m_contactList; ce; ce = ce->next)
			{
                P2DContact* contact = ce->contact;

				// Has this contact already been added to an island?
                if (contact->m_flags & P2DContact::e_islandFlag)
				{
					continue;
				}

				// Is this contact solid and touching?
				if (contact->IsEnabled() == false ||
					contact->IsTouching() == false)
				{
					continue;
				}

				// Skip sensors.
				bool sensorA = contact->m_fixtureA->m_isSensor;
				bool sensorB = contact->m_fixtureB->m_isSensor;
				if (sensorA || sensorB)
				{
					continue;
				}

				island.Add(contact);
                contact->m_flags |= P2DContact::e_islandFlag;

                P2DBody* other = ce->other;

				// Was the other body already added to this island?
                if (other->m_flags & P2DBody::e_islandFlag)
				{
					continue;
				}

                assert(stackCount < stackSize);
				stack[stackCount++] = other;
                other->m_flags |= P2DBody::e_islandFlag;
			}

            /*
			// Search all joints connect to this body.
            for (P2DJointEdge* je = b->m_jointList; je; je = je->next)
			{
				if (je->joint->m_islandFlag == true)
				{
					continue;
				}

                P2DBody* other = je->other;

				// Don't simulate joints connected to inactive bodies.
				if (other->IsActive() == false)
				{
					continue;
				}

				island.Add(je->joint);
				je->joint->m_islandFlag = true;

                if (other->m_flags & P2DBody::e_islandFlag)
				{
					continue;
				}

                assert(stackCount < stackSize);
				stack[stackCount++] = other;
                other->m_flags |= P2DBody::e_islandFlag;
			}
            */
		}

        P2DProfile profile;
		island.Solve(&profile, step, m_gravity, m_allowSleep);
		m_profile.solveInit += profile.solveInit;
		m_profile.solveVelocity += profile.solveVelocity;
		m_profile.solvePosition += profile.solvePosition;

		// Post solve cleanup.
		for (int32 i = 0; i < island.m_bodyCount; ++i)
		{
			// Allow static bodies to participate in other islands.
            P2DBody* b = island.m_bodies[i];
            if (b->GetType() == P2D_STATIC_BODY)
			{
                b->m_flags &= ~P2DBody::e_islandFlag;
			}
		}
	}

	m_stackAllocator.Free(stack);

	{
        P2DTimer timer;
		// Synchronize fixtures, check for out of range bodies.
        for (P2DBody* b = m_bodyList; b; b = b->GetNext())
		{
			// If a body was not in an island then it did not move.
            if ((b->m_flags & P2DBody::e_islandFlag) == 0)
			{
				continue;
			}

            if (b->GetType() == P2D_STATIC_BODY)
			{
				continue;
			}

			// Update fixtures (for broad-phase).
			b->SynchronizeFixtures();
		}

		// Look for new contacts.
		m_contactManager.FindNewContacts();
        m_profile.coarseCollision = timer.GetMilliseconds();
	}
}

// Find TOI contacts and solve them.
void P2DScene::SolveTOI(const P2DTimeStep& step)
{
    P2DIsland island(2 * P2D_MAX_TOI_CONTACTS, P2D_MAX_TOI_CONTACTS, 0, &m_stackAllocator, m_contactManager.m_contactListener);

	if (m_stepComplete)
	{
        for (P2DBody* b = m_bodyList; b; b = b->m_next)
		{
            b->m_flags &= ~P2DBody::e_islandFlag;
			b->m_sweep.alpha0 = 0.0f;
		}

        for (P2DContact* c = m_contactManager.m_contactList; c; c = c->m_next)
		{
			// Invalidate TOI
            c->m_flags &= ~(P2DContact::e_toiFlag | P2DContact::e_islandFlag);
			c->m_toiCount = 0;
			c->m_toi = 1.0f;
		}
	}

	// Find TOI events and solve them.
	for (;;)
	{
		// Find the first TOI.
        P2DContact* minContact = NULL;
		float32 minAlpha = 1.0f;

        for (P2DContact* c = m_contactManager.m_contactList; c; c = c->m_next)
		{
			// Is this contact disabled?
			if (c->IsEnabled() == false)
			{
				continue;
			}

			// Prevent excessive sub-stepping.
            if (c->m_toiCount > P2D_MAX_SUB_STEPS)
			{
				continue;
			}

			float32 alpha = 1.0f;
            if (c->m_flags & P2DContact::e_toiFlag)
			{
				// This contact has a valid cached TOI.
				alpha = c->m_toi;
			}
			else
			{
                P2DFixture* fA = c->GetFixtureA();
                P2DFixture* fB = c->GetFixtureB();

				// Is there a sensor?
				if (fA->IsSensor() || fB->IsSensor())
				{
					continue;
				}

                P2DBody* bA = fA->GetBody();
                P2DBody* bB = fB->GetBody();

                P2DBodyType typeA = bA->m_type;
                P2DBodyType typeB = bB->m_type;
                assert(typeA == P2D_DYNAMIC_BODY || typeB == P2D_DYNAMIC_BODY);

                bool activeA = bA->IsAwake() && typeA != P2D_STATIC_BODY;
                bool activeB = bB->IsAwake() && typeB != P2D_STATIC_BODY;

				// Is at least one body active (awake and dynamic or kinematic)?
				if (activeA == false && activeB == false)
				{
					continue;
				}

                bool collideA = bA->IsBullet() || typeA != P2D_DYNAMIC_BODY;
                bool collideB = bB->IsBullet() || typeB != P2D_DYNAMIC_BODY;

				// Are these two non-bullet dynamic bodies?
				if (collideA == false && collideB == false)
				{
					continue;
				}

				// Compute the TOI for this contact.
				// Put the sweeps onto the same time interval.
				float32 alpha0 = bA->m_sweep.alpha0;

				if (bA->m_sweep.alpha0 < bB->m_sweep.alpha0)
				{
					alpha0 = bB->m_sweep.alpha0;
					bA->m_sweep.Advance(alpha0);
				}
				else if (bB->m_sweep.alpha0 < bA->m_sweep.alpha0)
				{
					alpha0 = bA->m_sweep.alpha0;
					bB->m_sweep.Advance(alpha0);
				}

                assert(alpha0 < 1.0f);

				int32 indexA = c->GetChildIndexA();
				int32 indexB = c->GetChildIndexB();

				// Compute the time of impact in interval [0, minTOI]
                P2DTOIInput input;
				input.proxyA.Set(fA->GetShape(), indexA);
				input.proxyB.Set(fB->GetShape(), indexB);
				input.sweepA = bA->m_sweep;
				input.sweepB = bB->m_sweep;
				input.tMax = 1.0f;

                P2DTOIOutput output;
                P2DTimeOfImpact(&output, &input);

				// Beta is the fraction of the remaining portion of the .
				float32 beta = output.t;
                if (output.state == P2DTOIOutput::e_touching)
				{
                    alpha = P2DMin(alpha0 + (1.0f - alpha0) * beta, 1.0f);
				}
				else
				{
					alpha = 1.0f;
				}

				c->m_toi = alpha;
                c->m_flags |= P2DContact::e_toiFlag;
			}

			if (alpha < minAlpha)
			{
				// This is the minimum TOI found so far.
				minContact = c;
				minAlpha = alpha;
			}
		}

        if (minContact == NULL || 1.0f - 10.0f * FLT_EPSILON < minAlpha)
		{
			// No more TOI events. Done!
			m_stepComplete = true;
			break;
		}

		// Advance the bodies to the TOI.
        P2DFixture* fA = minContact->GetFixtureA();
        P2DFixture* fB = minContact->GetFixtureB();
        P2DBody* bA = fA->GetBody();
        P2DBody* bB = fB->GetBody();

        P2DSweep backup1 = bA->m_sweep;
        P2DSweep backup2 = bB->m_sweep;

		bA->Advance(minAlpha);
		bB->Advance(minAlpha);

		// The TOI contact likely has some new contact points.
		minContact->Update(m_contactManager.m_contactListener);
        minContact->m_flags &= ~P2DContact::e_toiFlag;
		++minContact->m_toiCount;

		// Is the contact solid?
		if (minContact->IsEnabled() == false || minContact->IsTouching() == false)
		{
			// Restore the sweeps.
			minContact->SetEnabled(false);
			bA->m_sweep = backup1;
			bB->m_sweep = backup2;
			bA->SynchronizeTransform();
			bB->SynchronizeTransform();
			continue;
		}

		bA->SetAwake(true);
		bB->SetAwake(true);

		// Build the island
		island.Clear();
		island.Add(bA);
		island.Add(bB);
		island.Add(minContact);

        bA->m_flags |= P2DBody::e_islandFlag;
        bB->m_flags |= P2DBody::e_islandFlag;
        minContact->m_flags |= P2DContact::e_islandFlag;

		// Get contacts on bodyA and bodyB.
        P2DBody* bodies[2] = {bA, bB};
		for (int32 i = 0; i < 2; ++i)
		{
            P2DBody* body = bodies[i];
            if (body->m_type == P2D_DYNAMIC_BODY)
			{
                for (P2DContactEdge* ce = body->m_contactList; ce; ce = ce->next)
				{
					if (island.m_bodyCount == island.m_bodyCapacity)
					{
						break;
					}

					if (island.m_contactCount == island.m_contactCapacity)
					{
						break;
					}

                    P2DContact* contact = ce->contact;

					// Has this contact already been added to the island?
                    if (contact->m_flags & P2DContact::e_islandFlag)
					{
						continue;
					}

					// Only add static, kinematic, or bullet bodies.
                    P2DBody* other = ce->other;
                    if (other->m_type == P2D_DYNAMIC_BODY &&
						body->IsBullet() == false && other->IsBullet() == false)
					{
						continue;
					}

					// Skip sensors.
					bool sensorA = contact->m_fixtureA->m_isSensor;
					bool sensorB = contact->m_fixtureB->m_isSensor;
					if (sensorA || sensorB)
					{
						continue;
					}

					// Tentatively advance the body to the TOI.
                    P2DSweep backup = other->m_sweep;
                    if ((other->m_flags & P2DBody::e_islandFlag) == 0)
					{
						other->Advance(minAlpha);
					}

					// Update the contact points
					contact->Update(m_contactManager.m_contactListener);

					// Was the contact disabled by the user?
					if (contact->IsEnabled() == false)
					{
						other->m_sweep = backup;
						other->SynchronizeTransform();
						continue;
					}

					// Are there contact points?
					if (contact->IsTouching() == false)
					{
						other->m_sweep = backup;
						other->SynchronizeTransform();
						continue;
					}

					// Add the contact to the island
                    contact->m_flags |= P2DContact::e_islandFlag;
					island.Add(contact);

					// Has the other body already been added to the island?
                    if (other->m_flags & P2DBody::e_islandFlag)
					{
						continue;
					}
					
					// Add the other body to the island.
                    other->m_flags |= P2DBody::e_islandFlag;

                    if (other->m_type != P2D_STATIC_BODY)
					{
						other->SetAwake(true);
					}

					island.Add(other);
				}
			}
		}

        P2DTimeStep subStep;
		subStep.dt = (1.0f - minAlpha) * step.dt;
		subStep.inv_dt = 1.0f / subStep.dt;
		subStep.dtRatio = 1.0f;
		subStep.positionIterations = 20;
		subStep.velocityIterations = step.velocityIterations;
		subStep.warmStarting = false;
		island.SolveTOI(subStep, bA->m_islandIndex, bB->m_islandIndex);

		// Reset island flags and synchronize broad-phase proxies.
		for (int32 i = 0; i < island.m_bodyCount; ++i)
		{
            P2DBody* body = island.m_bodies[i];
            body->m_flags &= ~P2DBody::e_islandFlag;

            if (body->m_type != P2D_DYNAMIC_BODY)
			{
				continue;
			}

			body->SynchronizeFixtures();

			// Invalidate all contact TOIs on this displaced body.
            for (P2DContactEdge* ce = body->m_contactList; ce; ce = ce->next)
			{
                ce->contact->m_flags &= ~(P2DContact::e_toiFlag | P2DContact::e_islandFlag);
			}
		}

		// Commit fixture proxy movements to the broad-phase so that new contacts are created.
		// Also, some contacts can be destroyed.
		m_contactManager.FindNewContacts();

		if (m_subStepping)
		{
			m_stepComplete = false;
			break;
		}
	}
}

void P2DScene::Step(float32 dt, int32 velocityIterations, int32 positionIterations)
{
    P2DTimer stepTimer;

	// If new fixtures were added, we need to find the new contacts.
	if (m_flags & e_newFixture)
	{
		m_contactManager.FindNewContacts();
		m_flags &= ~e_newFixture;
	}

	m_flags |= e_locked;

    P2DTimeStep step;
	step.dt = dt;
	step.velocityIterations	= velocityIterations;
	step.positionIterations = positionIterations;
	if (dt > 0.0f)
	{
		step.inv_dt = 1.0f / dt;
	}
	else
	{
		step.inv_dt = 0.0f;
	}

	step.dtRatio = m_inv_dt0 * dt;

	step.warmStarting = m_warmStarting;
	
	// Update contacts. This is where some contacts are destroyed.
	{
        P2DTimer timer;
		m_contactManager.Collide();
		m_profile.collide = timer.GetMilliseconds();
	}

	// Integrate velocities, solve velocity constraints, and integrate positions.
	if (m_stepComplete && step.dt > 0.0f)
	{
        P2DTimer timer;
		Solve(step);
		m_profile.solve = timer.GetMilliseconds();
	}

	// Handle TOI events.
	if (m_continuousPhysics && step.dt > 0.0f)
	{
        P2DTimer timer;
		SolveTOI(step);
		m_profile.solveTOI = timer.GetMilliseconds();
	}

	if (step.dt > 0.0f)
	{
		m_inv_dt0 = step.inv_dt;
	}

	if (m_flags & e_clearForces)
	{
		ClearForces();
	}

	m_flags &= ~e_locked;

	m_profile.step = stepTimer.GetMilliseconds();
}

void P2DScene::ClearForces()
{
    for (P2DBody* body = m_bodyList; body; body = body->GetNext())
	{
		body->m_force.SetZero();
		body->m_torque = 0.0f;
	}
}

struct P2DSceneQueryWrapper
{
	bool QueryCallback(int32 proxyId)
	{
        P2DFixtureProxy* proxy = (P2DFixtureProxy*)coarseCollision->GetUserData(proxyId);
		return callback->ReportFixture(proxy->fixture);
	}

    const P2DCoarseCollision* coarseCollision;
    P2DQueryCallback* callback;
};

void P2DScene::QueryAABB(P2DQueryCallback* callback, const P2DAABB& aabb) const
{
    P2DSceneQueryWrapper wrapper;
    wrapper.coarseCollision = &m_contactManager.m_broadPhase;
	wrapper.callback = callback;
	m_contactManager.m_broadPhase.Query(&wrapper, aabb);
}

struct P2DSceneRayCastWrapper
{
    float32 RayCastCallback(const P2DRayCastInput& input, int32 proxyId)
	{
        void* userData = coarseCollision->GetUserData(proxyId);
        P2DFixtureProxy* proxy = (P2DFixtureProxy*)userData;
        P2DFixture* fixture = proxy->fixture;
		int32 index = proxy->childIndex;
        P2DRayCastOutput output;
		bool hit = fixture->RayCast(&output, input, index);

		if (hit)
		{
			float32 fraction = output.fraction;
            P2DVec2 point = (1.0f - fraction) * input.p1 + fraction * input.p2;
			return callback->ReportFixture(fixture, point, output.normal, fraction);
		}

		return input.maxFraction;
	}

    const P2DCoarseCollision* coarseCollision;
    P2DRayCastCallback* callback;
};

void P2DScene::RayCast(P2DRayCastCallback* callback, const P2DVec2& point1, const P2DVec2& point2) const
{
    P2DSceneRayCastWrapper wrapper;
    wrapper.coarseCollision = &m_contactManager.m_broadPhase;
	wrapper.callback = callback;
    P2DRayCastInput input;
	input.maxFraction = 1.0f;
	input.p1 = point1;
	input.p2 = point2;
	m_contactManager.m_broadPhase.RayCast(&wrapper, input);
}

/*
void P2DScene::DrawShape(P2DFixture* fixture, const P2DTransform& xf, const P2DColor& color)
{
	switch (fixture->GetType())
	{
    /
    case P2DBaseObject::e_circle:
		{
            P2DCircleShape* circle = (P2DCircleShape*)fixture->GetShape();

            P2DVec2 center = P2DMul(xf, circle->m_p);
			float32 radius = circle->m_radius;
            P2DVec2 axis = P2DMul(xf.rotation, P2DVec2(1.0f, 0.0f));

			g_debugDraw->DrawSolidCircle(center, radius, axis, color);
		}
		break;

    case P2DBaseObject::e_edge:
		{
            P2DEdgeShape* edge = (P2DEdgeShape*)fixture->GetShape();
            P2DVec2 v1 = P2DMul(xf, edge->m_vertex1);
            P2DVec2 v2 = P2DMul(xf, edge->m_vertex2);
			g_debugDraw->DrawSegment(v1, v2, color);
		}
		break;

    case P2DBaseObject::e_chain:
		{
            P2DChainShape* chain = (P2DChainShape*)fixture->GetShape();
			int32 count = chain->m_count;
            const P2DVec2* vertices = chain->m_vertices;

            P2DVec2 v1 = P2DMul(xf, vertices[0]);
			for (int32 i = 1; i < count; ++i)
			{
                P2DVec2 v2 = P2DMul(xf, vertices[i]);
				g_debugDraw->DrawSegment(v1, v2, color);
				g_debugDraw->DrawCircle(v1, 0.05f, color);
				v1 = v2;
			}
		}
		break;
    /

    case P2DBaseObject::PolygonType:
		{
            P2DPolygonObject* poly = (P2DPolygonObject*)fixture->GetShape();
			int32 vertexCount = poly->m_count;
            assert(vertexCount <= P2D_MAX_POLYGON_VERTICES);
            P2DVec2 vertices[P2D_MAX_POLYGON_VERTICES];

			for (int32 i = 0; i < vertexCount; ++i)
			{
                vertices[i] = P2DMul(xf, poly->m_vertices[i]);
			}

			g_debugDraw->DrawSolidPolygon(vertices, vertexCount, color);
		}
		break;
            
    default:
        break;
	}
}
*/


/*
void P2DScene::DrawJoint(P2DJoint* joint)
{
    P2DBody* bodyA = joint->GetBodyA();
    P2DBody* bodyB = joint->GetBodyB();
    const P2DTransform& xf1 = bodyA->GetTransform();
    const P2DTransform& xf2 = bodyB->GetTransform();
    P2DVec2 x1 = xf1.position;
    P2DVec2 x2 = xf2.position;
    P2DVec2 p1 = joint->GetAnchorA();
    P2DVec2 p2 = joint->GetAnchorB();

    P2DColor color(0.5f, 0.8f, 0.8f);

	switch (joint->GetType())
	{
	case e_distanceJoint:
		g_debugDraw->DrawSegment(p1, p2, color);
		break;

	case e_pulleyJoint:
		{
            P2DPulleyJoint* pulley = (P2DPulleyJoint*)joint;
            P2DVec2 s1 = pulley->GetGroundAnchorA();
            P2DVec2 s2 = pulley->GetGroundAnchorB();
			g_debugDraw->DrawSegment(s1, p1, color);
			g_debugDraw->DrawSegment(s2, p2, color);
			g_debugDraw->DrawSegment(s1, s2, color);
		}
		break;

	case e_mouseJoint:
		// don't draw this
		break;

	default:
		g_debugDraw->DrawSegment(x1, p1, color);
		g_debugDraw->DrawSegment(p1, p2, color);
		g_debugDraw->DrawSegment(x2, p2, color);
	}
}
*/

/*
void P2DScene::DrawDebugData()
{
	if (g_debugDraw == NULL)
	{
		return;
	}

	uint32 flags = g_debugDraw->GetFlags();

    if (flags & P2DDraw::e_shapeBit)
	{
        for (P2DBody* b = m_bodyList; b; b = b->GetNext())
		{
            const P2DTransform& xf = b->GetTransform();
            for (P2DFixture* f = b->GetFixtureList(); f; f = f->GetNext())
			{
				if (b->IsActive() == false)
				{
                    DrawShape(f, xf, P2DColor(0.5f, 0.5f, 0.3f));
				}
                else if (b->GetType() == P2D_STATIC_BODY)
				{
                    DrawShape(f, xf, P2DColor(0.5f, 0.9f, 0.5f));
				}
                else if (b->GetType() == P2D_KINEMATIC_BODY)
				{
                    DrawShape(f, xf, P2DColor(0.5f, 0.5f, 0.9f));
				}
				else if (b->IsAwake() == false)
				{
                    DrawShape(f, xf, P2DColor(0.6f, 0.6f, 0.6f));
				}
				else
				{
                    DrawShape(f, xf, P2DColor(0.9f, 0.7f, 0.7f));
				}
			}
		}
	}

    if (flags & P2DDraw::e_jointBit)
	{
        for (P2DJoint* j = m_jointList; j; j = j->GetNext())
		{
			DrawJoint(j);
		}
	}

    if (flags & P2DDraw::e_pairBit)
	{
        P2DColor color(0.3f, 0.9f, 0.9f);
        for (P2DContact* c = m_contactManager.m_contactList; c; c = c->GetNext())
		{
            //P2DFixture* fixtureA = c->GetFixtureA();
            //P2DFixture* fixtureB = c->GetFixtureB();

            //P2DVec2 cA = fixtureA->GetAABB().GetCenter();
            //P2DVec2 cB = fixtureB->GetAABB().GetCenter();

			//g_debugDraw->DrawSegment(cA, cB, color);
		}
	}

    if (flags & P2DDraw::e_aabbBit)
	{
        P2DColor color(0.9f, 0.3f, 0.9f);
        P2DCoarseCollision* bp = &m_contactManager.m_broadPhase;

        for (P2DBody* b = m_bodyList; b; b = b->GetNext())
		{
			if (b->IsActive() == false)
			{
				continue;
			}

            for (P2DFixture* f = b->GetFixtureList(); f; f = f->GetNext())
			{
				for (int32 i = 0; i < f->m_proxyCount; ++i)
				{
                    P2DFixtureProxy* proxy = f->m_proxies + i;
                    P2DAABB aabb = bp->GetFatAABB(proxy->proxyId);
                    P2DVec2 vs[4];
					vs[0].Set(aabb.lowerBound.x, aabb.lowerBound.y);
					vs[1].Set(aabb.upperBound.x, aabb.lowerBound.y);
					vs[2].Set(aabb.upperBound.x, aabb.upperBound.y);
					vs[3].Set(aabb.lowerBound.x, aabb.upperBound.y);

					g_debugDraw->DrawPolygon(vs, 4, color);
				}
			}
		}
	}

    if (flags & P2DDraw::e_centerOfMassBit)
	{
        for (P2DBody* b = m_bodyList; b; b = b->GetNext())
		{
            P2DTransform xf = b->GetTransform();
            xf.position = b->GetWorldCenter();
			g_debugDraw->DrawTransform(xf);
		}
	}
}
*/

int32 P2DScene::GetProxyCount() const
{
	return m_contactManager.m_broadPhase.GetProxyCount();
}

int32 P2DScene::GetTreeHeight() const
{
	return m_contactManager.m_broadPhase.GetTreeHeight();
}

int32 P2DScene::GetTreeBalance() const
{
	return m_contactManager.m_broadPhase.GetTreeBalance();
}

float32 P2DScene::GetTreeQuality() const
{
	return m_contactManager.m_broadPhase.GetTreeQuality();
}

void P2DScene::ShiftOrigin(const P2DVec2& newOrigin)
{
    assert((m_flags & e_locked) == 0);
	if ((m_flags & e_locked) == e_locked)
	{
		return;
	}

    for (P2DBody* b = m_bodyList; b; b = b->m_next)
	{
        b->m_xf.position -= newOrigin;
		b->m_sweep.c0 -= newOrigin;
		b->m_sweep.c -= newOrigin;
	}

    /*
    for (P2DJoint* j = m_jointList; j; j = j->m_next)
	{
		j->ShiftOrigin(newOrigin);
	}
    */

	m_contactManager.m_broadPhase.ShiftOrigin(newOrigin);
}

void P2DScene::Dump()
{
	if ((m_flags & e_locked) == e_locked)
	{
		return;
	}

    /*
    P2DLog("P2DVec2 g(%.15lef, %.15lef);\n", m_gravity.x, m_gravity.y);
    P2DLog("m_world->SetGravity(g);\n");

    P2DLog("P2DBody** bodies = (P2DBody**)P2DAlloc(%d * sizeof(P2DBody*));\n", m_bodyCount);
    P2DLog("P2DJoint** joints = (P2DJoint**)P2DAlloc(%d * sizeof(P2DJoint*));\n", m_jointCount);
	int32 i = 0;
    for (P2DBody* b = m_bodyList; b; b = b->m_next)
	{
		b->m_islandIndex = i;
		b->Dump();
		++i;
	}

	i = 0;
    for (P2DJoint* j = m_jointList; j; j = j->m_next)
	{
		j->m_index = i;
		++i;
	}

	// First pass on joints, skip gear joints.
    for (P2DJoint* j = m_jointList; j; j = j->m_next)
	{
		if (j->m_type == e_gearJoint)
		{
			continue;
		}

        P2DLog("{\n");
		j->Dump();
        P2DLog("}\n");
	}

	// Second pass on joints, only gear joints.
    for (P2DJoint* j = m_jointList; j; j = j->m_next)
	{
		if (j->m_type != e_gearJoint)
		{
			continue;
		}

        P2DLog("{\n");
		j->Dump();
        P2DLog("}\n");
	}

    P2DLog("P2DFree(joints);\n");
    P2DLog("P2DFree(bodies);\n");
    P2DLog("joints = NULL;\n");
    P2DLog("bodies = NULL;\n");
    */
}
