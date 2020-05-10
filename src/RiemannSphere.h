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
	DualVec3r c = { 0.0f, 0.0f, 0.0f };
	//bool julia_mode = false;

	//TODO: implement mat3x3
	DualVec3r m1 = { 1.0f, 0.0f, 0.0f };
	DualVec3r m2 = { 0.0f, 1.0f, 0.0f };
	DualVec3r m3 = { 0.0f, 0.0f, 1.0f };

	virtual void init(const DualVec3r& p_0) noexcept override final
	{
		//if (!julia_mode)
		//	c = p_0;
	}

	virtual void eval(const DualVec3r& p_in, DualVec3r& p_out) const noexcept override final
	{
		DualVec3r p;
		//rotate
		p = p_in * m1 + p_in * m2 + p_in * m3;

		Dual3r s, t;
		Dual3r r = length(p);
		p = p * scale / r;

		real one_my = fabs(real(1) - p.y.v[0]);
		if (one_my > real(1e-5))
		{
			Dual3r q = Dual3r(1) / (Dual3r(1) - p.y);
			s = p.x * q;
			t = p.z * q;
		}
		else
		{
			s = p.x;
			t = p.z;
		}

		Dual3r d = Dual3r(1) + (s * s + t * t);
		s = fabs(sin(s * pi + s_shift));
		t = fabs(sin(t * pi + t_shift));
		s = fabs(s + x_shift);
		t = fabs(t + x_shift);
		r = Dual3r(-0.25 + r_shift) + pow(r, d.v[0] * r_pow);
		d = Dual3r(2) / d;

		p_out = DualVec3r(
			c.x + r * s * d,
			c.y + r * (Dual3r(1) - d),
			c.z + r * t * d
		);
	}

	virtual real getPower() const noexcept override final { return r_pow; }

	virtual IterationFunction* clone() const override
	{
		return new DualRiemannSphereIteration(*this);
	}
};
