#include "p2dcollision.h"
#include "p2ddistance.h"


void P2DSceneManifold::Initialize(const P2DManifold* manifold,
						  const P2DTransform& xfA, float32 radiusA,
						  const P2DTransform& xfB, float32 radiusB)
{
	if (manifold->pointCount == 0)
	{
		return;
	}

	switch (manifold->type)
	{
	case P2DManifold::e_circles:
		{
			normal.Set(1.0f, 0.0f);
			P2DVec2 pointA = P2DMul(xfA, manifold->localPoint);
			P2DVec2 pointB = P2DMul(xfB, manifold->points[0].localPoint);
			if (P2DDistanceSquared(pointA, pointB) > FLT_EPSILON * FLT_EPSILON)
			{
				normal = pointB - pointA;
				normal.Normalize();
			}

			P2DVec2 cA = pointA + radiusA * normal;
			P2DVec2 cB = pointB - radiusB * normal;
			points[0] = 0.5f * (cA + cB);
			separations[0] = P2DVecDot(cB - cA, normal);
		}
		break;

	case P2DManifold::e_faceA:
		{
            normal = P2DMul(xfA.rotation, manifold->localNormal);
			P2DVec2 planePoint = P2DMul(xfA, manifold->localPoint);
			
			for (int32 i = 0; i < manifold->pointCount; ++i)
			{
				P2DVec2 clipPoint = P2DMul(xfB, manifold->points[i].localPoint);
				P2DVec2 cA = clipPoint + (radiusA - P2DVecDot(clipPoint - planePoint, normal)) * normal;
				P2DVec2 cB = clipPoint - radiusB * normal;
				points[i] = 0.5f * (cA + cB);
				separations[i] = P2DVecDot(cB - cA, normal);
			}
		}
		break;

	case P2DManifold::e_faceB:
		{
            normal = P2DMul(xfB.rotation, manifold->localNormal);
			P2DVec2 planePoint = P2DMul(xfB, manifold->localPoint);

			for (int32 i = 0; i < manifold->pointCount; ++i)
			{
				P2DVec2 clipPoint = P2DMul(xfA, manifold->points[i].localPoint);
				P2DVec2 cB = clipPoint + (radiusB - P2DVecDot(clipPoint - planePoint, normal)) * normal;
				P2DVec2 cA = clipPoint - radiusA * normal;
				points[i] = 0.5f * (cA + cB);
				separations[i] = P2DVecDot(cA - cB, normal);
			}

			// Ensure normal points from A to B.
			normal = -normal;
		}
		break;
	}
}


void P2DGetPointStates(P2DPointState state1[P2D_MAX_MANIFOLD_POINTS], P2DPointState state2[P2D_MAX_MANIFOLD_POINTS],
                      const P2DManifold* manifold1, const P2DManifold* manifold2)
{
    for (int32 i = 0; i < P2D_MAX_MANIFOLD_POINTS; ++i)
	{
        state1[i] = NullState;
        state2[i] = NullState;
	}

	// Detect persists and removes.
	for (int32 i = 0; i < manifold1->pointCount; ++i)
	{
        P2DContactID id = manifold1->points[i].id;

        state1[i] = RemoveState;

		for (int32 j = 0; j < manifold2->pointCount; ++j)
		{
			if (manifold2->points[j].id.key == id.key)
			{
                state1[i] = PersistState;
				break;
			}
		}
	}

	// Detect persists and adds.
	for (int32 i = 0; i < manifold2->pointCount; ++i)
	{
        P2DContactID id = manifold2->points[i].id;

        state2[i] = AddState;

		for (int32 j = 0; j < manifold1->pointCount; ++j)
		{
			if (manifold1->points[j].id.key == id.key)
			{
                state2[i] = PersistState;
				break;
			}
		}
	}
}

// From Real-time Collision Detection, p179.
bool P2DAABB::RayCast(P2DRayCastOutput* output, const P2DRayCastInput& input) const
{
    float32 tmin = -FLT_MAX;
    float32 tmax = FLT_MAX;

    P2DVec2 p = input.p1;
    P2DVec2 d = input.p2 - input.p1;
    P2DVec2 absD = P2DAbs(d);

    P2DVec2 normal;

	for (int32 i = 0; i < 2; ++i)
	{
        if (absD(i) < FLT_EPSILON)
		{
			// Parallel.
			if (p(i) < lowerBound(i) || upperBound(i) < p(i))
			{
				return false;
			}
		}
		else
		{
			float32 inv_d = 1.0f / d(i);
			float32 t1 = (lowerBound(i) - p(i)) * inv_d;
			float32 t2 = (upperBound(i) - p(i)) * inv_d;

			// Sign of the normal vector.
			float32 s = -1.0f;

			if (t1 > t2)
			{
                P2DSwap(t1, t2);
				s = 1.0f;
			}

			// Push the min up
			if (t1 > tmin)
			{
				normal.SetZero();
				normal(i) = s;
				tmin = t1;
			}

			// Pull the max down
            tmax = P2DMin(tmax, t2);

			if (tmin > tmax)
			{
				return false;
			}
		}
	}

	// Does the ray start inside the box?
	// Does the ray intersect beyond the max fraction?
	if (tmin < 0.0f || input.maxFraction < tmin)
	{
		return false;
	}

	// Intersection.
	output->fraction = tmin;
	output->normal = normal;
	return true;
}



// Sutherland-Hodgman clipping.
int32 P2DClipSegmentToLine(P2DClipVertex vOut[2], const P2DClipVertex vIn[2],
                            const P2DVec2& normal, float32 offset, int32 vertexIndexA)
{
	// Start with no output points
	int32 numOut = 0;

	// Calculate the distance of end points to the line
	float32 distance0 = P2DVecDot(normal, vIn[0].v) - offset;
	float32 distance1 = P2DVecDot(normal, vIn[1].v) - offset;

	// If the points are behind the plane
	if (distance0 <= 0.0f) vOut[numOut++] = vIn[0];
	if (distance1 <= 0.0f) vOut[numOut++] = vIn[1];

	// If the points are on different sides of the plane
	if (distance0 * distance1 < 0.0f)
	{
		// Find intersection point of edge and plane
		float32 interp = distance0 / (distance0 - distance1);
		vOut[numOut].v = vIn[0].v + interp * (vIn[1].v - vIn[0].v);

		// VertexA is hitting edgeB.
		vOut[numOut].id.cf.indexA = static_cast<uint8>(vertexIndexA);
		vOut[numOut].id.cf.indexB = vIn[0].id.cf.indexB;
		vOut[numOut].id.cf.typeA = P2DContactFeature::e_vertex;
		vOut[numOut].id.cf.typeB = P2DContactFeature::e_face;
		++numOut;
	}

	return numOut;
}


bool P2DTestOverlap(const P2DBaseObject* shapeA, int32 indexA,
                    const P2DBaseObject* shapeB, int32 indexB,
                    const P2DTransform& xfA, const P2DTransform& xfB)
{
    P2DDistanceInput input;
	input.proxyA.Set(shapeA, indexA);
	input.proxyB.Set(shapeB, indexB);
	input.transformA = xfA;
	input.transformB = xfB;
	input.useRadii = true;

    P2DSimplexCache cache;
	cache.count = 0;

    P2DDistanceOutput output;

    P2DDistance(&output, &cache, &input);

    return output.distance < 10.0f * FLT_EPSILON;
}

