#pragma once

#include <cmath>
#include <limits>

#include "maths/vec.h"


struct ColouringFunction
{
	virtual ~ColouringFunction() = default;

	virtual void init(const DualVec3r & p_0) noexcept = 0;
	virtual void iter(const DualVec3r & p_in) noexcept = 0;

	// 4D overloads for standalone 4D formulas (e.g. Hopfbrot)
	virtual void init(const DualVec4r &) noexcept { }
	virtual void iter(const DualVec4r &) noexcept { }

	virtual void getMaterial(vec3f & albedo_out, vec3f & emit_out) const noexcept = 0;

	virtual ColouringFunction * clone() const = 0;
};


struct OrbitTrapColouring final : public ColouringFunction
{
	virtual void init(const DualVec3r &) noexcept override final
	{
		r2_min = std::numeric_limits<real>::infinity();
		trap_pos = { 0, 0, 0, 0 };
		iter_at_min = 0;
		iter_count = 0;
		angular_sum = 0;
	}

	virtual void init(const DualVec4r &) noexcept override final
	{
		r2_min = std::numeric_limits<real>::infinity();
		trap_pos = { 0, 0, 0, 0 };
		iter_at_min = 0;
		iter_count = 0;
		angular_sum = 0;
	}

	virtual void iter(const DualVec3r & p_in) noexcept override final
	{
		const real r2 = length2(p_in);
		if (r2 < r2_min)
		{
			r2_min = r2;
			trap_pos = { p_in.x().v[0], p_in.y().v[0], p_in.z().v[0], 0 };
			iter_at_min = iter_count;
		}
		iter_count++;
	}

	virtual void iter(const DualVec4r & p_in) noexcept override final
	{
		const real x = p_in.x().v[0], y = p_in.y().v[0], z = p_in.z().v[0], w = p_in.w().v[0];
		const real r2 = x*x + y*y + z*z + w*w;

		// Accumulate angular momentum between the two complex planes (xy and zw)
		// This captures the Hopf fibration structure
		const real xy2 = x*x + y*y;
		const real zw2 = z*z + w*w;
		if (xy2 + zw2 > 1e-20f)
			angular_sum += std::atan2(std::sqrt(zw2), std::sqrt(xy2));

		if (r2 < r2_min)
		{
			r2_min = r2;
			trap_pos = { x, y, z, w };
			iter_at_min = iter_count;
		}
		iter_count++;
	}

	virtual void getMaterial(vec3f & albedo_out, vec3f & emit_out) const noexcept override final
	{
		const float r = (float)std::sqrt(r2_min);

		// Orbit trap position encodes spatial structure
		const float trap_angle = (float)std::atan2(trap_pos.e[2], trap_pos.e[0]);

		// Hopf fiber angle: how much zw vs xy at closest approach
		const float hopf = (float)std::atan2(
			std::sqrt(trap_pos.e[2] * trap_pos.e[2] + trap_pos.e[3] * trap_pos.e[3]),
			std::sqrt(trap_pos.e[0] * trap_pos.e[0] + trap_pos.e[1] * trap_pos.e[1]));

		// Combine orbit trap distance and angular position
		const float t1 = r * 2.0f + iter_at_min * 0.12f;
		const float t2 = trap_angle * 0.3f + hopf * 0.8f;

		// Cool-toned palette: restricted hue range (blues, teals, slate)
		// Low c values = narrow hue band, similar phase offsets = cool tones
		// Ref: https://iquilezles.org/articles/palettes/
		const vec3f col1 = cosinePalette(t1,
			{ 0.35f, 0.45f, 0.55f },
			{ 0.25f, 0.25f, 0.30f },
			{ 0.6f,  0.6f,  0.6f  },
			{ 0.58f, 0.65f, 0.72f });
		const vec3f col2 = cosinePalette(t2,
			{ 0.40f, 0.50f, 0.55f },
			{ 0.20f, 0.20f, 0.25f },
			{ 0.5f,  0.5f,  0.5f  },
			{ 0.50f, 0.55f, 0.65f });

		const vec3f base = col1 * 0.65f + col2 * 0.35f;

		// Gamma-shaped response for richer darks
		albedo_out = base * base * 0.92f + 0.03f;

		// Subtle cool emission in tight orbit traps
		const float glow = std::exp(-r * 10.0f) * 0.3f;
		emit_out = vec3f{ 0.4f, 0.6f, 0.9f } * glow;
	}

	virtual ColouringFunction * clone() const override final
	{
		return new OrbitTrapColouring(*this);
	}

private:
	static vec3f cosinePalette(float t, const vec3f & a, const vec3f & b, const vec3f & c, const vec3f & d)
	{
		return vec3f(
			a.x() + b.x() * std::cos((float)two_pi * (c.x() * t + d.x())),
			a.y() + b.y() * std::cos((float)two_pi * (c.y() * t + d.y())),
			a.z() + b.z() * std::cos((float)two_pi * (c.z() * t + d.z())));
	}

	real r2_min = std::numeric_limits<real>::infinity();
	vec4r trap_pos = { 0, 0, 0, 0 };
	real angular_sum = 0;
	int iter_at_min = 0;
	int iter_count = 0;
};
