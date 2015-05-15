#ifndef P2D_SCENE_CALLBACK_H
#define P2D_SCENE_CALLBACK_H

#include "../general/p2dparams.h"

struct P2DVec2;
struct P2DTransform;
class P2DFixture;
class P2DBody;
//class P2DJoint;
class P2DContact;
struct P2DContactResult;
struct P2DManifold;

/// Joints and fixtures are destroyed when their associated
/// body is destroyed. Implement this listener so that you
/// may nullify references to these joints and shapes.
class P2DDestructionListener
{
public:
	virtual ~P2DDestructionListener() {}

	/// Called when any joint is about to be destroyed due
	/// to the destruction of one of its attached bodies.
	//virtual void SayGoodbye(P2DJoint* joint) = 0;

	/// Called when any fixture is about to be destroyed due
	/// to the destruction of its parent body.
	virtual void SayGoodbye(P2DFixture* fixture) = 0;
};

/// Implement this class to provide collision filtering. In other words, you can implement
/// this class if you want finer control over contact creation.
class P2DContactFilter
{
public:
	virtual ~P2DContactFilter() {}

	/// Return true if contact calculations should be performed between these two shapes.
	/// @warning for performance reasons this is only called when the AABBs begin to overlap.
	virtual bool ShouldCollide(P2DFixture* fixtureA, P2DFixture* fixtureB);
};

/// Contact impulses for reporting. Impulses are used instead of forces because
/// sub-step forces may approach infinity for rigid body collisions. These
/// match up one-to-one with the contact points in P2DManifold.
struct P2DContactImpulse
{
	float32 normalImpulses[P2D_MAX_MANIFOLD_POINTS];
	float32 tangentImpulses[P2D_MAX_MANIFOLD_POINTS];
	int32 count;
};

/// Implement this class to get contact information. You can use these results for
/// things like sounds and game logic. You can also get contact results by
/// traversing the contact lists after the time step. However, you might miss
/// some contacts because continuous physics leads to sub-stepping.
/// Additionally you may receive multiple callbacks for the same contact in a
/// single time step.
/// You should strive to make your callbacks efficient because there may be
/// many callbacks per time step.
/// @warning You cannot create/destroy entities inside these callbacks.
class P2DContactListener
{
public:
	virtual ~P2DContactListener() {}

	/// Called when two fixtures begin to touch.
	virtual void BeginContact(P2DContact* contact) { NOT_USED(contact); }

	/// Called when two fixtures cease to touch.
	virtual void EndContact(P2DContact* contact) { NOT_USED(contact); }

	/// This is called after a contact is updated. This allows you to inspect a
	/// contact before it goes to the solver. If you are careful, you can modify the
	/// contact manifold (e.g. disable contact).
	/// A copy of the old manifold is provided so that you can detect changes.
	/// Note: this is called only for awake bodies.
	/// Note: this is called even when the number of contact points is zero.
	/// Note: this is not called for sensors.
	/// Note: if you set the number of contact points to zero, you will not
	/// get an EndContact callback. However, you may get a BeginContact callback
	/// the next step.
	virtual void PreSolve(P2DContact* contact, const P2DManifold* oldManifold)
	{
		NOT_USED(contact);
		NOT_USED(oldManifold);
	}

	/// This lets you inspect a contact after the solver is finished. This is useful
	/// for inspecting impulses.
	/// Note: the contact manifold does not include time of impact impulses, which can be
	/// arbitrarily large if the sub-step is small. Hence the impulse is provided explicitly
	/// in a separate data structure.
	/// Note: this is only called for contacts that are touching, solid, and awake.
	virtual void PostSolve(P2DContact* contact, const P2DContactImpulse* impulse)
	{
		NOT_USED(contact);
		NOT_USED(impulse);
	}
};

/// Callback class for AABB queries.
/// See P2DScene::Query
class P2DQueryCallback
{
public:
	virtual ~P2DQueryCallback() {}

	/// Called for each fixture found in the query AABB.
	/// @return false to terminate the query.
	virtual bool ReportFixture(P2DFixture* fixture) = 0;
};

/// Callback class for ray casts.
/// See P2DScene::RayCast
class P2DRayCastCallback
{
public:
	virtual ~P2DRayCastCallback() {}

	/// Called for each fixture found in the query. You control how the ray cast
	/// proceeds by returning a float:
	/// return -1: ignore this fixture and continue
	/// return 0: terminate the ray cast
	/// return fraction: clip the ray to this point
	/// return 1: don't clip the ray and continue
	/// @param fixture the fixture hit by the ray
	/// @param point the point of initial intersection
	/// @param normal the normal vector at the point of intersection
	/// @return -1 to filter, 0 to terminate, fraction to clip the ray for
	/// closest hit, 1 to continue
	virtual float32 ReportFixture(	P2DFixture* fixture, const P2DVec2& point,
									const P2DVec2& normal, float32 fraction) = 0;
};

#endif
