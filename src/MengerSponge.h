#pragma once

#include "AnalyticDEObject.h"
#include "DualDEObject.h"



// knighty's distance estimator:
// http://www.fractalforums.com/sierpinski-gasket/kaleidoscopic-(escape-time-ifs)/
// simplified implementation:
// https://github.com/buddhi1980/mandelbulber2/blob/517423cc5b9ac960464cbcde612a0d8c61df3375/mandelbulber2/formula/definition/fractal_menger_sponge.cpp
struct MengerSpongeAnalytic final : public AnalyticDEObject
{
	virtual real getDE(const vec3r & p_os) noexcept override final
	{
		vec3r z = p_os;
		real m = dot(z, z);
		real dz = 1;

		for (int i = 0; i < 16; i++)
		{
			z.x = fabs(z.x);
			z.y = fabs(z.y);
			z.z = fabs(z.z);

			if (z.x - z.y < 0) std::swap(z.x, z.y);
			if (z.x - z.z < 0) std::swap(z.x, z.z);
			if (z.y - z.z < 0) std::swap(z.y, z.z);

			z  *= 3;
			dz *= 3;

			z.x -= 2;
			z.y -= 2;
			if (z.z > 1) z.z -= 2;

			m = dot(z, z);
			if (m > 256)
				break;
		}

		// from a distance it looks like a sphere
		return (sqrt(m) - radius) / dz;
	}

	virtual SceneObject * clone() const override
	{
		return new MengerSpongeAnalytic(*this);
	}
};


struct MengerSpongeDual final : public DualDEObject
{
	virtual real getDE(const DualVec3r & p_os, vec3r & normal_os_out) noexcept override final
	{
		DualVec3r z(p_os);

		for (int i = 0; i < 16; i++)
		{
			z.x = fabs(z.x);
			z.y = fabs(z.y);
			z.z = fabs(z.z);

			if (z.x.v[0] - z.y.v[0] < 0) std::swap(z.x, z.y);
			if (z.x.v[0] - z.z.v[0] < 0) std::swap(z.x, z.z);
			if (z.y.v[0] - z.z.v[0] < 0) std::swap(z.y, z.z);

			z *= 3;

			z.x -= 2;
			z.y -= 2;
			if (z.z.v[0] > 1) z.z -= 2;

			const real m = z.x.v[0] * z.x.v[0] + z.y.v[0] * z.y.v[0] + z.z.v[0] * z.z.v[0];
			if (m > 256)
				break;
		}

#if 1
		return getHybridDE(3, 1, z, normal_os_out);
#else
		return getLinearDE(z, normal_os_out);
#endif
	}

	virtual SceneObject * clone() const override
	{
		return new MengerSpongeDual(*this);
	}
};
