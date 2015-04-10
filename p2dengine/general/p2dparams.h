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

/// The maximum number of contact points between two convex shapes.
#define P2D_MAX_CONTACT_POINTS 2

/// The maximum number of vertices on a convex polygon.
#define P2D_MAX_POLYGON_VERTICES 8



typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

typedef signed char	int8;
typedef signed short int16;
typedef signed int int32;

typedef float float32;
typedef double float64;

#endif
