#include "p2dcollision.h"
#include "../objects/p2dpolygonobject.h"

// Find the max separation between poly1 and poly2 using edge normals from poly1.
static float32 P2DFindMaxSeparation(int32* edgeIndex,
                                 const P2DPolygonObject* poly1, const P2DTransform& xf1,
                                 const P2DPolygonObject* poly2, const P2DTransform& xf2)
{
	int32 count1 = poly1->m_count;
	int32 count2 = poly2->m_count;
    const P2DVec2* n1s = poly1->m_normals;
    const P2DVec2* v1s = poly1->m_vertices;
    const P2DVec2* v2s = poly2->m_vertices;
    P2DTransform xf = P2DMulT(xf2, xf1);

	int32 bestIndex = 0;
    float32 maxSeparation = -FLT_MAX;
	for (int32 i = 0; i < count1; ++i)
	{
		// Get poly1 normal in frame2.
        P2DVec2 n = P2DMul(xf.rotation, n1s[i]);
        P2DVec2 v1 = P2DMul(xf, v1s[i]);

		// Find deepest point for normal i.
        float32 si = FLT_MAX;
		for (int32 j = 0; j < count2; ++j)
		{
            float32 sij = P2DVecDot(n, v2s[j] - v1);
			if (sij < si)
			{
				si = sij;
			}
		}

		if (si > maxSeparation)
		{
			maxSeparation = si;
			bestIndex = i;
		}
	}

	*edgeIndex = bestIndex;
	return maxSeparation;
}

static void P2DFindIncidentEdge(P2DClipVertex c[2],
                             const P2DPolygonObject* poly1, const P2DTransform& xf1, int32 edge1,
                             const P2DPolygonObject* poly2, const P2DTransform& xf2)
{
    const P2DVec2* normals1 = poly1->m_normals;

	int32 count2 = poly2->m_count;
    const P2DVec2* vertices2 = poly2->m_vertices;
    const P2DVec2* normals2 = poly2->m_normals;

    assert(0 <= edge1 && edge1 < poly1->m_count);

	// Get the normal of the reference edge in poly2's frame.
    P2DVec2 normal1 = P2DMulT(xf2.rotation, P2DMul(xf1.rotation, normals1[edge1]));

	// Find the incident edge on poly2.
	int32 index = 0;
    float32 minDot = FLT_MAX;
	for (int32 i = 0; i < count2; ++i)
	{
        float32 dot = P2DVecDot(normal1, normals2[i]);
		if (dot < minDot)
		{
			minDot = dot;
			index = i;
		}
	}

	// Build the clip vertices for the incident edge.
	int32 i1 = index;
	int32 i2 = i1 + 1 < count2 ? i1 + 1 : 0;

    c[0].v = P2DMul(xf2, vertices2[i1]);
	c[0].id.cf.indexA = (uint8)edge1;
	c[0].id.cf.indexB = (uint8)i1;
    c[0].id.cf.typeA = P2DContactFeature::e_face;
    c[0].id.cf.typeB = P2DContactFeature::e_vertex;

    c[1].v = P2DMul(xf2, vertices2[i2]);
	c[1].id.cf.indexA = (uint8)edge1;
	c[1].id.cf.indexB = (uint8)i2;
    c[1].id.cf.typeA = P2DContactFeature::e_face;
    c[1].id.cf.typeB = P2DContactFeature::e_vertex;
}

// Find edge normal of max separation on A - return if separating axis is found
// Find edge normal of max separation on B - return if separation axis is found
// Choose reference edge as min(minA, minB)
// Find incident edge
// Clip

// The normal points from 1 to 2
void P2DCollidePolygons(P2DManifold* manifold,
                      const P2DPolygonObject* polyA, const P2DTransform& xfA,
                      const P2DPolygonObject* polyB, const P2DTransform& xfB)
{
	manifold->pointCount = 0;
	float32 totalRadius = polyA->m_radius + polyB->m_radius;

	int32 edgeA = 0;
    float32 separationA = P2DFindMaxSeparation(&edgeA, polyA, xfA, polyB, xfB);
	if (separationA > totalRadius)
		return;

	int32 edgeB = 0;
    float32 separationB = P2DFindMaxSeparation(&edgeB, polyB, xfB, polyA, xfA);
	if (separationB > totalRadius)
		return;

    const P2DPolygonObject* poly1;	// reference polygon
    const P2DPolygonObject* poly2;	// incident polygon
    P2DTransform xf1, xf2;
    int32 edge1;					// reference edge
	uint8 flip;
    const float32 k_tol = 0.1f * P2D_LINEAR_SLOP;

	if (separationB > separationA + k_tol)
	{
		poly1 = polyB;
		poly2 = polyA;
		xf1 = xfB;
		xf2 = xfA;
		edge1 = edgeB;
        manifold->type = P2DManifold::e_faceB;
		flip = 1;
	}
	else
	{
		poly1 = polyA;
		poly2 = polyB;
		xf1 = xfA;
		xf2 = xfB;
		edge1 = edgeA;
        manifold->type = P2DManifold::e_faceA;
		flip = 0;
	}

    P2DClipVertex incidentEdge[2];
    P2DFindIncidentEdge(incidentEdge, poly1, xf1, edge1, poly2, xf2);

	int32 count1 = poly1->m_count;
    const P2DVec2* vertices1 = poly1->m_vertices;

	int32 iv1 = edge1;
	int32 iv2 = edge1 + 1 < count1 ? edge1 + 1 : 0;

    P2DVec2 v11 = vertices1[iv1];
    P2DVec2 v12 = vertices1[iv2];

    P2DVec2 localTangent = v12 - v11;
	localTangent.Normalize();
	
    P2DVec2 localNormal = P2DVecCross(localTangent, 1.0f);
    P2DVec2 planePoint = 0.5f * (v11 + v12);

    P2DVec2 tangent = P2DMul(xf1.rotation, localTangent);
    P2DVec2 normal = P2DVecCross(tangent, 1.0f);
	
    v11 = P2DMul(xf1, v11);
    v12 = P2DMul(xf1, v12);

	// Face offset.
    float32 frontOffset = P2DVecDot(normal, v11);

	// Side offsets, extended by polytope skin thickness.
    float32 sideOffset1 = -P2DVecDot(tangent, v11) + totalRadius;
    float32 sideOffset2 = P2DVecDot(tangent, v12) + totalRadius;

	// Clip incident edge against extruded edge1 side edges.
    P2DClipVertex clipPoints1[2];
    P2DClipVertex clipPoints2[2];
	int np;

	// Clip to box side 1
    np = P2DClipSegmentToLine(clipPoints1, incidentEdge, -tangent, sideOffset1, iv1);

	if (np < 2)
		return;

	// Clip to negative box side 1
    np = P2DClipSegmentToLine(clipPoints2, clipPoints1,  tangent, sideOffset2, iv2);

	if (np < 2)
	{
		return;
	}

	// Now clipPoints2 contains the clipped points.
	manifold->localNormal = localNormal;
	manifold->localPoint = planePoint;

	int32 pointCount = 0;
    for (int32 i = 0; i < P2D_MAX_MANIFOLD_POINTS; ++i)
	{
        float32 separation = P2DVecDot(normal, clipPoints2[i].v) - frontOffset;

		if (separation <= totalRadius)
		{
            P2DManifoldPoint* cp = manifold->points + pointCount;
            cp->localPoint = P2DMulT(xf2, clipPoints2[i].v);
			cp->id = clipPoints2[i].id;
			if (flip)
			{
				// Swap features
                P2DContactFeature cf = cp->id.cf;
				cp->id.cf.indexA = cf.indexB;
				cp->id.cf.indexB = cf.indexA;
				cp->id.cf.typeA = cf.typeB;
				cp->id.cf.typeB = cf.typeA;
			}
			++pointCount;
		}
	}

	manifold->pointCount = pointCount;
}
