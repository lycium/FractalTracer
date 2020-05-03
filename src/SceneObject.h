#pragma once

#include "vec3.h"
#include "Ray.h"


struct SceneObject
{
	virtual real  intersect(const Ray   & r) noexcept = 0;
	virtual vec3r getNormal(const vec3r & p) noexcept = 0;

	virtual SceneObject * clone() const = 0;


	vec3f albedo   = 0;
	vec3f emission = 0;

	bool use_fresnel = false;
	float r0 = 0.02f; // Reflection at normal incidence, for Fresnel
};
