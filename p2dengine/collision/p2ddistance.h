#ifndef P2D_DISTANCE_H
#define P2D_DISTANCE_H

#include "../general/p2dmath.h"

class P2DBaseObject;

/// A distance proxy is used by the GJK algorithm.
/// It encapsulates any shape.
struct P2DDistanceProxy
{
	P2DDistanceProxy() : m_vertices(NULL), m_count(0), m_radius(0.0f) {}

	/// Initialize the proxy using the given shape. The shape
	/// must remain in scope while the proxy is in use.
	void Set(const P2DBaseObject* shape, int32 index);

	/// Get the supporting vertex index in the given direction.
	int32 GetSupport(const P2DVec2& d) const;

	/// Get the supporting vertex in the given direction.
	const P2DVec2& GetSupportVertex(const P2DVec2& d) const;

	/// Get the vertex count.
	int32 GetVertexCount() const;

    /// Get a vertex by index. Used by P2DDistance.
	const P2DVec2& GetVertex(int32 index) const;

	P2DVec2 m_buffer[2];
	const P2DVec2* m_vertices;
	int32 m_count;
	float32 m_radius;
};

/// Used to warm start P2DDistance.
/// Set count to zero on first call.
struct P2DSimplexCache
{
	float32 metric;		///< length or area
	uint16 count;
	uint8 indexA[3];	///< vertices on shape A
	uint8 indexB[3];	///< vertices on shape B
};

/// Input for P2DDistance.
/// You have to option to use the shape radii
/// in the computation. Even 
struct P2DDistanceInput
{
	P2DDistanceProxy proxyA;
	P2DDistanceProxy proxyB;
	P2DTransform transformA;
	P2DTransform transformB;
	bool useRadii;
};

/// Output for P2DDistance.
struct P2DDistanceOutput
{
	P2DVec2 pointA;		///< closest point on shapeA
	P2DVec2 pointB;		///< closest point on shapeB
	float32 distance;
	int32 iterations;	///< number of GJK iterations used
};

/// Compute the closest points between two shapes. Supports any combination of:
/// Circle, Polygon, Edge. The simplex cache is input/output.
/// On the first call set P2DSimplexCache.count to zero.
void P2DDistance(P2DDistanceOutput* output,
				P2DSimplexCache* cache, 
				const P2DDistanceInput* input);


//////////////////////////////////////////////////////////////////////////

inline int32 P2DDistanceProxy::GetVertexCount() const
{
	return m_count;
}

inline const P2DVec2& P2DDistanceProxy::GetVertex(int32 index) const
{
	assert(0 <= index && index < m_count);
	return m_vertices[index];
}

inline int32 P2DDistanceProxy::GetSupport(const P2DVec2& d) const
{
	int32 bestIndex = 0;
	float32 bestValue = P2DVecDot(m_vertices[0], d);
	for (int32 i = 1; i < m_count; ++i)
	{
		float32 value = P2DVecDot(m_vertices[i], d);
		if (value > bestValue)
		{
			bestIndex = i;
			bestValue = value;
		}
	}

	return bestIndex;
}

inline const P2DVec2& P2DDistanceProxy::GetSupportVertex(const P2DVec2& d) const
{
	int32 bestIndex = 0;
	float32 bestValue = P2DVecDot(m_vertices[0], d);
	for (int32 i = 1; i < m_count; ++i)
	{
		float32 value = P2DVecDot(m_vertices[i], d);
		if (value > bestValue)
		{
			bestIndex = i;
			bestValue = value;
		}
	}

	return m_vertices[bestIndex];
}

#endif
