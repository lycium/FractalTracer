#pragma once

#include "maths/vec.h"
#include "renderer/ColouringFunction.h"



struct Material
{
	vec3f albedo   = 0;
	vec3f emission = 0;

	bool use_fresnel = false;
	float r0 = 0.02f; // Reflection at normal incidence, for Fresnel

	ColouringFunction * colouring = nullptr;

	~Material() { delete colouring; }

	Material() = default;

	Material(const Material & m)
		: albedo(m.albedo), emission(m.emission), use_fresnel(m.use_fresnel), r0(m.r0),
		  colouring(m.colouring ? m.colouring->clone() : nullptr) { }

	Material & operator=(const Material & m)
	{
		if (this != &m)
		{
			albedo      = m.albedo;
			emission    = m.emission;
			use_fresnel = m.use_fresnel;
			r0          = m.r0;

			delete colouring;
			colouring = m.colouring ? m.colouring->clone() : nullptr;
		}
		return *this;
	}
};
