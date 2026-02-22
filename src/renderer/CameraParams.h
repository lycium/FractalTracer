#pragma once

#include "maths/vec.h"


struct CameraParams
{
	vec3r position  = { 2.5f, 1.25f, -2.5f };
	vec3r look_at   = { 0, -0.125f, 0 };
	vec3r world_up  = { 0, 1, 0 };

	real fov_deg          = 80;
	real dof_amount       = 0.1f;
	real lens_radius      = 0.005f;
	real focal_dist_scale = 0.65f;

	// Precomputed basis vectors (call recompute() after changing position/look_at/world_up)
	vec3r forward = { 0, 0, 1 };
	vec3r right   = { 1, 0, 0 };
	vec3r up      = { 0, 1, 0 };

	void recompute()
	{
		forward = normalise(look_at - position);
		right   = normalise(cross(world_up, forward));
		up      = normalise(cross(forward, right));
	}
};
