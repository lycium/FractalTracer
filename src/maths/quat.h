#pragma once

#include "vec.h"

// quaternion

template <typename real_type>
struct quat
{
	vec<4, real_type> v;


	inline quat() { }
	inline quat(const quat & t) : v(t.v) { }
	inline quat(const vec<4, real_type> & v) : v(v) { }
	inline quat(const real_type & x, const real_type & y, const real_type & z, const real_type & w) : v(x, y, z, w) { }
	inline quat(const real_type & x, const real_type & y, const real_type & z) : v(x, y, z, real_type(0)) { }
	inline quat(const real_type & x, const real_type & y) : v(x, y, real_type(0)) { }
	inline quat(const real_type & x) : v(x, real_type(0), real_type(0), real_type(0) ) { }

	inline explicit operator const vec<4, real_type> & () const { return v; }

	inline const quat & operator=(const quat & t) { v = t.v; return *this; }

	inline const real_type & x() const { return v.x(); }
	inline const real_type & y() const { return v.y(); }
	inline const real_type & z() const { return v.z(); }
	inline const real_type & w() const { return v.w(); }

	inline real_type & x() { return v.x(); }
	inline real_type & y() { return v.y(); }
	inline real_type & z() { return v.z(); }
	inline real_type & w() { return v.w(); }
};


template <typename real_type>
inline quat<real_type> operator+(const quat<real_type> & a, const quat<real_type> & b)
{
	return a.v + b.v;
}


template <typename real_type>
inline quat<real_type> operator-(const quat<real_type> & a, const quat<real_type> & b)
{
	return a.v - b.v;
}


template <typename real_type>
inline quat<real_type> operator-(const quat<real_type> & b)
{
	return -b.v;
}


template <typename real_type>
inline quat<real_type> operator*(const quat<real_type> & a, const real_type & b)
{
	return a.v * b;
}


template <typename real_type>
inline quat<real_type> operator *(const real_type & a, const quat<real_type> & b)
{
	return b.v * a;
}


template <typename real_type>
inline quat<real_type> operator/(const quat<real_type> & a, const real_type & b)
{
	return a.v / b;
}


template <typename real_type>
inline auto length2(const quat<real_type> & a)
{
	return length2(a.v);
}

template <typename real_type>
inline quat<real_type> fabs(const quat<real_type> & a)
{
	return fabs(a.v);
}

template <typename real_type>
inline quat<real_type> sqr(const quat<real_type> & a)
{
	const real_type x2(sqr(a.x()));
	const real_type y2(sqr(a.y()));
	const real_type z2(sqr(a.z()));
	const real_type w2(sqr(a.w()));
	return
	{
		x2 - y2 - z2 - w2,
		real_type(2) * a.x() * a.y(),
		real_type(2) * a.x() * a.z(),
		real_type(2) * a.x() * a.w()
	};
}


using quatr = quat<real>;
using quatf = quat<float>;
using quatd = quat<double>;

using DualQuat4r = quat<Dual4r>;
using DualQuat4f = quat<Dual4f>;
using DualQuat4d = quat<Dual4d>;
