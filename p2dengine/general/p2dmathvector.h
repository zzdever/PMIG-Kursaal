#ifndef P2D_MATH_VECTOR_H
#define P2D_MATH_VECTOR_H

/// This function is used to ensure that a floating point number is not a NaN or infinity.
inline bool P2DIsFloatValid(float32 x)
{
    int32 ix = *reinterpret_cast<int32*>(&x);
    return (ix & 0x7f800000) != 0x7f800000;
}


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
	
    /*
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
    */

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





#endif
