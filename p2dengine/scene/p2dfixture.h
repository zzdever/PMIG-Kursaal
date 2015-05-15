#ifndef P2D_FIXTURE_H
#define P2D_FIXTURE_H

#include "p2dbody.h"
#include "../collision/p2dcollision.h"
#include "../objects/p2dbaseobject.h"

class P2DBlockMem;
class P2DBody;
class P2DCoarseCollision;
class P2DFixture;

/// This holds contact filtering data.
struct P2DContactFilterData
{
	P2DContactFilterData()
	{
		categoryBits = 0x0001;
		maskBits = 0xFFFF;
		groupIndex = 0;
	}

	/// The collision category bits. Normally you would just set one bit.
	uint16 categoryBits;

	/// The collision mask bits. This states the categories that this
	/// shape would accept for collision.
	uint16 maskBits;

	/// Collision groups allow a certain group of objects to never collide (negative)
	/// or always collide (positive). Zero means no collision group. Non-zero group
	/// filtering always wins against the mask bits.
	int16 groupIndex;
};

/// A fixture definition is used to create a fixture. This class defines an
/// abstract fixture definition. You can reuse fixture definitions safely.
struct P2DFixtureDef
{
	/// The constructor sets the default fixture definition values.
	P2DFixtureDef()
	{
		shape = NULL;
		userData = NULL;
		friction = 0.2f;
		restitution = 0.0f;
		density = 0.0f;
		isSensor = false;
	}

	/// The shape, this must be set. The shape will be cloned, so you
	/// can create the shape on the stack.
	// TODO mod
	const P2DBaseObject* shape;

	/// Use this to store application specific fixture data.
	void* userData;

	/// The friction coefficient, usually in the range [0,1].
	float32 friction;

	/// The restitution (elasticity) usually in the range [0,1].
	float32 restitution;

	/// The density, usually in kg/m^2.
	float32 density;

	/// A sensor shape collects contact information but never generates a collision
	/// response.
	bool isSensor;

	/// Contact filtering data.
	P2DContactFilterData filter;
};

/// This proxy is used internally to connect fixtures to the broad-phase.
struct P2DFixtureProxy
{
	P2DAABB aabb;
	P2DFixture* fixture;
	int32 childIndex;
	int32 proxyId;
};

/// A fixture is used to attach a shape to a body for collision detection. A fixture
/// inherits its transform from its parent. Fixtures hold additional non-geometric data
/// such as friction, collision filters, etc.
/// Fixtures are created via P2DBody::CreateFixture.
/// @warning you cannot reuse fixtures.
class P2DFixture
{
public:
	/// Get the type of the child shape. You can use this to down cast to the concrete shape.
	/// @return the shape type.
	P2DBaseObject::Type GetType() const;

	/// Get the child shape. You can modify the child shape, however you should not change the
	/// number of vertices because this will crash some collision caching mechanisms.
	/// Manipulating the shape may lead to non-physical behavior.
	P2DBaseObject* GetShape();
	const P2DBaseObject* GetShape() const;


	/// Set if this fixture is a sensor.
	void SetSensor(bool sensor);

	/// Is this fixture a sensor (non-solid)?
	/// @return the true if the shape is a sensor.
	bool IsSensor() const;


	/// Set the contact filtering data. This will not update contacts until the next time
	/// step when either parent body is active and awake.
	/// This automatically calls Refilter.
	void SetFilterData(const P2DContactFilterData& filter);

	/// Get the contact filtering data.
	const P2DContactFilterData& GetFilterData() const;

	/// Call this if you want to establish collision that was previously disabled by P2DContactFilter::ShouldCollide.
	void Refilter();

	/// Get the parent body of this fixture. This is NULL if the fixture is not attached.
	/// @return the parent body.
	P2DBody* GetBody();
	const P2DBody* GetBody() const;

	/// Get the next fixture in the parent body's fixture list.
	/// @return the next shape.
	P2DFixture* GetNext();
	const P2DFixture* GetNext() const;

	/// Get the user data that was assigned in the fixture definition. Use this to
	/// store your application specific data.
	void* GetUserData() const;

	/// Set the user data. Use this to store your application specific data.
	void SetUserData(void* data);

	/// Test a point for containment in this fixture.
	/// @param p a point in world coordinates.
	bool TestPoint(const P2DVec2& p) const;

	/// Cast a ray against this shape.
	/// @param output the ray-cast results.
	/// @param input the ray-cast input parameters.
    bool RayCast(P2DRayCastOutput* output, const P2DRayCastInput& input, int32 childIndex) const;

	/// Get the mass data for this fixture. The mass data is based on the density and
	/// the shape. The rotational inertia is about the shape's origin. This operation
	/// may be expensive.
	void GetMassData(P2DMass* massData) const;

	/// Set the density of this fixture. This will _not_ automatically adjust the mass
	/// of the body. You must call P2DBody::ResetMassData to update the body's mass.
	void SetDensity(float32 density);

	/// Get the density of this fixture.
	float32 GetDensity() const;

	/// Get the coefficient of friction.
	float32 GetFriction() const;

	/// Set the coefficient of friction. This will _not_ change the friction of
	/// existing contacts.
	void SetFriction(float32 friction);


	/// Get the coefficient of restitution.
	float32 GetRestitution() const;

	/// Set the coefficient of restitution. This will _not_ change the restitution of
	/// existing contacts.
	void SetRestitution(float32 restitution);


	/// Get the fixture's AABB. This AABB may be enlarge and/or stale.
	/// If you need a more accurate AABB, compute it using the shape and
	/// the body transform.
    const P2DAABB& GetAABB(int32 childIndex) const;

	/// Dump this fixture to the log file.
	void Dump(int32 bodyIndex);

protected:

	friend class P2DBody;
	friend class P2DScene;
	friend class P2DContact;
	friend class P2DContactManager;

	P2DFixture();

	// We need separation create/destroy functions from the constructor/destructor because
	// the destructor cannot access the allocator (no destructor arguments allowed by C++).
	void Create(P2DBlockMem* allocator, P2DBody* body, const P2DFixtureDef* def);
	void Destroy(P2DBlockMem* allocator);

	// These support body activation/deactivation.
	void CreateProxies(P2DCoarseCollision* broadPhase, const P2DTransform& xf);
	void DestroyProxies(P2DCoarseCollision* broadPhase);

	void Synchronize(P2DCoarseCollision* broadPhase, const P2DTransform& xf1, const P2DTransform& xf2);

	float32 m_density;

	P2DFixture* m_next;
	P2DBody* m_body;

	P2DBaseObject* m_shape;

	float32 m_friction;
	float32 m_restitution;

	P2DFixtureProxy* m_proxies;
	int32 m_proxyCount;

	P2DContactFilterData m_filter;

	bool m_isSensor;

	void* m_userData;
};

inline P2DBaseObject::Type P2DFixture::GetType() const
{
	return m_shape->GetType();
}

inline P2DBaseObject* P2DFixture::GetShape()
{
	return m_shape;
}

inline const P2DBaseObject* P2DFixture::GetShape() const
{
	return m_shape;
}

inline bool P2DFixture::IsSensor() const
{
	return m_isSensor;
}

inline const P2DContactFilterData& P2DFixture::GetFilterData() const
{
	return m_filter;
}

inline void* P2DFixture::GetUserData() const
{
	return m_userData;
}

inline void P2DFixture::SetUserData(void* data)
{
	m_userData = data;
}

inline P2DBody* P2DFixture::GetBody()
{
	return m_body;
}

inline const P2DBody* P2DFixture::GetBody() const
{
	return m_body;
}

inline P2DFixture* P2DFixture::GetNext()
{
	return m_next;
}

inline const P2DFixture* P2DFixture::GetNext() const
{
	return m_next;
}

inline void P2DFixture::SetDensity(float32 density)
{
	assert(P2DIsFloatValid(density) && density >= 0.0f);
	m_density = density;
}

inline float32 P2DFixture::GetDensity() const
{
	return m_density;
}

inline float32 P2DFixture::GetFriction() const
{
	return m_friction;
}

inline void P2DFixture::SetFriction(float32 friction)
{
	m_friction = friction;
}

inline float32 P2DFixture::GetRestitution() const
{
	return m_restitution;
}

inline void P2DFixture::SetRestitution(float32 restitution)
{
	m_restitution = restitution;
}

inline bool P2DFixture::TestPoint(const P2DVec2& p) const
{
	return m_shape->TestPoint(m_body->GetTransform(), p);
}


inline bool P2DFixture::RayCast(P2DRayCastOutput* output, const P2DRayCastInput& input, int32 childIndex) const
{
	return m_shape->RayCast(output, input, m_body->GetTransform(), childIndex);
}


inline void P2DFixture::GetMassData(P2DMass* massData) const
{
	m_shape->ComputeMass(massData, m_density);
}

inline const P2DAABB& P2DFixture::GetAABB(int32 childIndex) const
{
	assert(0 <= childIndex && childIndex < m_proxyCount);
	return m_proxies[childIndex].aabb;
}

#endif
