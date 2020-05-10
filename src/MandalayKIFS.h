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
	Dual3r fold = 1;
	Dual3r xy_tower = 1;
	Dual3r z_tower = 0;
	DualVec3r rot_m1 = DualVec3r{ 1.0f, 0.0f, 0.0f };
	DualVec3r rot_m2 = DualVec3r{ 0.0f, 1.0f, 0.0f };
	DualVec3r rot_m3 = DualVec3r{ 0.0f, 0.0f, 1.0f };
	DualVec3r c = { 0.0f, 0.0f, 0.0f };
	bool julia_mode = true;


	virtual void init(const DualVec3r & p_0) noexcept override final
	{
		if (!julia_mode)
			c = p_0;
	}

	virtual void eval(const DualVec3r & p_in, DualVec3r & p_out) const noexcept override final
	{
		DualVec3r p = p_in;

		// Rotate
		p = p * rot_m1 + p * rot_m2 + p * rot_m3;
		
		p = DualVec3r(fabs(p.x), fabs(p.y), fabs(p.z));

		// Octahedral fold
		if (p.y.v[0] > p.x.v[0])
			p = DualVec3r(p.y, p.x, p.z);
		if (p.z.v[0] > p.y.v[0])
			p = DualVec3r(p.x, p.z, p.y);
		if (p.y.v[0] > p.x.v[0])
			p = DualVec3r(p.y, p.x, p.z);

		// ABoxKali-like abs folding
		Dual3r fx = fold * -2 + p.x;

		// Edges
		DualVec3r q;
		q.x = fold - fabs(-fold + p.x);
		q.y = fold - fabs(-fold + p.y);
		if (z_tower.v[0] > 0)
			q.z = z_tower - fabs(-fold + p.z);
		else
			q.z = z_tower + p.z;

		Dual3r g = xy_tower;
		Dual3r gy = g + p.y;
		if (fx.v[0] > 0 && fx.v[0] > p.y.v[0])
		{
			if (fx.v[0] > gy.v[0])
			{
				// Top
				q.x = q.x + g;
				q.y = fold - fabs(g - fold + p.y);
			}
			else
			{
				// Edges
				q.x = -p.y;
				q.y = fold - fabs(fold * -3 + p.x);
			}
		}
		p = q;

		//f (i < ColorIterations) 
		//	orbitTrap = min(orbitTrap, abs(vec4(p.xyz, r2)));

		sphereFold(p);

		p = p * scale + c;

		p_out = p;
	}

	virtual real getPower() const noexcept override final { return 1; }

	virtual IterationFunction * clone() const override
	{
		return new DualMandalayKIFSIteration(*this);
	}

protected:
	inline DualVec3r sphereFold(const DualVec3r & p_in) const
	{
		const real r2 = p_in.x.v[0] * p_in.x.v[0] + p_in.y.v[0] * p_in.y.v[0] + p_in.z.v[0] * p_in.z.v[0];
		return
			(r2 < min_r2) ? p_in * (fix_r2 / min_r2) : // linear inner scaling
			(r2 < fix_r2) ? p_in * (fix_r2 / r2) : // this is the actual sphere inversion
			p_in;
	}
};
