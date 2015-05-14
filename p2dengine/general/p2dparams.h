/// @file
/// Tuning constants.
/// Metrics are m, kg and s.

#ifndef P2D_PARAMS_H
#define P2D_PARAMS_H

#include <stddef.h>
#include <float.h>
#include <assert.h>


#define NOT_USED(x) ((void)(x))

#define PI 3.14159265359f

// Collision

/// The maximum number of contact points between two convex shapes. Do
/// not change this value.
#define P2D_MAX_MANIFOLD_POINTS 2

/// The maximum number of contact points between two convex shapes.
#define P2D_MAX_CONTACT_POINTS 2

/// The maximum number of vertices on a convex polygon.
#define P2D_MAX_POLYGON_VERTICES 256

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

/// This is used to fatten AABBs in the dynamic b-tree. This allows proxies
/// to move by a small amount without triggering a tree adjustment.
#define P2D_AABB_EXTENSION 0.1f

/// This is used to fatten AABBs in the dynamic b-tree. This is used to predict
/// the future position based on the current displacement.
/// This is a dimensionless multiplier.
#define P2D_AABB_MULTIPLIER 2.0f


typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

typedef signed char	int8;
typedef signed short int16;
typedef signed int int32;

typedef float float32;
typedef double float64;

#endif
