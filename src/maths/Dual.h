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

	inline constexpr const Dual & operator+=(const Dual &    rhs) noexcept { *this = *this + rhs; return *this; }
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

	inline constexpr const Dual & operator-=(const Dual &    rhs) noexcept { *this = *this - rhs; return *this; }
	inline constexpr const Dual & operator-=(const real_type rhs) noexcept { *this = *this - rhs; return *this; }

	// Optimised method to avoid full product rule from promiting RHS to Dual
	inline constexpr Dual operator*(const real_type rhs) const noexcept
	{
		Dual r;
		for (int i = 0; i < vars + 1; ++i)
			r.v[i] = v[i] * rhs;
		return r;
	}

	inline constexpr const Dual & operator*=(const Dual &    rhs) noexcept { *this = *this * rhs; return *this; }
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

	inline constexpr const Dual & operator/=(const Dual &    rhs) noexcept { *this = *this / rhs; return *this; }
	inline constexpr const Dual & operator/=(const real_type rhs) noexcept { *this = *this * (1 / rhs); return *this; }

	inline constexpr Dual operator*(const Dual & rhs) const noexcept
	{
		Dual r;
		r.v[0] = v[0] * rhs.v[0];
		for (int i = 0; i < vars; ++i)
			r.v[i + 1] = v[0] * rhs.v[i + 1] + v[i + 1] * rhs.v[0]; // Product rule
		return r;
	}

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
	auto [si, co] = sincos(d.v[0]);

	Dual<real_type, vars> r;
	r.v[0] = si;
	for (int i = 0; i < vars; ++i)
		r.v[i + 1] = d.v[i + 1] * co;
	return r;
}


template <typename real_type, int vars>
inline constexpr Dual<real_type, vars> cos(const Dual<real_type, vars> & d) noexcept
{
	auto [si, co] = sincos(d.v[0]);

	Dual<real_type, vars> r;
	r.v[0] = co;
	si = -si;
	for (int i = 0; i < vars; ++i)
		r.v[i + 1] = d.v[i + 1] * si;
	return r;
}


template <typename real_type, int vars>
inline constexpr std::pair<Dual<real_type, vars>, Dual<real_type, vars>> sincos(const Dual<real_type, vars> & d) noexcept
{
	auto [si, co] = sincos(d.v[0]);

	Dual<real_type, vars> s, c;
	s.v[0] = si;
	c.v[0] = co;
	si = -si;
	for (int i = 0; i < vars; ++i)
	{
		s.v[i + 1] = d.v[i + 1] * co;
		c.v[i + 1] = d.v[i + 1] * si;
	}
	return { s, c };
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
inline constexpr Dual<real_type, vars> clamp(const Dual<real_type, vars> & p, const Dual<real_type, vars> & min_val, const Dual<real_type, vars> & max_val) noexcept
{
	return min(max(p, max_val), min_val);
}


template <typename real_type, int vars>
inline constexpr Dual<real_type, vars> clamp(const Dual<real_type, vars> & p, const real_type min_val, const real_type max_val) noexcept
{
	return min(max(p, max_val), min_val);
}


template <typename real_type, int vars>
inline constexpr Dual<real_type, vars> min(const Dual<real_type, vars> & p, const Dual<real_type, vars> & min_val) noexcept
{
	return (p.v[0] < min_val.v[0]) ? p : min_val;
}


template <typename real_type, int vars>
inline constexpr Dual<real_type, vars> min(const Dual<real_type, vars> & p, const real_type & min_val) noexcept
{
	return (p.v[0] < min_val) ? p : min_val; // Note: zero derivs left of min_val
}


template <typename real_type, int vars>
inline constexpr Dual<real_type, vars> max(const Dual<real_type, vars> & p, const Dual<real_type, vars> & max_val) noexcept
{
	return (p.v[0] > max_val.v[0]) ? p : max_val;
}


template <typename real_type, int vars>
inline constexpr Dual<real_type, vars> max(const Dual<real_type, vars> & p, const real_type & max_val) noexcept
{
	return (p.v[0] > max_val) ? p : max_val; // Note: zero derivs right of max_val
}


template <typename real_type, int vars>
inline constexpr Dual<real_type, vars> floor(const Dual<real_type, vars> & p) noexcept
{
	return floor(p.v[0]); // Note: zero derivatives
}


template <typename real_type, int vars>
inline constexpr Dual<real_type, vars> ceil(const Dual<real_type, vars> & p) noexcept
{
	return ceil(p.v[0]); // Note: zero derivatives
}


template <typename real_type, int vars>
inline constexpr Dual<real_type, vars> fmod(const Dual<real_type, vars> & p, const real_type & modulo) noexcept
{
	return p - floor(p.v[0] / modulo) * modulo;
}


template <typename real_type, int vars>
inline constexpr Dual<real_type, vars> sqr(const Dual<real_type, vars> & p)
{
	Dual<real_type, vars> r;
	r.v[0] = p.v[0] * p.v[0];
	for (int i = 0; i < vars; ++i)
		r.v[i + 1] = 2 * p.v[0] * p.v[i + 1]; // Product rule
	return r;
}


template <typename real_type, int vars>
inline constexpr Dual<real_type, vars> atan2(const Dual<real_type, vars> & y, const Dual<real_type, vars> & x)
{
	Dual<real_type, vars> r;
	if (y.v[0] == 0 && x.v[0] == 0)
	{
		return 0; // Arbitrary
	}
	r.v[0] = std::atan2(y.v[0], x.v[0]);
	real_type s = 1 / (x.v[0] * x.v[0] + y.v[0] * y.v[0]);
	for (int i = 0; i < vars; ++i)
		r.v[i + 1] = (x.v[0] * y.v[i + 1] - y.v[0] * x.v[i + 1]) * s;
	return r;
}


template <typename real_type, int vars>
inline constexpr Dual<real_type, vars> exp(const Dual<real_type, vars> & x)
{
	Dual<real_type, vars> r;
	r.v[0] = std::exp(x.v[0]);
	for (int i = 0; i < vars; ++i)
		r.v[i + 1] = r.v[0] * x.v[i + 1];
	return r;
}


template <typename real_type, int vars>
inline constexpr Dual<real_type, vars> log(const Dual<real_type, vars> & x)
{
	Dual<real_type, vars> r;
	r.v[0] = std::log(x.v[0]);
	for (int i = 0; i < vars; ++i)
		r.v[i + 1] = x.v[i + 1] / x.v[0];
	return r;
}


template <typename real_type, int vars>
inline constexpr Dual<real_type, vars> sinh(const Dual<real_type, vars> & z)
{
	return (exp(z) - exp(-z)) / 2; // FIXME catastrophic cancellation near z = 0
}


template <typename real_type, int vars>
inline constexpr Dual<real_type, vars> cosh(const Dual<real_type, vars> & z)
{
	return (exp(z) + exp(-z)) / 2;
}


template <typename real_type, int vars>
inline constexpr Dual<real_type, vars> tanh(const Dual<real_type, vars> & z)
{
  return sinh(z) / cosh(z);
}


template <typename real_type, int vars>
inline constexpr Dual<real_type, vars> asin(const Dual<real_type, vars> & z)
{
  Dual<real_type, vars> r;
  r.v[0] = std::asin(z.v[0]);
  real_type s = 1 / std::sqrt(1 - z.v[0] * z.v[0]);
	for (int i = 0; i < vars; ++i)
		r.v[i + 1] = z.v[i + 1] * s;
	return r;
}


template <typename real_type, int vars>
inline constexpr Dual<real_type, vars> acos(const Dual<real_type, vars> & z)
{
  Dual<real_type, vars> r;
  r.v[0] = std::acos(z.v[0]);
  real_type s = -1 / std::sqrt(1 - z.v[0] * z.v[0]);
	for (int i = 0; i < vars; ++i)
		r.v[i + 1] = z.v[i + 1] * s;
	return r;
}


template <typename real_type, int vars>
inline constexpr Dual<real_type, vars> atan(const Dual<real_type, vars> z)
{
  Dual<real_type, vars> r;
  r.v[0] = std::atan(z.v[0]);
  real_type s = 1 / (1 + z.v[0] * z.v[0]);
	for (int i = 0; i < vars; ++i)
		r.v[i + 1] = z.v[i + 1] * s;
	return r;
}


template <typename real_type, int vars>
inline constexpr Dual<real_type, vars> asinh(const Dual<real_type, vars> & z)
{
  Dual<real_type, vars> r;
  r.v[0] = std::asinh(z.v[0]);
  real_type s = 1 / std::sqrt(z.v[0] * z.v[0] + 1);
	for (int i = 0; i < vars; ++i)
		r.v[i + 1] = z.v[i + 1] * s;
	return r;
}


template <typename real_type, int vars>
inline constexpr Dual<real_type, vars> acosh(const Dual<real_type, vars> & z)
{
  Dual<real_type, vars> r;
  r.v[0] = std::acosh(z.v[0]);
  real_type s = 1 / (std::sqrt(z.v[0] - 1) * std::sqrt(z.v[0] + 1));
	for (int i = 0; i < vars; ++i)
		r.v[i + 1] = z.v[i + 1] * s;
	return r;
}


template <typename real_type, int vars>
inline constexpr Dual<real_type, vars> atanh(const Dual<real_type, vars> & z)
{
  Dual<real_type, vars> r;
  r.v[0] = std::atanh(z.v[0]);
  real_type s = 1 / (1 - z.v[0] * z.v[0]);
	for (int i = 0; i < vars; ++i)
		r.v[i + 1] = z.v[i + 1] * s;
	return r;
}
