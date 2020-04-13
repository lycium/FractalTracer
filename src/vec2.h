#pragma once

#include <cmath>

#include "real.h"
#include "Dual.h"



template<typename real>
class vec2
{
public:
	real x, y;

	inline vec2() { }
	inline vec2(const vec2 & v) : x(v.x), y(v.y) { }
	inline vec2(real v) : x(v), y(v) { }
	inline vec2(real x_, real y_) : x(x_), y(y_) { }

	inline vec2 operator+(const vec2 & rhs) const { return vec2(x + rhs.x, y + rhs.y); }
	inline vec2 operator-(const vec2 & rhs) const { return vec2(x - rhs.x, y - rhs.y); }
	inline vec2 operator*(const real rhs) const { return vec2(x * rhs, y * rhs); }
	inline vec2 operator/(const real rhs) const { return vec2(x / rhs, y / rhs); }

	inline vec2 operator*(const vec2 & rhs) const { return vec2(x * rhs.x, y * rhs.y); }

	inline vec2 operator-() const { return vec2(-x, -y); }

	inline const vec2 & operator =(const vec2 & rhs) { x = rhs.x; y = rhs.y; return *this; }

	inline const vec2 & operator+=(const vec2 & rhs) { x += rhs.x; y += rhs.y; return *this; }
	inline const vec2 & operator-=(const vec2 & rhs) { x -= rhs.x; y -= rhs.y; return *this; }
	inline const vec2 & operator*=(const real rhs) { x *= rhs; y *= rhs; return *this; }
};

using vec2i = vec2<int>;
using vec2r = vec2<real>;
using vec2f = vec2<float>;
using vec2d = vec2<double>;

using DualVec2r = vec2<real>;
using DualVec2f = vec2<Dual2f>;
using DualVec2d = vec2<Dual2d>;


template<typename real>
inline real dot(const vec2<real> & lhs, const vec2<real> & rhs) { return lhs.x * rhs.x + lhs.y * rhs.y; }


template<typename real>
inline real length(const vec2<real> & v) { return length(v); }


template<typename real>
inline vec2<real> normalise(const vec2<real> & v, const real len = 1) { return v * (len / length(v)); }


inline static vec2f toVec2f(const vec2d & v) { return vec2f((float)v.x, (float)v.y); }
inline static vec2d toVec2d(const vec2f & v) { return vec2d(v.x, v.y); }
