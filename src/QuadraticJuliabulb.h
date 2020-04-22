#pragma once

#include "AnalyticDEObject.h"
#include "DualDEObject.h"



struct QuadraticJuliabulbAnalytic final : public AnalyticDEObject
{
	virtual real getDE(const vec3r & p_os) const noexcept override final
	{
		const vec3r c = vec3r{ -1.1412f, 0.11f,  0.1513f } * 1.0f;
		vec3r z = p_os;
		real r, dr = 1;
		for (int i = 0; i < 64 * 8; i++)
		{
			const real xy_r2 = z.x * z.x + z.y * z.y;
			const real scale = 1 - z.z * z.z / xy_r2;

			r = std::sqrt(xy_r2 + z.z * z.z);
			if (r > 256 * 64)
				break;

			dr = r * 2 * dr + 1;
			z = vec3r
			{
				(z.x * z.x - z.y * z.y) * scale + c.x,
				(2 * z.x * z.y) * scale + c.y,
				-2 * z.z * std::sqrt(xy_r2) + c.z
			};
		}

		return 0.125f * std::log(r) * r / dr;
	}
};


struct QuadraticJuliabulbDual final : public DualDEObject
{
	virtual real getDE(const DualVec3r & p_os, vec3r & normal_os_out) const noexcept override final
	{
		const DualVec3r c(-1.1412f, 0.11f, 0.1513f);
		DualVec3r z = p_os;

		for (int i = 0; i < 64 * 8; i++)
		{
			const Dual3r xy_r2 = z.x * z.x + z.y * z.y;
			const Dual3r scale = Dual3r(1) - z.z * z.z / xy_r2;

			const real r2 = xy_r2.v[0] + z.z.v[0] * z.z.v[0];
			if (r2 > 65536 * 4096)
				break;

			const Dual3r zx_ = (z.x * z.x - z.y * z.y) * Dual3r(scale) + c.x;
			const Dual3r zy_ = (Dual3r(2) * z.x * z.y) * Dual3r(scale) + c.y;
			const Dual3r zz_ = Dual3r(-2) * z.z * sqrt(xy_r2) + c.z;

			z = { zx_, zy_, zz_ };
		}

#if 1
		return getHybridDE(1, 2, z, normal_os_out);
#else
		return 0.125f * getPolynomialDE(z, normal_os_out);
#endif
	}
};
