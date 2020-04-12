#pragma once

// TODO: Add specialised functions for +-*/ with scalar (to avoid promotion to Dual)
// Note that this is slightly suboptimal for single variable derivatives



template <typename real, int vars>
class Dual final
{
public:
	real v[vars + 1];

	inline constexpr Dual() noexcept { }

	// Constant constructor
	inline constexpr Dual(const real s) noexcept
	{
		v[0] = s;
		for (int i = 0; i < vars; ++i)
			v[i + 1] = 0;
	}

	// Constructor with derivative for var_idx'th variable
	inline constexpr Dual(const real s, const int var_idx) noexcept
	{
		v[0] = s;
		for (int i = 0; i < vars; ++i)
			v[i + 1] = (i == var_idx) ? static_cast<real>(1) : 0;
	}

	// Copy constructor
	inline constexpr Dual(const Dual&) noexcept = default;

	inline constexpr const Dual & operator=(const Dual & rhs) noexcept { for (int i = 0; i < vars + 1; ++i) v[i] = rhs.v[i]; return *this; }

	inline constexpr Dual operator+(const Dual & rhs) const noexcept { Dual r; for (int i = 0; i < vars + 1; ++i) r.v[i] = v[i] + rhs.v[i]; return r; }
	inline constexpr Dual operator-(const Dual & rhs) const noexcept { Dual r; for (int i = 0; i < vars + 1; ++i) r.v[i] = v[i] - rhs.v[i]; return r; }

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
		const real inv_v0 = 1 / rhs.v[0];
		const real scale = inv_v0 * inv_v0;
		Dual r;
		r.v[0] = v[0] * inv_v0;
		for (int i = 0; i < vars; ++i)
			r.v[i + 1] = (v[i + 1] * rhs.v[0] - v[0] * rhs.v[i + 1]) * scale; // Quotient rule
		return r;
	}
};

using Dual2f = Dual<float, 2>;
using Dual3f = Dual<float, 3>;

//using Dual2d = Dual<double, 2>;
//using Dual3d = Dual<double, 3>;


template <typename real, int vars>
inline constexpr Dual<real, vars> pow(const Dual<real, vars> & d, const real e) noexcept
{
	const real scale = std::pow(d.v[0], e - 1) * e;

	Dual<real, vars> r;
	r.v[0] = std::pow(d.v[0], e);
	for (int i = 0; i < vars; ++i)
		r.v[i + 1] = d.v[i + 1] * scale;
	return r;
}


// Optimised version of pow for square root
template <typename real, int vars>
inline constexpr Dual<real, vars> sqrt(const Dual<real, vars> & d) noexcept
{
	const real sqrt_v0 = std::sqrt(d.v[0]);
	const real scale = static_cast<real>(0.5) / sqrt_v0;

	Dual<real, vars> r;
	r.v[0] = sqrt_v0;
	for (int i = 0; i < vars; ++i)
		r.v[i + 1] = d.v[i + 1] * scale;
	return r;
}


template <typename real, int vars>
inline constexpr Dual<real, vars> sin(const Dual<real, vars> & d) noexcept
{
	const real scale = std::cos(d.v[0]);

	Dual<real, vars> r;
	r.v[0] = std::sin(d.v[0]);
	for (int i = 0; i < vars; ++i)
		r.v[i + 1] = d.v[i + 1] * scale;
	return r;
}


template <typename real, int vars>
inline constexpr Dual<real, vars> cos(const Dual<real, vars> & d) noexcept
{
	const real scale = -std::sin(d.v[0]);

	Dual<real, vars> r;
	r.v[0] = std::cos(d.v[0]);
	for (int i = 0; i < vars; ++i)
		r.v[i + 1] = d.v[i + 1] * scale;
	return r;
}


template <typename real, int vars>
inline constexpr Dual<real, vars> tan(const Dual<real, vars> & d) noexcept
{
	const real cos_v0 = std::cos(d.v[0]);
	const real scale = 1 / (cos_v0 * cos_v0);

	Dual<real, vars> r;
	r.v[0] = std::tan(d.v[0]);
	for (int i = 0; i < vars; ++i)
		r.v[i + 1] = d.v[i + 1] * scale;
	return r;
}
