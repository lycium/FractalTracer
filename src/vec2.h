#pragma once

#include <cmath>

#include "real.h"
#include "Dual.h"



template<typename real_type>
class vec2
{
public:
	real_type x, y;

	inline vec2() { }
	inline vec2(const vec2 & v) : x(v.x), y(v.y) { }
	inline vec2(real_type v) : x(v), y(v) { }
	inline vec2(real_type x_, real_type y_) : x(x_), y(y_) { }

	inline vec2 operator+(const vec2 & rhs) const { return vec2(x + rhs.x, y + rhs.y); }
	inline vec2 operator-(const vec2 & rhs) const { return vec2(x - rhs.x, y - rhs.y); }
	inline vec2 operator*(const real_type rhs) const { return vec2(x * rhs, y * rhs); }
	inline vec2 operator/(const real_type rhs) const { return vec2(x / rhs, y / rhs); }

	inline vec2 operator*(const vec2 & rhs) const { return vec2(x * rhs.x, y * rhs.y); }

	inline vec2 operator-() const { return vec2(-x, -y); }

	inline const vec2 & operator =(const vec2 & rhs) { x = rhs.x; y = rhs.y; return *this; }

	inline const vec2 & operator+=(const vec2 & rhs) { x += rhs.x; y += rhs.y; return *this; }
	inline const vec2 & operator-=(const vec2 & rhs) { x -= rhs.x; y -= rhs.y; return *this; }
	inline const vec2 & operator*=(const real_type rhs) { x *= rhs; y *= rhs; return *this; }
};

using vec2i = vec2<int>;
using vec2r = vec2<real>;
using vec2f = vec2<float>;
using vec2d = vec2<double>;

using DualVec2r = vec2<Dual2r>;
using DualVec2f = vec2<Dual2f>;
using DualVec2d = vec2<Dual2d>;


template<typename real_type>
inline real_type dot(const vec2<real_type> & lhs, const vec2<real_type> & rhs) { return lhs.x * rhs.x + lhs.y * rhs.y; }


template<typename real_type>
inline real_type length2(const vec2<real_type> & v) { return dot(v, v); }


template<typename real_type>
inline real_type length(const vec2<real_type> & v) { return std::sqrt(length(v)); }


template<typename real_type>
inline vec2<real_type> normalise(const vec2<real_type> & v, const real_type len = 1) { return v * (len / length(v)); }


inline vec2f toVec2f(const vec2d & v) { return vec2f((float)v.x, (float)v.y); }
inline vec2d toVec2d(const vec2f & v) { return vec2d(v.x, v.y); }


// Overrides for squared length and length of dual vectors, since the dual part does nothing
inline real length2(const DualVec2r & v) { return v.x.v[0] * v.x.v[0] + v.y.v[0] * v.y.v[0]; }
inline real length (const DualVec2r & v) { return std::sqrt(length2(v)); }

inline Dual2r dot(const DualVec2r & lhs, const vec2r & rhs) { return lhs.x * rhs.x + lhs.y * rhs.y; }
