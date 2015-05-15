#ifndef P2D_COLLISION_H
#define P2D_COLLISION_H

#include "../general/p2dmath.h"
#include "../general/p2dparams.h"
#include <limits.h>


/// Structures and functions used for computing contact points, distance
/// queries, and TOI queries.

class P2DBaseObject;
//class P2DCircleShape;
//class P2DEdgeShape;
class P2DPolygonObject;



const uint8 p2d_nullFeature = UCHAR_MAX;

/// The features that intersect to form the contact point
/// This must be 4 bytes or less.
struct P2DContactFeature
{
	enum Type
	{
		e_vertex = 0,
		e_face = 1
	};

	uint8 indexA;		///< Feature index on shapeA
	uint8 indexB;		///< Feature index on shapeB
	uint8 typeA;		///< The feature type on shapeA
	uint8 typeB;		///< The feature type on shapeB
};

/// Contact ids to facilitate warm starting.
union P2DContactID
{
    P2DContactFeature cf;
	uint32 key;					///< Used to quickly compare contact ids.
};

/// A manifold point is a contact point belonging to a contact
/// manifold. It holds details related to the geometry and dynamics
/// of the contact points.
/// The local point usage depends on the manifold type:
/// -e_circles: the local center of circleB
/// -e_faceA: the local center of cirlceB or the clip point of polygonB
/// -e_faceB: the clip point of polygonA
/// This structure is stored across time steps, so we keep it small.
/// Note: the impulses are used for internal caching and may not
/// provide reliable contact forces, especially for high speed collisions.
struct P2DManifoldPoint
{
    P2DVec2 localPoint;		///< usage depends on manifold type
	float32 normalImpulse;	///< the non-penetration impulse
	float32 tangentImpulse;	///< the friction impulse
    P2DContactID id;			///< uniquely identifies a contact point between two shapes
};

/// A manifold for two touching convex shapes.
/// Supports multiple types of contact:
/// - clip point versus plane with radius
/// - point versus point with radius (circles)
/// The local point usage depends on the manifold type:
/// -e_circles: the local center of circleA
/// -e_faceA: the center of faceA
/// -e_faceB: the center of faceB
/// Similarly the local normal usage:
/// -e_circles: not used
/// -e_faceA: the normal on polygonA
/// -e_faceB: the normal on polygonB
/// We store contacts in this way so that position correction can
/// account for movement, which is critical for continuous physics.
/// All contact scenarios must be expressed in one of these types.
/// This structure is stored across time steps, so we keep it small.
struct P2DManifold
{
	enum Type
	{
		e_circles,
		e_faceA,
		e_faceB
	};

    P2DManifoldPoint points[P2D_MAX_MANIFOLD_POINTS];	///< the points of contact
    P2DVec2 localNormal;								///< not use for Type::e_points
    P2DVec2 localPoint;								///< usage depends on manifold type
	Type type;
	int32 pointCount;								///< the number of manifold points
};


/// This is used to compute the current state of a contact manifold.
struct P2DSceneManifold
{
	/// Evaluate the manifold with supplied transforms. This assumes
	/// modest motion from the original state. This does not change the
	/// point count, impulses, etc. The radii must come from the shapes
	/// that generated the manifold.
    void Initialize(const P2DManifold* manifold,
                    const P2DTransform& xfA, float32 radiusA,
                    const P2DTransform& xfB, float32 radiusB);

    P2DVec2 normal;								///< world vector pointing from A to B
    P2DVec2 points[P2D_MAX_MANIFOLD_POINTS];		///< world contact point (point of intersection)
	float32 separations[P2D_MAX_MANIFOLD_POINTS];	///< a negative value indicates overlap, in meters
};


/// This is used for determining the state of contact points.
enum P2DPointState
{
	NullState,		///< point does not exist
	AddState,		///< point was added in the update
	PersistState,	///< point persisted across the update
	RemoveState		///< point was removed in the update
};

/// Compute the point states given two manifolds. The states pertain to the transition from manifold1
/// to manifold2. So state1 is either persist or remove while state2 is either add or persist.
void P2DGetPointStates(P2DPointState state1[P2D_MAX_MANIFOLD_POINTS], P2DPointState state2[P2D_MAX_MANIFOLD_POINTS],
					  const P2DManifold* manifold1, const P2DManifold* manifold2);

/// Used for computing contact manifolds.
struct P2DClipVertex
{
    P2DVec2 v;
	P2DContactID id;
};

/// Ray-cast input data. The ray extends from p1 to p1 + maxFraction * (p2 - p1).
struct P2DRayCastInput
{
    P2DVec2 p1, p2;
	float32 maxFraction;
};

/// Ray-cast output data. The ray hits at p1 + fraction * (p2 - p1), where p1 and p2
/// come from P2DRayCastInput.
struct P2DRayCastOutput
{
    P2DVec2 normal;
	float32 fraction;
};

/// An axis aligned bounding box.
struct P2DAABB
{
	/// Verify that the bounds are sorted.
	bool IsValid() const;

	/// Get the center of the AABB.
    P2DVec2 GetCenter() const
	{
		return 0.5f * (lowerBound + upperBound);
	}

	/// Get the extents of the AABB (half-widths).
    P2DVec2 GetExtents() const
	{
		return 0.5f * (upperBound - lowerBound);
	}

	/// Get the perimeter length
	float32 GetPerimeter() const
	{
		float32 wx = upperBound.x - lowerBound.x;
		float32 wy = upperBound.y - lowerBound.y;
		return 2.0f * (wx + wy);
	}

	/// Combine an AABB into this one.
    void Combine(const P2DAABB& aabb)
	{
        lowerBound = P2DMin(lowerBound, aabb.lowerBound);
        upperBound = P2DMax(upperBound, aabb.upperBound);
	}

	/// Combine two AABBs into this one.
    void Combine(const P2DAABB& aabb1, const P2DAABB& aabb2)
	{
        lowerBound = P2DMin(aabb1.lowerBound, aabb2.lowerBound);
        upperBound = P2DMax(aabb1.upperBound, aabb2.upperBound);
	}

	/// Does this aabb contain the provided AABB.
    bool Contains(const P2DAABB& aabb) const
	{
		bool result = true;
		result = result && lowerBound.x <= aabb.lowerBound.x;
		result = result && lowerBound.y <= aabb.lowerBound.y;
		result = result && aabb.upperBound.x <= upperBound.x;
		result = result && aabb.upperBound.y <= upperBound.y;
		return result;
	}

	bool RayCast(P2DRayCastOutput* output, const P2DRayCastInput& input) const;

    P2DVec2 lowerBound;	///< the lower vertex
    P2DVec2 upperBound;	///< the upper vertex
};

/*
/// Compute the collision manifold between two circles.
void P2DCollideCircles(P2DManifold* manifold,
					  const P2DCircleObject* circleA, const P2DTransform& xfA,
					  const P2DCircleObject* circleB, const P2DTransform& xfB);

/// Compute the collision manifold between a polygon and a circle.
void P2DCollidePolygonAndCircle(P2DManifold* manifold,
							   const P2DPolygonObject* polygonA, const P2DTransform& xfA,
							   const P2DCircleObject* circleB, const P2DTransform& xfB);
*/

/// Compute the collision manifold between two polygons.
void P2DCollidePolygons(P2DManifold* manifold,
					   const P2DPolygonObject* polygonA, const P2DTransform& xfA,
					   const P2DPolygonObject* polygonB, const P2DTransform& xfB);


/*
/// Compute the collision manifold between an edge and a circle.
void P2DCollideEdgeAndCircle(P2DManifold* manifold,
							   const P2DEdgeObject* polygonA, const P2DTransform& xfA,
							   const P2DCircleObject* circleB, const P2DTransform& xfB);

/// Compute the collision manifold between an edge and a circle.
void P2DCollideEdgeAndPolygon(P2DManifold* manifold,
							   const P2DEdgeObject* edgeA, const P2DTransform& xfA,
							   const P2DPolygonObject* circleB, const P2DTransform& xfB);
*/
					   
/// Clipping for contact manifolds.
int32 P2DClipSegmentToLine(P2DClipVertex vOut[2], const P2DClipVertex vIn[2],
							const P2DVec2& normal, float32 offset, int32 vertexIndexA);

/// Determine if two generic shapes overlap.
bool P2DTestOverlap(const P2DBaseObject* shapeA, int32 indexA,
					const P2DBaseObject* shapeB, int32 indexB,
					const P2DTransform& xfA, const P2DTransform& xfB);

// ---------------- Inline Functions ------------------------------------------

inline bool P2DAABB::IsValid() const
{
    P2DVec2 d = upperBound - lowerBound;
	bool valid = d.x >= 0.0f && d.y >= 0.0f;
	valid = valid && lowerBound.IsValid() && upperBound.IsValid();
	return valid;
}

inline bool P2DTestOverlap(const P2DAABB& a, const P2DAABB& b)
{
    P2DVec2 d1, d2;
	d1 = b.lowerBound - a.upperBound;
	d2 = a.lowerBound - b.upperBound;

	if (d1.x > 0.0f || d1.y > 0.0f)
		return false;

	if (d2.x > 0.0f || d2.y > 0.0f)
		return false;

	return true;
}

#endif
