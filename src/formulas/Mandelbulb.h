#pragma once

#include "scene_objects/AnalyticDEObject.h"
#include "scene_objects/DualDEObject.h"

#include "maths/triplex.h"



// Inigo Quilez's power 8 Mandelbulb distance estimator
// Ref: https://www.iquilezles.org/www/articles/mandelbulb/mandelbulb.htm
struct MandelbulbAnalytic final : public AnalyticDEObject
{
	virtual real getDE(const vec3r & p_os) noexcept override final
	{
		vec3r w = p_os;
		real m = dot(w, w);
		real dz = 1;

		for (int i = 0; i < 4; i++)
		{
			const real m2 = m * m;
			const real m4 = m2 * m2;
			dz = 8 * std::sqrt(m4 * m2 * m) * dz + 1;

			const real x = w.x(), x2 = x*x, x4 = x2*x2;
			const real y = w.y(), y2 = y*y, y4 = y2*y2;
			const real z = w.z(), z2 = z*z, z4 = z2*z2;

			const real k3 = x2 + z2;
			const real k2 = 1 / std::sqrt(k3*k3*k3*k3*k3*k3*k3);
			const real k1 = x4 + y4 + z4 - 6*y2*z2 - 6*x2*y2 + 2*z2*x2;
			const real k4 = x2 - y2 + z2;

			w.x() = p_os.x() +  64 * x*y*z*(x2-z2)*k4*(x4 - 6 * x2*z2+z4)*k1*k2;
			w.y() = p_os.y() + -16 * y2*k3*k4*k4 + k1*k1;
			w.z() = p_os.z() +  -8 * y*k4*(x4*x4 - 28 * x4*x2*z2 + 70 * x4*z4 - 28 * x2*z2*z4 + z4*z4) * k1*k2;

			m = dot(w, w);
			if (m > 256)
				break;
		}

		return 0.25f * std::log(m) * std::sqrt(m) / dz;// * 0.25f;
	}

	virtual SceneObject * clone() const override
	{
		return new MandelbulbAnalytic(*this);
	}
};


struct MandelbulbDual final : public DualDEObject
{
	virtual real getDE(const DualVec3r & p_os, vec3r & normal_os_out) noexcept override final
	{
		DualVec3r w = p_os;

		for (int i = 0; i < 4; i++)
		{
			const Dual3r x = w.x(), x2 = x*x, x4 = x2*x2;
			const Dual3r y = w.y(), y2 = y*y, y4 = y2*y2;
			const Dual3r z = w.z(), z2 = z*z, z4 = z2*z2;

			const Dual3r k3 = x2 + z2;
			const Dual3r k2 = Dual3r(1) / sqrt(k3*k3*k3*k3*k3*k3*k3);
			const Dual3r k1 = x4 + y4 + z4 - y2*z2 * 6 - x2*y2 * 6 + z2*x2 * 2;
			const Dual3r k4 = x2 - y2 + z2;

			w.x() = p_os.x() + x*y*z * 64 * (x2 - z2) * k4 * (x4 - x2*z2 * 6 + z4) * k1*k2;
			w.y() = p_os.y() + y2*k3*k4*k4 * -16 + k1*k1;
			w.z() = p_os.z() + y*k4 * (x4*x4 - x4*x2*z2 * 28 + x4*z4 * 70 - x2*z2*z4 * 28 + z4*z4) * k1*k2 * -8;

			const real m = length2(w);
			if (m > 256)
				break;
		}

#if 1
		return getHybridDEClaude(1, 8, w, normal_os_out);
#else
		return getPolynomialDE(w, normal_os_out);
#endif
	}

	virtual SceneObject * clone() const override final
	{
		return new MandelbulbDual(*this);
	}
};


struct DualMandelbulbIteration final : public IterationFunction
{
	virtual void init(const DualVec3r & p_0) noexcept override final
	{
		c = p_0;
	}

	virtual void eval(const DualVec3r & p_in, DualVec3r & p_out) const noexcept override final
	{
		const Dual3r x = p_in.x(), x2 = x*x, x4 = x2*x2;
		const Dual3r y = p_in.y(), y2 = y*y, y4 = y2*y2;
		const Dual3r z = p_in.z(), z2 = z*z, z4 = z2*z2;

		const Dual3r k3 = x2 + z2;
		const Dual3r k2 = Dual3r(1) / sqrt(k3*k3*k3*k3*k3*k3*k3);
		const Dual3r k1 = x4 + y4 + z4 - y2*z2 * 6 - x2*y2 * 6 + z2*x2 * 2;
		const Dual3r k4 = x2 - y2 + z2;

		p_out = DualVec3r(
			c.x() + x*y*z * 64 * (x2 - z2) * k4 * (x4 - x2*z2 * 6 + z4) * k1*k2,
			c.y() + y2*k3*k4*k4 * -16 + k1*k1,
			c.z() + y*k4 * (x4*x4 - x4*x2*z2 * 28 + x4*z4 * 70 - x2*z2*z4 * 28 + z4*z4) * k1*k2 * -8);
	}

	virtual real getPower() const noexcept override final { return 8; }

	virtual IterationFunction * clone() const override final
	{
		return new DualMandelbulbIteration(*this);
	}

private:
	DualVec3r c;
};


struct DualTriplexMandelbulbIteration final : public IterationFunction
{
	virtual void init(const DualVec3r & p_0) noexcept override final
	{
		// Change of coordinate system to Z+ up
		const DualTriplex3r z(p_0.x(), p_0.z(), p_0.y());

		c = z;
	}

	virtual void eval(const DualVec3r & p_in, DualVec3r & p_out) const noexcept override final
	{
		// Change of coordinate system to Z+ up
		const DualTriplex3r z(p_in.x(), p_in.z(), p_in.y());

		const DualTriplex3r z_ = sqr(sqr(sqr(z))) + c;

		// Un-rotate back to Y+ up
		p_out = { z_.x(), z_.z(), z_.y() };
	}

	virtual real getPower() const noexcept override final { return 8; }

	virtual IterationFunction * clone() const override final
	{
		return new DualTriplexMandelbulbIteration(*this);
	}

private:
	DualTriplex3r c;
};
