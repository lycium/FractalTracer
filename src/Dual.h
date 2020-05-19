#pragma once

#include "real.h"



// TODO: Add specialised functions for +-*/ with scalar (to avoid promotion to Dual)
// TODO: Add more functions (e.g. asin, acos, atan)
// Note that this is slightly suboptimal for single variable derivatives
template <typename real_type, int vars>
class Dual final
{
public:
	real_type v[vars + 1];


	inline constexpr Dual() noexcept { }

	// Constant constructor
	inline constexpr Dual(const real_type s) noexcept
	{
		v[0] = s;
		for (int i = 0; i < vars; ++i)
			v[i + 1] = 0;
	}

	// Constructor with derivative for var_idx'th variable
	inline constexpr Dual(const real_type s, const int var_idx) noexcept
	{
		v[0] = s;
		for (int i = 0; i < vars; ++i)
			v[i + 1] = (i == var_idx) ? static_cast<real_type>(1) : 0;
	}

	// Copy constructor
	inline constexpr Dual(const Dual &) noexcept = default;

	inline constexpr const Dual & operator=(const Dual & rhs) noexcept { for (int i = 0; i < vars + 1; ++i) v[i] = rhs.v[i]; return *this; }

	inline constexpr Dual operator-() const noexcept { Dual r; for (int i = 0; i < vars + 1; ++i) r.v[i] = -v[i]; return r; }

	inline constexpr Dual operator+(const Dual & rhs) const noexcept { Dual r; for (int i = 0; i < vars + 1; ++i) r.v[i] = v[i] + rhs.v[i]; return r; }
	inline constexpr Dual operator-(const Dual & rhs) const noexcept { Dual r; for (int i = 0; i < vars + 1; ++i) r.v[i] = v[i] - rhs.v[i]; return r; }

	// Optimised method to avoid promiting RHS to Dual
	inline constexpr Dual operator+(const real_type rhs) const noexcept
	{
		Dual r;
		r.v[0] = v[0] + rhs;
		for (int i = 0; i < vars; ++i)
			r.v[i + 1] = v[i + 1];
		return r;
	}

	inline constexpr const Dual & operator+=(const real_type rhs) noexcept { *this = *this + rhs; return *this; }

	// Optimised method to avoid promiting RHS to Dual
	inline constexpr Dual operator-(const real_type rhs) const noexcept
	{
		Dual r;
		r.v[0] = v[0] - rhs;
		for (int i = 0; i < vars; ++i)
			r.v[i + 1] = v[i + 1];
		return r;
	}

	inline constexpr const Dual & operator-=(const real_type rhs) noexcept { *this = *this - rhs; return *this; }

	// Optimised method to avoid full product rule from promiting RHS to Dual
	inline constexpr Dual operator*(const real_type rhs) const noexcept
	{
		Dual r;
		for (int i = 0; i < vars + 1; ++i)
			r.v[i] = v[i] * rhs;
		return r;
	}

	inline constexpr const Dual & operator*=(const real_type rhs) noexcept { *this = *this * rhs; return *this; }

	// Optimised method to avoid full quotient rule from promiting RHS to Dual
	inline constexpr Dual operator/(const real_type rhs) const noexcept
	{
		const real_type inv = 1 / rhs;
		Dual r;
		for (int i = 0; i < vars + 1; ++i)
			r.v[i] = v[i] * inv;
		return r;
	}

	inline constexpr const Dual & operator/=(const real_type rhs) noexcept { *this = *this * (1 / rhs); return *this; }

	inline constexpr Dual operator*(const Dual & rhs) const noexcept
	{
		Dual r;
		r.v[0] = v[0] * rhs.v[0];
		for (int i = 0; i < vars; ++i)
			r.v[i + 1] = v[0] * rhs.v[i + 1] + v[i + 1] * rhs.v[0]; // Product rule
		return r;
	}

	inline constexpr const Dual & operator*=(const Dual & rhs) noexcept { *this = *this * rhs; return *this; }

	inline constexpr Dual operator/(const Dual & rhs) const noexcept
	{
		const real_type inv_v0 = 1 / rhs.v[0];
		const real_type scale = inv_v0 * inv_v0;
		Dual r;
		r.v[0] = v[0] * inv_v0;
		for (int i = 0; i < vars; ++i)
			r.v[i + 1] = (v[i + 1] * rhs.v[0] - v[0] * rhs.v[i + 1]) * scale; // Quotient rule
		return r;
	}
};

using Dual2r = Dual<real, 2>;
using Dual2f = Dual<float, 2>;
using Dual2d = Dual<double, 2>;

using Dual3r = Dual<real, 3>;
using Dual3f = Dual<float, 3>;
using Dual3d = Dual<double, 3>;


template <typename real_type, int vars>
inline constexpr Dual<real_type, vars> pow(const Dual<real_type, vars> & d, const real_type e) noexcept
{
	const real_type scale = std::pow(d.v[0], e - 1) * e;

	Dual<real_type, vars> r;
	r.v[0] = std::pow(d.v[0], e);
	for (int i = 0; i < vars; ++i)
		r.v[i + 1] = d.v[i + 1] * scale;
	return r;
}


// Optimised version of pow for square root
template <typename real_type, int vars>
inline constexpr Dual<real_type, vars> sqrt(const Dual<real_type, vars> & d) noexcept
{
	const real_type sqrt_v0 = std::sqrt(d.v[0]);
	const real_type scale = static_cast<real_type>(0.5) / sqrt_v0;

	Dual<real_type, vars> r;
	r.v[0] = sqrt_v0;
	for (int i = 0; i < vars; ++i)
		r.v[i + 1] = d.v[i + 1] * scale;
	return r;
}


template <typename real_type, int vars>
inline constexpr Dual<real_type, vars> sin(const Dual<real_type, vars> & d) noexcept
{
	const real_type scale = std::cos(d.v[0]);

	Dual<real_type, vars> r;
	r.v[0] = std::sin(d.v[0]);
	for (int i = 0; i < vars; ++i)
		r.v[i + 1] = d.v[i + 1] * scale;
	return r;
}


template <typename real_type, int vars>
inline constexpr Dual<real_type, vars> cos(const Dual<real_type, vars> & d) noexcept
{
	const real_type scale = -std::sin(d.v[0]);

	Dual<real_type, vars> r;
	r.v[0] = std::cos(d.v[0]);
	for (int i = 0; i < vars; ++i)
		r.v[i + 1] = d.v[i + 1] * scale;
	return r;
}


template <typename real_type, int vars>
inline constexpr Dual<real_type, vars> tan(const Dual<real_type, vars> & d) noexcept
{
	const real_type cos_v0 = std::cos(d.v[0]);
	const real_type scale = 1 / (cos_v0 * cos_v0);

	Dual<real_type, vars> r;
	r.v[0] = std::tan(d.v[0]);
	for (int i = 0; i < vars; ++i)
		r.v[i + 1] = d.v[i + 1] * scale;
	return r;
}


template <typename real_type, int vars>
inline constexpr Dual<real_type, vars> fabs(const Dual<real_type, vars> & d) noexcept
{
	return (d.v[0] < 0) ? -d : d;
}


template <typename real_type, int vars>
inline constexpr Dual<real_type, vars> min(const Dual<real_type, vars> & p, const real_type min_val) noexcept
{
	return (p.v[0] < min_val) ? min_val : p; // Note: zero derivs left of min_val
}


template <typename real_type, int vars>
inline constexpr Dual<real_type, vars> max(const Dual<real_type, vars> & p, const real_type max_val) noexcept
{
	return (p.v[0] > max_val) ? max_val : p; // Note: zero derivs right of max_val
}


template <typename real_type, int vars>
inline constexpr Dual<real_type, vars> clamp(const Dual<real_type, vars> & p, const real_type min_val, const real_type max_val) noexcept
{
	return min(max(p, max_val), min_val);
}
