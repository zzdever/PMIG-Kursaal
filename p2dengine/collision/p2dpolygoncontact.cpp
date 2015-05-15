#include "p2dpolygoncontact.h"
#include "../general/p2dmem.h"
#include "../collision/p2dtoi.h"
#include "../scene/p2dbody.h"
#include "../scene/p2dfixture.h"
#include "../scene/p2dscenecallback.h"

#include <new>

P2DContact* P2DPolygonContact::Create(P2DFixture* fixtureA, int32, P2DFixture* fixtureB, int32, P2DBlockMem* allocator)
{
    void* mem = allocator->Allocate(sizeof(P2DPolygonContact));
    return new (mem) P2DPolygonContact(fixtureA, fixtureB);
}

void P2DPolygonContact::Destroy(P2DContact* contact, P2DBlockMem* allocator)
{
    ((P2DPolygonContact*)contact)->~P2DPolygonContact();
    allocator->Free(contact, sizeof(P2DPolygonContact));
}

P2DPolygonContact::P2DPolygonContact(P2DFixture* fixtureA, P2DFixture* fixtureB)
    : P2DContact(fixtureA, 0, fixtureB, 0)
{
    assert(m_fixtureA->GetType() == P2DBaseObject::PolygonType);
    assert(m_fixtureB->GetType() == P2DBaseObject::PolygonType);
}

void P2DPolygonContact::Evaluate(P2DManifold* manifold, const P2DTransform& xfA, const P2DTransform& xfB)
{
    P2DCollidePolygons(	manifold,
                        (P2DPolygonObject*)m_fixtureA->GetShape(), xfA,
                        (P2DPolygonObject*)m_fixtureB->GetShape(), xfB);
}
