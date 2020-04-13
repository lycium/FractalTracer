#pragma once

#include "real.h"
#include "vec3.h"



#if USE_DOUBLE
constexpr static real ray_epsilon = 3e-9; // Ignore intersections closer than this to avoid self-intersection artifacts
constexpr static real DE_thresh = 2e-10; // Nearest intersection distance for DE objects
#else
constexpr static real ray_epsilon = 3e-4f;
constexpr static real DE_thresh = 2e-5f;
#endif


struct Ray
{
	vec3r o; // Origin
	vec3r d; // Direction normalised
};
