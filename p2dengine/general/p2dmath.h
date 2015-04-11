#ifndef P2D_MATH_H
#define P2D_MATH_H

#include "p2dparams.h"
#include <math.h>

/// This function is used to ensure that a floating point number is not a NaN or infinity.
inline bool P2DIsFloatValid(float32 x)
{
    int32 ix = *reinterpret_cast<int32*>(&x);
    return (ix & 0x7f800000) != 0x7f800000;
}

/*
/// This is a approximate yet fast inverse square-root.
inline float32 P2DInvSqrt(float32 x)
{
    union
    {
        float32 x;
        int32 i;
    } convert;

    convert.x = x;
    float32 xhalf = 0.5f * x;
    convert.i = 0x5f3759df - (convert.i >> 1);
    x = convert.x;
    x = x * (1.5f - xhalf * x * x);
    return x;
}
*/

//#define	P2DSqrt(x)	sqrtf(x)
//#define	P2DAtan2(y, x)	atan2f(y, x)

/// A 2D column vector.
struct P2DVec2
{
    /// Default constructor does nothing (for performance).
    P2DVec2() {}

    /// Construct using coordinates.
    P2DVec2(float32 x, float32 y) : x(x), y(y) {}

    /// Set this vector to all zeros.
    void SetZero() { x = 0.0f; y = 0.0f; }

    /// Set this vector to some specified coordinates.
    void Set(float32 x_, float32 y_) { x = x_; y = y_; }

    /// Negate this vector.
    P2DVec2 operator -() const { P2DVec2 v; v.Set(-x, -y); return v; }


    /// Read from and indexed element.
    float32 operator () (int32 i) const
    {
        return (&x)[i];
    }

    /// Write to an indexed element.
    float32& operator () (int32 i)
    {
        return (&x)[i];
    }


    /// Add a vector to this vector.
    void operator += (const P2DVec2& v)
    {
        x += v.x; y += v.y;
    }

    /// Subtract a vector from this vector.
    void operator -= (const P2DVec2& v)
    {
        x -= v.x; y -= v.y;
    }

    /// Multiply this vector by a scalar.
    void operator *= (float32 a)
    {
        x *= a; y *= a;
    }

    /// Get the length of this vector (the norm).
    float32 Length() const
    {
        return sqrtf(x * x + y * y);
    }

    /// Get the length squared. For performance, use this instead of
    /// P2DVec2::Length (if possible).
    float32 LengthSquared() const
    {
        return x * x + y * y;
    }

    /// Convert this vector into a unit vector. Returns the length.
    float32 Normalize()
    {
        float32 length = Length();
        if (length < FLT_EPSILON)
        {
            return 0.0f;
        }
        float32 invLength = 1.0f / length;
        x *= invLength;
        y *= invLength;

        return length;
    }

    /// Does this vector contain finite coordinates?
    bool IsValid() const
    {
        return P2DIsFloatValid(x) && P2DIsFloatValid(y);
    }

    /// Get the skew vector such that dot(skew_vec, other) == cross(vec, other)
    P2DVec2 Skew() const
    {
        return P2DVec2(-y, x);
    }

    float32 x, y;
};

/// A 2D column vector with 3 elements.
struct P2DVec3
{
    /// Default constructor does nothing (for performance).
    P2DVec3() {}

    /// Construct using coordinates.
    P2DVec3(float32 x, float32 y, float32 z) : x(x), y(y), z(z) {}

    /// Set this vector to all zeros.
    void SetZero() { x = 0.0f; y = 0.0f; z = 0.0f; }

    /// Set this vector to some specified coordinates.
    void Set(float32 x_, float32 y_, float32 z_) { x = x_; y = y_; z = z_; }

    /// Negate this vector.
    P2DVec3 operator -() const { P2DVec3 v; v.Set(-x, -y, -z); return v; }

    /// Add a vector to this vector.
    void operator += (const P2DVec3& v)
    {
        x += v.x; y += v.y; z += v.z;
    }

    /// Subtract a vector from this vector.
    void operator -= (const P2DVec3& v)
    {
        x -= v.x; y -= v.y; z -= v.z;
    }

    /// Multiply this vector by a scalar.
    void operator *= (float32 s)
    {
        x *= s; y *= s; z *= s;
    }

    float32 x, y, z;
};

/// A 2-by-2 matrix. Stored in column-major order.
struct P2DMat22
{
    /// The default constructor does nothing (for performance).
    P2DMat22() {}

    /// Construct this matrix using columns.
    P2DMat22(const P2DVec2& c1, const P2DVec2& c2)
    {
        ex = c1;
        ey = c2;
    }

    /// Construct this matrix using scalars.
    P2DMat22(float32 a11, float32 a12, float32 a21, float32 a22)
    {
        ex.x = a11; ex.y = a21;
        ey.x = a12; ey.y = a22;
    }

    /// Initialize this matrix using columns.
    void Set(const P2DVec2& c1, const P2DVec2& c2)
    {
        ex = c1;
        ey = c2;
    }

    /// Set this to the identity matrix.
    void SetIdentity()
    {
        ex.x = 1.0f; ey.x = 0.0f;
        ex.y = 0.0f; ey.y = 1.0f;
    }

    /// Set this matrix to all zeros.
    void SetZero()
    {
        ex.x = 0.0f; ey.x = 0.0f;
        ex.y = 0.0f; ey.y = 0.0f;
    }

    P2DMat22 GetInverse() const
    {
        float32 a = ex.x, b = ey.x, c = ex.y, d = ey.y;
        P2DMat22 inv;
        float32 det = a * d - b * c;
        if (det != 0.0f){
            det = 1.0f / det;
        }
        inv.ex.x =  det * d;	inv.ey.x = -det * b;
        inv.ex.y = -det * c;	inv.ey.y =  det * a;
        return inv;
    }

    /// Solve A * x = b, where b is a column vector. This is more efficient
    /// than computing the inverse in one-shot cases.
    P2DVec2 Solve(const P2DVec2& b) const
    {
        float32 a11 = ex.x, a12 = ey.x, a21 = ex.y, a22 = ey.y;
        float32 det = a11 * a22 - a12 * a21;
        if (det != 0.0f){
            det = 1.0f / det;
        }
        P2DVec2 x;
        x.x = det * (a22 * b.x - a12 * b.y);
        x.y = det * (a11 * b.y - a21 * b.x);
        return x;
    }

    P2DVec2 ex, ey;
};

/// A 3-by-3 matrix. Stored in column-major order.
struct P2DMat33
{
    /// The default constructor does nothing (for performance).
    P2DMat33() {}

    /// Construct this matrix using columns.
    P2DMat33(const P2DVec3& c1, const P2DVec3& c2, const P2DVec3& c3)
    {
        ex = c1;
        ey = c2;
        ez = c3;
    }

    /// Set this matrix to all zeros.
    void SetZero()
    {
        ex.SetZero();
        ey.SetZero();
        ez.SetZero();
    }

    /// Solve A * x = b, where b is a column vector. This is more efficient
    /// than computing the inverse in one-shot cases.
    P2DVec3 Solve33(const P2DVec3& b) const;

    /// Solve A * x = b, where b is a column vector. This is more efficient
    /// than computing the inverse in one-shot cases. Solve only the upper
    /// 2-by-2 matrix equation.
    P2DVec2 Solve22(const P2DVec2& b) const;

    /// Get the inverse of this matrix as a 2-by-2.
    /// Returns the zero matrix if singular.
    void GetInverse22(P2DMat33* M) const;

    /// Get the symmetric inverse of this matrix as a 3-by-3.
    /// Returns the zero matrix if singular.
    void GetSymInverse33(P2DMat33* M) const;

    P2DVec3 ex, ey, ez;
};

/// Rotation
struct P2DRot
{
    P2DRot() {}

    /// Initialize from an angle in radians
    explicit P2DRot(float32 angle)
    {
        /// TODO_ERIN optimize
        s = sinf(angle);
        c = cosf(angle);
    }

    /// Set using an angle in radians.
    void Set(float32 angle)
    {
        /// TODO_ERIN optimize
        s = sinf(angle);
        c = cosf(angle);
    }

    /// Set to the identity rotation
    void SetIdentity()
    {
        s = 0.0f;
        c = 1.0f;
    }

    /// Get the angle in radians
    float32 GetAngle() const
    {
        return atan2(s, c);
    }

    /// Get the x-axis
    P2DVec2 GetXAxis() const
    {
        return P2DVec2(c, s);
    }

    /// Get the u-axis
    P2DVec2 GetYAxis() const
    {
        return P2DVec2(-s, c);
    }

    /// Sine and cosine
    float32 s, c;
};

/// A transform contains translation and rotation. It is used to represent
/// the position and orientation of rigid frames.
struct P2DTransform
{
    /// The default constructor does nothing.
    P2DTransform() {}

    /// Initialize using a position vector and a rotation.
    P2DTransform(const P2DVec2& position, const P2DRot& rotation) : position(position), rotation(rotation) {}

    /// Set this to the identity transform.
    void SetIdentity()
    {
        position.SetZero();
        rotation.SetIdentity();
    }

    /// Set this based on the position and angle.
    void Set(const P2DVec2& pos, float32 angle)
    {
        position = pos;
        rotation.Set(angle);
    }

    P2DVec2 position;
    P2DRot rotation;
};


/// This describes the motion of a body/shape for TOI computation.
/// Shapes are defined with respect to the body origin, which may
/// no coincide with the center of mass. However, to support dynamics
/// we must interpolate the center of mass position.
struct P2DSweep
{
    /// Get the interpolated transform at a specific time.
    /// @param beta is a factor in [0,1], where 0 indicates alpha0.
    void GetTransform(P2DTransform* xfb, float32 beta) const;

    /// Advance the sweep forward, yielding a new initial state.
    /// @param alpha the new initial time.
    void Advance(float32 alpha);

    /// Normalize the angles.
    void Normalize();

    P2DVec2 localCenter;	///< local center of mass position
    P2DVec2 c0, c;		///< center world positions
    float32 a0, a;		///< world angles

    /// Fraction of the current time step in the range [0,1]
    /// c0 and a0 are the positions at alpha0.
    float32 alpha0;
};



/// Useful constant
extern const P2DVec2 P2DVec2_0;

/// Perform the dot product on two vectors.
inline float32 P2DVecDot(const P2DVec2& a, const P2DVec2& b)
{
    return a.x * b.x + a.y * b.y;
}

/// Perform the cross product on two vectors. In 2D this produces a scalar.
inline float32 P2DVecCross(const P2DVec2& a, const P2DVec2& b)
{
    return a.x * b.y - a.y * b.x;
}

/// Perform the cross product on a vector and a scalar. In 2D this produces
/// a vector.
inline P2DVec2 P2DVecCross(const P2DVec2& a, float32 s)
{
    return P2DVec2(s * a.y, -s * a.x);
}

/// Perform the cross product on a scalar and a vector. In 2D this produces
/// a vector.
inline P2DVec2 P2DVecCross(float32 s, const P2DVec2& a)
{
    return P2DVec2(-s * a.y, s * a.x);
}

/// Multiply a matrix times a vector. If a rotation matrix is provided,
/// then this transforms the vector from one frame to another.
inline P2DVec2 P2DMul(const P2DMat22& A, const P2DVec2& v)
{
    return P2DVec2(A.ex.x * v.x + A.ey.x * v.y, A.ex.y * v.x + A.ey.y * v.y);
}

/// Multiply a matrix transpose times a vector. If a rotation matrix is provided,
/// then this transforms the vector from one frame to another (inverse transform).
inline P2DVec2 P2DMulT(const P2DMat22& A, const P2DVec2& v)
{
    return P2DVec2(P2DVecDot(v, A.ex), P2DVecDot(v, A.ey));
}

/// Add two vectors component-wise.
inline P2DVec2 operator + (const P2DVec2& a, const P2DVec2& b)
{
    return P2DVec2(a.x + b.x, a.y + b.y);
}

/// Subtract two vectors component-wise.
inline P2DVec2 operator - (const P2DVec2& a, const P2DVec2& b)
{
    return P2DVec2(a.x - b.x, a.y - b.y);
}

inline P2DVec2 operator * (float32 s, const P2DVec2& a)
{
    return P2DVec2(s * a.x, s * a.y);
}

inline bool operator == (const P2DVec2& a, const P2DVec2& b)
{
    return a.x == b.x && a.y == b.y;
}

inline float32 P2DDistance(const P2DVec2& a, const P2DVec2& b)
{
    P2DVec2 c = a - b;
    return c.Length();
}

inline float32 P2DDistanceSquared(const P2DVec2& a, const P2DVec2& b)
{
    P2DVec2 c = a - b;
    return P2DVecDot(c, c);
}

inline P2DVec3 operator * (float32 s, const P2DVec3& a)
{
    return P2DVec3(s * a.x, s * a.y, s * a.z);
}

/// Add two vectors component-wise.
inline P2DVec3 operator + (const P2DVec3& a, const P2DVec3& b)
{
    return P2DVec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

/// Subtract two vectors component-wise.
inline P2DVec3 operator - (const P2DVec3& a, const P2DVec3& b)
{
    return P2DVec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

/// Perform the dot product on two vectors.
inline float32 P2DVecDot(const P2DVec3& a, const P2DVec3& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

/// Perform the cross product on two vectors.
inline P2DVec3 P2DVecCross(const P2DVec3& a, const P2DVec3& b)
{
    return P2DVec3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

inline P2DMat22 operator + (const P2DMat22& A, const P2DMat22& B)
{
    return P2DMat22(A.ex + B.ex, A.ey + B.ey);
}

// A * B
inline P2DMat22 P2DMul(const P2DMat22& A, const P2DMat22& B)
{
    return P2DMat22(P2DMul(A, B.ex), P2DMul(A, B.ey));
}

// A^T * B
inline P2DMat22 P2DMulT(const P2DMat22& A, const P2DMat22& B)
{
    P2DVec2 c1(P2DVecDot(A.ex, B.ex), P2DVecDot(A.ey, B.ex));
    P2DVec2 c2(P2DVecDot(A.ex, B.ey), P2DVecDot(A.ey, B.ey));
    return P2DMat22(c1, c2);
}

/// Multiply a matrix times a vector.
inline P2DVec3 P2DMul(const P2DMat33& A, const P2DVec3& v)
{
    return v.x * A.ex + v.y * A.ey + v.z * A.ez;
}

/// Multiply a matrix times a vector.
inline P2DVec2 P2DMul22(const P2DMat33& A, const P2DVec2& v)
{
    return P2DVec2(A.ex.x * v.x + A.ey.x * v.y, A.ex.y * v.x + A.ey.y * v.y);
}

/// Multiply two rotations: q * r
inline P2DRot P2DMul(const P2DRot& q, const P2DRot& r)
{
    // [qc -qs] * [rc -rs] = [qc*rc-qs*rs -qc*rs-qs*rc]
    // [qs  qc]   [rs  rc]   [qs*rc+qc*rs -qs*rs+qc*rc]
    // s = qs * rc + qc * rs
    // c = qc * rc - qs * rs
    P2DRot qr;
    qr.s = q.s * r.c + q.c * r.s;
    qr.c = q.c * r.c - q.s * r.s;
    return qr;
}

/// Transpose multiply two rotations: qT * r
inline P2DRot P2DMulT(const P2DRot& q, const P2DRot& r)
{
    // [ qc qs] * [rc -rs] = [qc*rc+qs*rs -qc*rs+qs*rc]
    // [-qs qc]   [rs  rc]   [-qs*rc+qc*rs qs*rs+qc*rc]
    // s = qc * rs - qs * rc
    // c = qc * rc + qs * rs
    P2DRot qr;
    qr.s = q.c * r.s - q.s * r.c;
    qr.c = q.c * r.c + q.s * r.s;
    return qr;
}

/// Rotate a vector
inline P2DVec2 P2DMul(const P2DRot& q, const P2DVec2& v)
{
    return P2DVec2(q.c * v.x - q.s * v.y, q.s * v.x + q.c * v.y);
}

/// Inverse rotate a vector
inline P2DVec2 P2DMulT(const P2DRot& q, const P2DVec2& v)
{
    return P2DVec2(q.c * v.x + q.s * v.y, -q.s * v.x + q.c * v.y);
}

inline P2DVec2 P2DMul(const P2DTransform& T, const P2DVec2& v)
{
    float32 x = (T.rotation.c * v.x - T.rotation.s * v.y) + T.position.x;
    float32 y = (T.rotation.s * v.x + T.rotation.c * v.y) + T.position.y;

    return P2DVec2(x, y);
}

inline P2DVec2 P2DMulT(const P2DTransform& T, const P2DVec2& v)
{
    float32 px = v.x - T.position.x;
    float32 py = v.y - T.position.y;
    float32 x = (T.rotation.c * px + T.rotation.s * py);
    float32 y = (-T.rotation.s * px + T.rotation.c * py);

    return P2DVec2(x, y);
}

// v2 = A.q.Rot(B.q.Rot(v1) + B.p) + A.p
//    = (A.q * B.q).Rot(v1) + A.q.Rot(B.p) + A.p
inline P2DTransform P2DMul(const P2DTransform& A, const P2DTransform& B)
{
    P2DTransform C;
    C.rotation = P2DMul(A.rotation, B.rotation);
    C.position = P2DMul(A.rotation, B.position) + A.position;
    return C;
}

// v2 = A.q' * (B.q * v1 + B.p - A.p)
//    = A.q' * B.q * v1 + A.q' * (B.p - A.p)
inline P2DTransform P2DMulT(const P2DTransform& A, const P2DTransform& B)
{
    P2DTransform C;
    C.rotation = P2DMulT(A.rotation, B.rotation);
    C.position = P2DMulT(A.rotation, B.position - A.position);
    return C;
}




template <typename T>
inline T P2DAbs(T a)
{
    return a > T(0) ? a : -a;
}

inline P2DVec2 P2DAbs(const P2DVec2& a)
{
    return P2DVec2(P2DAbs(a.x), P2DAbs(a.y));
}

inline P2DMat22 P2DAbs(const P2DMat22& A)
{
    return P2DMat22(P2DAbs(A.ex), P2DAbs(A.ey));
}


template <typename T>
inline T P2DMin(T a, T b)
{
    return a < b ? a : b;
}

inline P2DVec2 P2DMin(const P2DVec2& a, const P2DVec2& b)
{
    return P2DVec2(P2DMin(a.x, b.x), P2DMin(a.y, b.y));
}

template <typename T>
inline T P2DMax(T a, T b)
{
    return a > b ? a : b;
}

inline P2DVec2 P2DMax(const P2DVec2& a, const P2DVec2& b)
{
    return P2DVec2(P2DMax(a.x, b.x), P2DMax(a.y, b.y));
}

template <typename T>
inline T P2DClamp(T a, T low, T high)
{
    return P2DMax(low, P2DMin(a, high));
}

inline P2DVec2 P2DClamp(const P2DVec2& a, const P2DVec2& low, const P2DVec2& high)
{
    return P2DMax(low, P2DMin(a, high));
}

template<typename T> inline void P2DSwap(T& a, T& b)
{
    T tmp = a;
    a = b;
    b = tmp;
}

/// "Next Largest Power of 2
/// Given a binary integer value x, the next largest power of 2 can be computed by a SWAR algorithm
/// that recursively "folds" the upper bits into the lower bits. This process yields a bit vector with
/// the same most significant 1 as x, but all 1's below it. Adding 1 to that value yields the next
/// largest power of 2. For a 32-bit value:"
inline uint32 P2DNextLargestPowerOfTwo(uint32 x)
{
    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    x |= (x >> 8);
    x |= (x >> 16);
    return x + 1;
}

inline bool P2DIsPowerOfTwo(uint32 x)
{
    bool result = x > 0 && (x & (x - 1)) == 0;
    return result;
}

inline void P2DSweep::GetTransform(P2DTransform *xf, float32 beta) const
{
    xf->position = (1.0f - beta) * c0 + beta * c;
    float32 angle = (1.0f - beta) * a0 + beta * a;
    xf->rotation.Set(angle);

    // Shift to origin
    xf->position -= P2DMul(xf->rotation, localCenter);
}

inline void P2DSweep::Advance(float32 alpha)
{
    assert(alpha0 < 1.0f);
    float32 beta = (alpha - alpha0) / (1.0f - alpha0);
    c0 += beta * (c - c0);
    a0 += beta * (a - a0);
    alpha0 = alpha;
}

/// Normalize an angle in radians to be between -pi and pi
inline void P2DSweep::Normalize()
{
    float32 twoPi = 2.0f * PI;
    float32 d =  twoPi * floorf(a0 / twoPi);
    a0 -= d;
    a -= d;
}

#endif
