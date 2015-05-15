#include "p2dbody.h"
#include "p2dfixture.h"
#include "p2dscenemanager.h"
#include "../collision/p2dcontact.h"
//#include "p2djoint.h"

P2DBody::P2DBody(const P2DBodyDef* bd, P2DScene* world)
{
    assert(bd->position.IsValid());
    assert(bd->linearVelocity.IsValid());
    assert(P2DIsFloatValid(bd->angle));
    assert(P2DIsFloatValid(bd->angularVelocity));
    assert(P2DIsFloatValid(bd->angularDamping) && bd->angularDamping >= 0.0f);
    assert(P2DIsFloatValid(bd->linearDamping) && bd->linearDamping >= 0.0f);

	m_flags = 0;

	if (bd->bullet)
	{
		m_flags |= e_bulletFlag;
	}
	if (bd->fixedRotation)
	{
		m_flags |= e_fixedRotationFlag;
	}
	if (bd->allowSleep)
	{
		m_flags |= e_autoSleepFlag;
	}
	if (bd->awake)
	{
		m_flags |= e_awakeFlag;
	}
	if (bd->active)
	{
		m_flags |= e_activeFlag;
	}

	m_world = world;

    m_xf.position = bd->position;
    m_xf.rotation.Set(bd->angle);

	m_sweep.localCenter.SetZero();
    m_sweep.c0 = m_xf.position;
    m_sweep.c = m_xf.position;
	m_sweep.a0 = bd->angle;
	m_sweep.a = bd->angle;
	m_sweep.alpha0 = 0.0f;

    //ying m_jointList = NULL;
	m_contactList = NULL;
	m_prev = NULL;
	m_next = NULL;

	m_linearVelocity = bd->linearVelocity;
	m_angularVelocity = bd->angularVelocity;

	m_linearDamping = bd->linearDamping;
	m_angularDamping = bd->angularDamping;
	m_gravityScale = bd->gravityScale;

	m_force.SetZero();
	m_torque = 0.0f;

	m_sleepTime = 0.0f;

	m_type = bd->type;

    if (m_type == P2D_DYNAMIC_BODY)
	{
		m_mass = 1.0f;
		m_invMass = 1.0f;
	}
	else
	{
		m_mass = 0.0f;
		m_invMass = 0.0f;
	}

	m_I = 0.0f;
	m_invI = 0.0f;

	m_userData = bd->userData;

	m_fixtureList = NULL;
	m_fixtureCount = 0;
}

P2DBody::~P2DBody()
{
    // shapes and joints are destroyed in P2DScene::Destroy
}

void P2DBody::SetType(P2DBodyType type)
{
    assert(m_world->IsLocked() == false);
	if (m_world->IsLocked() == true)
	{
		return;
	}

	if (m_type == type)
	{
		return;
	}

	m_type = type;

	ResetMassData();

    if (m_type == P2D_STATIC_BODY)
	{
		m_linearVelocity.SetZero();
		m_angularVelocity = 0.0f;
		m_sweep.a0 = m_sweep.a;
		m_sweep.c0 = m_sweep.c;
		SynchronizeFixtures();
	}

	SetAwake(true);

	m_force.SetZero();
	m_torque = 0.0f;

	// Delete the attached contacts.
    P2DContactEdge* ce = m_contactList;
	while (ce)
	{
        P2DContactEdge* ce0 = ce;
		ce = ce->next;
		m_world->m_contactManager.Destroy(ce0->contact);
	}
	m_contactList = NULL;

	// Touch the proxies so that new contacts will be created (when appropriate)
    P2DCoarseCollision* coarseCollision = &m_world->m_contactManager.m_broadPhase;
    for (P2DFixture* f = m_fixtureList; f; f = f->m_next)
	{
		int32 proxyCount = f->m_proxyCount;
		for (int32 i = 0; i < proxyCount; ++i)
		{
            coarseCollision->TouchProxy(f->m_proxies[i].proxyId);
		}
	}
}

P2DFixture* P2DBody::CreateFixture(const P2DFixtureDef* def)
{
    assert(m_world->IsLocked() == false);
	if (m_world->IsLocked() == true)
	{
		return NULL;
	}

    P2DBlockMem* allocator = &m_world->m_blockAllocator;

    void* memory = allocator->Allocate(sizeof(P2DFixture));
    P2DFixture* fixture = new (memory) P2DFixture;
	fixture->Create(allocator, this, def);

	if (m_flags & e_activeFlag)
	{
        P2DCoarseCollision* coarseCollision = &m_world->m_contactManager.m_broadPhase;
        fixture->CreateProxies(coarseCollision, m_xf);
	}

	fixture->m_next = m_fixtureList;
	m_fixtureList = fixture;
	++m_fixtureCount;

	fixture->m_body = this;

	// Adjust mass properties if needed.
	if (fixture->m_density > 0.0f)
	{
		ResetMassData();
	}

	// Let the world know we have a new fixture. This will cause new contacts
	// to be created at the beginning of the next time step.
    m_world->m_flags |= P2DScene::e_newFixture;

	return fixture;
}

P2DFixture* P2DBody::CreateFixture(const P2DBaseObject* shape, float32 density)
{
    P2DFixtureDef def;
	def.shape = shape;
	def.density = density;

	return CreateFixture(&def);
}

void P2DBody::DestroyFixture(P2DFixture* fixture)
{
    assert(m_world->IsLocked() == false);
	if (m_world->IsLocked() == true)
	{
		return;
	}

    assert(fixture->m_body == this);

	// Remove the fixture from this body's singly linked list.
    assert(m_fixtureCount > 0);
    P2DFixture** node = &m_fixtureList;
	bool found = false;
	while (*node != NULL)
	{
		if (*node == fixture)
		{
			*node = fixture->m_next;
			found = true;
			break;
		}

		node = &(*node)->m_next;
	}

	// You tried to remove a shape that is not attached to this body.
    assert(found);

	// Destroy any contacts associated with the fixture.
    P2DContactEdge* edge = m_contactList;
	while (edge)
	{
        P2DContact* c = edge->contact;
		edge = edge->next;

        P2DFixture* fixtureA = c->GetFixtureA();
        P2DFixture* fixtureB = c->GetFixtureB();

		if (fixture == fixtureA || fixture == fixtureB)
		{
			// This destroys the contact and removes it from
			// this body's contact list.
			m_world->m_contactManager.Destroy(c);
		}
	}

    P2DBlockMem* allocator = &m_world->m_blockAllocator;

	if (m_flags & e_activeFlag)
	{
        P2DCoarseCollision* coarseCollision = &m_world->m_contactManager.m_broadPhase;
        fixture->DestroyProxies(coarseCollision);
	}

	fixture->Destroy(allocator);
	fixture->m_body = NULL;
	fixture->m_next = NULL;
    fixture->~P2DFixture();
    allocator->Free(fixture, sizeof(P2DFixture));

	--m_fixtureCount;

	// Reset the mass data.
	ResetMassData();
}

void P2DBody::ResetMassData()
{
	// Compute mass data from shapes. Each shape has its own density.
	m_mass = 0.0f;
	m_invMass = 0.0f;
	m_I = 0.0f;
	m_invI = 0.0f;
	m_sweep.localCenter.SetZero();

	// Static and kinematic bodies have zero mass.
    if (m_type == P2D_STATIC_BODY || m_type == P2D_KINEMATIC_BODY)
	{
        m_sweep.c0 = m_xf.position;
        m_sweep.c = m_xf.position;
		m_sweep.a0 = m_sweep.a;
		return;
	}

    assert(m_type == P2D_DYNAMIC_BODY);

	// Accumulate mass over all fixtures.
    P2DVec2 localCenter = P2DVec2_0;
    for (P2DFixture* f = m_fixtureList; f; f = f->m_next)
	{
		if (f->m_density == 0.0f)
		{
			continue;
		}

        P2DMass massData;
		f->GetMassData(&massData);
		m_mass += massData.mass;
		localCenter += massData.mass * massData.center;
		m_I += massData.I;
	}

	// Compute center of mass.
	if (m_mass > 0.0f)
	{
		m_invMass = 1.0f / m_mass;
		localCenter *= m_invMass;
	}
	else
	{
		// Force all dynamic bodies to have a positive mass.
		m_mass = 1.0f;
		m_invMass = 1.0f;
	}

	if (m_I > 0.0f && (m_flags & e_fixedRotationFlag) == 0)
	{
		// Center the inertia about the center of mass.
        m_I -= m_mass * P2DVecDot(localCenter, localCenter);
        assert(m_I > 0.0f);
		m_invI = 1.0f / m_I;

	}
	else
	{
		m_I = 0.0f;
		m_invI = 0.0f;
	}

	// Move center of mass.
    P2DVec2 oldCenter = m_sweep.c;
	m_sweep.localCenter = localCenter;
    m_sweep.c0 = m_sweep.c = P2DMul(m_xf, m_sweep.localCenter);

	// Update center of mass velocity.
    m_linearVelocity += P2DVecCross(m_angularVelocity, m_sweep.c - oldCenter);
}

void P2DBody::SetMassData(const P2DMass* massData)
{
    assert(m_world->IsLocked() == false);
	if (m_world->IsLocked() == true)
	{
		return;
	}

    if (m_type != P2D_DYNAMIC_BODY)
	{
		return;
	}

	m_invMass = 0.0f;
	m_I = 0.0f;
	m_invI = 0.0f;

	m_mass = massData->mass;
	if (m_mass <= 0.0f)
	{
		m_mass = 1.0f;
	}

	m_invMass = 1.0f / m_mass;

    if (massData->I > 0.0f && (m_flags & P2DBody::e_fixedRotationFlag) == 0)
	{
        m_I = massData->I - m_mass * P2DVecDot(massData->center, massData->center);
        assert(m_I > 0.0f);
		m_invI = 1.0f / m_I;
	}

	// Move center of mass.
    P2DVec2 oldCenter = m_sweep.c;
	m_sweep.localCenter =  massData->center;
    m_sweep.c0 = m_sweep.c = P2DMul(m_xf, m_sweep.localCenter);

	// Update center of mass velocity.
    m_linearVelocity += P2DVecCross(m_angularVelocity, m_sweep.c - oldCenter);
}

bool P2DBody::ShouldCollide(const P2DBody* other) const
{
	// At least one body should be dynamic.
    if (m_type != P2D_DYNAMIC_BODY && other->m_type != P2D_DYNAMIC_BODY)
	{
		return false;
	}

	// Does a joint prevent collision?
    /*
    for (P2DJointEdge* jn = m_jointList; jn; jn = jn->next)
	{
		if (jn->other == other)
		{
			if (jn->joint->m_collideConnected == false)
			{
				return false;
			}
		}
	}
    */

	return true;
}

void P2DBody::SetTransform(const P2DVec2& position, float32 angle)
{
    assert(m_world->IsLocked() == false);
	if (m_world->IsLocked() == true)
	{
		return;
	}

    m_xf.rotation.Set(angle);
    m_xf.position = position;

    m_sweep.c = P2DMul(m_xf, m_sweep.localCenter);
	m_sweep.a = angle;

	m_sweep.c0 = m_sweep.c;
	m_sweep.a0 = angle;

    P2DCoarseCollision* coarseCollision = &m_world->m_contactManager.m_broadPhase;
    for (P2DFixture* f = m_fixtureList; f; f = f->m_next)
	{
        f->Synchronize(coarseCollision, m_xf, m_xf);
	}
}

void P2DBody::SynchronizeFixtures()
{
    P2DTransform xf1;
    xf1.rotation.Set(m_sweep.a0);
    xf1.position = m_sweep.c0 - P2DMul(xf1.rotation, m_sweep.localCenter);

    P2DCoarseCollision* coarseCollision = &m_world->m_contactManager.m_broadPhase;
    for (P2DFixture* f = m_fixtureList; f; f = f->m_next)
	{
        f->Synchronize(coarseCollision, xf1, m_xf);
	}
}

void P2DBody::SetActive(bool flag)
{
    assert(m_world->IsLocked() == false);

	if (flag == IsActive())
	{
		return;
	}

	if (flag)
	{
		m_flags |= e_activeFlag;

		// Create all proxies.
        P2DCoarseCollision* coarseCollision = &m_world->m_contactManager.m_broadPhase;
        for (P2DFixture* f = m_fixtureList; f; f = f->m_next)
		{
            f->CreateProxies(coarseCollision, m_xf);
		}

		// Contacts are created the next time step.
	}
	else
	{
		m_flags &= ~e_activeFlag;

		// Destroy all proxies.
        P2DCoarseCollision* coarseCollision = &m_world->m_contactManager.m_broadPhase;
        for (P2DFixture* f = m_fixtureList; f; f = f->m_next)
		{
            f->DestroyProxies(coarseCollision);
		}

		// Destroy the attached contacts.
        P2DContactEdge* ce = m_contactList;
		while (ce)
		{
            P2DContactEdge* ce0 = ce;
			ce = ce->next;
			m_world->m_contactManager.Destroy(ce0->contact);
		}
		m_contactList = NULL;
	}
}

void P2DBody::SetFixedRotation(bool flag)
{
	bool status = (m_flags & e_fixedRotationFlag) == e_fixedRotationFlag;
	if (status == flag)
	{
		return;
	}

	if (flag)
	{
		m_flags |= e_fixedRotationFlag;
	}
	else
	{
		m_flags &= ~e_fixedRotationFlag;
	}

	m_angularVelocity = 0.0f;

	ResetMassData();
}

void P2DBody::Dump()
{
	int32 bodyIndex = m_islandIndex;
    NOT_USED(bodyIndex);

    /*
    P2DLog("{\n");
    P2DLog("  P2DBodyDef bd;\n");
    P2DLog("  bd.type = P2DBodyType(%d);\n", m_type);
    P2DLog("  bd.position.Set(%.15lef, %.15lef);\n", m_xf.p.x, m_xf.p.y);
    P2DLog("  bd.angle = %.15lef;\n", m_sweep.a);
    P2DLog("  bd.linearVelocity.Set(%.15lef, %.15lef);\n", m_linearVelocity.x, m_linearVelocity.y);
    P2DLog("  bd.angularVelocity = %.15lef;\n", m_angularVelocity);
    P2DLog("  bd.linearDamping = %.15lef;\n", m_linearDamping);
    P2DLog("  bd.angularDamping = %.15lef;\n", m_angularDamping);
    P2DLog("  bd.allowSleep = bool(%d);\n", m_flags & e_autoSleepFlag);
    P2DLog("  bd.awake = bool(%d);\n", m_flags & e_awakeFlag);
    P2DLog("  bd.fixedRotation = bool(%d);\n", m_flags & e_fixedRotationFlag);
    P2DLog("  bd.bullet = bool(%d);\n", m_flags & e_bulletFlag);
    P2DLog("  bd.active = bool(%d);\n", m_flags & e_activeFlag);
    P2DLog("  bd.gravityScale = %.15lef;\n", m_gravityScale);
    P2DLog("  bodies[%d] = m_world->CreateBody(&bd);\n", m_islandIndex);
    P2DLog("\n");
    for (P2DFixture* f = m_fixtureList; f; f = f->m_next)
	{
        P2DLog("  {\n");
		f->Dump(bodyIndex);
        P2DLog("  }\n");
	}
    P2DLog("}\n");
    */
}
