#pragma once

#include "vec3.h"
#include "Ray.h"


struct SceneObject
{
	virtual real  intersect(const Ray   & r) const noexcept = 0;
	virtual vec3r getNormal(const vec3r & p) const noexcept = 0;

	vec3f colour = { 0, 0, 0 };
};
