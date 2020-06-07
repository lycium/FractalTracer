#pragma once

#include <vector>

#include "SceneObject.h"



struct Scene
{
	Scene() = default;

	// Copy constructor
	Scene(const Scene & s)
	{
		objects.resize(s.objects.size());

		for (size_t i = 0; i < s.objects.size(); ++i)
			objects[i] = s.objects[i]->clone();
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


	std::vector<SceneObject *> objects;
};
