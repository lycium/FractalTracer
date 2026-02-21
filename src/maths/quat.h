#pragma once

#include "vec.h"
#include <cmath>

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

	// Quaternion from Euler angles
	static inline quat from_euler(const real_type & x_angle, const real_type & y_angle, const real_type & z_angle) noexcept
	{
		const real_type sx = sin(x_angle * real_type(0.5));
		const real_type cx = cos(x_angle * real_type(0.5));
		const real_type sy = sin(y_angle * real_type(0.5));
		const real_type cy = cos(y_angle * real_type(0.5));
		const real_type sz = sin(z_angle * real_type(0.5));
		const real_type cz = cos(z_angle * real_type(0.5));
		
		return quat(
			sx * cy * cz - cx * sy * sz,
			cx * sy * cz + sx * cy * sz,
			cx * cy * sz - sx * sy * cz,
			cx * cy * cz + sx * sy * sz
		);
	}

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

// Hamilton product
template <typename real_type>
inline quat<real_type> operator*(const quat<real_type> & a, const quat<real_type> & b)
{
	return quat<real_type>(
		a.x() * b.w() + a.w() * b.x() + a.y() * b.z() - a.z() * b.y(),
		a.y() * b.w() + a.w() * b.y() + a.z() * b.x() - a.x() * b.z(),
		a.z() * b.w() + a.w() * b.z() + a.x() * b.y() - a.y() * b.x(),
		a.w() * b.w() - a.x() * b.x() - a.y() * b.y() - a.z() * b.z()
	);
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
