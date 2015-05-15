#include "p2dcollision.h"
#include "p2ddistance.h"
#include "p2dtoi.h"
//#include "CircleShape.h"
#include "../objects/p2dpolygonobject.h"
#include "../general/p2dtimer.h"

#include <stdio.h>

float32 p2d_toiTime, p2d_toiMaxTime;
int32 p2d_toiCalls, p2d_toiIters, p2d_toiMaxIters;
int32 p2d_toiRootIters, p2d_toiMaxRootIters;

struct P2DSeparationFunction
{
	enum Type
	{
		e_points,
		e_faceA,
		e_faceB
	};

	// TODO might not need to return the separation

	float32 Initialize(const P2DSimplexCache* cache,
		const P2DDistanceProxy* proxyA, const P2DSweep& sweepA,
		const P2DDistanceProxy* proxyB, const P2DSweep& sweepB,
		float32 t1)
	{
		m_proxyA = proxyA;
		m_proxyB = proxyB;
		int32 count = cache->count;
		assert(0 < count && count < 3);

		m_sweepA = sweepA;
		m_sweepB = sweepB;

		P2DTransform xfA, xfB;
		m_sweepA.GetTransform(&xfA, t1);
		m_sweepB.GetTransform(&xfB, t1);

		if (count == 1)
		{
			m_type = e_points;
			P2DVec2 localPointA = m_proxyA->GetVertex(cache->indexA[0]);
			P2DVec2 localPointB = m_proxyB->GetVertex(cache->indexB[0]);
			P2DVec2 pointA = P2DMul(xfA, localPointA);
			P2DVec2 pointB = P2DMul(xfB, localPointB);
			m_axis = pointB - pointA;
			float32 s = m_axis.Normalize();
			return s;
		}
		else if (cache->indexA[0] == cache->indexA[1])
		{
			// Two points on B and one on A.
			m_type = e_faceB;
			P2DVec2 localPointB1 = proxyB->GetVertex(cache->indexB[0]);
			P2DVec2 localPointB2 = proxyB->GetVertex(cache->indexB[1]);

			m_axis = P2DVecCross(localPointB2 - localPointB1, 1.0f);
			m_axis.Normalize();
            P2DVec2 normal = P2DMul(xfB.rotation, m_axis);

			m_localPoint = 0.5f * (localPointB1 + localPointB2);
			P2DVec2 pointB = P2DMul(xfB, m_localPoint);

			P2DVec2 localPointA = proxyA->GetVertex(cache->indexA[0]);
			P2DVec2 pointA = P2DMul(xfA, localPointA);

			float32 s = P2DVecDot(pointA - pointB, normal);
			if (s < 0.0f)
			{
				m_axis = -m_axis;
				s = -s;
			}
			return s;
		}
		else
		{
			// Two points on A and one or two points on B.
			m_type = e_faceA;
			P2DVec2 localPointA1 = m_proxyA->GetVertex(cache->indexA[0]);
			P2DVec2 localPointA2 = m_proxyA->GetVertex(cache->indexA[1]);
			
			m_axis = P2DVecCross(localPointA2 - localPointA1, 1.0f);
			m_axis.Normalize();
            P2DVec2 normal = P2DMul(xfA.rotation, m_axis);

			m_localPoint = 0.5f * (localPointA1 + localPointA2);
			P2DVec2 pointA = P2DMul(xfA, m_localPoint);

			P2DVec2 localPointB = m_proxyB->GetVertex(cache->indexB[0]);
			P2DVec2 pointB = P2DMul(xfB, localPointB);

			float32 s = P2DVecDot(pointB - pointA, normal);
			if (s < 0.0f)
			{
				m_axis = -m_axis;
				s = -s;
			}
			return s;
		}
	}

	//
	float32 FindMinSeparation(int32* indexA, int32* indexB, float32 t) const
	{
		P2DTransform xfA, xfB;
		m_sweepA.GetTransform(&xfA, t);
		m_sweepB.GetTransform(&xfB, t);

		switch (m_type)
		{
		case e_points:
			{
                P2DVec2 axisA = P2DMulT(xfA.rotation,  m_axis);
                P2DVec2 axisB = P2DMulT(xfB.rotation, -m_axis);

				*indexA = m_proxyA->GetSupport(axisA);
				*indexB = m_proxyB->GetSupport(axisB);

				P2DVec2 localPointA = m_proxyA->GetVertex(*indexA);
				P2DVec2 localPointB = m_proxyB->GetVertex(*indexB);
				
				P2DVec2 pointA = P2DMul(xfA, localPointA);
				P2DVec2 pointB = P2DMul(xfB, localPointB);

				float32 separation = P2DVecDot(pointB - pointA, m_axis);
				return separation;
			}

		case e_faceA:
			{
                P2DVec2 normal = P2DMul(xfA.rotation, m_axis);
				P2DVec2 pointA = P2DMul(xfA, m_localPoint);

                P2DVec2 axisB = P2DMulT(xfB.rotation, -normal);
				
				*indexA = -1;
				*indexB = m_proxyB->GetSupport(axisB);

				P2DVec2 localPointB = m_proxyB->GetVertex(*indexB);
				P2DVec2 pointB = P2DMul(xfB, localPointB);

				float32 separation = P2DVecDot(pointB - pointA, normal);
				return separation;
			}

		case e_faceB:
			{
                P2DVec2 normal = P2DMul(xfB.rotation, m_axis);
				P2DVec2 pointB = P2DMul(xfB, m_localPoint);

                P2DVec2 axisA = P2DMulT(xfA.rotation, -normal);

				*indexB = -1;
				*indexA = m_proxyA->GetSupport(axisA);

				P2DVec2 localPointA = m_proxyA->GetVertex(*indexA);
				P2DVec2 pointA = P2DMul(xfA, localPointA);

				float32 separation = P2DVecDot(pointA - pointB, normal);
				return separation;
			}

		default:
			assert(false);
			*indexA = -1;
			*indexB = -1;
			return 0.0f;
		}
	}

	//
	float32 Evaluate(int32 indexA, int32 indexB, float32 t) const
	{
		P2DTransform xfA, xfB;
		m_sweepA.GetTransform(&xfA, t);
		m_sweepB.GetTransform(&xfB, t);

		switch (m_type)
		{
		case e_points:
			{
				P2DVec2 localPointA = m_proxyA->GetVertex(indexA);
				P2DVec2 localPointB = m_proxyB->GetVertex(indexB);

				P2DVec2 pointA = P2DMul(xfA, localPointA);
				P2DVec2 pointB = P2DMul(xfB, localPointB);
				float32 separation = P2DVecDot(pointB - pointA, m_axis);

				return separation;
			}

		case e_faceA:
			{
                P2DVec2 normal = P2DMul(xfA.rotation, m_axis);
				P2DVec2 pointA = P2DMul(xfA, m_localPoint);

				P2DVec2 localPointB = m_proxyB->GetVertex(indexB);
				P2DVec2 pointB = P2DMul(xfB, localPointB);

				float32 separation = P2DVecDot(pointB - pointA, normal);
				return separation;
			}

		case e_faceB:
			{
                P2DVec2 normal = P2DMul(xfB.rotation, m_axis);
				P2DVec2 pointB = P2DMul(xfB, m_localPoint);

				P2DVec2 localPointA = m_proxyA->GetVertex(indexA);
				P2DVec2 pointA = P2DMul(xfA, localPointA);

				float32 separation = P2DVecDot(pointA - pointB, normal);
				return separation;
			}

		default:
			assert(false);
			return 0.0f;
		}
	}

	const P2DDistanceProxy* m_proxyA;
	const P2DDistanceProxy* m_proxyB;
	P2DSweep m_sweepA, m_sweepB;
	Type m_type;
	P2DVec2 m_localPoint;
	P2DVec2 m_axis;
};

// CCD via the local separating axis method. This seeks progression
// by computing the largest time at which separation is maintained.
void P2DTimeOfImpact(P2DTOIOutput* output, const P2DTOIInput* input)
{
	P2DTimer timer;

	++p2d_toiCalls;

	output->state = P2DTOIOutput::e_unknown;
	output->t = input->tMax;

	const P2DDistanceProxy* proxyA = &input->proxyA;
	const P2DDistanceProxy* proxyB = &input->proxyB;

	P2DSweep sweepA = input->sweepA;
	P2DSweep sweepB = input->sweepB;

	// Large rotations can make the root finder fail, so we normalize the
	// sweep angles.
	sweepA.Normalize();
	sweepB.Normalize();

	float32 tMax = input->tMax;

	float32 totalRadius = proxyA->m_radius + proxyB->m_radius;
	float32 target = P2DMax(P2D_LINEAR_SLOP, totalRadius - 3.0f * P2D_LINEAR_SLOP);
	float32 tolerance = 0.25f * P2D_LINEAR_SLOP;
	assert(target > tolerance);

	float32 t1 = 0.0f;
    const int32 k_maxIterations = 20;	// TODO P2DParams
	int32 iter = 0;

	// Prepare input for distance query.
	P2DSimplexCache cache;
	cache.count = 0;
	P2DDistanceInput distanceInput;
	distanceInput.proxyA = input->proxyA;
	distanceInput.proxyB = input->proxyB;
	distanceInput.useRadii = false;

	// The outer loop progressively attempts to compute new separating axes.
	// This loop terminates when an axis is repeated (no progress is made).
	for(;;)
	{
		P2DTransform xfA, xfB;
		sweepA.GetTransform(&xfA, t1);
		sweepB.GetTransform(&xfB, t1);

		// Get the distance between shapes. We can also use the results
		// to get a separating axis.
		distanceInput.transformA = xfA;
		distanceInput.transformB = xfB;
		P2DDistanceOutput distanceOutput;
		P2DDistance(&distanceOutput, &cache, &distanceInput);

		// If the shapes are overlapped, we give up on continuous collision.
		if (distanceOutput.distance <= 0.0f)
		{
			// Failure!
			output->state = P2DTOIOutput::e_overlapped;
			output->t = 0.0f;
			break;
		}

		if (distanceOutput.distance < target + tolerance)
		{
			// Victory!
			output->state = P2DTOIOutput::e_touching;
			output->t = t1;
			break;
		}

		// Initialize the separating axis.
		P2DSeparationFunction fcn;
		fcn.Initialize(&cache, proxyA, sweepA, proxyB, sweepB, t1);
#if 0
		// Dump the curve seen by the root finder
		{
			const int32 N = 100;
			float32 dx = 1.0f / N;
			float32 xs[N+1];
			float32 fs[N+1];

			float32 x = 0.0f;

			for (int32 i = 0; i <= N; ++i)
			{
				sweepA.GetTransform(&xfA, x);
				sweepB.GetTransform(&xfB, x);
				float32 f = fcn.Evaluate(xfA, xfB) - target;

				printf("%g %g\n", x, f);

				xs[i] = x;
				fs[i] = f;

				x += dx;
			}
		}
#endif

		// Compute the TOI on the separating axis. We do this by successively
		// resolving the deepest point. This loop is bounded by the number of vertices.
		bool done = false;
		float32 t2 = tMax;
		int32 pushBackIter = 0;
		for (;;)
		{
			// Find the deepest point at t2. Store the witness point indices.
			int32 indexA, indexB;
			float32 s2 = fcn.FindMinSeparation(&indexA, &indexB, t2);

			// Is the final configuration separated?
			if (s2 > target + tolerance)
			{
				// Victory!
				output->state = P2DTOIOutput::e_separated;
				output->t = tMax;
				done = true;
				break;
			}

			// Has the separation reached tolerance?
			if (s2 > target - tolerance)
			{
				// Advance the sweeps
				t1 = t2;
				break;
			}

			// Compute the initial separation of the witness points.
			float32 s1 = fcn.Evaluate(indexA, indexB, t1);

			// Check for initial overlap. This might happen if the root finder
			// runs out of iterations.
			if (s1 < target - tolerance)
			{
				output->state = P2DTOIOutput::e_failed;
				output->t = t1;
				done = true;
				break;
			}

			// Check for touching
			if (s1 <= target + tolerance)
			{
				// Victory! t1 should hold the TOI (could be 0.0).
				output->state = P2DTOIOutput::e_touching;
				output->t = t1;
				done = true;
				break;
			}

			// Compute 1D root of: f(x) - target = 0
			int32 rootIterCount = 0;
			float32 a1 = t1, a2 = t2;
			for (;;)
			{
				// Use a mix of the secant rule and bisection.
				float32 t;
				if (rootIterCount & 1)
				{
					// Secant rule to improve convergence.
					t = a1 + (target - s1) * (a2 - a1) / (s2 - s1);
				}
				else
				{
					// Bisection to guarantee progress.
					t = 0.5f * (a1 + a2);
				}

				++rootIterCount;
				++p2d_toiRootIters;

				float32 s = fcn.Evaluate(indexA, indexB, t);

				if (P2DAbs(s - target) < tolerance)
				{
					// t2 holds a tentative value for t1
					t2 = t;
					break;
				}

				// Ensure we continue to bracket the root.
				if (s > target)
				{
					a1 = t;
					s1 = s;
				}
				else
				{
					a2 = t;
					s2 = s;
				}
				
				if (rootIterCount == 50)
				{
					break;
				}
			}

			p2d_toiMaxRootIters = P2DMax(p2d_toiMaxRootIters, rootIterCount);

			++pushBackIter;

            if (pushBackIter == P2D_MAX_POLYGON_VERTICES)
			{
				break;
			}
		}

		++iter;
		++p2d_toiIters;

		if (done)
		{
			break;
		}

		if (iter == k_maxIterations)
		{
			// Root finder got stuck. Semi-victory.
			output->state = P2DTOIOutput::e_failed;
			output->t = t1;
			break;
		}
	}

	p2d_toiMaxIters = P2DMax(p2d_toiMaxIters, iter);

	float32 time = timer.GetMilliseconds();
	p2d_toiMaxTime = P2DMax(p2d_toiMaxTime, time);
	p2d_toiTime += time;
}
