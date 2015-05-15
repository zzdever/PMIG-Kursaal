#include "p2dscenecallback.h"
#include "p2dfixture.h"

// Return true if contact calculations should be performed between these two shapes.
// If you implement your own collision filter you may want to build from this implementation.
bool P2DContactFilter::ShouldCollide(P2DFixture* fixtureA, P2DFixture* fixtureB)
{
	const P2DContactFilterData& filterA = fixtureA->GetFilterData();
	const P2DContactFilterData& filterB = fixtureB->GetFilterData();

	if (filterA.groupIndex == filterB.groupIndex && filterA.groupIndex != 0)
	{
		return filterA.groupIndex > 0;
	}

	bool collide = (filterA.maskBits & filterB.categoryBits) != 0 && (filterA.categoryBits & filterB.maskBits) != 0;
	return collide;
}
