#pragma once

#include "scene_objects/AnalyticDEObject.h"
#include "scene_objects/DualDEObject.h"



struct QuadraticJuliabulbAnalytic final : public AnalyticDEObject
{
	virtual real getDE(const vec3r & p_os) noexcept override final
	{
		const vec3r c = vec3r{ -1.1412f, 0.11f,  0.1513f } * 1.0f;
		vec3r z = p_os;
		real r, dr = 1;
		for (int i = 0; i < 64 * 8; i++)
		{
			const real xy_r2 = z.x() * z.x() + z.y() * z.y();
			const real scale = 1 - z.z() * z.z() / xy_r2;

			r = std::sqrt(xy_r2 + z.z() * z.z());
			if (r > 256 * 64)
				break;

			dr = r * 2 * dr + 1;
			z = vec3r
			{
				(z.x() * z.x() - z.y() * z.y()) * scale,
				(2 * z.x() * z.y()) * scale,
				-2 * z.z() * std::sqrt(xy_r2)
			} + c;
		}

		return 0.125f * std::log(r) * r / dr;
	}

	virtual SceneObject * clone() const override final
	{
		return new QuadraticJuliabulbAnalytic(*this);
	}
};


struct QuadraticJuliabulbDual final : public DualDEObject
{
	virtual real getDE(const DualVec3r & p_os, vec3r & normal_os_out) noexcept override final
	{
		const DualVec3r c(-1.1412f, 0.11f, 0.1513f);
		DualVec3r z = p_os;

		for (int i = 0; i < 64 * 8; i++)
		{
			const Dual3r xy_r2 = z.x() * z.x() + z.y() * z.y();
			const Dual3r scale = Dual3r(1) - z.z() * z.z() / xy_r2;

			const real r2 = xy_r2.v[0] + z.z().v[0] * z.z().v[0];
			if (r2 > 65536 * 4096)
				break;

			z =
			{
				(z.x() * z.x() - z.y() * z.y()) * Dual3r(scale) + c.x(),
				(Dual3r(2) * z.x() * z.y()) * Dual3r(scale) + c.y(),
				Dual3r(-2) * z.z() * sqrt(xy_r2) + c.z()
			};
		}

#if 1
		return getHybridDEClaude(1, 2, z, normal_os_out);
#else
		return 0.125f * getPolynomialDE(z, normal_os_out);
#endif
	}

	virtual SceneObject * clone() const override final
	{
		return new QuadraticJuliabulbDual(*this);
	}
};
