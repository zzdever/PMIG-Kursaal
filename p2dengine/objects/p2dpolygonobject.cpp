#include "p2dpolygonobject.h"
//#include "p2dengine/objects/p2dpolygonobject.h"
#include <new>

 P2DPolygonObject::P2DPolygonObject()
{
    m_type = PolygonType;
    m_radius = P2D_POLYGON_RADIUS;
    m_count = 0;
    m_centroid.SetZero();
}

/*
b2Shape* P2DPolygonShape::Clone(b2BlockAllocator* allocator) const
{
	void* mem = allocator->Allocate(sizeof(b2PolygonShape));
	b2PolygonShape* clone = new (mem) b2PolygonShape;
	*clone = *this;
	return clone;
}
*/

void P2DPolygonObject::SetPoints(const P2DVec2* vertices, int32 count)
{
    assert(3 <= count && count <= P2D_MAX_POLYGON_VERTICES);

    if (count < 3)
    {
        SetARect(1.0f, 1.0f);
        return;
    }

    int32 n = P2DMin(count, P2D_MAX_POLYGON_VERTICES);

    // Merge points if they are too close
    P2DVec2 points[P2D_MAX_POLYGON_VERTICES];
    int32 tempCount = 0;
    for (int32 i = 0; i < n; ++i)
    {
        P2DVec2 v = vertices[i];

        bool unique = true;
        for (int32 j = 0; j < tempCount; ++j)
        {
            if (P2DDistanceSquared(v, points[j]) < 0.5f * P2D_LINEAR_SLOP)
            {
                unique = false;
                break;
            }
        }

        if (unique)
        {
            points[tempCount++] = v;
        }
    }

    n = tempCount;
    if (n < 3)
    {
        // Polygon is degenerated to a rectangle.
        assert(false);
        SetARect(1.0f, 1.0f);
        return;
    }

    // Find the lower right most point on the hull
    int32 i0 = 0;
    float32 x0 = points[0].x;
    for (int32 i = 1; i < n; ++i)
    {
        float32 x = points[i].x;
        if (x > x0 || (x == x0 && points[i].y < points[i0].y))
        {
            i0 = i;
            x0 = x;
        }
    }

    // Create the convex hull using the Gift wrapping algorithm
    int32 hull[P2D_MAX_POLYGON_VERTICES];
    int32 m = 0;
    int32 ih = i0;

    for (;;)
    {
        hull[m] = ih;

        int32 ie = 0;
        for (int32 j = 1; j < n; ++j)
        {
            if (ie == ih)
            {
                ie = j;
                continue;
            }

            P2DVec2 r = points[ie] - points[hull[m]];
            P2DVec2 v = points[j] - points[hull[m]];
            float32 c = P2DVecCross(r, v);
            if (c < 0.0f)
            {
                ie = j;
            }

            // Check if they are collinear
            if (c == 0.0f && v.LengthSquared() > r.LengthSquared())
            {
                ie = j;
            }
        }

        ++m;
        ih = ie;

        // Hull closed
        if (ie == i0)
        {
            break;
        }
    }

    if (m < 3)
    {
        // Polygon is degenerated to a rectangle.
        assert(false);
        SetARect(1.0f, 1.0f);
        return;
    }

    m_count = m;

    // Copy vertices.
    for (int32 i = 0; i < m; ++i)
    {
        m_vertices[i] = points[hull[i]];
    }

    // Compute normals. Ensure the edges have non-zero length.
    for (int32 i = 0; i < m; ++i)
    {
        int32 i1 = i;
        int32 i2 = i + 1 < m ? i + 1 : 0;
        P2DVec2 edge = m_vertices[i2] - m_vertices[i1];
        assert(edge.LengthSquared() > FLT_EPSILON * FLT_EPSILON);
        m_normals[i] = P2DVecCross(edge, 1.0f);
        m_normals[i].Normalize();
    }

    // Compute the polygon centroid.
    m_centroid = GetCentroid(m_vertices, m);
}

void P2DPolygonObject::SetARect(float32 hx, float32 hy)
{
	m_count = 4;
	m_vertices[0].Set(-hx, -hy);
	m_vertices[1].Set( hx, -hy);
	m_vertices[2].Set( hx,  hy);
	m_vertices[3].Set(-hx,  hy);
	m_normals[0].Set(0.0f, -1.0f);
	m_normals[1].Set(1.0f, 0.0f);
	m_normals[2].Set(0.0f, 1.0f);
	m_normals[3].Set(-1.0f, 0.0f);
	m_centroid.SetZero();
}

void P2DPolygonObject::SetARect(float32 hx, float32 hy, const P2DVec2& center, float32 angle)
{
	m_count = 4;
	m_vertices[0].Set(-hx, -hy);
	m_vertices[1].Set( hx, -hy);
	m_vertices[2].Set( hx,  hy);
	m_vertices[3].Set(-hx,  hy);
	m_normals[0].Set(0.0f, -1.0f);
	m_normals[1].Set(1.0f, 0.0f);
	m_normals[2].Set(0.0f, 1.0f);
	m_normals[3].Set(-1.0f, 0.0f);
	m_centroid = center;

    P2DTransform transform;
    transform.position = center;
    transform.rotation.Set(angle);

	// Transform vertices and normals.
	for (int32 i = 0; i < m_count; ++i)
	{
        m_vertices[i] = P2DMul(transform, m_vertices[i]);
        m_normals[i] = P2DMul(transform.rotation, m_normals[i]);
	}
}

int32 P2DPolygonObject::GetChildCount() const
{
    return 1;
}

bool P2DPolygonObject::TestPoint(const P2DTransform& transform, const P2DVec2& p) const
{
    P2DVec2 pLocal = P2DMulT(transform.rotation, p - transform.position);

	for (int32 i = 0; i < m_count; ++i)
	{
        float32 dot = P2DVecDot(m_normals[i], pLocal - m_vertices[i]);
		if (dot > 0.0f)
		{
			return false;
		}
	}

	return true;
}

/*
bool P2DPolygonObject::RayCast(b2RayCastOutput* output, const b2RayCastInput& input,
								const b2Transform& xf, int32 childIndex) const
{
    NOT_USED(childIndex);

	// Put the ray into the polygon's frame of reference.
    P2DVec2 p1 = b2MulT(xf.q, input.p1 - xf.p);
    P2DVec2 p2 = b2MulT(xf.q, input.p2 - xf.p);
    P2DVec2 d = p2 - p1;

	float32 lower = 0.0f, upper = input.maxFraction;

	int32 index = -1;

	for (int32 i = 0; i < m_count; ++i)
	{
		// p = p1 + a * d
		// dot(normal, p - v) = 0
		// dot(normal, p1 - v) + a * dot(normal, d) = 0
		float32 numerator = b2Dot(m_normals[i], m_vertices[i] - p1);
		float32 denominator = b2Dot(m_normals[i], d);

		if (denominator == 0.0f)
		{	
			if (numerator < 0.0f)
			{
				return false;
			}
		}
		else
		{
			// Note: we want this predicate without division:
			// lower < numerator / denominator, where denominator < 0
			// Since denominator < 0, we have to flip the inequality:
			// lower < numerator / denominator <==> denominator * lower > numerator.
			if (denominator < 0.0f && numerator < lower * denominator)
			{
				// Increase lower.
				// The segment enters this half-space.
				lower = numerator / denominator;
				index = i;
			}
			else if (denominator > 0.0f && numerator < upper * denominator)
			{
				// Decrease upper.
				// The segment exits this half-space.
				upper = numerator / denominator;
			}
		}

		// The use of epsilon here causes the assert on lower to trip
		// in some cases. Apparently the use of epsilon was to make edge
		// shapes work, but now those are handled separately.
		//if (upper < lower - b2_epsilon)
		if (upper < lower)
		{
			return false;
		}
	}

	b2Assert(0.0f <= lower && lower <= input.maxFraction);

	if (index >= 0)
	{
		output->fraction = lower;
		output->normal = b2Mul(xf.q, m_normals[index]);
		return true;
	}

	return false;
}
*/


void P2DPolygonObject::ComputeAxisAlignedBoundingBox(P2DAABB *aabb, const P2DTransform& transform, int32 childIndex) const
{
    NOT_USED(childIndex);

    P2DVec2 lower = P2DMul(transform, m_vertices[0]);
    P2DVec2 upper = lower;

	for (int32 i = 1; i < m_count; ++i)
	{
        P2DVec2 v = P2DMul(transform, m_vertices[i]);
        lower = P2DMin(lower, v);
        upper = P2DMax(upper, v);
	}

    P2DVec2 r(m_radius, m_radius);
	aabb->lowerBound = lower - r;
	aabb->upperBound = upper + r;
}


void P2DPolygonObject::ComputeMass(P2DMass* massData, float32 density) const
{
	// Polygon mass, centroid, and inertia.
	// Let rho be the polygon density in mass per unit area.
	// Then:
	// mass = rho * int(dA)
	// centroid.x = (1/mass) * rho * int(x * dA)
	// centroid.y = (1/mass) * rho * int(y * dA)
	// I = rho * int((x*x + y*y) * dA)
	//
	// We can compute these integrals by summing all the integrals
	// for each triangle of the polygon. To evaluate the integral
	// for a single triangle, we make a change of variables to
	// the (u,v) coordinates of the triangle:
	// x = x0 + e1x * u + e2x * v
	// y = y0 + e1y * u + e2y * v
	// where 0 <= u && 0 <= v && u + v <= 1.
	//
	// We integrate u from [0,1-v] and then v from [0,1].
	// We also need to use the Jacobian of the transformation:
	// D = cross(e1, e2)
	//
	// Simplification: triangle centroid = (1/3) * (p1 + p2 + p3)
	//
	// The rest of the derivation is handled by computer algebra.

    assert(m_count >= 3);

    P2DVec2 center;
    center.Set(0.0f, 0.0f);
	float32 area = 0.0f;
	float32 I = 0.0f;

	// s is the reference point for forming triangles.
	// It's location doesn't change the result (except for rounding error).
    P2DVec2 s(0.0f, 0.0f);

	// This code would put the reference point inside the polygon.
	for (int32 i = 0; i < m_count; ++i)
	{
		s += m_vertices[i];
	}
	s *= 1.0f / m_count;

    const float32 inv3 = 1.0f / 3.0f;

	for (int32 i = 0; i < m_count; ++i)
	{
		// Triangle vertices.
        P2DVec2 e1 = m_vertices[i] - s;
        P2DVec2 e2 = i + 1 < m_count ? m_vertices[i+1] - s : m_vertices[0] - s;

        float32 triangleArea = 0.5f * P2DVecCross(e1, e2);
		area += triangleArea;

		// Area weighted centroid
        center += triangleArea * inv3 * (e1 + e2);

		float32 ex1 = e1.x, ey1 = e1.y;
		float32 ex2 = e2.x, ey2 = e2.y;

		float32 intx2 = ex1*ex1 + ex2*ex1 + ex2*ex2;
		float32 inty2 = ey1*ey1 + ey2*ey1 + ey2*ey2;

        I += (0.25f * inv3 * P2DVecCross(e1, e2)) * (intx2 + inty2);
	}

	// Total mass
	massData->mass = density * area;

	// Center of mass
    assert(area > FLT_EPSILON);
	center *= 1.0f / area;
	massData->center = center + s;

	// Inertia tensor relative to the local origin (point s).
	massData->I = density * I;
	
	// Shift to center of mass then to original body origin.
    massData->I += massData->mass * (P2DVecDot(massData->center, massData->center) - P2DVecDot(center, center));
}

const P2DVec2& P2DPolygonObject::GetVertex(int32 index) const
{
    assert(0 <= index && index < m_count);
    return m_vertices[index];
}

bool P2DPolygonObject::ValidateConvexity() const
{
	for (int32 i = 0; i < m_count; ++i)
	{
		int32 i1 = i;
		int32 i2 = i < m_count - 1 ? i1 + 1 : 0;
        P2DVec2 p = m_vertices[i1];
        P2DVec2 e = m_vertices[i2] - p;

		for (int32 j = 0; j < m_count; ++j)
		{
			if (j == i1 || j == i2)
			{
				continue;
			}

            P2DVec2 v = m_vertices[j] - p;
            float32 c = P2DVecCross(e, v);
			if (c < 0.0f)
			{
				return false;
			}
		}
	}

	return true;
}

P2DVec2 P2DPolygonObject::GetCentroid(const P2DVec2* vs, int32 count)
{
    assert(count >= 3);

    P2DVec2 c;
    c.Set(0.0f, 0.0f);
    float32 area = 0.0f;

    // pRef is the reference point for forming triangles.
    // Its location doesn't change the result (except for rounding error).
    P2DVec2 pRef(0.0f, 0.0f);
#if 0
    // This code would put the reference point inside the polygon.
    for (int32 i = 0; i < count; ++i)
    {
        pRef += vs[i];
    }
    pRef *= 1.0f / count;
#endif

    const float32 inv3 = 1.0f / 3.0f;

    for (int32 i = 0; i < count; ++i)
    {
        // Triangle vertices.
        P2DVec2 p1 = pRef;
        P2DVec2 p2 = vs[i];
        P2DVec2 p3 = i + 1 < count ? vs[i+1] : vs[0];

        P2DVec2 e1 = p2 - p1;
        P2DVec2 e2 = p3 - p1;

        float32 triangleArea = 0.5f * P2DVecCross(e1, e2);
        area += triangleArea;

        // Area weighted centroid
        c += triangleArea * inv3 * (p1 + p2 + p3);
    }

    // Centroid
    assert(area > FLT_EPSILON);
    c *= 1.0f / area;
    return c;
}

