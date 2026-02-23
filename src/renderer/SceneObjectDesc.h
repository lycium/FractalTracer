#pragma once

#include <string>
#include <vector>
#include "maths/vec.h"
#include "scene_objects/DualDEObject.h"


struct SceneObjectDesc
{
	std::string type = "fractal";  // "sphere", "quad", "fractal"
	std::string name = "Fractal";

	// Material
	vec3f albedo   = { 0.2f, 0.6f, 0.9f };
	vec3f emission = { 0, 0, 0 };
	bool  use_fresnel = true;
	float r0 = 0.25f;
	bool  use_orbit_trap_colouring = true;

	// Geometry (type-dependent)
	vec3r position = { 0, 0, 0 };
	real  radius   = 1.35f;

	// Quad vectors
	vec3r quad_u = { 1, 0, 0 };
	vec3r quad_v = { 0, 1, 0 };

	// Fractal DE params
	std::string formula_name = "amosersine";
	int  max_iters       = 30;
	real scene_scale     = 1;
	real step_scale      = 0.25f;
	real bailout_radius2 = 64;

	// Formula instances with their params (owned, cloned on copy)
	std::vector<IterationFunction *> formulas;
	std::vector<char> iteration_sequence = { 0 };


	SceneObjectDesc() = default;

	SceneObjectDesc(const SceneObjectDesc & other) :
		type(other.type), name(other.name),
		albedo(other.albedo), emission(other.emission),
		use_fresnel(other.use_fresnel), r0(other.r0),
		use_orbit_trap_colouring(other.use_orbit_trap_colouring),
		position(other.position), radius(other.radius),
		quad_u(other.quad_u), quad_v(other.quad_v),
		formula_name(other.formula_name),
		max_iters(other.max_iters), scene_scale(other.scene_scale),
		step_scale(other.step_scale), bailout_radius2(other.bailout_radius2),
		iteration_sequence(other.iteration_sequence)
	{
		for (auto * f : other.formulas)
			formulas.push_back(f ? f->clone() : nullptr);
	}

	SceneObjectDesc & operator=(const SceneObjectDesc & other)
	{
		if (this == &other) return *this;

		for (auto * f : formulas) delete f;
		formulas.clear();

		type = other.type;
		name = other.name;
		albedo = other.albedo;
		emission = other.emission;
		use_fresnel = other.use_fresnel;
		r0 = other.r0;
		use_orbit_trap_colouring = other.use_orbit_trap_colouring;
		position = other.position;
		radius = other.radius;
		quad_u = other.quad_u;
		quad_v = other.quad_v;
		formula_name = other.formula_name;
		max_iters = other.max_iters;
		scene_scale = other.scene_scale;
		step_scale = other.step_scale;
		bailout_radius2 = other.bailout_radius2;
		iteration_sequence = other.iteration_sequence;

		for (auto * f : other.formulas)
			formulas.push_back(f ? f->clone() : nullptr);

		return *this;
	}

	~SceneObjectDesc()
	{
		for (auto * f : formulas) delete f;
	}
};
