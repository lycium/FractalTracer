#pragma once

#include "SceneObject.h"



struct Sphere final : public SceneObject
{
	vec3r centre = { 0, 0, 0 };
	real  radius = 1; 


	virtual real intersect(const Ray & r) noexcept override
	{
		const vec3r s = r.o - centre;
		const real  b = dot(s, r.d);
		const real  c = dot(s, s) - radius * radius;

		const real discriminant = b * b - c;
		if (discriminant < 0)
			return -1;

		// Compute both roots and return the nearest one that's > 0
		const real t1 = -b - std::sqrt(discriminant);
		const real t2 = -b + std::sqrt(discriminant);
		return (t1 >= 0) ? t1 : t2;
	}

	virtual vec3r getNormal(const vec3r & p) noexcept override
	{
		return (p - centre) * (1 / radius);
	}

	virtual SceneObject * clone() const override
	{
		Sphere * o = new Sphere;
		*o = *this;
		return o;
	}
};
