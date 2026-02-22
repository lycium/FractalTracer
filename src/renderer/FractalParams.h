#pragma once

#include <string>
#include "maths/vec.h"


struct FractalParams
{
	std::string formula_name = "mandalay";

	// Common DE parameters
	real radius          = 1.35f;
	real scene_scale     = 1;
	real step_scale      = 0.25f;
	real bailout_radius2 = 64;
	int  max_iters       = 30;

	// Material
	vec3f albedo   = { 0.2f, 0.6f, 0.9f };
	vec3f emission = { 0, 0, 0 };
	bool  use_fresnel = true;
	float r0 = 0.25f;
	bool  use_orbit_trap_colouring = true;

	// MandalayKIFS-specific
	real  mandalay_scale          = 3;
	real  mandalay_min_r2         = 0;
	real  mandalay_folding_offset = 1;
	real  mandalay_z_tower        = 0;
	real  mandalay_xy_tower       = 0;
	vec3r mandalay_rotate         = { 0, 0, 0 };
	vec3r mandalay_julia_c        = { 0, 0, 0 };
	bool  mandalay_julia_mode     = true;

	// Amazingbox-specific
	real amazingbox_scale      = -2;
	real amazingbox_min_r2     = 0.25f;
	real amazingbox_fix_r2     = 1;
	real amazingbox_fold_limit = 1;
	bool amazingbox_julia_mode = false;

	// Hybrid amazingbox_mandalay overrides
	real hybrid_amazingbox_scale      = -1.77f;
	real hybrid_amazingbox_fold_limit = 1.0f;
	real hybrid_amazingbox_min_r2     = 0.25f;
	real hybrid_mandalay_scale        = 2.8f;
	real hybrid_mandalay_folding_offset = 1.0f;
	real hybrid_mandalay_z_tower      = 0.35f;
	real hybrid_mandalay_xy_tower     = 0.2f;
	vec3r hybrid_mandalay_rotate      = { 0.12f, 0.08f, 0.0f };
	bool  hybrid_mandalay_julia_mode  = false;

	// Lambdabulb-specific
	real  lambdabulb_power = 4;
	vec3r lambdabulb_c     = { 1.035f, -0.317f, 0.013f };

	// Octopus-specific
	real octopus_xz_mul    = 1.25f;
	real octopus_sq_mul    = 1;
	bool octopus_julia_mode = true;

	// Cubicbulb-specific
	real  cubicbulb_y_mul      = 3;
	real  cubicbulb_z_mul      = 3;
	real  cubicbulb_aux_mul    = 1;
	vec3r cubicbulb_c          = { -0.5f, -0.5f, -0.25f };
	bool  cubicbulb_julia_mode = true;

	// BenesiPine2-specific
	real benesipine2_scale      = 2.5f;
	real benesipine2_offset     = 0.75f;
	bool benesipine2_julia_mode = true;

	// MengerSpongeC-specific
	real  mengersponge_scale        = 3;
	vec3r mengersponge_scale_centre = { 1.0f, 1.0f, 1.0f };

	// PseudoKleinian-specific
	real pseudokleinian_mins[4] = { -0.8323f, -0.694f, -0.5045f, 0.8067f };
	real pseudokleinian_maxs[4] = {  0.8579f,  1.0883f, 0.8937f, 0.9411f };

	// RiemannSphere-specific
	real riemannsphere_scale   = 1;
	real riemannsphere_s_shift = 0;
	real riemannsphere_t_shift = 0;
	real riemannsphere_x_shift = 1;
	real riemannsphere_r_shift = -0.25f;
	real riemannsphere_r_pow   = 2;

	// Sphere (mirror)
	float sphere_r0 = 0.95f;

	// Hopfbrot-specific
	real hopfbrot_scene_scale = 2.0f;

	// Show bounding box
	bool show_box = false;
};
