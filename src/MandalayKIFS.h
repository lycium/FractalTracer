#pragma once

#include "DualDEObject.h"



// Mandalay-KIFS formula
// There are multiple mandalay variations, this one is based on dark-beam's kifs mandalay from mandelbulb3d.
// Ref: http://www.fractalforums.com/amazing-box-amazing-surf-and-variations/'new'-fractal-type-mandalay/msg81434/#msg81434
struct DualMandalayKIFSIteration final : public IterationFunction
{
	real scale = 2;
	real min_r2 = 0;
	real fix_r2 = 1;
	real fold = 1;
	real xy_tower = 1;
	real z_tower = 0;
	vec3r rot_m1 = { 1, 0, 0 };
	vec3r rot_m2 = { 0, 1, 0 };
	vec3r rot_m3 = { 0, 0, 1 };
	DualVec3r c  = { 0, 0, 0 };
	bool julia_mode = true;


	virtual void init(const DualVec3r & p_0) noexcept override final
	{
		if (!julia_mode)
			c = p_0;
	}

	virtual void eval(const DualVec3r & p_in, DualVec3r & p_out) const noexcept override final
	{
		// Rotate
		DualVec3r p = DualVec3r(
			dot(p_in, rot_m1),
			dot(p_in, rot_m2),
			dot(p_in, rot_m3));

		p = DualVec3r(fabs(p.x), fabs(p.y), fabs(p.z));

		// Octahedral fold
		if (p.y.v[0] > p.x.v[0]) p = DualVec3r(p.y, p.x, p.z);
		if (p.z.v[0] > p.y.v[0]) p = DualVec3r(p.x, p.z, p.y);
		if (p.y.v[0] > p.x.v[0]) p = DualVec3r(p.y, p.x, p.z);

		// ABoxKali-like abs folding
		const Dual3r fx = p.x + fold * -2;

		// Edges
		const DualVec3r q0(
			-fabs(p.x - fold) + fold,
			-fabs(p.y - fold) + fold,
			((z_tower > 0) ? -fabs(p.z - fold) : p.z) + z_tower);

		const Dual3r g  = xy_tower;
		const Dual3r gy = g + p.y;

		DualVec3r q = q0;
		if (fx.v[0] > 0 && fx.v[0] > p.y.v[0])
		{
			if (fx.v[0] > gy.v[0])
			{
				// Top
				q.x = q.x + g;
				q.y = -fabs(g - fold + p.y) + fold;
			}
			else
			{
				// Edges
				q.x = -p.y;
				q.y = -fabs(p.x + fold * -3) + fold;
			}
		}
		p = q;

		sphereFold(p);

		p = p * scale + c;

		p_out = p;
	}

	virtual real getPower() const noexcept override final { return 1; }

	virtual IterationFunction * clone() const override final
	{
		return new DualMandalayKIFSIteration(*this);
	}

private:
	inline DualVec3r sphereFold(const DualVec3r & p_in) const
	{
		const real r2 = length2(p_in);
		return
			(r2 < min_r2) ? p_in * (fix_r2 / min_r2) : // linear inner scaling
			(r2 < fix_r2) ? p_in * (fix_r2 / r2) : // this is the actual sphere inversion
			p_in;
	}
};
