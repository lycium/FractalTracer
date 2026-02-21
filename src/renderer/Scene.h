#pragma once

#include <vector>

#include "scene_objects/SceneObject.h"



struct Scene
{
	std::vector<SceneObject *> objects;


	Scene() = default;

	// Copy constructor
	Scene(const Scene & s)
	{
		for (SceneObject * const o : objects)
			delete o;

		objects.resize(0);

		for (const SceneObject * const o : s.objects)
			objects.push_back(o->clone());
	}

	// Scene owns all the object pointers, so delete them
	~Scene()
	{
		for (SceneObject * const o : objects)
			delete o;
	}

	std::pair<SceneObject *, real> nearestIntersection(const Ray & r) noexcept
	{
		SceneObject * nearest_obj = nullptr;
		real nearest_t = real_inf;

		for (SceneObject * const o : objects)
		{
			const real hit_t = o->intersect(r);
			if (hit_t > ray_epsilon && hit_t < nearest_t)
			{
				nearest_obj = o;
				nearest_t = hit_t;
			}
		}

		return { nearest_obj, nearest_t };
	}
};
