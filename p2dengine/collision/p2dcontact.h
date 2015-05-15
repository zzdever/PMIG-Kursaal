#ifndef P2D_CONTACT_H
#define P2D_CONTACT_H

#include "../general/p2dmath.h"
#include "p2dcollision.h"
#include "../objects/p2dbaseobject.h"
#include "../scene/p2dfixture.h"

class P2DBody;
class P2DContact;
class P2DFixture;
class P2DScene;
class P2DBlockMem;
class P2DStackMem;
class P2DContactListener;

/// Friction mixing law. The idea is to allow either fixture to drive the restitution to zero.
/// For example, anything slides on ice.
inline float32 P2DMixFriction(float32 friction1, float32 friction2)
{
	return sqrtf(friction1 * friction2);
}

/// Restitution mixing law. The idea is allow for anything to bounce off an inelastic surface.
/// For example, a superball bounces on anything.
inline float32 P2DMixRestitution(float32 restitution1, float32 restitution2)
{
	return restitution1 > restitution2 ? restitution1 : restitution2;
}

typedef P2DContact* P2DContactCreateFcn(P2DFixture* fixtureA, int32 indexA,
										P2DFixture* fixtureB, int32 indexB,
										P2DBlockMem* allocator);
typedef void P2DContactDestroyFcn(P2DContact* contact, P2DBlockMem* allocator);

struct P2DContactRegister
{
	P2DContactCreateFcn* createFcn;
	P2DContactDestroyFcn* destroyFcn;
	bool primary;
};

/// A contact edge is used to connect bodies and contacts together
/// in a contact graph where each body is a node and each contact
/// is an edge. A contact edge belongs to a doubly linked list
/// maintained in each attached body. Each contact has two contact
/// nodes, one for each attached body.
struct P2DContactEdge
{
	P2DBody* other;			///< provides quick access to the other body attached.
	P2DContact* contact;		///< the contact
	P2DContactEdge* prev;	///< the previous contact edge in the body's contact list
	P2DContactEdge* next;	///< the next contact edge in the body's contact list
};

/// The class manages contact between two shapes. A contact exists for each overlapping
/// AABB in the broad-phase (except if filtered). Therefore a contact object may exist
/// that has no contact points.
class P2DContact
{
public:

	/// Get the contact manifold. Do not modify the manifold unless you understand the
    /// internals of p2dengine.
	P2DManifold* GetManifold();
	const P2DManifold* GetManifold() const;

	/// Get the world manifold.
	void GetSceneManifold(P2DSceneManifold* sceneManifold) const;

	/// Is this contact touching?
	bool IsTouching() const;

	/// Enable/disable this contact. This can be used inside the pre-solve
	/// contact listener. The contact is only disabled for the current
	/// time step (or sub-step in continuous collisions).
	void SetEnabled(bool flag);

	/// Has this contact been disabled?
	bool IsEnabled() const;

	/// Get the next contact in the world's contact list.
	P2DContact* GetNext();
	const P2DContact* GetNext() const;

	/// Get fixture A in this contact.
	P2DFixture* GetFixtureA();
	const P2DFixture* GetFixtureA() const;

	/// Get the child primitive index for fixture A.
	int32 GetChildIndexA() const;

	/// Get fixture B in this contact.
	P2DFixture* GetFixtureB();
	const P2DFixture* GetFixtureB() const;

	/// Get the child primitive index for fixture B.
	int32 GetChildIndexB() const;

	/// Override the default friction mixture. You can call this in P2DContactListener::PreSolve.
	/// This value persists until set or reset.
	void SetFriction(float32 friction);

	/// Get the friction.
	float32 GetFriction() const;

	/// Reset the friction mixture to the default value.
	void ResetFriction();

	/// Override the default restitution mixture. You can call this in P2DContactListener::PreSolve.
	/// The value persists until you set or reset.
	void SetRestitution(float32 restitution);

	/// Get the restitution.
	float32 GetRestitution() const;

	/// Reset the restitution to the default value.
	void ResetRestitution();

	/// Set the desired tangent speed for a conveyor belt behavior. In meters per second.
	void SetTangentSpeed(float32 speed);

	/// Get the desired tangent speed. In meters per second.
	float32 GetTangentSpeed() const;

	/// Evaluate this contact with your own manifold and transforms.
	virtual void Evaluate(P2DManifold* manifold, const P2DTransform& xfA, const P2DTransform& xfB) = 0;

protected:
	friend class P2DContactManager;
	friend class P2DScene;
	friend class P2DContactSolver;
	friend class P2DBody;
	friend class P2DFixture;

	// Flags stored in m_flags
	enum
	{
		// Used when crawling contact graph when forming islands.
		e_islandFlag		= 0x0001,

        // Set when the shapes are touching.
		e_touchingFlag		= 0x0002,

		// This contact can be disabled (by user)
		e_enabledFlag		= 0x0004,

		// This contact needs filtering because a fixture filter was changed.
		e_filterFlag		= 0x0008,

		// This bullet contact had a TOI event
		e_bulletHitFlag		= 0x0010,

		// This contact has a valid TOI in m_toi
		e_toiFlag			= 0x0020
	};

	/// Flag this contact for filtering. Filtering will occur the next time step.
	void FlagForFiltering();

	static void AddType(P2DContactCreateFcn* createFcn, P2DContactDestroyFcn* destroyFcn,
						P2DBaseObject::Type typeA, P2DBaseObject::Type typeB);
	static void InitializeRegisters();
	static P2DContact* Create(P2DFixture* fixtureA, int32 indexA, P2DFixture* fixtureB, int32 indexB, P2DBlockMem* allocator);
	static void Destroy(P2DContact* contact, P2DBaseObject::Type typeA, P2DBaseObject::Type typeB, P2DBlockMem* allocator);
	static void Destroy(P2DContact* contact, P2DBlockMem* allocator);

	P2DContact() : m_fixtureA(NULL), m_fixtureB(NULL) {}
	P2DContact(P2DFixture* fixtureA, int32 indexA, P2DFixture* fixtureB, int32 indexB);
	virtual ~P2DContact() {}

	void Update(P2DContactListener* listener);

	static P2DContactRegister s_registers[P2DBaseObject::TypeCount][P2DBaseObject::TypeCount];
	static bool s_initialized;

	uint32 m_flags;

	// World pool and list pointers.
	P2DContact* m_prev;
	P2DContact* m_next;

	// Nodes for connecting bodies.
	P2DContactEdge m_nodeA;
	P2DContactEdge m_nodeB;

	P2DFixture* m_fixtureA;
	P2DFixture* m_fixtureB;

	int32 m_indexA;
	int32 m_indexB;

	P2DManifold m_manifold;

	int32 m_toiCount;
	float32 m_toi;

	float32 m_friction;
	float32 m_restitution;

	float32 m_tangentSpeed;
};

inline P2DManifold* P2DContact::GetManifold()
{
	return &m_manifold;
}

inline const P2DManifold* P2DContact::GetManifold() const
{
	return &m_manifold;
}

inline void P2DContact::GetSceneManifold(P2DSceneManifold* sceneManifold) const
{
	const P2DBody* bodyA = m_fixtureA->GetBody();
	const P2DBody* bodyB = m_fixtureB->GetBody();
	const P2DBaseObject* shapeA = m_fixtureA->GetShape();
	const P2DBaseObject* shapeB = m_fixtureB->GetShape();

	sceneManifold->Initialize(&m_manifold, bodyA->GetTransform(), shapeA->m_radius, bodyB->GetTransform(), shapeB->m_radius);
}

inline void P2DContact::SetEnabled(bool flag)
{
	if (flag)
	{
		m_flags |= e_enabledFlag;
	}
	else
	{
		m_flags &= ~e_enabledFlag;
	}
}

inline bool P2DContact::IsEnabled() const
{
	return (m_flags & e_enabledFlag) == e_enabledFlag;
}

inline bool P2DContact::IsTouching() const
{
	return (m_flags & e_touchingFlag) == e_touchingFlag;
}

inline P2DContact* P2DContact::GetNext()
{
	return m_next;
}

inline const P2DContact* P2DContact::GetNext() const
{
	return m_next;
}

inline P2DFixture* P2DContact::GetFixtureA()
{
	return m_fixtureA;
}

inline const P2DFixture* P2DContact::GetFixtureA() const
{
	return m_fixtureA;
}

inline P2DFixture* P2DContact::GetFixtureB()
{
	return m_fixtureB;
}

inline int32 P2DContact::GetChildIndexA() const
{
	return m_indexA;
}

inline const P2DFixture* P2DContact::GetFixtureB() const
{
	return m_fixtureB;
}

inline int32 P2DContact::GetChildIndexB() const
{
	return m_indexB;
}

inline void P2DContact::FlagForFiltering()
{
	m_flags |= e_filterFlag;
}

inline void P2DContact::SetFriction(float32 friction)
{
	m_friction = friction;
}

inline float32 P2DContact::GetFriction() const
{
	return m_friction;
}

inline void P2DContact::ResetFriction()
{
	m_friction = P2DMixFriction(m_fixtureA->m_friction, m_fixtureB->m_friction);
}

inline void P2DContact::SetRestitution(float32 restitution)
{
	m_restitution = restitution;
}

inline float32 P2DContact::GetRestitution() const
{
	return m_restitution;
}

inline void P2DContact::ResetRestitution()
{
	m_restitution = P2DMixRestitution(m_fixtureA->m_restitution, m_fixtureB->m_restitution);
}

inline void P2DContact::SetTangentSpeed(float32 speed)
{
	m_tangentSpeed = speed;
}

inline float32 P2DContact::GetTangentSpeed() const
{
	return m_tangentSpeed;
}

#endif
