#pragma once

#include <vector>

#include "SceneObject.h"
#include "SimpleObjects.h"
#include "Mandelbulb.h"
#include "QuadraticJuliabulb.h"
#include "MengerSponge.h"



struct Scene
{
	void init()
	{
		objects.resize(0);

		for (auto & o : spheres)                 objects.push_back((SceneObject *)&o);
		for (auto & o : analytic_mandelbulbs)    objects.push_back((SceneObject *)&o);
		for (auto & o : dual_mandelbulbs)        objects.push_back((SceneObject *)&o);
		for (auto & o : analytic_juliabulbs)     objects.push_back((SceneObject *)&o);
		for (auto & o : dual_juliabulbs)         objects.push_back((SceneObject *)&o);
		for (auto & o : analytic_menger_sponges) objects.push_back((SceneObject *)&o);
		for (auto & o : dual_menger_sponges)     objects.push_back((SceneObject *)&o);
	}


	std::pair<const SceneObject *, real> nearestIntersection(const Ray & r) const noexcept
	{
		const SceneObject * nearest_obj = nullptr;
		real nearest_t = real_inf;

		for (const SceneObject * const o : objects)
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


	std::vector<Sphere> spheres;

	std::vector<MandelbulbAnalytic> analytic_mandelbulbs;
	std::vector<MandelbulbDual> dual_mandelbulbs;

	std::vector<QuadraticJuliabulbAnalytic> analytic_juliabulbs;
	std::vector<QuadraticJuliabulbDual> dual_juliabulbs;

	std::vector<MengerSpongeAnalytic> analytic_menger_sponges;
	std::vector<MengerSpongeDual> dual_menger_sponges;

protected:
	std::vector<SceneObject *> objects;
};
