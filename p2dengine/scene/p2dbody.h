#ifndef P2D_BODY_H
#define P2D_BODY_H

#include "../general/p2dmath.h"
#include "../objects/p2dbaseobject.h"
#include <memory>

class P2DFixture;
//class P2DJoint;
class P2DContact;
class P2DController;
class P2DScene;
struct P2DFixtureDef;
//struct P2DJointEdge;
struct P2DContactEdge;

/// The body type.
/// static: zero mass, zero velocity, may be manually moved
/// kinematic: zero mass, non-zero velocity set by user, moved by solver
/// dynamic: positive mass, non-zero velocity determined by forces, moved by solver
enum P2DBodyType
{
	P2D_STATIC_BODY = 0,
	P2D_KINEMATIC_BODY = 1,
	P2D_DYNAMIC_BODY = 2

	// TODO
	//bulletBody,
};

/// A body definition holds all the data needed to construct a rigid body.
/// You can safely re-use body definitions. Shapes are added to a body after construction.
struct P2DBodyDef
{
	/// This constructor sets the body definition default values.
	P2DBodyDef()
	{
		userData = NULL;
		position.Set(0.0f, 0.0f);
		angle = 0.0f;
		linearVelocity.Set(0.0f, 0.0f);
		angularVelocity = 0.0f;
		linearDamping = 0.0f;
		angularDamping = 0.0f;
		allowSleep = true;
		awake = true;
		fixedRotation = false;
		bullet = false;
        type = P2D_STATIC_BODY;
		active = true;
		gravityScale = 1.0f;
	}

	/// The body type: static, kinematic, or dynamic.
	/// Note: if a dynamic body would have zero mass, the mass is set to one.
	P2DBodyType type;

	/// The world position of the body. Avoid creating bodies at the origin
	/// since this can lead to many overlapping shapes.
	P2DVec2 position;

	/// The world angle of the body in radians.
	float32 angle;

	/// The linear velocity of the body's origin in world co-ordinates.
	P2DVec2 linearVelocity;

	/// The angular velocity of the body.
	float32 angularVelocity;

	/// Linear damping is use to reduce the linear velocity. The damping parameter
	/// can be larger than 1.0f but the damping effect becomes sensitive to the
	/// time step when the damping parameter is large.
	float32 linearDamping;

	/// Angular damping is use to reduce the angular velocity. The damping parameter
	/// can be larger than 1.0f but the damping effect becomes sensitive to the
	/// time step when the damping parameter is large.
	float32 angularDamping;

	/// Set this flag to false if this body should never fall asleep. Note that
	/// this increases CPU usage.
	bool allowSleep;

	/// Is this body initially awake or sleeping?
	bool awake;

	/// Should this body be prevented from rotating? Useful for characters.
	bool fixedRotation;

	/// Is this a fast moving body that should be prevented from tunneling through
	/// other moving bodies? Note that all bodies are prevented from tunneling through
	/// kinematic and static bodies. This setting is only considered on dynamic bodies.
	/// @warning You should use this flag sparingly since it increases processing time.
	bool bullet;

	/// Does this body start out active?
	bool active;

	/// Use this to store application specific body data.
	void* userData;

	/// Scale the gravity applied to this body.
	float32 gravityScale;
};

/// A rigid body.
class P2DBody
{
public:
	/// Creates a fixture and attach it to this body. Use this function if you need
	/// to set some fixture parameters, like friction. Otherwise you can create the
	/// fixture directly from a shape.
	/// If the density is non-zero, this function automatically updates the mass of the body.
	/// Contacts are not created until the next time step.
	/// @param def the fixture definition.
	/// @warning This function is locked during callbacks.
    P2DFixture* CreateFixture(const P2DFixtureDef* def);

	/// Creates a fixture from a shape and attach it to this body.
    /// This is a convenience function. Use P2DFixtureDef if you need to set parameters
	/// like friction, restitution, user data, or filtering.
	/// If the density is non-zero, this function automatically updates the mass of the body.
	/// @param shape the shape to be cloned.
	/// @param density the shape density (set to zero for static bodies).
	/// @warning This function is locked during callbacks.
    P2DFixture* CreateFixture(const P2DBaseObject* shape, float32 density);

	/// Destroy a fixture. This removes the fixture from the broad-phase and
	/// destroys all contacts associated with this fixture. This will
	/// automatically adjust the mass of the body if the body is dynamic and the
	/// fixture has positive density.
	/// All fixtures attached to a body are implicitly destroyed when the body is destroyed.
	/// @param fixture the fixture to be removed.
	/// @warning This function is locked during callbacks.
    void DestroyFixture(P2DFixture *fixture);

	/// Set the position of the body's origin and rotation.
	/// Manipulating a body's transform may cause non-physical behavior.
    /// Note: contacts are updated on the next call to P2DScene::Step.
	/// @param position the world position of the body's local origin.
	/// @param angle the world rotation in radians.
	void SetTransform(const P2DVec2& position, float32 angle);

	/// Get the body transform for the body's origin.
	/// @return the world transform of the body's origin.
	const P2DTransform& GetTransform() const;

	/// Get the world body origin position.
	/// @return the world position of the body's origin.
	const P2DVec2& GetPosition() const;

	/// Get the angle in radians.
	/// @return the current world rotation angle in radians.
	float32 GetAngle() const;

	/// Get the world position of the center of mass.
	const P2DVec2& GetWorldCenter() const;

	/// Get the local position of the center of mass.
	const P2DVec2& GetLocalCenter() const;

	/// Set the linear velocity of the center of mass.
	/// @param v the new linear velocity of the center of mass.
	void SetLinearVelocity(const P2DVec2& v);

	/// Get the linear velocity of the center of mass.
	/// @return the linear velocity of the center of mass.
	const P2DVec2& GetLinearVelocity() const;

	/// Set the angular velocity.
	/// @param omega the new angular velocity in radians/second.
	void SetAngularVelocity(float32 omega);

	/// Get the angular velocity.
	/// @return the angular velocity in radians/second.
	float32 GetAngularVelocity() const;

	/// Apply a force at a world point. If the force is not
	/// applied at the center of mass, it will generate a torque and
	/// affect the angular velocity. This wakes up the body.
	/// @param force the world force vector, usually in Newtons (N).
	/// @param point the world position of the point of application.
	/// @param wake also wake up the body
	void ApplyForce(const P2DVec2& force, const P2DVec2& point, bool wake);

	/// Apply a force to the center of mass. This wakes up the body.
	/// @param force the world force vector, usually in Newtons (N).
	/// @param wake also wake up the body
	void ApplyForceToCenter(const P2DVec2& force, bool wake);

	/// Apply a torque. This affects the angular velocity
	/// without affecting the linear velocity of the center of mass.
	/// This wakes up the body.
	/// @param torque about the z-axis (out of the screen), usually in N-m.
	/// @param wake also wake up the body
	void ApplyTorque(float32 torque, bool wake);

	/// Apply an impulse at a point. This immediately modifies the velocity.
	/// It also modifies the angular velocity if the point of application
	/// is not at the center of mass. This wakes up the body.
	/// @param impulse the world impulse vector, usually in N-seconds or kg-m/s.
	/// @param point the world position of the point of application.
	/// @param wake also wake up the body
	void ApplyLinearImpulse(const P2DVec2& impulse, const P2DVec2& point, bool wake);

	/// Apply an angular impulse.
	/// @param impulse the angular impulse in units of kg*m*m/s
	/// @param wake also wake up the body
	void ApplyAngularImpulse(float32 impulse, bool wake);

	/// Get the total mass of the body.
	/// @return the mass, usually in kilograms (kg).
	float32 GetMass() const;

	/// Get the rotational inertia of the body about the local origin.
	/// @return the rotational inertia, usually in kg-m^2.
	float32 GetInertia() const;

	/// Get the mass data of the body.
	/// @return a struct containing the mass, inertia and center of the body.
	void GetMassData(P2DMass* data) const;

	/// Set the mass properties to override the mass properties of the fixtures.
	/// Note that this changes the center of mass position.
	/// Note that creating or destroying fixtures can also alter the mass.
	/// This function has no effect if the body isn't dynamic.
	/// @param massData the mass properties.
	void SetMassData(const P2DMass* data);

	/// This resets the mass properties to the sum of the mass properties of the fixtures.
	/// This normally does not need to be called unless you called SetMassData to override
	/// the mass and you later want to reset the mass.
	void ResetMassData();

	/// Get the world coordinates of a point given the local coordinates.
	/// @param localPoint a point on the body measured relative the the body's origin.
	/// @return the same point expressed in world coordinates.
	P2DVec2 GetWorldPoint(const P2DVec2& localPoint) const;

	/// Get the world coordinates of a vector given the local coordinates.
	/// @param localVector a vector fixed in the body.
	/// @return the same vector expressed in world coordinates.
	P2DVec2 GetWorldVector(const P2DVec2& localVector) const;

	/// Gets a local point relative to the body's origin given a world point.
	/// @param a point in world coordinates.
	/// @return the corresponding local point relative to the body's origin.
	P2DVec2 GetLocalPoint(const P2DVec2& worldPoint) const;

	/// Gets a local vector given a world vector.
	/// @param a vector in world coordinates.
	/// @return the corresponding local vector.
	P2DVec2 GetLocalVector(const P2DVec2& worldVector) const;

	/// Get the world linear velocity of a world point attached to this body.
	/// @param a point in world coordinates.
	/// @return the world velocity of a point.
	P2DVec2 GetLinearVelocityFromWorldPoint(const P2DVec2& worldPoint) const;

	/// Get the world velocity of a local point.
	/// @param a point in local coordinates.
	/// @return the world velocity of a point.
	P2DVec2 GetLinearVelocityFromLocalPoint(const P2DVec2& localPoint) const;

	/// Get the linear damping of the body.
	float32 GetLinearDamping() const;

	/// Set the linear damping of the body.
	void SetLinearDamping(float32 linearDamping);

	/// Get the angular damping of the body.
	float32 GetAngularDamping() const;

	/// Set the angular damping of the body.
	void SetAngularDamping(float32 angularDamping);

	/// Get the gravity scale of the body.
	float32 GetGravityScale() const;

	/// Set the gravity scale of the body.
	void SetGravityScale(float32 scale);

	/// Set the type of this body. This may alter the mass and velocity.
	void SetType(P2DBodyType type);

	/// Get the type of this body.
	P2DBodyType GetType() const;

	/// Should this body be treated like a bullet for continuous collision detection?
	void SetBullet(bool flag);

	/// Is this body treated like a bullet for continuous collision detection?
	bool IsBullet() const;

	/// You can disable sleeping on this body. If you disable sleeping, the
	/// body will be woken.
	void SetSleepingAllowed(bool flag);

	/// Is this body allowed to sleep
	bool IsSleepingAllowed() const;

	/// Set the sleep state of the body. A sleeping body has very
	/// low CPU cost.
	/// @param flag set to true to wake the body, false to put it to sleep.
	void SetAwake(bool flag);

	/// Get the sleeping state of this body.
	/// @return true if the body is awake.
	bool IsAwake() const;

	/// Set the active state of the body. An inactive body is not
	/// simulated and cannot be collided with or woken up.
	/// If you pass a flag of true, all fixtures will be added to the
	/// broad-phase.
	/// If you pass a flag of false, all fixtures will be removed from
	/// the broad-phase and all contacts will be destroyed.
	/// Fixtures and joints are otherwise unaffected. You may continue
	/// to create/destroy fixtures and joints on inactive bodies.
	/// Fixtures on an inactive body are implicitly inactive and will
	/// not participate in collisions, ray-casts, or queries.
	/// Joints connected to an inactive body are implicitly inactive.
    /// An inactive body is still owned by a P2DScene object and remains
	/// in the body list.
	void SetActive(bool flag);

	/// Get the active state of the body.
	bool IsActive() const;

	/// Set this body to have fixed rotation. This causes the mass
	/// to be reset.
	void SetFixedRotation(bool flag);

	/// Does this body have fixed rotation?
	bool IsFixedRotation() const;

	/// Get the list of all fixtures attached to this body.
    P2DFixture* GetFixtureList();
    const P2DFixture* GetFixtureList() const;

	/*
	/// Get the list of all joints attached to this body.
    P2DJointEdge* GetJointList();
    const P2DJointEdge* GetJointList() const;
	*/

	/// Get the list of all contacts attached to this body.
	/// @warning this list changes during the time step and you may
	/// miss some collisions if you don't use ContactListener.
    P2DContactEdge* GetContactList();
    const P2DContactEdge* GetContactList() const;

	/// Get the next body in the world's body list.
	P2DBody* GetNext();
	const P2DBody* GetNext() const;

	/// Get the user data pointer that was provided in the body definition.
	void* GetUserData() const;

	/// Set the user data. Use this to store your application specific data.
	void SetUserData(void* data);

	/// Get the parent world of this body.
    P2DScene* GetWorld();
    const P2DScene* GetWorld() const;

	/// Dump this body to a log file
	void Dump();

private:

    friend class P2DScene;
    friend class P2DIsland;
    friend class P2DContactManager;
    friend class P2DContactSolver;
    friend class P2DContact;
	
	/*
	friend class DistanceJoint;
	friend class FrictionJoint;
	friend class GearJoint;
	friend class MotorJoint;
	friend class MouseJoint;
	friend class PrismaticJoint;
	friend class PulleyJoint;
	friend class RevoluteJoint;
	friend class RopeJoint;
	friend class WeldJoint;
	friend class WheelJoint;
	*/

	// m_flags
	enum
	{
		e_islandFlag		= 0x0001,
		e_awakeFlag			= 0x0002,
		e_autoSleepFlag		= 0x0004,
		e_bulletFlag		= 0x0008,
		e_fixedRotationFlag	= 0x0010,
		e_activeFlag		= 0x0020,
		e_toiFlag			= 0x0040
	};

    P2DBody(const P2DBodyDef* bd, P2DScene *world);
	~P2DBody();

	void SynchronizeFixtures();
	void SynchronizeTransform();

	// This is used to prevent connected bodies from colliding.
	// It may lie, depending on the collideConnected flag.
	bool ShouldCollide(const P2DBody* other) const;

	void Advance(float32 t);

	P2DBodyType m_type;

	uint16 m_flags;

	int32 m_islandIndex;

	P2DTransform m_xf;		// the body origin transform
	P2DSweep m_sweep;		// the swept motion for CCD

	P2DVec2 m_linearVelocity;
	float32 m_angularVelocity;

	P2DVec2 m_force;
	float32 m_torque;

    P2DScene* m_world;
	P2DBody* m_prev;
	P2DBody* m_next;

    P2DFixture* m_fixtureList;
	int32 m_fixtureCount;
	
    //ying P2DJointEdge* m_jointList;
    P2DContactEdge* m_contactList;

	float32 m_mass, m_invMass;

	// Rotational inertia about the center of mass.
	float32 m_I, m_invI;

	float32 m_linearDamping;
	float32 m_angularDamping;
	float32 m_gravityScale;

	float32 m_sleepTime;

	void* m_userData;
};

inline P2DBodyType P2DBody::GetType() const
{
	return m_type;
}

inline const P2DTransform& P2DBody::GetTransform() const
{
	return m_xf;
}

inline const P2DVec2& P2DBody::GetPosition() const
{
    return m_xf.position;
}

inline float32 P2DBody::GetAngle() const
{
	return m_sweep.a;
}

inline const P2DVec2& P2DBody::GetWorldCenter() const
{
	return m_sweep.c;
}

inline const P2DVec2& P2DBody::GetLocalCenter() const
{
	return m_sweep.localCenter;
}

inline void P2DBody::SetLinearVelocity(const P2DVec2& v)
{
	if (m_type == P2D_STATIC_BODY)
	{
		return;
	}

	if (P2DVecDot(v,v) > 0.0f)
	{
		SetAwake(true);
	}

	m_linearVelocity = v;
}

inline const P2DVec2& P2DBody::GetLinearVelocity() const
{
	return m_linearVelocity;
}

inline void P2DBody::SetAngularVelocity(float32 w)
{
	if (m_type == P2D_STATIC_BODY)
	{
		return;
	}

	if (w * w > 0.0f)
	{
		SetAwake(true);
	}

	m_angularVelocity = w;
}

inline float32 P2DBody::GetAngularVelocity() const
{
	return m_angularVelocity;
}

inline float32 P2DBody::GetMass() const
{
	return m_mass;
}

inline float32 P2DBody::GetInertia() const
{
	return m_I + m_mass * P2DVecDot(m_sweep.localCenter, m_sweep.localCenter);
}

inline void P2DBody::GetMassData(P2DMass* data) const
{
	data->mass = m_mass;
	data->I = m_I + m_mass * P2DVecDot(m_sweep.localCenter, m_sweep.localCenter);
	data->center = m_sweep.localCenter;
}

inline P2DVec2 P2DBody::GetWorldPoint(const P2DVec2& localPoint) const
{
	return P2DMul(m_xf, localPoint);
}

inline P2DVec2 P2DBody::GetWorldVector(const P2DVec2& localVector) const
{
    return P2DMul(m_xf.rotation, localVector);
}

inline P2DVec2 P2DBody::GetLocalPoint(const P2DVec2& worldPoint) const
{
	return P2DMulT(m_xf, worldPoint);
}

inline P2DVec2 P2DBody::GetLocalVector(const P2DVec2& worldVector) const
{
    return P2DMulT(m_xf.rotation, worldVector);
}

inline P2DVec2 P2DBody::GetLinearVelocityFromWorldPoint(const P2DVec2& worldPoint) const
{
	return m_linearVelocity + P2DVecCross(m_angularVelocity, worldPoint - m_sweep.c);
}

inline P2DVec2 P2DBody::GetLinearVelocityFromLocalPoint(const P2DVec2& localPoint) const
{
	return GetLinearVelocityFromWorldPoint(GetWorldPoint(localPoint));
}

inline float32 P2DBody::GetLinearDamping() const
{
	return m_linearDamping;
}

inline void P2DBody::SetLinearDamping(float32 linearDamping)
{
	m_linearDamping = linearDamping;
}

inline float32 P2DBody::GetAngularDamping() const
{
	return m_angularDamping;
}

inline void P2DBody::SetAngularDamping(float32 angularDamping)
{
	m_angularDamping = angularDamping;
}

inline float32 P2DBody::GetGravityScale() const
{
	return m_gravityScale;
}

inline void P2DBody::SetGravityScale(float32 scale)
{
	m_gravityScale = scale;
}

inline void P2DBody::SetBullet(bool flag)
{
	if (flag) {
		m_flags |= e_bulletFlag;
	} else {
		m_flags &= ~e_bulletFlag;
	}
}

inline bool P2DBody::IsBullet() const
{
	return (m_flags & e_bulletFlag) == e_bulletFlag;
}

inline void P2DBody::SetAwake(bool flag)
{
	if (flag)
	{
		if ((m_flags & e_awakeFlag) == 0)
		{
			m_flags |= e_awakeFlag;
			m_sleepTime = 0.0f;
		}
	}
	else
	{
		m_flags &= ~e_awakeFlag;
		m_sleepTime = 0.0f;
		m_linearVelocity.SetZero();
		m_angularVelocity = 0.0f;
		m_force.SetZero();
		m_torque = 0.0f;
	}
}

inline bool P2DBody::IsAwake() const
{
	return (m_flags & e_awakeFlag) == e_awakeFlag;
}

inline bool P2DBody::IsActive() const
{
	return (m_flags & e_activeFlag) == e_activeFlag;
}

inline bool P2DBody::IsFixedRotation() const
{
	return (m_flags & e_fixedRotationFlag) == e_fixedRotationFlag;
}

inline void P2DBody::SetSleepingAllowed(bool flag)
{
	if (flag) {
		m_flags |= e_autoSleepFlag;
	} else {
		m_flags &= ~e_autoSleepFlag;
		SetAwake(true);
	}
}

inline bool P2DBody::IsSleepingAllowed() const
{
	return (m_flags & e_autoSleepFlag) == e_autoSleepFlag;
}

inline P2DFixture* P2DBody::GetFixtureList()
{
	return m_fixtureList;
}

inline const P2DFixture* P2DBody::GetFixtureList() const
{
	return m_fixtureList;
}

/*
inline P2DJointEdge* P2DBody::GetJointList()
{
	return m_jointList;
}

inline const P2DJointEdge* P2DBody::GetJointList() const
{
	return m_jointList;
}
*/

inline P2DContactEdge* P2DBody::GetContactList()
{
	return m_contactList;
}

inline const P2DContactEdge* P2DBody::GetContactList() const
{
	return m_contactList;
}

inline P2DBody* P2DBody::GetNext()
{
	return m_next;
}

inline const P2DBody* P2DBody::GetNext() const
{
	return m_next;
}

inline void P2DBody::SetUserData(void* data)
{
	m_userData = data;
}

inline void* P2DBody::GetUserData() const
{
	return m_userData;
}

inline void P2DBody::ApplyForce(const P2DVec2& force, const P2DVec2& point, bool wake)
{
    if (m_type != P2D_DYNAMIC_BODY)
	{
		return;
	}

	if (wake && (m_flags & e_awakeFlag) == 0)
	{
		SetAwake(true);
	}

	// Don't accumulate a force if the body is sleeping.
	if (m_flags & e_awakeFlag)
	{
		m_force += force;
        m_torque += P2DVecCross(point - m_sweep.c, force);
	}
}

inline void P2DBody::ApplyForceToCenter(const P2DVec2& force, bool wake)
{
	if (m_type != P2D_DYNAMIC_BODY)
	{
		return;
	}

	if (wake && (m_flags & e_awakeFlag) == 0)
	{
		SetAwake(true);
	}

	// Don't accumulate a force if the body is sleeping
	if (m_flags & e_awakeFlag)
	{
		m_force += force;
	}
}

inline void P2DBody::ApplyTorque(float32 torque, bool wake)
{
	if (m_type != P2D_DYNAMIC_BODY)
	{
		return;
	}

	if (wake && (m_flags & e_awakeFlag) == 0)
	{
		SetAwake(true);
	}

	// Don't accumulate a force if the body is sleeping
	if (m_flags & e_awakeFlag)
	{
		m_torque += torque;
	}
}

inline void P2DBody::ApplyLinearImpulse(const P2DVec2& impulse, const P2DVec2& point, bool wake)
{
	if (m_type != P2D_DYNAMIC_BODY)
	{
		return;
	}

	if (wake && (m_flags & e_awakeFlag) == 0)
	{
		SetAwake(true);
	}

	// Don't accumulate velocity if the body is sleeping
	if (m_flags & e_awakeFlag)
	{
		m_linearVelocity += m_invMass * impulse;
        m_angularVelocity += m_invI * P2DVecCross(point - m_sweep.c, impulse);
	}
}

inline void P2DBody::ApplyAngularImpulse(float32 impulse, bool wake)
{
	if (m_type != P2D_DYNAMIC_BODY)
	{
		return;
	}

	if (wake && (m_flags & e_awakeFlag) == 0)
	{
		SetAwake(true);
	}

	// Don't accumulate velocity if the body is sleeping
	if (m_flags & e_awakeFlag)
	{
		m_angularVelocity += m_invI * impulse;
	}
}

inline void P2DBody::SynchronizeTransform()
{
    m_xf.rotation.Set(m_sweep.a);
    m_xf.position = m_sweep.c - P2DMul(m_xf.rotation, m_sweep.localCenter);
}

inline void P2DBody::Advance(float32 alpha)
{
	// Advance to the new safe time. This doesn't sync the broad-phase.
	m_sweep.Advance(alpha);
	m_sweep.c = m_sweep.c0;
	m_sweep.a = m_sweep.a0;
    m_xf.rotation.Set(m_sweep.a);
    m_xf.position = m_sweep.c - P2DMul(m_xf.rotation, m_sweep.localCenter);
}

inline P2DScene *P2DBody::GetWorld()
{
	return m_world;
}

inline const P2DScene* P2DBody::GetWorld() const
{
	return m_world;
}

#endif
