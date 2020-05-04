#pragma once

#include "vec3.h"



struct Material
{
	vec3f albedo   = 0;
	vec3f emission = 0;

	bool use_fresnel = false;
	float r0 = 0.02f; // Reflection at normal incidence, for Fresnel
};
