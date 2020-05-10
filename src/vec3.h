#pragma once

#include <cmath>

#include "real.h"
#include "Dual.h"



template<typename real_type>
class vec3
{
public:
	real_type x, y, z;

	inline vec3() { }
	inline vec3(const vec3 & v) : x(v.x), y(v.y), z(v.z) { }
	inline vec3(real_type v) : x(v), y(v), z(v) { }
	inline vec3(real_type x_, real_type y_, real_type z_) : x(x_), y(y_), z(z_) { }

	inline vec3 operator+(const vec3 & rhs) const { return vec3(x + rhs.x, y + rhs.y, z + rhs.z); }
	inline vec3 operator-(const vec3 & rhs) const { return vec3(x - rhs.x, y - rhs.y, z - rhs.z); }
	inline vec3 operator*(const real_type rhs) const { return vec3(x * rhs, y * rhs, z * rhs); }
	inline vec3 operator/(const real_type rhs) const { return vec3(x / rhs, y / rhs, z / rhs); }

	inline vec3 operator*(const vec3 & rhs) const { return vec3(x * rhs.x, y * rhs.y, z * rhs.z); }

	inline vec3 operator-() const { return vec3(-x, -y, -z); }

	inline const vec3 & operator =(const vec3 & rhs) { x = rhs.x; y = rhs.y; z = rhs.z; return *this; }

	inline const vec3 & operator+=(const vec3 & rhs) { x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
	inline const vec3 & operator-=(const vec3 & rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this; }
	inline const vec3 & operator*=(const vec3 & rhs) { x *= rhs.x; y *= rhs.y; z *= rhs.z; return *this; }
	inline const vec3 & operator*=(const real_type rhs) { x *= rhs; y *= rhs; z *= rhs; return *this; }
};

using vec3i = vec3<int>;
using vec3r = vec3<real>;
using vec3f = vec3<float>;
using vec3d = vec3<double>;

using DualVec3r = vec3<Dual3r>;
using DualVec3f = vec3<Dual3f>;
using DualVec3d = vec3<Dual3d>;


template<typename real_type>
inline real_type dot(const vec3<real_type> & lhs, const vec3<real_type> & rhs) { return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z; }


template<typename real_type>
inline real_type length2(const vec3<real_type> & v) { return dot(v, v); }


template<typename real_type>
inline real_type length(const vec3<real_type> & v) { return std::sqrt(length2(v)); }


template<typename real_type>
inline vec3<real_type> normalise(const vec3<real_type> & v, const real_type len = 1) { return v * (len / length(v)); }


template<typename real_type>
inline vec3<real_type> cross(const vec3<real_type> & a, const vec3<real_type> & b) { return vec3<real_type>(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x); }


inline vec3f toVec3f(const vec3d & v) { return vec3f((float)v.x, (float)v.y, (float)v.z); }
inline vec3d toVec3d(const vec3f & v) { return vec3d(v.x, v.y, v.z); }


// Overrides for squared length and length of dual vectors, since the dual part does nothing
inline real length2(const DualVec3r & v) { return v.x.v[0] * v.x.v[0] + v.y.v[0] * v.y.v[0] + v.z.v[0] * v.z.v[0]; }
inline real length (const DualVec3r & v) { return std::sqrt(length2(v)); }

inline Dual3r dot(const DualVec3r & lhs, const vec3r & rhs) { return lhs.x * (Dual3r)rhs.x + lhs.y * (Dual3r)rhs.y + lhs.z * (Dual3r)rhs.z; }
