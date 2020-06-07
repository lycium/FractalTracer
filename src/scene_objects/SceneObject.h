#pragma once

#include "renderer/Ray.h"
#include "renderer/Material.h"



struct SceneObject
{
	virtual ~SceneObject() = default;

	virtual real  intersect(const Ray   & r) noexcept = 0;
	virtual vec3r getNormal(const vec3r & p) noexcept = 0;

	virtual SceneObject * clone() const = 0;


	Material mat;
};
