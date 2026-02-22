#pragma once

#include "renderer/Scene.h"
#include "renderer/SceneParams.h"
#include "renderer/ColouringFunction.h"
#include "scene_objects/SimpleObjects.h"
#include "scene_objects/DualDEObject.h"

#include "formulas/Mandelbulb.h"
#include "formulas/QuadraticJuliabulb.h"
#include "formulas/MengerSponge.h"
#include "formulas/MengerSpongeC.h"
#include "formulas/Cubicbulb.h"
#include "formulas/Amazingbox.h"
#include "formulas/Octopus.h"
#include "formulas/PseudoKleinian.h"
#include "formulas/MandalayKIFS.h"
#include "formulas/BenesiPine2.h"
#include "formulas/RiemannSphere.h"
#include "formulas/SphereTree.h"
#include "formulas/Lambdabulb.h"
#include "formulas/BurningShip4D.h"
#include "formulas/Hopfbrot.h"


// Build a Scene from FractalParams
// Returns false if formula_name is unknown
inline bool buildScene(Scene & scene, const FractalParams & fp)
{
	// Clear existing objects
	for (SceneObject * o : scene.objects) delete o;
	scene.objects.clear();

	// Optional bounding box
	if (fp.show_box)
	{
		const real k = fp.radius;
		Quad q0(vec3r(-k,  k, -k), vec3r(2, 0, 0) * k, vec3r(0, 0, 2) * k); q0.mat.albedo = vec3f(0.7f, 0.7f, 0.7f); q0.mat.use_fresnel = true; scene.objects.push_back(q0.clone());
		Quad q1(vec3r(-k, -k, -k), vec3r(0, 0, 2) * k, vec3r(2, 0, 0) * k); q1.mat.albedo = vec3f(0.7f, 0.7f, 0.7f); q1.mat.use_fresnel = true; scene.objects.push_back(q1.clone());
		Quad q2(vec3r(-k, -k,  k), vec3r(0, 2, 0) * k, vec3r(2, 0, 0) * k); q2.mat.albedo = vec3f(0.7f, 0.7f, 0.7f); q2.mat.use_fresnel = true; scene.objects.push_back(q2.clone());
		Quad q4(vec3r(-k, -k, -k), vec3r(0, 2, 0) * k, vec3r(0, 0, 2) * k); q4.mat.albedo = vec3f(0.90f, 0.2f, 0.02f); q4.mat.use_fresnel = true; scene.objects.push_back(q4.clone());
		Quad q5(vec3r( k, -k, -k), vec3r(0, 0, 2) * k, vec3r(0, 2, 0) * k); q5.mat.albedo = vec3f(0.02f, 0.8f, 0.05f); q5.mat.use_fresnel = true; scene.objects.push_back(q5.clone());
	}

	const std::string & name = fp.formula_name;

	if (name == "sphere")
	{
		Sphere mirror;
		mirror.centre = { 0, 0, 0 };
		mirror.radius = fp.radius;
		mirror.mat.albedo = { 0.9f, 0.9f, 0.9f };
		mirror.mat.use_fresnel = true;
		mirror.mat.r0 = fp.sphere_r0;
		scene.objects.push_back(mirror.clone());
	}
	else if (name == "amazingbox_mandalay")
	{
		auto * amazingbox = new DualAmazingboxIteration;
		amazingbox->scale      = fp.hybrid_amazingbox_scale;
		amazingbox->fold_limit = fp.hybrid_amazingbox_fold_limit;
		amazingbox->min_r2     = fp.hybrid_amazingbox_min_r2;

		auto * mandalay = new DualMandalayKIFSIteration;
		mandalay->scale          = fp.hybrid_mandalay_scale;
		mandalay->folding_offset = fp.hybrid_mandalay_folding_offset;
		mandalay->z_tower        = fp.hybrid_mandalay_z_tower;
		mandalay->xy_tower       = fp.hybrid_mandalay_xy_tower;
		mandalay->rotate         = fp.hybrid_mandalay_rotate;
		mandalay->julia_mode     = fp.hybrid_mandalay_julia_mode;

		std::vector<IterationFunction *> iter_funcs;
		iter_funcs.push_back(amazingbox);
		iter_funcs.push_back(mandalay);
		const std::vector<char> iter_seq = { 0, 0, 1 };

		GeneralDualDE hybrid(fp.max_iters, iter_funcs, iter_seq);
		hybrid.radius     = fp.radius;
		hybrid.step_scale = fp.step_scale;
		hybrid.mat.albedo      = fp.albedo;
		hybrid.mat.use_fresnel = fp.use_fresnel;
		if (fp.use_orbit_trap_colouring) hybrid.mat.colouring = new OrbitTrapColouring();
		scene.objects.push_back(hybrid.clone());
	}
	else if (name == "hopfbrot")
	{
		Hopfbrot bulb;
		bulb.radius      = fp.radius;
		bulb.step_scale  = fp.step_scale;
		bulb.scene_scale = fp.hopfbrot_scene_scale;
		bulb.mat.albedo      = fp.albedo;
		bulb.mat.use_fresnel = fp.use_fresnel;
		if (fp.use_orbit_trap_colouring) bulb.mat.colouring = new OrbitTrapColouring();
		scene.objects.push_back(bulb.clone());
	}
	else if (name == "burningship4d")
	{
		BurningShip4D bulb;
		bulb.radius = fp.radius;
		bulb.mat.albedo      = fp.albedo;
		bulb.mat.use_fresnel = fp.use_fresnel;
		if (fp.use_orbit_trap_colouring) bulb.mat.colouring = new OrbitTrapColouring();
		scene.objects.push_back(bulb.clone());
	}
	else if (name == "mandelbulb")
	{
		MandelbulbDual bulb;
		bulb.radius = fp.radius;
		bulb.mat.albedo      = fp.albedo;
		bulb.mat.use_fresnel = fp.use_fresnel;
		if (fp.use_orbit_trap_colouring) bulb.mat.colouring = new OrbitTrapColouring();
		scene.objects.push_back(bulb.clone());
	}
	else
	{
		// IterationFunction-based formulas wrapped in GeneralDualDE
		IterationFunction * iter = nullptr;

		if      (name == "lambdabulb")     { auto * p = new DualLambdabulbIteration;        p->power = fp.lambdabulb_power; p->c = DualVec3r(fp.lambdabulb_c.x(), fp.lambdabulb_c.y(), fp.lambdabulb_c.z()); iter = p; }
		else if (name == "amazingbox")     { auto * p = new DualAmazingboxIteration;        p->scale = fp.amazingbox_scale; p->min_r2 = fp.amazingbox_min_r2; p->fix_r2 = fp.amazingbox_fix_r2; p->fold_limit = fp.amazingbox_fold_limit; p->julia_mode = fp.amazingbox_julia_mode; iter = p; }
		else if (name == "octopus")        { auto * p = new DualOctopusIteration;           p->xz_mul = fp.octopus_xz_mul; p->sq_mul = fp.octopus_sq_mul; p->julia_mode = fp.octopus_julia_mode; iter = p; }
		else if (name == "mengersponge")   { auto * p = new DualMengerSpongeCIteration;     p->scale = fp.mengersponge_scale; p->scale_centre = fp.mengersponge_scale_centre; iter = p; }
		else if (name == "cubicbulb")      { auto * p = new DualCubicbulbIteration;         p->y_mul = fp.cubicbulb_y_mul; p->z_mul = fp.cubicbulb_z_mul; p->aux_mul = fp.cubicbulb_aux_mul; p->c = DualVec3r(fp.cubicbulb_c.x(), fp.cubicbulb_c.y(), fp.cubicbulb_c.z()); p->julia_mode = fp.cubicbulb_julia_mode; iter = p; }
		else if (name == "pseudokleinian") { auto * p = new DualPseudoKleinianIteration;    for (int i = 0; i < 4; i++) { p->mins[i] = fp.pseudokleinian_mins[i]; p->maxs[i] = fp.pseudokleinian_maxs[i]; } iter = p; }
		else if (name == "riemannsphere")  { auto * p = new DualRiemannSphereIteration;     p->scale = fp.riemannsphere_scale; p->s_shift = fp.riemannsphere_s_shift; p->t_shift = fp.riemannsphere_t_shift; p->x_shift = fp.riemannsphere_x_shift; p->r_shift = fp.riemannsphere_r_shift; p->r_pow = fp.riemannsphere_r_pow; iter = p; }
		else if (name == "mandalay")       { auto * p = new DualMandalayKIFSIteration;      p->scale = fp.mandalay_scale; p->min_r2 = fp.mandalay_min_r2; p->folding_offset = fp.mandalay_folding_offset; p->z_tower = fp.mandalay_z_tower; p->xy_tower = fp.mandalay_xy_tower; p->rotate = fp.mandalay_rotate; p->julia_c = fp.mandalay_julia_c; p->julia_mode = fp.mandalay_julia_mode; iter = p; }
		else if (name == "spheretree")     { iter = new DualSphereTreeIteration; }
		else if (name == "benesipine2")    { auto * p = new DualBenesiPine2Iteration;       p->scale = fp.benesipine2_scale; p->offset = fp.benesipine2_offset; p->julia_mode = fp.benesipine2_julia_mode; iter = p; }
		else
			return false; // Unknown formula

		std::vector<IterationFunction *> iter_funcs;
		iter_funcs.push_back(iter);
		const std::vector<char> iter_seq = { 0 };

		GeneralDualDE hybrid(fp.max_iters, iter_funcs, iter_seq);
		hybrid.radius         = fp.radius;
		hybrid.step_scale     = fp.step_scale;
		hybrid.bailout_radius2 = fp.bailout_radius2;
		hybrid.mat.albedo      = fp.albedo;
		hybrid.mat.emission    = fp.emission;
		hybrid.mat.use_fresnel = fp.use_fresnel;
		hybrid.mat.r0          = fp.r0;
		if (fp.use_orbit_trap_colouring) hybrid.mat.colouring = new OrbitTrapColouring();
		scene.objects.push_back(hybrid.clone());
	}

	return true;
}
