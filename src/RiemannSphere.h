#pragma once

#include "DualDEObject.h"



// Riemann Sphere formula by Msltoe
// Based on implementations in Mandelbulber and Mandelbulb3D
// Ref: http://www.fractalforums.com/theory/choosing-the-squaring-formula-by-location/
struct DualRiemannSphereIteration final : public IterationFunction
{
	real scale = 1;
	real s_shift = 0;
	real t_shift = 0;
	real x_shift = 1;
	real r_shift = -0.25f;
	real r_pow = 2;
	DualVec3r c = { 0, 0, 0 };
	//bool julia_mode = false;

	vec3r rot_m1 = { 1, 0, 0 };
	vec3r rot_m2 = { 0, 1, 0 };
	vec3r rot_m3 = { 0, 0, 1 };


	virtual void init(const DualVec3r & p_0) noexcept override final
	{
		//if (!julia_mode)
		//	c = p_0;
	}

	virtual void eval(const DualVec3r & p_in, DualVec3r & p_out) const noexcept override final
	{
		// Rotate
		DualVec3r p = DualVec3r(
			dot(p_in, rot_m1),
			dot(p_in, rot_m2),
			dot(p_in, rot_m3));

		const real r = length(p);
		p *= (scale / r);

		const real one_my = fabs(-p.y.v[0] + 1);
		Dual3r s, t;
		if (one_my > real(1e-5))
		{
			const Dual3r q = Dual3r(1) / (-p.y + 1);
			s = p.x * q;
			t = p.z * q;
		}
		else
		{
			s = p.x;
			t = p.z;
		}

		const Dual3r d = s * s + t * t + 1;
		s = fabs(sin(s * pi + s_shift));
		t = fabs(sin(t * pi + t_shift));
		s = fabs(s + x_shift);
		t = fabs(t + x_shift);

		const Dual3r r_ = Dual3r(-0.25f + r_shift) + pow(r, d.v[0] * r_pow);
		const Dual3r d_ = Dual3r(2) / d;

		p_out = DualVec3r(
			c.x + r_ * s * d_,
			c.y + r_ * (-d_ + 1),
			c.z + r_ * t * d_
		);
	}

	virtual real getPower() const noexcept override final { return r_pow; }

	virtual IterationFunction * clone() const override final
	{
		return new DualRiemannSphereIteration(*this);
	}
};
