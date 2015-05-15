#include "p2dcontactsolver.h"

#include "p2dcontact.h"
#include "../scene/p2dbody.h"
#include "../scene/p2dfixture.h"
#include "../scene/p2dscenemanager.h"
#include "../general/p2dmem.h"

#define DEBUG_SOLVER 0

bool g_blockSolve = true;

struct P2DContactPositionConstraint
{
    P2DVec2 localPoints[P2D_MAX_MANIFOLD_POINTS];
    P2DVec2 localNormal;
    P2DVec2 localPoint;
	int32 indexA;
	int32 indexB;
	float32 invMassA, invMassB;
    P2DVec2 localCenterA, localCenterB;
	float32 invIA, invIB;
    P2DManifold::Type type;
	float32 radiusA, radiusB;
	int32 pointCount;
};

P2DContactSolver::P2DContactSolver(P2DContactSolverDef* def)
{
	m_step = def->step;
	m_allocator = def->allocator;
	m_count = def->count;
    m_positionConstraints = (P2DContactPositionConstraint*)m_allocator->Allocate(m_count * sizeof(P2DContactPositionConstraint));
    m_velocityConstraints = (P2DContactVelocityConstraint*)m_allocator->Allocate(m_count * sizeof(P2DContactVelocityConstraint));
	m_positions = def->positions;
	m_velocities = def->velocities;
	m_contacts = def->contacts;

	// Initialize position independent portions of the constraints.
	for (int32 i = 0; i < m_count; ++i)
	{
        P2DContact* contact = m_contacts[i];

        P2DFixture* fixtureA = contact->m_fixtureA;
        P2DFixture* fixtureB = contact->m_fixtureB;
        P2DBaseObject* shapeA = fixtureA->GetShape();
        P2DBaseObject* shapeB = fixtureB->GetShape();
		float32 radiusA = shapeA->m_radius;
		float32 radiusB = shapeB->m_radius;
        P2DBody* bodyA = fixtureA->GetBody();
        P2DBody* bodyB = fixtureB->GetBody();
        P2DManifold* manifold = contact->GetManifold();

		int32 pointCount = manifold->pointCount;
        assert(pointCount > 0);

        P2DContactVelocityConstraint* vc = m_velocityConstraints + i;
		vc->friction = contact->m_friction;
		vc->restitution = contact->m_restitution;
		vc->tangentSpeed = contact->m_tangentSpeed;
		vc->indexA = bodyA->m_islandIndex;
		vc->indexB = bodyB->m_islandIndex;
		vc->invMassA = bodyA->m_invMass;
		vc->invMassB = bodyB->m_invMass;
		vc->invIA = bodyA->m_invI;
		vc->invIB = bodyB->m_invI;
		vc->contactIndex = i;
		vc->pointCount = pointCount;
		vc->K.SetZero();
		vc->normalMass.SetZero();

        P2DContactPositionConstraint* pc = m_positionConstraints + i;
		pc->indexA = bodyA->m_islandIndex;
		pc->indexB = bodyB->m_islandIndex;
		pc->invMassA = bodyA->m_invMass;
		pc->invMassB = bodyB->m_invMass;
		pc->localCenterA = bodyA->m_sweep.localCenter;
		pc->localCenterB = bodyB->m_sweep.localCenter;
		pc->invIA = bodyA->m_invI;
		pc->invIB = bodyB->m_invI;
		pc->localNormal = manifold->localNormal;
		pc->localPoint = manifold->localPoint;
		pc->pointCount = pointCount;
		pc->radiusA = radiusA;
		pc->radiusB = radiusB;
		pc->type = manifold->type;

		for (int32 j = 0; j < pointCount; ++j)
		{
            P2DManifoldPoint* cp = manifold->points + j;
            P2DVelocityConstraintPoint* vcp = vc->points + j;
	
			if (m_step.warmStarting)
			{
				vcp->normalImpulse = m_step.dtRatio * cp->normalImpulse;
				vcp->tangentImpulse = m_step.dtRatio * cp->tangentImpulse;
			}
			else
			{
				vcp->normalImpulse = 0.0f;
				vcp->tangentImpulse = 0.0f;
			}

			vcp->rA.SetZero();
			vcp->rB.SetZero();
			vcp->normalMass = 0.0f;
			vcp->tangentMass = 0.0f;
			vcp->velocityBias = 0.0f;

			pc->localPoints[j] = cp->localPoint;
		}
	}
}

P2DContactSolver::~P2DContactSolver()
{
	m_allocator->Free(m_velocityConstraints);
	m_allocator->Free(m_positionConstraints);
}

// Initialize position dependent portions of the velocity constraints.
void P2DContactSolver::InitializeVelocityConstraints()
{
	for (int32 i = 0; i < m_count; ++i)
	{
        P2DContactVelocityConstraint* vc = m_velocityConstraints + i;
        P2DContactPositionConstraint* pc = m_positionConstraints + i;

		float32 radiusA = pc->radiusA;
		float32 radiusB = pc->radiusB;
        P2DManifold* manifold = m_contacts[vc->contactIndex]->GetManifold();

		int32 indexA = vc->indexA;
		int32 indexB = vc->indexB;

		float32 mA = vc->invMassA;
		float32 mB = vc->invMassB;
		float32 iA = vc->invIA;
		float32 iB = vc->invIB;
        P2DVec2 localCenterA = pc->localCenterA;
        P2DVec2 localCenterB = pc->localCenterB;

        P2DVec2 cA = m_positions[indexA].c;
		float32 aA = m_positions[indexA].a;
        P2DVec2 vA = m_velocities[indexA].v;
		float32 wA = m_velocities[indexA].w;

        P2DVec2 cB = m_positions[indexB].c;
		float32 aB = m_positions[indexB].a;
        P2DVec2 vB = m_velocities[indexB].v;
		float32 wB = m_velocities[indexB].w;

        assert(manifold->pointCount > 0);

        P2DTransform xfA, xfB;
        xfA.rotation.Set(aA);
        xfB.rotation.Set(aB);
        xfA.position = cA - P2DMul(xfA.rotation, localCenterA);
        xfB.position = cB - P2DMul(xfB.rotation, localCenterB);

        P2DSceneManifold worldManifold;
		worldManifold.Initialize(manifold, xfA, radiusA, xfB, radiusB);

		vc->normal = worldManifold.normal;

		int32 pointCount = vc->pointCount;
		for (int32 j = 0; j < pointCount; ++j)
		{
            P2DVelocityConstraintPoint* vcp = vc->points + j;

			vcp->rA = worldManifold.points[j] - cA;
			vcp->rB = worldManifold.points[j] - cB;

            float32 rnA = P2DVecCross(vcp->rA, vc->normal);
            float32 rnB = P2DVecCross(vcp->rB, vc->normal);

			float32 kNormal = mA + mB + iA * rnA * rnA + iB * rnB * rnB;

			vcp->normalMass = kNormal > 0.0f ? 1.0f / kNormal : 0.0f;

            P2DVec2 tangent = P2DVecCross(vc->normal, 1.0f);

            float32 rtA = P2DVecCross(vcp->rA, tangent);
            float32 rtB = P2DVecCross(vcp->rB, tangent);

			float32 kTangent = mA + mB + iA * rtA * rtA + iB * rtB * rtB;

			vcp->tangentMass = kTangent > 0.0f ? 1.0f /  kTangent : 0.0f;

			// Setup a velocity bias for restitution.
			vcp->velocityBias = 0.0f;
            float32 vRel = P2DVecDot(vc->normal, vB + P2DVecCross(wB, vcp->rB) - vA - P2DVecCross(wA, vcp->rA));
            if (vRel < -P2D_VELOCITY_THRESHOLD)
			{
				vcp->velocityBias = -vc->restitution * vRel;
			}
		}

		// If we have two points, then prepare the block solver.
		if (vc->pointCount == 2 && g_blockSolve)
		{
            P2DVelocityConstraintPoint* vcp1 = vc->points + 0;
            P2DVelocityConstraintPoint* vcp2 = vc->points + 1;

            float32 rn1A = P2DVecCross(vcp1->rA, vc->normal);
            float32 rn1B = P2DVecCross(vcp1->rB, vc->normal);
            float32 rn2A = P2DVecCross(vcp2->rA, vc->normal);
            float32 rn2B = P2DVecCross(vcp2->rB, vc->normal);

			float32 k11 = mA + mB + iA * rn1A * rn1A + iB * rn1B * rn1B;
			float32 k22 = mA + mB + iA * rn2A * rn2A + iB * rn2B * rn2B;
			float32 k12 = mA + mB + iA * rn1A * rn2A + iB * rn1B * rn2B;

			// Ensure a reasonable condition number.
			const float32 k_maxConditionNumber = 1000.0f;
			if (k11 * k11 < k_maxConditionNumber * (k11 * k22 - k12 * k12))
			{
				// K is safe to invert.
				vc->K.ex.Set(k11, k12);
				vc->K.ey.Set(k12, k22);
				vc->normalMass = vc->K.GetInverse();
			}
			else
			{
				// The constraints are redundant, just use one.
				// TODO_ERIN use deepest?
				vc->pointCount = 1;
			}
		}
	}
}

void P2DContactSolver::WarmStart()
{
	// Warm start.
	for (int32 i = 0; i < m_count; ++i)
	{
        P2DContactVelocityConstraint* vc = m_velocityConstraints + i;

		int32 indexA = vc->indexA;
		int32 indexB = vc->indexB;
		float32 mA = vc->invMassA;
		float32 iA = vc->invIA;
		float32 mB = vc->invMassB;
		float32 iB = vc->invIB;
		int32 pointCount = vc->pointCount;

        P2DVec2 vA = m_velocities[indexA].v;
		float32 wA = m_velocities[indexA].w;
        P2DVec2 vB = m_velocities[indexB].v;
		float32 wB = m_velocities[indexB].w;

        P2DVec2 normal = vc->normal;
        P2DVec2 tangent = P2DVecCross(normal, 1.0f);

		for (int32 j = 0; j < pointCount; ++j)
		{
            P2DVelocityConstraintPoint* vcp = vc->points + j;
            P2DVec2 P = vcp->normalImpulse * normal + vcp->tangentImpulse * tangent;
            wA -= iA * P2DVecCross(vcp->rA, P);
			vA -= mA * P;
            wB += iB * P2DVecCross(vcp->rB, P);
			vB += mB * P;
		}

		m_velocities[indexA].v = vA;
		m_velocities[indexA].w = wA;
		m_velocities[indexB].v = vB;
		m_velocities[indexB].w = wB;
	}
}

void P2DContactSolver::SolveVelocityConstraints()
{
	for (int32 i = 0; i < m_count; ++i)
	{
        P2DContactVelocityConstraint* vc = m_velocityConstraints + i;

		int32 indexA = vc->indexA;
		int32 indexB = vc->indexB;
		float32 mA = vc->invMassA;
		float32 iA = vc->invIA;
		float32 mB = vc->invMassB;
		float32 iB = vc->invIB;
		int32 pointCount = vc->pointCount;

        P2DVec2 vA = m_velocities[indexA].v;
		float32 wA = m_velocities[indexA].w;
        P2DVec2 vB = m_velocities[indexB].v;
		float32 wB = m_velocities[indexB].w;

        P2DVec2 normal = vc->normal;
        P2DVec2 tangent = P2DVecCross(normal, 1.0f);
		float32 friction = vc->friction;

        assert(pointCount == 1 || pointCount == 2);

		// Solve tangent constraints first because non-penetration is more important
		// than friction.
		for (int32 j = 0; j < pointCount; ++j)
		{
            P2DVelocityConstraintPoint* vcp = vc->points + j;

			// Relative velocity at contact
            P2DVec2 dv = vB + P2DVecCross(wB, vcp->rB) - vA - P2DVecCross(wA, vcp->rA);

			// Compute tangent force
            float32 vt = P2DVecDot(dv, tangent) - vc->tangentSpeed;
			float32 lambda = vcp->tangentMass * (-vt);

            // P2DClamp the accumulated force
			float32 maxFriction = friction * vcp->normalImpulse;
            float32 newImpulse = P2DClamp(vcp->tangentImpulse + lambda, -maxFriction, maxFriction);
			lambda = newImpulse - vcp->tangentImpulse;
			vcp->tangentImpulse = newImpulse;

			// Apply contact impulse
            P2DVec2 P = lambda * tangent;

			vA -= mA * P;
            wA -= iA * P2DVecCross(vcp->rA, P);

			vB += mB * P;
            wB += iB * P2DVecCross(vcp->rB, P);
		}

		// Solve normal constraints
		if (pointCount == 1 || g_blockSolve == false)
		{
			for (int32 i = 0; i < pointCount; ++i)
			{
                P2DVelocityConstraintPoint* vcp = vc->points + i;

				// Relative velocity at contact
                P2DVec2 dv = vB + P2DVecCross(wB, vcp->rB) - vA - P2DVecCross(wA, vcp->rA);

				// Compute normal impulse
                float32 vn = P2DVecDot(dv, normal);
				float32 lambda = -vcp->normalMass * (vn - vcp->velocityBias);

                // P2DClamp the accumulated impulse
                float32 newImpulse = P2DMax(vcp->normalImpulse + lambda, 0.0f);
				lambda = newImpulse - vcp->normalImpulse;
				vcp->normalImpulse = newImpulse;

				// Apply contact impulse
                P2DVec2 P = lambda * normal;
				vA -= mA * P;
                wA -= iA * P2DVecCross(vcp->rA, P);

				vB += mB * P;
                wB += iB * P2DVecCross(vcp->rB, P);
			}
		}
		else
		{
            // Block solver developed in collaboration with Dirk Gregorius.
			// Build the mini LCP for this contact patch
			//
			// vn = A * x + b, vn >= 0, , vn >= 0, x >= 0 and vn_i * x_i = 0 with i = 1..2
			//
			// A = J * W * JT and J = ( -n, -r1 x n, n, r2 x n )
			// b = vn0 - velocityBias
			//
			// The system is solved using the "Total enumeration method" (s. Murty). The complementary constraint vn_i * x_i
			// implies that we must have in any solution either vn_i = 0 or x_i = 0. So for the 2D contact problem the cases
			// vn1 = 0 and vn2 = 0, x1 = 0 and x2 = 0, x1 = 0 and vn2 = 0, x2 = 0 and vn1 = 0 need to be tested. The first valid
			// solution that satisfies the problem is chosen.
			// 
			// In order to account of the accumulated impulse 'a' (because of the iterative nature of the solver which only requires
			// that the accumulated impulse is clamped and not the incremental impulse) we change the impulse variable (x_i).
			//
			// Substitute:
			// 
			// x = a + d
			// 
			// a := old total impulse
			// x := new total impulse
			// d := incremental impulse 
			//
			// For the current iteration we extend the formula for the incremental impulse
			// to compute the new total impulse:
			//
			// vn = A * d + b
			//    = A * (x - a) + b
			//    = A * x + b - A * a
			//    = A * x + b'
			// b' = b - A * a;

            P2DVelocityConstraintPoint* cp1 = vc->points + 0;
            P2DVelocityConstraintPoint* cp2 = vc->points + 1;

            P2DVec2 a(cp1->normalImpulse, cp2->normalImpulse);
            assert(a.x >= 0.0f && a.y >= 0.0f);

			// Relative velocity at contact
            P2DVec2 dv1 = vB + P2DVecCross(wB, cp1->rB) - vA - P2DVecCross(wA, cp1->rA);
            P2DVec2 dv2 = vB + P2DVecCross(wB, cp2->rB) - vA - P2DVecCross(wA, cp2->rA);

			// Compute normal velocity
            float32 vn1 = P2DVecDot(dv1, normal);
            float32 vn2 = P2DVecDot(dv2, normal);

            P2DVec2 b;
			b.x = vn1 - cp1->velocityBias;
			b.y = vn2 - cp2->velocityBias;

			// Compute b'
            b -= P2DMul(vc->K, a);

			const float32 k_errorTol = 1e-3f;
            NOT_USED(k_errorTol);

			for (;;)
			{
				//
				// Case 1: vn = 0
				//
				// 0 = A * x + b'
				//
				// Solve for x:
				//
				// x = - inv(A) * b'
				//
                P2DVec2 x = - P2DMul(vc->normalMass, b);

				if (x.x >= 0.0f && x.y >= 0.0f)
				{
					// Get the incremental impulse
                    P2DVec2 d = x - a;

					// Apply incremental impulse
                    P2DVec2 P1 = d.x * normal;
                    P2DVec2 P2 = d.y * normal;
					vA -= mA * (P1 + P2);
                    wA -= iA * (P2DVecCross(cp1->rA, P1) + P2DVecCross(cp2->rA, P2));

					vB += mB * (P1 + P2);
                    wB += iB * (P2DVecCross(cp1->rB, P1) + P2DVecCross(cp2->rB, P2));

					// Accumulate
					cp1->normalImpulse = x.x;
					cp2->normalImpulse = x.y;

#if DEBUG_SOLVER == 1
					// Postconditions
                    dv1 = vB + P2DVecCross(wB, cp1->rB) - vA - P2DVecCross(wA, cp1->rA);
                    dv2 = vB + P2DVecCross(wB, cp2->rB) - vA - P2DVecCross(wA, cp2->rA);

					// Compute normal velocity
                    vn1 = P2DVecDot(dv1, normal);
                    vn2 = P2DVecDot(dv2, normal);

                    assert(P2DAbs(vn1 - cp1->velocityBias) < k_errorTol);
                    assert(P2DAbs(vn2 - cp2->velocityBias) < k_errorTol);
#endif
					break;
				}

				//
				// Case 2: vn1 = 0 and x2 = 0
				//
				//   0 = a11 * x1 + a12 * 0 + b1' 
				// vn2 = a21 * x1 + a22 * 0 + b2'
				//
				x.x = - cp1->normalMass * b.x;
				x.y = 0.0f;
				vn1 = 0.0f;
				vn2 = vc->K.ex.y * x.x + b.y;

				if (x.x >= 0.0f && vn2 >= 0.0f)
				{
					// Get the incremental impulse
                    P2DVec2 d = x - a;

					// Apply incremental impulse
                    P2DVec2 P1 = d.x * normal;
                    P2DVec2 P2 = d.y * normal;
					vA -= mA * (P1 + P2);
                    wA -= iA * (P2DVecCross(cp1->rA, P1) + P2DVecCross(cp2->rA, P2));

					vB += mB * (P1 + P2);
                    wB += iB * (P2DVecCross(cp1->rB, P1) + P2DVecCross(cp2->rB, P2));

					// Accumulate
					cp1->normalImpulse = x.x;
					cp2->normalImpulse = x.y;

#if DEBUG_SOLVER == 1
					// Postconditions
                    dv1 = vB + P2DVecCross(wB, cp1->rB) - vA - P2DVecCross(wA, cp1->rA);

					// Compute normal velocity
                    vn1 = P2DVecDot(dv1, normal);

                    assert(P2DAbs(vn1 - cp1->velocityBias) < k_errorTol);
#endif
					break;
				}


				//
				// Case 3: vn2 = 0 and x1 = 0
				//
				// vn1 = a11 * 0 + a12 * x2 + b1' 
				//   0 = a21 * 0 + a22 * x2 + b2'
				//
				x.x = 0.0f;
				x.y = - cp2->normalMass * b.y;
				vn1 = vc->K.ey.x * x.y + b.x;
				vn2 = 0.0f;

				if (x.y >= 0.0f && vn1 >= 0.0f)
				{
					// Resubstitute for the incremental impulse
                    P2DVec2 d = x - a;

					// Apply incremental impulse
                    P2DVec2 P1 = d.x * normal;
                    P2DVec2 P2 = d.y * normal;
					vA -= mA * (P1 + P2);
                    wA -= iA * (P2DVecCross(cp1->rA, P1) + P2DVecCross(cp2->rA, P2));

					vB += mB * (P1 + P2);
                    wB += iB * (P2DVecCross(cp1->rB, P1) + P2DVecCross(cp2->rB, P2));

					// Accumulate
					cp1->normalImpulse = x.x;
					cp2->normalImpulse = x.y;

#if DEBUG_SOLVER == 1
					// Postconditions
                    dv2 = vB + P2DVecCross(wB, cp2->rB) - vA - P2DVecCross(wA, cp2->rA);

					// Compute normal velocity
                    vn2 = P2DVecDot(dv2, normal);

                    assert(P2DAbs(vn2 - cp2->velocityBias) < k_errorTol);
#endif
					break;
				}

				//
				// Case 4: x1 = 0 and x2 = 0
				// 
				// vn1 = b1
				// vn2 = b2;
				x.x = 0.0f;
				x.y = 0.0f;
				vn1 = b.x;
				vn2 = b.y;

				if (vn1 >= 0.0f && vn2 >= 0.0f )
				{
					// Resubstitute for the incremental impulse
                    P2DVec2 d = x - a;

					// Apply incremental impulse
                    P2DVec2 P1 = d.x * normal;
                    P2DVec2 P2 = d.y * normal;
					vA -= mA * (P1 + P2);
                    wA -= iA * (P2DVecCross(cp1->rA, P1) + P2DVecCross(cp2->rA, P2));

					vB += mB * (P1 + P2);
                    wB += iB * (P2DVecCross(cp1->rB, P1) + P2DVecCross(cp2->rB, P2));

					// Accumulate
					cp1->normalImpulse = x.x;
					cp2->normalImpulse = x.y;

					break;
				}

				// No solution, give up. This is hit sometimes, but it doesn't seem to matter.
				break;
			}
		}

		m_velocities[indexA].v = vA;
		m_velocities[indexA].w = wA;
		m_velocities[indexB].v = vB;
		m_velocities[indexB].w = wB;
	}
}

void P2DContactSolver::StoreImpulses()
{
	for (int32 i = 0; i < m_count; ++i)
	{
        P2DContactVelocityConstraint* vc = m_velocityConstraints + i;
        P2DManifold* manifold = m_contacts[vc->contactIndex]->GetManifold();

		for (int32 j = 0; j < vc->pointCount; ++j)
		{
			manifold->points[j].normalImpulse = vc->points[j].normalImpulse;
			manifold->points[j].tangentImpulse = vc->points[j].tangentImpulse;
		}
	}
}

struct P2DPositionSolverManifold
{
    void Initialize(P2DContactPositionConstraint* pc, const P2DTransform& xfA, const P2DTransform& xfB, int32 index)
	{
        assert(pc->pointCount > 0);

		switch (pc->type)
		{
        case P2DManifold::e_circles:
			{
                P2DVec2 pointA = P2DMul(xfA, pc->localPoint);
                P2DVec2 pointB = P2DMul(xfB, pc->localPoints[0]);
				normal = pointB - pointA;
				normal.Normalize();
				point = 0.5f * (pointA + pointB);
                separation = P2DVecDot(pointB - pointA, normal) - pc->radiusA - pc->radiusB;
			}
			break;

        case P2DManifold::e_faceA:
			{
                normal = P2DMul(xfA.rotation, pc->localNormal);
                P2DVec2 planePoint = P2DMul(xfA, pc->localPoint);

                P2DVec2 clipPoint = P2DMul(xfB, pc->localPoints[index]);
                separation = P2DVecDot(clipPoint - planePoint, normal) - pc->radiusA - pc->radiusB;
				point = clipPoint;
			}
			break;

        case P2DManifold::e_faceB:
			{
                normal = P2DMul(xfB.rotation, pc->localNormal);
                P2DVec2 planePoint = P2DMul(xfB, pc->localPoint);

                P2DVec2 clipPoint = P2DMul(xfA, pc->localPoints[index]);
                separation = P2DVecDot(clipPoint - planePoint, normal) - pc->radiusA - pc->radiusB;
				point = clipPoint;

				// Ensure normal points from A to B
				normal = -normal;
			}
			break;
		}
	}

    P2DVec2 normal;
    P2DVec2 point;
	float32 separation;
};

// Sequential solver.
bool P2DContactSolver::SolvePositionConstraints()
{
	float32 minSeparation = 0.0f;

	for (int32 i = 0; i < m_count; ++i)
	{
        P2DContactPositionConstraint* pc = m_positionConstraints + i;

		int32 indexA = pc->indexA;
		int32 indexB = pc->indexB;
        P2DVec2 localCenterA = pc->localCenterA;
		float32 mA = pc->invMassA;
		float32 iA = pc->invIA;
        P2DVec2 localCenterB = pc->localCenterB;
		float32 mB = pc->invMassB;
		float32 iB = pc->invIB;
		int32 pointCount = pc->pointCount;

        P2DVec2 cA = m_positions[indexA].c;
		float32 aA = m_positions[indexA].a;

        P2DVec2 cB = m_positions[indexB].c;
		float32 aB = m_positions[indexB].a;

		// Solve normal constraints
		for (int32 j = 0; j < pointCount; ++j)
		{
            P2DTransform xfA, xfB;
            xfA.rotation.Set(aA);
            xfB.rotation.Set(aB);
            xfA.position = cA - P2DMul(xfA.rotation, localCenterA);
            xfB.position = cB - P2DMul(xfB.rotation, localCenterB);

            P2DPositionSolverManifold psm;
			psm.Initialize(pc, xfA, xfB, j);
            P2DVec2 normal = psm.normal;

            P2DVec2 point = psm.point;
			float32 separation = psm.separation;

            P2DVec2 rA = point - cA;
            P2DVec2 rB = point - cB;

			// Track max constraint error.
            minSeparation = P2DMin(minSeparation, separation);

			// Prevent large corrections and allow slop.
            float32 C = P2DClamp(P2D_BAUMGARTE * (separation + P2D_LINEAR_SLOP), -P2D_MAX_LINEAR_CORRECTION, 0.0f);

			// Compute the effective mass.
            float32 rnA = P2DVecCross(rA, normal);
            float32 rnB = P2DVecCross(rB, normal);
			float32 K = mA + mB + iA * rnA * rnA + iB * rnB * rnB;

			// Compute normal impulse
			float32 impulse = K > 0.0f ? - C / K : 0.0f;

            P2DVec2 P = impulse * normal;

			cA -= mA * P;
            aA -= iA * P2DVecCross(rA, P);

			cB += mB * P;
            aB += iB * P2DVecCross(rB, P);
		}

		m_positions[indexA].c = cA;
		m_positions[indexA].a = aA;

		m_positions[indexB].c = cB;
		m_positions[indexB].a = aB;
	}

    // We can't expect minSpeparation >= -P2D_LINEAR_SLOP because we don't
    // push the separation above -P2D_LINEAR_SLOP.
    return minSeparation >= -3.0f * P2D_LINEAR_SLOP;
}

// Sequential position solver for position constraints.
bool P2DContactSolver::SolveTOIPositionConstraints(int32 toiIndexA, int32 toiIndexB)
{
	float32 minSeparation = 0.0f;

	for (int32 i = 0; i < m_count; ++i)
	{
        P2DContactPositionConstraint* pc = m_positionConstraints + i;

		int32 indexA = pc->indexA;
		int32 indexB = pc->indexB;
        P2DVec2 localCenterA = pc->localCenterA;
        P2DVec2 localCenterB = pc->localCenterB;
		int32 pointCount = pc->pointCount;

		float32 mA = 0.0f;
		float32 iA = 0.0f;
		if (indexA == toiIndexA || indexA == toiIndexB)
		{
			mA = pc->invMassA;
			iA = pc->invIA;
		}

		float32 mB = 0.0f;
		float32 iB = 0.;
		if (indexB == toiIndexA || indexB == toiIndexB)
		{
			mB = pc->invMassB;
			iB = pc->invIB;
		}

        P2DVec2 cA = m_positions[indexA].c;
		float32 aA = m_positions[indexA].a;

        P2DVec2 cB = m_positions[indexB].c;
		float32 aB = m_positions[indexB].a;

		// Solve normal constraints
		for (int32 j = 0; j < pointCount; ++j)
		{
            P2DTransform xfA, xfB;
            xfA.rotation.Set(aA);
            xfB.rotation.Set(aB);
            xfA.position = cA - P2DMul(xfA.rotation, localCenterA);
            xfB.position = cB - P2DMul(xfB.rotation, localCenterB);

            P2DPositionSolverManifold psm;
			psm.Initialize(pc, xfA, xfB, j);
            P2DVec2 normal = psm.normal;

            P2DVec2 point = psm.point;
			float32 separation = psm.separation;

            P2DVec2 rA = point - cA;
            P2DVec2 rB = point - cB;

			// Track max constraint error.
            minSeparation = P2DMin(minSeparation, separation);

			// Prevent large corrections and allow slop.
            float32 C = P2DClamp(P2D_TOI_BAUMGARTE * (separation + P2D_LINEAR_SLOP), -P2D_MAX_LINEAR_CORRECTION, 0.0f);

			// Compute the effective mass.
            float32 rnA = P2DVecCross(rA, normal);
            float32 rnB = P2DVecCross(rB, normal);
			float32 K = mA + mB + iA * rnA * rnA + iB * rnB * rnB;

			// Compute normal impulse
			float32 impulse = K > 0.0f ? - C / K : 0.0f;

            P2DVec2 P = impulse * normal;

			cA -= mA * P;
            aA -= iA * P2DVecCross(rA, P);

			cB += mB * P;
            aB += iB * P2DVecCross(rB, P);
		}

		m_positions[indexA].c = cA;
		m_positions[indexA].a = aA;

		m_positions[indexB].c = cB;
		m_positions[indexB].a = aB;
	}

    // We can't expect minSpeparation >= -P2D_LINEAR_SLOP because we don't
    // push the separation above -P2D_LINEAR_SLOP.
    return minSeparation >= -1.5f * P2D_LINEAR_SLOP;
}
