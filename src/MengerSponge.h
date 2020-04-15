#pragma once

#include "AnalyticDEObject.h"
#include "DualDEObject.h"

using std::swap;

// knighty's distance estimator:
// http://www.fractalforums.com/sierpinski-gasket/kaleidoscopic-(escape-time-ifs)/
// simplified implementation:
// https://github.com/buddhi1980/mandelbulber2/blob/517423cc5b9ac960464cbcde612a0d8c61df3375/mandelbulber2/formula/definition/fractal_menger_sponge.cpp
struct MengerSpongeAnalytic final : public AnalyticDEObject
{
	virtual real getDE(const vec3r & p_os) const noexcept override final
	{
		vec3r z = p_os;
		real m = dot(z, z);
		real dz = 1;

		for (int i = 0; i < 16; i++)
		{

			z.x = fabs(z.x);
			z.y = fabs(z.y);
			z.z = fabs(z.z);

			if (z.x - z.y < 0.0) swap(z.x, z.y);
			if (z.x - z.z < 0.0) swap(z.x, z.z);
			if (z.y - z.z < 0.0) swap(z.y, z.z);

			z *= 3;
			dz *= 3;

			z.x -= 2.0;
			z.y -= 2.0;
			if (z.z > 1.0) z.z -= 2.0;

			m = dot(z, z);
			if (m > 256)
				break;
		}

		// from a distance it looks like a sphere
		return (sqrt(m) - radius) / dz;
	}
};


struct MengerSpongeDual final : public DualDEObject
{
	virtual real getDE(const DualVec3r & p_os, vec3r & normal_os_out) const noexcept override final
	{
		DualVec3r z(p_os);

		for (int i = 0; i < 16; i++)
		{
			z.x = fabs(z.x);
			z.y = fabs(z.y);
			z.z = fabs(z.z);

			if (z.x.v[0] - z.y.v[0] < 0.0) swap(z.x, z.y);
			if (z.x.v[0] - z.z.v[0] < 0.0) swap(z.x, z.z);
			if (z.y.v[0] - z.z.v[0] < 0.0) swap(z.y, z.z);

			z *= 3;

			z.x -= 2.0;
			z.y -= 2.0;
			if (z.z.v[0] > 1.0) z.z -= 2.0;

			const real m = z.x.v[0] * z.x.v[0] + z.y.v[0] * z.y.v[0] + z.z.v[0] * z.z.v[0];
			if (m > 256)
				break;
		}

		return getLinearDE(z, normal_os_out);
	}
};
