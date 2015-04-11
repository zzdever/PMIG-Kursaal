#ifndef P2D_POLYGON_OBJECT_H
#define P2D_POLYGON_OBJECT_H

#include "p2dbaseobject.h"
//#include "p2dengine/objects/p2dbaseobject.h"
#include  "../collision/p2dcollision.h"

/// A convex polygon. It is assumed that the interior of the polygon is to
/// the left of each edge.
/// Polygons have a maximum number of vertices equal to b2_maxPolygonVertices.
/// In most cases you should not need many vertices for a convex polygon.
class P2DPolygonObject : public P2DBaseObject
{
public:
    P2DPolygonObject();

    /// Implement P2DBaseShape.
    //P2DBaseShape* Clone(b2BlockAllocator* allocator) const;

    /// @see P2DBaseShape::GetChildCount
	int32 GetChildCount() const;

	/// Create a convex hull from the given array of local points.
	/// The count must be in the range [3, b2_maxPolygonVertices].
	/// @warning the points may be re-ordered, even if they form a convex polygon
	/// @warning collinear points are handled but not removed. Collinear points
	/// may lead to poor stacking behavior.
    void SetPoints(const P2DVec2* points, int32 count);

	/// Build vertices to represent an axis-aligned box centered on the local origin.
	/// @param hx the half-width.
	/// @param hy the half-height.
    void SetARect(float32 hx, float32 hy);

	/// Build vertices to represent an oriented box.
	/// @param hx the half-width.
	/// @param hy the half-height.
	/// @param center the center of the box in local coordinates.
	/// @param angle the rotation of the box in local coordinates.
    void SetARect(float32 hx, float32 hy, const P2DVec2& center, float32 angle);

    /// @see P2DBaseShape::TestPoint
    bool TestPoint(const P2DTransform& transform, const P2DVec2& p) const;

    /// Implement P2DBaseShape.
//	bool RayCast(b2RayCastOutput* output, const b2RayCastInput& input,
  //                  const P2DTransform& transform, int32 childIndex) const;

    /// @see P2DBaseShape::ComputeAxisAlignedBoundingBox
    void ComputeAxisAlignedBoundingBox(P2DAABB* aabb, const P2DTransform &transform, int32 childIndex=0) const;

    /// @see P2DBaseShape::ComputeMass
    void ComputeMass(P2DMass *massData, float32 density) const;

    /// Get the vertex count.
	int32 GetVertexCount() const { return m_count; }

	/// Get a vertex by index.
    const P2DVec2& GetVertex(int32 index) const;

	/// Validate convexity. This is a very time consuming operation.
	/// @returns true if valid
    bool ValidateConvexity() const;

private:
    P2DVec2 m_centroid;
    P2DVec2 m_vertices[P2D_MAX_POLYGON_VERTICES];
    P2DVec2 m_normals[P2D_MAX_POLYGON_VERTICES];
	int32 m_count;

    P2DVec2 GetCentroid(const P2DVec2* vs, int32 count);
};



#endif
