#include "p2dfixture.h"
#include "../collision/p2dcontact.h"
#include "p2dscenemanager.h"
#include "../objects/p2dpolygonobject.h"
#include "../collision/p2dcoarsecollision.h"
#include "../collision/p2dcollision.h"
#include "../general/p2dmem.h"

P2DFixture::P2DFixture()
{
	m_userData = NULL;
	m_body = NULL;
	m_next = NULL;
	m_proxies = NULL;
	m_proxyCount = 0;
	m_shape = NULL;
	m_density = 0.0f;
}

void P2DFixture::Create(P2DBlockMem* allocator, P2DBody* body, const P2DFixtureDef* def)
{
	m_userData = def->userData;
	m_friction = def->friction;
	m_restitution = def->restitution;

	m_body = body;
	m_next = NULL;

	m_filter = def->filter;

	m_isSensor = def->isSensor;

	m_shape = def->shape->Clone(allocator);

	// Reserve proxy space
	int32 childCount = m_shape->GetChildCount();
    m_proxies = (P2DFixtureProxy*)allocator->Allocate(childCount * sizeof(P2DFixtureProxy));
	for (int32 i = 0; i < childCount; ++i)
	{
		m_proxies[i].fixture = NULL;
        m_proxies[i].proxyId = P2DCoarseCollision::e_nullProxy;
	}
	m_proxyCount = 0;

	m_density = def->density;
}

void P2DFixture::Destroy(P2DBlockMem* allocator)
{
	// The proxies must be destroyed before calling this.
	assert(m_proxyCount == 0);

	// Free the proxy array.
	int32 childCount = m_shape->GetChildCount();
	allocator->Free(m_proxies, childCount * sizeof(P2DFixtureProxy));
	m_proxies = NULL;

	// Free the child shape.
	switch (m_shape->m_type)
	{
    /*
    case P2DBaseObject::e_circle:
		{
            P2DCircleObject* s = (P2DCircleObject*)m_shape;
            s->~P2DCircleObject();
            allocator->Free(s, sizeof(P2DCircleObject));
		}
		break;

    case P2DBaseObject::e_edge:
		{
            P2DEdgeObject* s = (P2DEdgeObject*)m_shape;
            s->~P2DEdgeObject();
            allocator->Free(s, sizeof(P2DEdgeObject));
		}
		break;
	*/

	case P2DBaseObject::PolygonType:
		{
			P2DPolygonObject* s = (P2DPolygonObject*)m_shape;
			s->~P2DPolygonObject();
			allocator->Free(s, sizeof(P2DPolygonObject));
		}
		break;
		
    /*
    case P2DBaseObject::e_chain:
		{
            P2DChainObject* s = (P2DChainObject*)m_shape;
            s->~P2DChainObject();
            allocator->Free(s, sizeof(P2DChainObject));
		}
		break;
	*/

	default:
		assert(false);
		break;
	}

	m_shape = NULL;
}

void P2DFixture::CreateProxies(P2DCoarseCollision* coarseCollision, const P2DTransform& xf)
{
	assert(m_proxyCount == 0);

	// Create proxies in the broad-phase.
	m_proxyCount = m_shape->GetChildCount();

	for (int32 i = 0; i < m_proxyCount; ++i)
	{
		P2DFixtureProxy* proxy = m_proxies + i;
		m_shape->ComputeAABB(&proxy->aabb, xf, i);
        proxy->proxyId = coarseCollision->CreateProxy(proxy->aabb, proxy);
		proxy->fixture = this;
		proxy->childIndex = i;
	}
}

void P2DFixture::DestroyProxies(P2DCoarseCollision* coarseCollision)
{
	// Destroy proxies in the broad-phase.
	for (int32 i = 0; i < m_proxyCount; ++i)
	{
		P2DFixtureProxy* proxy = m_proxies + i;
        coarseCollision->DestroyProxy(proxy->proxyId);
        proxy->proxyId = P2DCoarseCollision::e_nullProxy;
	}

	m_proxyCount = 0;
}

void P2DFixture::Synchronize(P2DCoarseCollision* coarseCollision, const P2DTransform& transform1, const P2DTransform& transform2)
{
	if (m_proxyCount == 0)
	{	
		return;
	}

	for (int32 i = 0; i < m_proxyCount; ++i)
	{
		P2DFixtureProxy* proxy = m_proxies + i;

		// Compute an AABB that covers the swept shape (may miss some rotation effect).
        P2DAABB aabb1, aabb2;
		m_shape->ComputeAABB(&aabb1, transform1, proxy->childIndex);
		m_shape->ComputeAABB(&aabb2, transform2, proxy->childIndex);
	
		proxy->aabb.Combine(aabb1, aabb2);

        P2DVec2 displacement = transform2.position - transform1.position;

        coarseCollision->MoveProxy(proxy->proxyId, proxy->aabb, displacement);
	}
}

/*
void P2DFixture::SetFilterData(const P2DFilter& filter)
{
	m_filter = filter;

	Refilter();
}

void P2DFixture::Refilter()
{
	if (m_body == NULL)
	{
		return;
	}

	// Flag associated contacts for filtering.
    P2DContactEdge* edge = m_body->GetContactList();
	while (edge)
	{
        P2DContact* contact = edge->contact;
        P2DFixture* fixtureA = contact->GetFixtureA();
        P2DFixture* fixtureB = contact->GetFixtureB();
		if (fixtureA == this || fixtureB == this)
		{
			contact->FlagForFiltering();
		}

		edge = edge->next;
	}

    P2DScene* world = m_body->GetWorld();

	if (world == NULL)
	{
		return;
	}

	// Touch each proxy so that new pairs may be created
    P2DCoarseCollision* coarseCollision = &world->m_contactManager.m_coarseCollision;
	for (int32 i = 0; i < m_proxyCount; ++i)
	{
        coarseCollision->TouchProxy(m_proxies[i].proxyId);
	}
}
*/

/*
void P2DFixture::SetSensor(bool sensor)
{
	if (sensor != m_isSensor)
	{
		m_body->SetAwake(true);
		m_isSensor = sensor;
	}
}
*/

void P2DFixture::Dump(int32 bodyIndex)
{
    NOT_USED(bodyIndex);
	/*
    P2DLog("    P2DFixtureDef fd;\n");
    P2DLog("    fd.friction = %.15lef;\n", m_friction);
    P2DLog("    fd.restitution = %.15lef;\n", m_restitution);
    P2DLog("    fd.density = %.15lef;\n", m_density);
    P2DLog("    fd.isSensor = bool(%d);\n", m_isSensor);
    P2DLog("    fd.filter.categoryBits = uint16(%d);\n", m_filter.categoryBits);
    P2DLog("    fd.filter.maskBits = uint16(%d);\n", m_filter.maskBits);
    P2DLog("    fd.filter.groupIndex = int16(%d);\n", m_filter.groupIndex);
	*/

	switch (m_shape->m_type)
	{
	/*
    case P2DBaseObject::CircleType:
		{
            P2DCircleObject* s = (P2DCircleObject*)m_shape;
            P2DLog("    P2DCircleObject shape;\n");
            P2DLog("    shape.m_radius = %.15lef;\n", s->m_radius);
            P2DLog("    shape.m_p.Set(%.15lef, %.15lef);\n", s->m_p.x, s->m_p.y);
		}
		break;

    case P2DBaseObject::EdgeType:
		{
            P2DEdgeObject* s = (P2DEdgeObject*)m_shape;
            P2DLog("    P2DEdgeObject shape;\n");
            P2DLog("    shape.m_radius = %.15lef;\n", s->m_radius);
            P2DLog("    shape.m_vertex0.Set(%.15lef, %.15lef);\n", s->m_vertex0.x, s->m_vertex0.y);
            P2DLog("    shape.m_vertex1.Set(%.15lef, %.15lef);\n", s->m_vertex1.x, s->m_vertex1.y);
            P2DLog("    shape.m_vertex2.Set(%.15lef, %.15lef);\n", s->m_vertex2.x, s->m_vertex2.y);
            P2DLog("    shape.m_vertex3.Set(%.15lef, %.15lef);\n", s->m_vertex3.x, s->m_vertex3.y);
            P2DLog("    shape.m_hasVertex0 = bool(%d);\n", s->m_hasVertex0);
            P2DLog("    shape.m_hasVertex3 = bool(%d);\n", s->m_hasVertex3);
		}
		break;
	*/

	case P2DBaseObject::PolygonType:
		{
        /*
            P2DPolygonObject* s = (P2DPolygonObject*)m_shape;

            P2DLog("    P2DPolygonObject shape;\n");
            P2DLog("    P2DVec2 vs[%d];\n", P2D_MAX_POLYGON_VERTICES);
			for (int32 i = 0; i < s->m_count; ++i)
			{
                P2DLog("    vs[%d].Set(%.15lef, %.15lef);\n", i, s->m_vertices[i].x, s->m_vertices[i].y);
			}
            P2DLog("    shape.Set(vs, %d);\n", s->m_count);
        */
		}
		break;

		/*
    case P2DBaseObject::ChainType:
		{
            P2DChainObject* s = (P2DChainObject*)m_shape;
            P2DLog("    P2DChainObject shape;\n");
            P2DLog("    P2DVec2 vs[%d];\n", s->m_count);
			for (int32 i = 0; i < s->m_count; ++i)
			{
                P2DLog("    vs[%d].Set(%.15lef, %.15lef);\n", i, s->m_vertices[i].x, s->m_vertices[i].y);
			}
            P2DLog("    shape.CreateChain(vs, %d);\n", s->m_count);
            P2DLog("    shape.m_prevVertex.Set(%.15lef, %.15lef);\n", s->m_prevVertex.x, s->m_prevVertex.y);
            P2DLog("    shape.m_nextVertex.Set(%.15lef, %.15lef);\n", s->m_nextVertex.x, s->m_nextVertex.y);
            P2DLog("    shape.m_hasPrevVertex = bool(%d);\n", s->m_hasPrevVertex);
            P2DLog("    shape.m_hasNextVertex = bool(%d);\n", s->m_hasNextVertex);
		}
		break;
	*/

	default:
		return;
	}

	/*
    P2DLog("\n");
    P2DLog("    fd.shape = &shape;\n");
    P2DLog("\n");
    P2DLog("    bodies[%d]->CreateFixture(&fd);\n", bodyIndex);
	*/
}
