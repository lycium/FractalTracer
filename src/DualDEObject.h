#pragma once

#include <algorithm>

#include "SceneObject.h"



// Base class for dual number based distance estimated (DE) objects
struct DualDEObject : public SceneObject
{
	vec3r centre = { 0, 0, 0 };
	real  radius = 1; 


	real getPolynomialDE(const DualVec3r & p_os, vec3r & normal_os_out) const noexcept
	{
		// Extract the position vector and Jacobian
		const vec3r p  = vec3r{ p_os.x.v[0], p_os.y.v[0], p_os.z.v[0] };
		const vec3r jx = vec3r{ p_os.x.v[1], p_os.y.v[1], p_os.z.v[1] };
		const vec3r jy = vec3r{ p_os.x.v[2], p_os.y.v[2], p_os.z.v[2] };
		const vec3r jz = vec3r{ p_os.x.v[3], p_os.y.v[3], p_os.z.v[3] };

		const real len2 = dot(p, p); 
		const real len = sqrt(len2);
		const vec3r u = p * (1 / len); // Normalise p first to avoid overflow in dot products

		// Vector-matrix norm: ||J||_u = |u.J|/|u|
		// Ref: https://fractalforums.org/fractal-image-gallery/18/burning-ship-distance-estimation/647/msg3207#msg3207
		const vec3r dr = vec3r
		{
			dot(u, jx),
			dot(u, jy),
			dot(u, jz)
		};

#if 0
		// Should work in theory, but unfortunately does not.
		// Ref: https://www.evl.uic.edu/hypercomplex/html/book/book.pdf chapter 9.6
		return 1.0f * len / length(dr);
#else
		// Modifications and explanations by claude to take into account polynomial-ness:
		const real len_dr = length(dr);

		// The basic DE formula has a log for functions that escape like |z_{n+k}| ~ |z_n|^{p^k}
		// Ref: http://linas.org/art-gallery/mderive/mderive.html
		const real de_base = len * std::log(len) / len_dr;

		// Koebe 1/4 theorem for complex-analytic functions implies that
		// the distance estimate is accurate up to a factor of 2.  Mandelbulb
		// is not complex-analytic (or even complex) but one might hope that
		// similar arguments could apply.  Divide by 2 to get the lower bound.
		const real koebe_factor = 0.5f;

#if 0
		// The distance estimate needs a log(power) factor.
		// Not sure if this is included in the derivative calculation.
		// Ref: https://fractalforums.org/fractal-mathematics-and-new-theories/28/extension-of-numerical-de-bounds-to-other-powersdimensions/3004
		const real power_factor = 0.48089834696298780245330822700063; // 1 / std::log(8.0);
#else
		const real power_factor = 1;
#endif

		const real de = koebe_factor * power_factor * de_base;

		if (std::isfinite(len_dr)) // TODO: this function is probably slow, find a replacement
		{
			// At some parts of the fractal, m can become NaN (hairs),
			// which pollutes everything downstream.
			// Calling code should deal with it.
			normal_os_out = normalise(dr);
			return de;
		}
		else
		{
			// The derivatives have overflowed to infinity
			// and then further operations on them yield NaN.
			// Assuming m is finite it might as well return 0 here.
			normal_os_out = vec3r{ 0,0,0 };
			return 0;
		}
#endif
	}

	// Get the distance estimate and normal vector for point p in object space
	virtual real getDE(const DualVec3r & p_os, vec3r & normal_os_out) const noexcept = 0;

	// Dual numbers provide exact normals as part of the evaluation
	virtual vec3r getNormal(const vec3r & p) const noexcept override final
	{
		const DualVec3r p_dual(Dual3r(p.x, 0), Dual3r(p.y, 1), Dual3r(p.z, 2));

		vec3r normal_os;
		const real de_ignored = getDE(p_dual, normal_os);
		(void) de_ignored;
		return normal_os;
	}

	virtual real intersect(const Ray & r) const noexcept override final
	{
		const vec3r s = r.o - centre;
		const real  b = dot(s, r.d);
		const real  c = dot(s, s) - radius * radius;

		const real discriminant = b * b - c;
		if (discriminant < 0)
			return -1;

		// Compute bounding interval
		const real t1 = -b - std::sqrt(discriminant);
		const real t2 = -b + std::sqrt(discriminant);
		if (t2 <= ray_epsilon) return -1;

		// Ray could be inside bounding sphere, start from ray epsilon
		real t = std::max(ray_epsilon, t1);
		while (t < t2)
		{
			const vec3r p_os = s + r.d * t;
			const DualVec3r p_os_dual(Dual3r(p_os.x, 0), Dual3r(p_os.y, 1), Dual3r(p_os.z, 2));
	
			vec3r normal_ignored;
			const real DE = getDE(p_os_dual, normal_ignored);
			(void) normal_ignored;
			t += DE;

			// If we're close enough to the surface, return a valid intersection
			if (DE < DE_thresh)
				return t;
		}

		return -1; // No intersection found
	}
};
