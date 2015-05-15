
#include "p2dcontact.h"
#include "p2dpolygoncontact.h"
//#include <CircleContact.h>
//#include <PolygonAndCircleContact.h>
//#include <EdgeAndCircleContact.h>
//#include <EdgeAndPolygonContact.h>
//#include <ChainAndCircleContact.h>
//#include <ChainAndPolygonContact.h>
#include "p2dcontactsolver.h"

#include "p2dcollision.h"
#include "p2dtoi.h"
#include "../objects/p2dbaseobject.h"
#include "../general/p2dmem.h"
#include "../scene/p2dbody.h"
#include "../scene/p2dfixture.h"
#include "../scene/p2dscenemanager.h"

P2DContactRegister P2DContact::s_registers[P2DBaseObject::TypeCount][P2DBaseObject::TypeCount];
bool P2DContact::s_initialized = false;

void P2DContact::InitializeRegisters()
{
	AddType(P2DPolygonContact::Create, P2DPolygonContact::Destroy, P2DBaseObject::PolygonType, P2DBaseObject::PolygonType);
	
	//AddType(P2DCircleContact::Create, P2DCircleContact::Destroy, P2DBaseObject::e_circle, P2DBaseObject::e_circle);
	//AddType(P2DPolygonAndCircleContact::Create, P2DPolygonAndCircleContact::Destroy, P2DBaseObject::e_polygon, P2DBaseObject::e_circle);
	//AddType(P2DEdgeAndCircleContact::Create, P2DEdgeAndCircleContact::Destroy, P2DBaseObject::e_edge, P2DBaseObject::e_circle);
	//AddType(P2DEdgeAndPolygonContact::Create, P2DEdgeAndPolygonContact::Destroy, P2DBaseObject::e_edge, P2DBaseObject::e_polygon);
	//AddType(P2DChainAndCircleContact::Create, P2DChainAndCircleContact::Destroy, P2DBaseObject::e_chain, P2DBaseObject::e_circle);
	//AddType(P2DChainAndPolygonContact::Create, P2DChainAndPolygonContact::Destroy, P2DBaseObject::e_chain, P2DBaseObject::e_polygon);
}

void P2DContact::AddType(P2DContactCreateFcn* createFcn, P2DContactDestroyFcn* destoryFcn,
						P2DBaseObject::Type type1, P2DBaseObject::Type type2)
{
	assert(0 <= type1 && type1 < P2DBaseObject::TypeCount);
	assert(0 <= type2 && type2 < P2DBaseObject::TypeCount);
	
	s_registers[type1][type2].createFcn = createFcn;
	s_registers[type1][type2].destroyFcn = destoryFcn;
	s_registers[type1][type2].primary = true;

	if (type1 != type2)
	{
		s_registers[type2][type1].createFcn = createFcn;
		s_registers[type2][type1].destroyFcn = destoryFcn;
		s_registers[type2][type1].primary = false;
	}
}

P2DContact* P2DContact::Create(P2DFixture* fixtureA, int32 indexA, P2DFixture* fixtureB, int32 indexB, P2DBlockMem* allocator)
{
	if (s_initialized == false)
	{
		InitializeRegisters();
		s_initialized = true;
	}

	P2DBaseObject::Type type1 = fixtureA->GetType();
	P2DBaseObject::Type type2 = fixtureB->GetType();

	assert(0 <= type1 && type1 < P2DBaseObject::TypeCount);
	assert(0 <= type2 && type2 < P2DBaseObject::TypeCount);
	
	P2DContactCreateFcn* createFcn = s_registers[type1][type2].createFcn;
	if (createFcn)
	{
		if (s_registers[type1][type2].primary)
		{
			return createFcn(fixtureA, indexA, fixtureB, indexB, allocator);
		}
		else
		{
			return createFcn(fixtureB, indexB, fixtureA, indexA, allocator);
		}
	}
	else
	{
		return NULL;
	}
}

void P2DContact::Destroy(P2DContact* contact, P2DBlockMem* allocator)
{
	assert(s_initialized == true);

	P2DFixture* fixtureA = contact->m_fixtureA;
	P2DFixture* fixtureB = contact->m_fixtureB;

	if (contact->m_manifold.pointCount > 0 &&
		fixtureA->IsSensor() == false &&
		fixtureB->IsSensor() == false)
	{
		fixtureA->GetBody()->SetAwake(true);
		fixtureB->GetBody()->SetAwake(true);
	}

	P2DBaseObject::Type typeA = fixtureA->GetType();
	P2DBaseObject::Type typeB = fixtureB->GetType();

	assert(0 <= typeA && typeB < P2DBaseObject::TypeCount);
	assert(0 <= typeA && typeB < P2DBaseObject::TypeCount);

	P2DContactDestroyFcn* destroyFcn = s_registers[typeA][typeB].destroyFcn;
	destroyFcn(contact, allocator);
}

P2DContact::P2DContact(P2DFixture* fA, int32 indexA, P2DFixture* fB, int32 indexB)
{
	m_flags = e_enabledFlag;

	m_fixtureA = fA;
	m_fixtureB = fB;

	m_indexA = indexA;
	m_indexB = indexB;

	m_manifold.pointCount = 0;

	m_prev = NULL;
	m_next = NULL;

	m_nodeA.contact = NULL;
	m_nodeA.prev = NULL;
	m_nodeA.next = NULL;
	m_nodeA.other = NULL;

	m_nodeB.contact = NULL;
	m_nodeB.prev = NULL;
	m_nodeB.next = NULL;
	m_nodeB.other = NULL;

	m_toiCount = 0;

	m_friction = P2DMixFriction(m_fixtureA->m_friction, m_fixtureB->m_friction);
	m_restitution = P2DMixRestitution(m_fixtureA->m_restitution, m_fixtureB->m_restitution);

	m_tangentSpeed = 0.0f;
}

// Update the contact manifold and touching status.
// Note: do not assume the fixture AABBs are overlapping or are valid.
void P2DContact::Update(P2DContactListener* listener)
{
	P2DManifold oldManifold = m_manifold;

	// Re-enable this contact.
	m_flags |= e_enabledFlag;

	bool touching = false;
	bool wasTouching = (m_flags & e_touchingFlag) == e_touchingFlag;

	bool sensorA = m_fixtureA->IsSensor();
	bool sensorB = m_fixtureB->IsSensor();
	bool sensor = sensorA || sensorB;

	P2DBody* bodyA = m_fixtureA->GetBody();
	P2DBody* bodyB = m_fixtureB->GetBody();
	const P2DTransform& xfA = bodyA->GetTransform();
	const P2DTransform& xfB = bodyB->GetTransform();

	// Is this contact a sensor?
	if (sensor)
	{
		const P2DBaseObject* shapeA = m_fixtureA->GetShape();
		const P2DBaseObject* shapeB = m_fixtureB->GetShape();
		touching = P2DTestOverlap(shapeA, m_indexA, shapeB, m_indexB, xfA, xfB);

		// Sensors don't generate manifolds.
		m_manifold.pointCount = 0;
	}
	else
	{
		Evaluate(&m_manifold, xfA, xfB);
		touching = m_manifold.pointCount > 0;

		// Match old contact ids to new contact ids and copy the
		// stored impulses to warm start the solver.
		for (int32 i = 0; i < m_manifold.pointCount; ++i)
		{
			P2DManifoldPoint* mp2 = m_manifold.points + i;
			mp2->normalImpulse = 0.0f;
			mp2->tangentImpulse = 0.0f;
			P2DContactID id2 = mp2->id;

			for (int32 j = 0; j < oldManifold.pointCount; ++j)
			{
				P2DManifoldPoint* mp1 = oldManifold.points + j;

				if (mp1->id.key == id2.key)
				{
					mp2->normalImpulse = mp1->normalImpulse;
					mp2->tangentImpulse = mp1->tangentImpulse;
					break;
				}
			}
		}

		if (touching != wasTouching)
		{
			bodyA->SetAwake(true);
			bodyB->SetAwake(true);
		}
	}

	if (touching)
	{
		m_flags |= e_touchingFlag;
	}
	else
	{
		m_flags &= ~e_touchingFlag;
	}

	if (wasTouching == false && touching == true && listener)
	{
		listener->BeginContact(this);
	}

	if (wasTouching == true && touching == false && listener)
	{
		listener->EndContact(this);
	}

	if (sensor == false && touching && listener)
	{
		listener->PreSolve(this, &oldManifold);
	}
}
