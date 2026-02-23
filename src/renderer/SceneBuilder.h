#pragma once

#include "renderer/Scene.h"
#include "renderer/SceneParams.h"
#include "renderer/ColouringFunction.h"
#include "renderer/FormulaFactory.h"
#include "scene_objects/SimpleObjects.h"

#include "formulas/Mandelbulb.h"
#include "formulas/BurningShip4D.h"
#include "formulas/Hopfbrot.h"


// Build a Scene from a vector of SceneObjectDescs
inline bool buildScene(Scene & scene, const std::vector<SceneObjectDesc> & objects, bool show_box = false)
{
	// Clear existing objects
	for (SceneObject * o : scene.objects) delete o;
	scene.objects.clear();

	// Optional bounding box (uses first fractal object's radius)
	if (show_box)
	{
		real k = 1.35f;
		for (const auto & obj : objects)
			if (obj.type == "fractal") { k = obj.radius; break; }

		Quad q0(vec3r(-k,  k, -k), vec3r(2, 0, 0) * k, vec3r(0, 0, 2) * k); q0.mat.albedo = vec3f(0.7f, 0.7f, 0.7f); q0.mat.use_fresnel = true; scene.objects.push_back(q0.clone());
		Quad q1(vec3r(-k, -k, -k), vec3r(0, 0, 2) * k, vec3r(2, 0, 0) * k); q1.mat.albedo = vec3f(0.7f, 0.7f, 0.7f); q1.mat.use_fresnel = true; scene.objects.push_back(q1.clone());
		Quad q2(vec3r(-k, -k,  k), vec3r(0, 2, 0) * k, vec3r(2, 0, 0) * k); q2.mat.albedo = vec3f(0.7f, 0.7f, 0.7f); q2.mat.use_fresnel = true; scene.objects.push_back(q2.clone());
		Quad q4(vec3r(-k, -k, -k), vec3r(0, 2, 0) * k, vec3r(0, 0, 2) * k); q4.mat.albedo = vec3f(0.90f, 0.2f, 0.02f); q4.mat.use_fresnel = true; scene.objects.push_back(q4.clone());
		Quad q5(vec3r( k, -k, -k), vec3r(0, 0, 2) * k, vec3r(0, 2, 0) * k); q5.mat.albedo = vec3f(0.02f, 0.8f, 0.05f); q5.mat.use_fresnel = true; scene.objects.push_back(q5.clone());
	}

	for (const SceneObjectDesc & obj : objects)
	{
		if (obj.type == "sphere")
		{
			auto * s = new Sphere;
			s->centre = obj.position;
			s->radius = obj.radius;
			s->mat.albedo      = obj.albedo;
			s->mat.emission    = obj.emission;
			s->mat.use_fresnel = obj.use_fresnel;
			s->mat.r0          = obj.r0;
			scene.objects.push_back(s);
		}
		else if (obj.type == "quad")
		{
			auto * q = new Quad(obj.position, obj.quad_u, obj.quad_v);
			q->mat.albedo      = obj.albedo;
			q->mat.emission    = obj.emission;
			q->mat.use_fresnel = obj.use_fresnel;
			q->mat.r0          = obj.r0;
			scene.objects.push_back(q);
		}
		else if (obj.type == "fractal")
		{
			if (obj.formula_name == "mandelbulb")
			{
				auto * bulb = new MandelbulbDual;
				bulb->radius         = obj.radius;
				bulb->mat.albedo      = obj.albedo;
				bulb->mat.emission    = obj.emission;
				bulb->mat.use_fresnel = obj.use_fresnel;
				bulb->mat.r0          = obj.r0;
				if (obj.use_orbit_trap_colouring) bulb->mat.colouring = new OrbitTrapColouring();
				scene.objects.push_back(bulb->clone());
			}
			else if (obj.formula_name == "hopfbrot")
			{
				auto * bulb = new Hopfbrot;
				bulb->radius         = obj.radius;
				bulb->step_scale     = obj.step_scale;
				bulb->scene_scale    = obj.scene_scale;
				bulb->mat.albedo      = obj.albedo;
				bulb->mat.emission    = obj.emission;
				bulb->mat.use_fresnel = obj.use_fresnel;
				bulb->mat.r0          = obj.r0;
				if (obj.use_orbit_trap_colouring) bulb->mat.colouring = new OrbitTrapColouring();
				scene.objects.push_back(bulb->clone());
			}
			else if (obj.formula_name == "burningship4d")
			{
				auto * bulb = new BurningShip4D;
				bulb->radius         = obj.radius;
				bulb->mat.albedo      = obj.albedo;
				bulb->mat.emission    = obj.emission;
				bulb->mat.use_fresnel = obj.use_fresnel;
				bulb->mat.r0          = obj.r0;
				if (obj.use_orbit_trap_colouring) bulb->mat.colouring = new OrbitTrapColouring();
				scene.objects.push_back(bulb->clone());
			}
			else if (!obj.formulas.empty())
			{
				// Clone the formulas for thread safety
				std::vector<IterationFunction *> cloned_funcs;
				for (auto * f : obj.formulas)
					cloned_funcs.push_back(f->clone());

				GeneralDualDE hybrid(obj.max_iters, cloned_funcs, obj.iteration_sequence);
				hybrid.radius          = obj.radius;
				hybrid.step_scale      = obj.step_scale;
				hybrid.bailout_radius2 = obj.bailout_radius2;
				hybrid.mat.albedo      = obj.albedo;
				hybrid.mat.emission    = obj.emission;
				hybrid.mat.use_fresnel = obj.use_fresnel;
				hybrid.mat.r0          = obj.r0;
				if (obj.use_orbit_trap_colouring) hybrid.mat.colouring = new OrbitTrapColouring();
				scene.objects.push_back(hybrid.clone());
			}
			else
				return false; // Unknown formula with no formulas set up
		}
	}

	return true;
}


// Convenience: build from SceneParams
inline bool buildScene(Scene & scene, const SceneParams & params)
{
	return buildScene(scene, params.objects, params.show_box);
}
