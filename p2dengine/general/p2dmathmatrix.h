#ifndef P2D_MATH_MATRIX_H
#define P2D_MATH_MATRIX_H

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

/// Solve A * x = b, where b is a column vector. This is more efficient
/// than computing the inverse in one-shot cases.
P2DVec3 P2DMat33::Solve33(const P2DVec3& b) const
{
    float32 det = P2DVecDot(ex, P2DVecCross(ey, ez));
	if (det != 0.0f){
		det = 1.0f / det;
	}
    P2DVec3 x;
    x.x = det * P2DVecDot(b, P2DVecCross(ey, ez));
    x.y = det * P2DVecDot(ex, P2DVecCross(b, ez));
    x.z = det * P2DVecDot(ex, P2DVecCross(ey, b));
	return x;
}

/// Solve A * x = b, where b is a column vector. This is more efficient
/// than computing the inverse in one-shot cases.
P2DVec2 P2DMat33::Solve22(const P2DVec2& b) const
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

///
void P2DMat33::GetInverse22(P2DMat33* M) const
{
	float32 a = ex.x, b = ey.x, c = ex.y, d = ey.y;
	float32 det = a * d - b * c;
	if (det != 0.0f){
		det = 1.0f / det;
	}

	M->ex.x =  det * d;	M->ey.x = -det * b; M->ex.z = 0.0f;
	M->ex.y = -det * c;	M->ey.y =  det * a; M->ey.z = 0.0f;
	M->ez.x = 0.0f; M->ez.y = 0.0f; M->ez.z = 0.0f;
}

/// Returns the zero matrix if singular.
void P2DMat33::GetSymInverse33(P2DMat33* M) const
{
    float32 det = P2DVecDot(ex, P2DVecCross(ey, ez));
	if (det != 0.0f){
		det = 1.0f / det;
	}

	float32 a11 = ex.x, a12 = ey.x, a13 = ez.x;
	float32 a22 = ey.y, a23 = ez.y;
	float32 a33 = ez.z;

	M->ex.x = det * (a22 * a33 - a23 * a23);
	M->ex.y = det * (a13 * a23 - a12 * a33);
	M->ex.z = det * (a12 * a23 - a13 * a22);

	M->ey.x = M->ex.y;
	M->ey.y = det * (a11 * a33 - a13 * a13);
	M->ey.z = det * (a13 * a12 - a11 * a23);

	M->ez.x = M->ex.z;
	M->ez.y = M->ey.z;
	M->ez.z = det * (a11 * a22 - a12 * a12);
}


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
