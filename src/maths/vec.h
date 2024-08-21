#pragma once

#include <cmath>

#include "real.h"
#include "Dual.h"



template<int n, typename real_type>
struct vec
{
	real_type e[n];


	constexpr vec() noexcept { }
	constexpr vec(const vec & v) noexcept { for (int i = 0; i < n; ++i) e[i] = v.e[i]; }
	constexpr vec(const real_type & v) noexcept { for (int i = 0; i < n; ++i) e[i] = v; }

	// Some brutal C++ hackery to enable initializer lists
	template<typename val, typename... vals, std::enable_if_t<(sizeof...(vals) > 0), int> = 0>
	constexpr vec(const val v, const vals... vs) noexcept : e { (real_type)v, (real_type)vs... } { }

	constexpr vec operator+(const vec & rhs) const noexcept { vec r; for (int i = 0; i < n; ++i) r.e[i] = e[i] + rhs.e[i]; return r; }
	constexpr vec operator-(const vec & rhs) const noexcept { vec r; for (int i = 0; i < n; ++i) r.e[i] = e[i] - rhs.e[i]; return r; }
	constexpr vec operator*(const real_type & rhs) const noexcept { vec r; for (int i = 0; i < n; ++i) r.e[i] = e[i] * rhs; return r; }
	constexpr vec operator/(const real_type & rhs) const noexcept { return *this * (real_type(1) / rhs); }

	constexpr vec operator*(const vec & rhs) const noexcept { vec r; for (int i = 0; i < n; ++i) r.e[i] = e[i] * rhs.e[i]; return r; }
	constexpr vec operator/(const vec & rhs) const noexcept { vec r; for (int i = 0; i < n; ++i) r.e[i] = e[i] / rhs.e[i]; return r; }

	constexpr vec operator-() const noexcept { vec r; for (int i = 0; i < n; ++i) r.e[i] = -e[i]; return r; }

	constexpr const vec & operator =(const vec & rhs) noexcept { for (int i = 0; i < n; ++i) e[i]  = rhs.e[i]; return *this; }
	constexpr const vec & operator+=(const vec & rhs) noexcept { for (int i = 0; i < n; ++i) e[i] += rhs.e[i]; return *this; }
	constexpr const vec & operator-=(const vec & rhs) noexcept { for (int i = 0; i < n; ++i) e[i] -= rhs.e[i]; return *this; }
	constexpr const vec & operator*=(const vec & rhs) noexcept { for (int i = 0; i < n; ++i) e[i] *= rhs.e[i]; return *this; }
	constexpr const vec & operator/=(const vec & rhs) noexcept { for (int i = 0; i < n; ++i) e[i] /= rhs.e[i]; return *this; }

	constexpr const vec & operator =(const real_type & rhs) noexcept { for (int i = 0; i < n; ++i) e[i]  = rhs; return *this; }
	constexpr const vec & operator+=(const real_type & rhs) noexcept { for (int i = 0; i < n; ++i) e[i] += rhs; return *this; }
	constexpr const vec & operator-=(const real_type & rhs) noexcept { for (int i = 0; i < n; ++i) e[i] -= rhs; return *this; }
	constexpr const vec & operator*=(const real_type & rhs) noexcept { for (int i = 0; i < n; ++i) e[i] *= rhs; return *this; }
	constexpr const vec & operator/=(const real_type & rhs) noexcept
	{
		const real_type s = real_type(1) / rhs;
		for (int i = 0; i < n; ++i)
			e[i] *= s;
		return *this;
	}

	// xyzw accessors enabled only if dimensions are present
	// NOTE: could be made more robust by some hacks to prevent an explicit overload with m != n
	// See: <https://stackoverflow.com/questions/13401716/selecting-a-member-function-using-different-enable-if-conditions/13401982#13401982>
	template <int m = n> typename std::enable_if<(m > 0), real_type &>::type inline x() { return e[0]; }
	template <int m = n> typename std::enable_if<(m > 1), real_type &>::type inline y() { return e[1]; }
	template <int m = n> typename std::enable_if<(m > 2), real_type &>::type inline z() { return e[2]; }
	template <int m = n> typename std::enable_if<(m > 3), real_type &>::type inline w() { return e[3]; }
	template <int m = n> typename std::enable_if<(m > 0), const real_type &>::type inline x() const { return e[0]; }
	template <int m = n> typename std::enable_if<(m > 1), const real_type &>::type inline y() const { return e[1]; }
	template <int m = n> typename std::enable_if<(m > 2), const real_type &>::type inline z() const { return e[2]; }
	template <int m = n> typename std::enable_if<(m > 3), const real_type &>::type inline w() const { return e[3]; }
};


template<int n, typename real_type>
constexpr real_type dot(const vec<n, real_type> & lhs, const vec<n, real_type> & rhs) noexcept
{
	real_type d = 0;
	for (int i = 0; i < n; ++i)
		d += lhs.e[i] * rhs.e[i];
	return d;
}


// Optimised method for Dual dot product with real-vector RHS
template<int n, typename real_type>
constexpr real dot(const vec<n, Dual<real_type, n>> & lhs, const vec<n, real> & rhs) noexcept
{
	real d = 0;
	for (int i = 0; i < n; ++i)
		d += lhs.e[i].v[0] * rhs.e[i];
	return d;
}


template<int n, typename real_type>
inline real_type length2(const vec<n, real_type> & v) noexcept
{
	real_type d = 0;
	for (int i = 0; i < n; ++i)
		d += v.e[i] * v.e[i];
	return d;
}


template<int n, typename real_type>
constexpr real_type length2(const vec<n, Dual<real_type, 3>> & v) noexcept
{
	real_type d = 0;
	for (int i = 0; i < n; ++i)
		d += v.e[i].v[0] * v.e[i].v[0];
	return d;
}


template<int n, typename real_type>
constexpr real_type length(const vec<n, real_type> & v) noexcept { return std::sqrt(length2(v)); }


template<int n, typename real_type>
constexpr real_type length(const vec<n, Dual<real_type, 3>> & v) noexcept { return std::sqrt(length2(v)); }


template<int n, typename real_type>
constexpr vec<n, real_type> normalise(const vec<n, real_type> & v, const real_type len = 1) noexcept { return v * (len / length(v)); }


template<typename real_type>
constexpr vec<3, real_type> cross(const vec<3, real_type> & a, const vec<3, real_type> & b) noexcept
{
	return vec<3, real_type>(
		a.y() * b.z() - a.z() * b.y(),
		a.z() * b.x() - a.x() * b.z(),
		a.x() * b.y() - a.y() * b.x());
}


using vec2i = vec<2, int>;
using vec2r = vec<2, real>;
using vec2f = vec<2, float>;
using vec2d = vec<2, double>;

using DualVec2r = vec<2, Dual3r>;
using DualVec2f = vec<2, Dual3f>;
using DualVec2d = vec<2, Dual3d>;


using vec3i = vec<3, int>;
using vec3r = vec<3, real>;
using vec3f = vec<3, float>;
using vec3d = vec<3, double>;

using DualVec3r = vec<3, Dual3r>;
using DualVec3f = vec<3, Dual3f>;
using DualVec3d = vec<3, Dual3d>;

using vec4i = vec<4, int>;
using vec4r = vec<4, real>;
using vec4f = vec<4, float>;
using vec4d = vec<4, double>;
