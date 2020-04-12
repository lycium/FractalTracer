#pragma once

#include <cmath>



template<typename real>
class vec3
{
public:
	real x, y, z;

	inline vec3() { }
	inline vec3(const vec3 & v) : x(v.x), y(v.y), z(v.z) { }
	inline vec3(real v) : x(v), y(v), z(v) { }
	inline vec3(real x_, real y_, real z_) : x(x_), y(y_), z(z_) { }

	inline vec3 operator+(const vec3 & rhs) const { return vec3(x + rhs.x, y + rhs.y, z + rhs.z); }
	inline vec3 operator-(const vec3 & rhs) const { return vec3(x - rhs.x, y - rhs.y, z - rhs.z); }
	inline vec3 operator*(const real rhs) const { return vec3(x * rhs, y * rhs, z * rhs); }
	inline vec3 operator/(const real rhs) const { return vec3(x / rhs, y / rhs, z / rhs); }

	inline vec3 operator*(const vec3 & rhs) const { return vec3(x * rhs.x, y * rhs.y, z * rhs.z); }

	inline vec3 operator-() const { return vec3(-x, -y, -z); }

	inline const vec3 & operator =(const vec3 & rhs) { x = rhs.x; y = rhs.y; z = rhs.z; return *this; }

	inline const vec3 & operator+=(const vec3 & rhs) { x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
	inline const vec3 & operator-=(const vec3 & rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this; }
	inline const vec3 & operator*=(const real rhs) { x *= rhs; y *= rhs; z *= rhs; return *this; }
};

using vec3i = vec3<int>;
using vec3f = vec3<float>;
using vec3d = vec3<double>;


template<typename real>
inline real dot(const vec3<real> & lhs, const vec3<real> & rhs) { return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z; }


template<typename real>
inline real length(const vec3<real> & v) { return std::sqrt(dot(v, v)); }


template<typename real>
inline vec3<real> normalise(const vec3<real> & v, const real len = 1) { return v * (len / length(v)); }


template<typename real>
inline vec3<real> cross(const vec3<real> & a, const vec3<real> & b) { return vec3<real>(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x); }


inline vec3f toVec3f(const vec3d & v) { return vec3f((float)v.x, (float)v.y, (float)v.z); }
inline vec3d toVec3d(const vec3f & v) { return vec3d(v.x, v.y, v.z); }
