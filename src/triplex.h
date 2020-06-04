#pragma once

#include "vec.h"


// Lifted vector operations
// Ref: http://www.fractalforums.com/theory/non-trigonometric-expansions-for-cosine-formula/
template <typename real_type>
struct triplex
{
	vec<3, real_type> v;


	inline triplex() { }
	inline triplex(const triplex & t) : v(t.v) { }
	inline triplex(const vec<3, real_type> & v) : v(v) { }
	inline triplex(const real_type & x, const real_type & y, const real_type & z) : v(x, y, z) { }
	inline triplex(const real_type & x, const real_type & y) : v(x, y, real_type(0)) { }
	inline triplex(const real_type & x) : v(x, real_type(0), real_type(0)) { }

	inline explicit operator const vec<3, real_type> & () const { return v; }

	inline const triplex & operator=(const triplex & t) { v = t.v; return *this; }

	inline const real_type & x() const { return v.x(); }
	inline const real_type & y() const { return v.y(); }
	inline const real_type & z() const { return v.z(); }
};


template <typename real_type>
inline triplex<real_type> operator+(const triplex<real_type> & a, const triplex<real_type> & b)
{
	return a.v + b.v;
}


template <typename real_type>
inline triplex<real_type> operator-(const triplex<real_type> & a, const triplex<real_type> & b)
{
	return a.v - b.v;
}


template <typename real_type>
inline triplex<real_type> operator-(const triplex<real_type> & b)
{
	return -b.v;
}


template <typename real_type>
inline triplex<real_type> operator*(const triplex<real_type> & a, const real_type & b)
{
	return a.v * b;
}


template <typename real_type>
inline triplex<real_type> operator *(const real_type & a, const triplex<real_type> & b)
{
	return b.v * a;
}


template <typename real_type>
inline triplex<real_type> operator/(const triplex<real_type> & a, const real_type & b)
{
	return a.v / b;
}


template <typename real_type>
inline triplex<real_type> length2(const triplex<real_type> & a)
{
	return length2(a.v);
}


// Triplex operations:

template <typename real_type>
inline triplex<real_type> operator*(const triplex<real_type> & a, const triplex<real_type> & b)
{
	const real_type arho(sqrt(sqr(a.x()) + sqr(a.y())));
	const real_type brho(sqrt(sqr(b.x()) + sqr(b.y())));
	const real_type A(real_type(1) - a.z() * b.z() / (arho * brho));

	return
	{
		A * (a.x() * b.x() - a.y() * b.y()),
		A * (a.x() * b.y() + a.y() * b.x()),
		arho * b.z() + brho * a.z()
	};
}


template <typename real_type>
inline triplex<real_type> operator/(const triplex<real_type> & a, const triplex<real_type> & b)
{
	const real_type arho(sqrt(sqr(a.x()) + sqr(a.y())));
	const real_type brho(sqrt(sqr(b.x()) + sqr(b.y())));
	const real_type A(real_type(1) + a.z() * b.z() / (arho * brho));

	return triplex<real_type>
	{
		A * (a.x() * b.x() + a.y() * b.y()),
		A * (a.y() * b.x() - a.x() * b.y()),
		brho * a.z() - arho * b.z()
	} / length2(b);
}


template <typename real_type>
inline triplex<real_type> sqr(const triplex<real_type> & a)
{
	const real_type x2(sqr(a.x()));
	const real_type y2(sqr(a.y()));
	const real_type arho2 = x2 + y2;
	const real_type A = real_type(1) - sqr(a.z()) / arho2;

	return
	{
		A * (x2 - y2),
		A * 2 * a.x() * a.y(),
		sqrt(arho2) * a.z() * 2
	};
}


using triplexr = triplex<real>;
using triplexf = triplex<float>;
using triplexd = triplex<double>;

using DualTriplex3r = triplex<Dual3r>;
using DualTriplex3f = triplex<Dual3f>;
using DualTriplex3d = triplex<Dual3d>;
