#pragma once

#include "AnalyticDEObject.h"
#include "DualDEObject.h"



// knighty's distance estimator:
// http://www.fractalforums.com/sierpinski-gasket/kaleidoscopic-(escape-time-ifs)/
// simplified implementation:
// https://github.com/buddhi1980/mandelbulber2/blob/517423cc5b9ac960464cbcde612a0d8c61df3375/mandelbulber2/formula/definition/fractal_menger_sponge.cpp
struct MengerSpongeCAnalytic final : public AnalyticDEObject
{
	real scale = 3;
	vec3r scale_centre = { 1.0f, 1.0f, 1.0f };


	virtual real getDE(const vec3r & p_os) noexcept override final
	{
		vec3r z = p_os;
		real m = dot(z, z);
		real dz = 1;

		for (int i = 0; i < 10; i++)
		{
			z.x() = fabs(z.x());
			z.y() = fabs(z.y());
			z.z() = fabs(z.z());

			if (z.x() - z.y() < 0) std::swap(z.x(), z.y());
			if (z.x() - z.z() < 0) std::swap(z.x(), z.z());
			if (z.y() - z.z() < 0) std::swap(z.y(), z.z());

			real t = std::min((real)0, -z.z() + (real)0.5 * scale_centre.y() * (scale - 1) / scale);
			z.z() += 2 * t;

			z.x() = scale * z.x() - scale_centre.x() * (scale - 1);
			z.y() = scale * z.y() - scale_centre.y() * (scale - 1);
			z.z() = scale * z.z();

			dz *= scale;

			m = dot(z, z);
			if (m > 256)
				break;
		}

		// From a distance it looks like a sphere
		return (std::sqrt(m) - radius) / dz;
	}

	virtual SceneObject * clone() const override final
	{
		return new MengerSpongeCAnalytic(*this);
	}
};


struct MengerSpongeCDual final : public DualDEObject
{
	real scale = 3;
	vec3r scale_centre = { 1.0f, 1.0f, 1.0f };


	virtual real getDE(const DualVec3r & p_os, vec3r & normal_os_out) noexcept override final
	{
		DualVec3r z(p_os);

		for (int i = 0; i < 16; i++)
		{
			z.x() = fabs(z.x());
			z.y() = fabs(z.y());
			z.z() = fabs(z.z());

			if (z.x().v[0] - z.y().v[0] < 0) std::swap(z.x(), z.y());
			if (z.x().v[0] - z.z().v[0] < 0) std::swap(z.x(), z.z());
			if (z.y().v[0] - z.z().v[0] < 0) std::swap(z.y(), z.z());

			Dual3r t = min(Dual3r(0), -z.z() + Dual3r(0.5 * scale_centre.y() * (scale - 1) / scale));
			z.z() = z.z() + t * 2;

			z.x() *= scale; z.x() -= scale_centre.x() * (scale - 1);
			z.y() *= scale; z.y() -= scale_centre.y() * (scale - 1);
			z.z() *= scale;

			const real m = z.x().v[0] * z.x().v[0] + z.y().v[0] * z.y().v[0] + z.z().v[0] * z.z().v[0];
			if (m > 256)
				break;
		}

#if 0
		return getHybridDEClaude(3, 1, z, normal_os_out);
#else
		return getHybridDEKnighty(3, 1, z, normal_os_out);
#endif
	}

	virtual SceneObject * clone() const override final
	{
		return new MengerSpongeCDual(*this);
	}
};


struct DualMengerSpongeCIteration final : public IterationFunction
{
	real scale = 3;
	vec3r scale_centre = { 1.0f, 1.0f, 1.0f };


	virtual void eval(const DualVec3r & p_in, DualVec3r & p_out) const noexcept override final
	{
		DualVec3r z(
			fabs(p_in.x()),
			fabs(p_in.y()),
			fabs(p_in.z()));

		if (z.x().v[0] - z.y().v[0] < 0) std::swap(z.x(), z.y());
		if (z.x().v[0] - z.z().v[0] < 0) std::swap(z.x(), z.z());
		if (z.y().v[0] - z.z().v[0] < 0) std::swap(z.y(), z.z());

		Dual3r t = min(Dual3r(0), -z.z() + Dual3r(0.5 * scale_centre.y() * (scale - 1) / scale));
		z.z() = z.z() + t * 2;

		z.x() *= scale; z.x() -= scale_centre.x() * (scale - 1);
		z.y() *= scale; z.y() -= scale_centre.y() * (scale - 1);
		z.z() *= scale;

		p_out = z;
	}

	virtual real getPower() const noexcept override final { return 1; }

	virtual IterationFunction * clone() const override final
	{
		return new DualMengerSpongeCIteration(*this);
	}
};
