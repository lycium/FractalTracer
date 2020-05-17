#pragma once

#include "vec3.h"



struct ColouringFunction
{
	virtual void init(const DualVec3r & p_0) noexcept { }
	virtual void iter(const DualVec3r & p_in) noexcept = 0;

	virtual void getMaterial(vec3f & albedo_out, vec3f & emit_out) const noexcept = 0;

	virtual ColouringFunction * clone() const = 0;
};


struct MinRadiusPaletteColouring final : public ColouringFunction
{

	virtual void init(const DualVec3r & p_0) noexcept override final
	{
		r2_min = -std::log((real)0);
	}

	virtual void iter(const DualVec3r & p_in) noexcept override final
	{
		r2_min = std::min(r2_min, length2(p_in));
	}

	virtual void getMaterial(vec3f & albedo_out, vec3f & emit_out) const noexcept override final
	{
		const float r = (float)std::sqrt(r2_min) * 3.0f + 0.25f;

		const vec3f a = { 0.5f, 0.5f, 0.5f };
		const vec3f b = { 0.5f, 0.5f, 0.5f };
		const vec3f c = { 1.0f, 1.0f, 0.5f };
		const vec3f d = { 0.8f, 0.9f, 0.3f };
		const vec3f base = cosinePalette(r, a, b, c, d);
		const vec3f modified = base * base * 0.85f + 0.05f;

		const vec3f yellow = vec3f{ 1.0f, 1.0f, 0 };
		const float yellowness = (fabsf(r - 2.2f) < 0.05f) ? 1 : 0; //dot(modified, yellow) / length(yellow);
		const float y4 = yellowness * yellowness * yellowness * yellowness;
		const vec3f albedo = modified;// - yellow * (y4 * 0.125f);

		albedo_out = albedo;// - yellow;
		emit_out = vec3f{ 1.0f, 1.0f, 1.0f } * (y4 * 2);
	}

	virtual ColouringFunction * clone() const override final
	{
		return new MinRadiusPaletteColouring(*this);
	}

private:
	vec3f cosinePalette(float t, const vec3f & a, const vec3f & b, const vec3f & c, const vec3f & d) const
	{
		return vec3f(
			a.x + b.x * std::cos((float)two_pi * (c.x * t + d.x)),
			a.y + b.y * std::cos((float)two_pi * (c.y * t + d.y)),
			a.z + b.z * std::cos((float)two_pi * (c.z * t + d.z)));
	}

	real r2_min;
};


struct Material
{
	vec3f albedo   = 0;
	vec3f emission = 0;

	bool use_fresnel = false;
	float r0 = 0.02f; // Reflection at normal incidence, for Fresnel

	MinRadiusPaletteColouring colouring;
};
