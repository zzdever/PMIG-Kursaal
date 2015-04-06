/// @file
/// Tuning constants. Metrics are m, kg and s.
///

#ifndef P2D_PARAMS_H
#define P2D_PARAMS_H

#include <stddef.h>
#include <assert.h>
#include <float.h>

#define NOT_USED(x) ((void)(x))

#define PI			3.14159265359f

// Collision

/// The maximum number of contact points between two convex shapes.
#define P2D_MAX_CONTACT_POINTS 2

/// The maximum number of vertices on a convex polygon.
#define P2D_MAX_POLYGON_VERTICES 8

/// This is used to fatten AABBs in the dynamic tree. This allows proxies
/// to move by a small amount without triggering a tree adjustment.
/// This is in meters.
#define P2D_AABB_RADIUS 0.1f

/// This is used to fatten AABBs in the dynamic tree. This is used to predict
/// the future position based on the current displacement.
/// This is a dimensionless multiplier.
#define P2D_AABB_MULTIPLIER 2.0f

/// A small length used as a collision and constraint tolerance. Usually it is
/// chosen to be numerically significant, but visually insignificant.
#define P2D_LINEAR_SLOP 0.005f

/// A small angle used as a collision and constraint tolerance. Usually it is
/// chosen to be numerically significant, but visually insignificant.
#define P2D_ANGULAR_SLOP (2.0f / 180.0f * PI)

/// The radius of the polygon/edge shape skin. This should not be modified. Making
/// this smaller means polygons will have an insufficient buffer for continuous collision.
/// Making it larger may create artifacts for vertex collision.
#define P2D_POLYGON_RADIUS (2.0f * P2D_LINEAR_SLOP)

/// Maximum number of sub-steps per contact in continuous physics simulation.
#define P2D_MAX_SUB_STEPS 8


// Dynamics

/// Maximum number of contacts to be handled to solve a TOI impact.
#define P2D_MAX_TOI_CONTACTS 32

/// A velocity threshold for elastic collisions. Any collision with a relative linear
/// velocity below this threshold will be treated as inelastic.
#define P2D_VELOCITY_THRESHOLD 1.0f

/// The maximum linear position correction used when solving constraints. This helps to
/// prevent overshoot.
#define P2D_MAX_LINEAR_CORRECTION 0.2f

/// The maximum angular position correction used when solving constraints. This helps to
/// prevent overshoot.
#define P2D_MAX_ANGULAR_CORRECTION (8.0f / 180.0f * PI)

/// The maximum linear velocity of a body. This limit is very large and is used
/// to prevent numerical problems. You shouldn't need to adjust this.
#define P2D_MAX_TRANSLATION 2.0f
#define P2D_MAX_TRANSLATION_SQUARED	(P2D_MAX_TRANSLATION * P2D_MAX_TRANSLATION)

/// The maximum angular velocity of a body. This limit is very large and is used
/// to prevent numerical problems. You shouldn't need to adjust this.
#define P2D_MAX_ROTATION (0.5f * PI)
#define P2D_MAX_ROTATION_SQUARED (P2D_MAX_ROTATION * P2D_MAX_ROTATION)

/// This scale factor controls how fast overlap is resolved. Ideally this would be 1 so
/// that overlap is removed in one time step. However using values close to 1 often lead
/// to overshoot.
//#define P2D_baumgarte 0.2f
//#define P2D_toiBaugarte 0.75f


// Sleep

/// The time that a body must be still before it will go to sleep.
#define P2D_TIME_TO_SLEEP 0.5f

/// A body cannot sleep if its linear velocity is above this tolerance.
#define P2D_LINEAR_SLEEP_TOLERANCE 0.01f

/// A body cannot sleep if its angular velocity is above this tolerance.
#define P2D_ANGULAR_SLEEP_TOLERANCE (2.0f / 180.0f * PI)


typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

typedef signed char	int8;
typedef signed short int16;
typedef signed int int32;

typedef float float32;
typedef double float64;

#endif
