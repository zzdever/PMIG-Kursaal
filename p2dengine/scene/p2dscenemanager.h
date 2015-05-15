#ifndef P2D_SCENE_MANAGER_H
#define P2D_SCENE_MANAGER_H

#include "../general/p2dmath.h"
#include "../general/p2dmem.h"
#include "p2dcontactmanager.h"
#include "p2dscenecallback.h"
#include "../general/p2dcommonstructs.h"
#include "p2dfixture.h"

struct P2DAABB;
struct P2DBodyDef;
//struct P2DColor;
//struct P2DJointDef;
class P2DBody;
//class P2DDraw;
class P2DFixture;
//class P2DJoint;

/// The world class manages all physics entities, dynamic simulation,
/// and asynchronous queries. The world also contains efficient memory
/// management facilities.
class P2DScene
{
public:
	/// Construct a world object.
	/// @param gravity the world gravity vector.
	P2DScene(const P2DVec2& gravity);

	/// Destruct the world. All physics entities are destroyed and all heap memory is released.
	~P2DScene();

	/// Register a destruction listener. The listener is owned by you and must
	/// remain in scope.
	void SetDestructionListener(P2DDestructionListener* listener);

	/// Register a contact filter to provide specific control over collision.
	/// Otherwise the default filter is used (p2d_defaultFilter). The listener is
	/// owned by you and must remain in scope. 
	void SetContactFilter(P2DContactFilter* filter);

	/// Register a contact event listener. The listener is owned by you and must
	/// remain in scope.
	void SetContactListener(P2DContactListener* listener);

	/// Register a routine for debug drawing. The debug draw functions are called
	/// inside with P2DScene::DrawDebugData method. The debug draw object is owned
	/// by you and must remain in scope.
	//void SetDebugDraw(P2DDraw* debugDraw);

	/// Create a rigid body given a definition. No reference to the definition
	/// is retained.
	/// @warning This function is locked during callbacks.
	P2DBody* CreateBody(const P2DBodyDef* def);

	/// Destroy a rigid body given a definition. No reference to the definition
	/// is retained. This function is locked during callbacks.
	/// @warning This automatically deletes all associated shapes and joints.
	/// @warning This function is locked during callbacks.
	void DestroyBody(P2DBody* body);

	/// Create a joint to constrain bodies together. No reference to the definition
	/// is retained. This may cause the connected bodies to cease colliding.
	/// @warning This function is locked during callbacks.
	//P2DJoint* CreateJoint(const P2DJointDef* def);

	/// Destroy a joint. This may cause the connected bodies to begin colliding.
	/// @warning This function is locked during callbacks.
	//void DestroyJoint(P2DJoint* joint);

	/// Take a time step. This performs collision detection, integration,
	/// and constraint solution.
	/// @param timeStep the amount of time to simulate, this should not vary.
	/// @param velocityIterations for the velocity constraint solver.
	/// @param positionIterations for the position constraint solver.
	void Step(	float32 timeStep,
				int32 velocityIterations,
				int32 positionIterations);

	/// Manually clear the force buffer on all bodies. By default, forces are cleared automatically
	/// after each call to Step. The default behavior is modified by calling SetAutoClearForces.
	/// The purpose of this function is to support sub-stepping. Sub-stepping is often used to maintain
	/// a fixed sized time step under a variable frame-rate.
	/// When you perform sub-stepping you will disable auto clearing of forces and instead call
	/// ClearForces after all sub-steps are complete in one pass of your game loop.
	/// @see SetAutoClearForces
	void ClearForces();

	/// Call this to draw shapes and other debug draw data. This is intentionally non-const.
	//void DrawDebugData();

	/// Query the world for all fixtures that potentially overlap the
	/// provided AABB.
	/// @param callback a user implemented callback class.
	/// @param aabb the query box.
	void QueryAABB(P2DQueryCallback* callback, const P2DAABB& aabb) const;

	/// Ray-cast the world for all fixtures in the path of the ray. Your callback
	/// controls whether you get the closest point, any point, or n-points.
	/// The ray-cast ignores shapes that contain the starting point.
	/// @param callback a user implemented callback class.
	/// @param point1 the ray starting point
	/// @param point2 the ray ending point
	void RayCast(P2DRayCastCallback* callback, const P2DVec2& point1, const P2DVec2& point2) const;

	/// Get the world body list. With the returned body, use P2DBody::GetNext to get
	/// the next body in the world list. A NULL body indicates the end of the list.
	/// @return the head of the world body list.
	P2DBody* GetBodyList();
	const P2DBody* GetBodyList() const;

	/// Get the world joint list. With the returned joint, use P2DJoint::GetNext to get
	/// the next joint in the world list. A NULL joint indicates the end of the list.
	/// @return the head of the world joint list.
	//P2DJoint* GetJointList();
	//const P2DJoint* GetJointList() const;

	/// Get the world contact list. With the returned contact, use P2DContact::GetNext to get
	/// the next contact in the world list. A NULL contact indicates the end of the list.
	/// @return the head of the world contact list.
	/// @warning contacts are created and destroyed in the middle of a time step.
	/// Use P2DContactListener to avoid missing contacts.
	P2DContact* GetContactList();
	const P2DContact* GetContactList() const;

	/// Enable/disable sleep.
	void SetAllowSleeping(bool flag);
	bool GetAllowSleeping() const { return m_allowSleep; }

	/// Enable/disable warm starting. For testing.
	void SetWarmStarting(bool flag) { m_warmStarting = flag; }
	bool GetWarmStarting() const { return m_warmStarting; }

	/// Enable/disable continuous physics. For testing.
	void SetContinuousPhysics(bool flag) { m_continuousPhysics = flag; }
	bool GetContinuousPhysics() const { return m_continuousPhysics; }

	/// Enable/disable single stepped continuous physics. For testing.
	void SetSubStepping(bool flag) { m_subStepping = flag; }
	bool GetSubStepping() const { return m_subStepping; }

	/// Get the number of broad-phase proxies.
	int32 GetProxyCount() const;

	/// Get the number of bodies.
	int32 GetBodyCount() const;

	/// Get the number of joints.
	//int32 GetJointCount() const;

	/// Get the number of contacts (each may have 0 or more contact points).
	int32 GetContactCount() const;

	/// Get the height of the dynamic tree.
	int32 GetTreeHeight() const;

	/// Get the balance of the dynamic tree.
	int32 GetTreeBalance() const;

	/// Get the quality metric of the dynamic tree. The smaller the better.
	/// The minimum is 1.
	float32 GetTreeQuality() const;

	/// Change the global gravity vector.
	void SetGravity(const P2DVec2& gravity);
	
	/// Get the global gravity vector.
	P2DVec2 GetGravity() const;

	/// Is the world locked (in the middle of a time step).
	bool IsLocked() const;

	/// Set flag to control automatic clearing of forces after each time step.
	void SetAutoClearForces(bool flag);

	/// Get the flag that controls automatic clearing of forces after each time step.
	bool GetAutoClearForces() const;

	/// Shift the world origin. Useful for large worlds.
	/// The body shift formula is: position -= newOrigin
	/// @param newOrigin the new origin with respect to the old origin
	void ShiftOrigin(const P2DVec2& newOrigin);

	/// Get the contact manager for testing.
	const P2DContactManager& GetContactManager() const;

	/// Get the current profile.
	const P2DProfile& GetProfile() const;

	/// Dump the world into the log file.
	/// @warning this should be called outside of a time step.
	void Dump();

private:

	// m_flags
	enum
	{
		e_newFixture	= 0x0001,
		e_locked		= 0x0002,
		e_clearForces	= 0x0004
	};

	friend class P2DBody;
	friend class P2DFixture;
	friend class P2DContactManager;
    //friend class P2DController;

	void Solve(const P2DTimeStep& step);
	void SolveTOI(const P2DTimeStep& step);

	//void DrawJoint(P2DJoint* joint);
	//void DrawShape(P2DFixture* shape, const P2DTransform& xf, const P2DColor& color);

	P2DBlockMem m_blockAllocator;
	P2DStackMem m_stackAllocator;

	int32 m_flags;

	P2DContactManager m_contactManager;

	P2DBody* m_bodyList;
	//P2DJoint* m_jointList;

	int32 m_bodyCount;
	int32 m_jointCount;

	P2DVec2 m_gravity;
	bool m_allowSleep;

	P2DDestructionListener* m_destructionListener;
	//P2DDraw* g_debugDraw;

	// This is used to compute the time step ratio to
	// support a variable time step.
	float32 m_inv_dt0;

	// These are for debugging the solver.
	bool m_warmStarting;
	bool m_continuousPhysics;
	bool m_subStepping;

	bool m_stepComplete;

	P2DProfile m_profile;
};

inline P2DBody* P2DScene::GetBodyList()
{
	return m_bodyList;
}

inline const P2DBody* P2DScene::GetBodyList() const
{
	return m_bodyList;
}

/*
inline P2DJoint* P2DScene::GetJointList()
{
	return m_jointList;
}

inline const P2DJoint* P2DScene::GetJointList() const
{
	return m_jointList;
}
*/

inline P2DContact* P2DScene::GetContactList()
{
	return m_contactManager.m_contactList;
}

inline const P2DContact* P2DScene::GetContactList() const
{
	return m_contactManager.m_contactList;
}

inline int32 P2DScene::GetBodyCount() const
{
	return m_bodyCount;
}

/*
inline int32 P2DScene::GetJointCount() const
{
	return m_jointCount;
}
*/

inline int32 P2DScene::GetContactCount() const
{
	return m_contactManager.m_contactCount;
}

inline void P2DScene::SetGravity(const P2DVec2& gravity)
{
	m_gravity = gravity;
}

inline P2DVec2 P2DScene::GetGravity() const
{
	return m_gravity;
}

inline bool P2DScene::IsLocked() const
{
	return (m_flags & e_locked) == e_locked;
}

inline void P2DScene::SetAutoClearForces(bool flag)
{
	if (flag)
	{
		m_flags |= e_clearForces;
	}
	else
	{
		m_flags &= ~e_clearForces;
	}
}

/// Get the flag that controls automatic clearing of forces after each time step.
inline bool P2DScene::GetAutoClearForces() const
{
	return (m_flags & e_clearForces) == e_clearForces;
}

inline const P2DContactManager& P2DScene::GetContactManager() const
{
	return m_contactManager;
}

inline const P2DProfile& P2DScene::GetProfile() const
{
	return m_profile;
}

#endif
