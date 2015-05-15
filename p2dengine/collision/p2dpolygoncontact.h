#ifndef P2D_POLYGON_CONTACT_H
#define P2D_POLYGON_CONTACT_H

#include "p2dcontact.h"

class P2DBlockMem;

class P2DPolygonContact : public P2DContact
{
public:
    static P2DContact* Create(	P2DFixture* fixtureA, int32 indexA,
                                P2DFixture* fixtureB, int32 indexB, P2DBlockMem* allocator);
    static void Destroy(P2DContact* contact, P2DBlockMem* allocator);

    P2DPolygonContact(P2DFixture* fixtureA, P2DFixture* fixtureB);
    ~P2DPolygonContact() {}

    void Evaluate(P2DManifold* manifold, const P2DTransform& xfA, const P2DTransform& xfB);
};

#endif
