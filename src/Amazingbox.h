#pragma once

#include "DualDEObject.h"



// Amazingbox (aka Mandelbox) formula by Tglad
// Ref: http://www.fractalforums.com/amazing-box-amazing-surf-and-variations/amazing-fractal/
struct DualAmazingboxIteration final : public IterationFunction
{
	real scale = 3;
	real min_r2 = 0.5f;
	real fix_r2 = 1;
	real fold_limit = 1;
	DualVec3r c = { 2.5f, -0.7f, 2.15f };
	bool julia_mode = false;


	virtual void init(const DualVec3r & p_0) noexcept override final
	{
		if (!julia_mode)
			c = p_0;
	}

	virtual void eval(const DualVec3r & p_in, DualVec3r & p_out) const noexcept override final
	{
		DualVec3r p = p_in;
		p = boxFold(p);
		p = sphereFold(p);
		p = p * scale + c;

		p_out = p;
	}

	virtual real getPower() const noexcept override final { return 1; } // Knighty: Well... the DE formula for this fractal doesn't have a log()

	virtual IterationFunction * clone() const override
	{
		return new DualAmazingboxIteration(*this);
	}

protected:
	inline DualVec3r boxFold(const DualVec3r & p_in) const
	{
		return DualVec3r(
			clamp(p_in.x, -fold_limit, fold_limit) * 2 - p_in.x,
			clamp(p_in.y, -fold_limit, fold_limit) * 2 - p_in.y,
			clamp(p_in.z, -fold_limit, fold_limit) * 2 - p_in.z);
	}

	inline DualVec3r sphereFold(const DualVec3r & p_in) const
	{
		const real r2 = p_in.x.v[0] * p_in.x.v[0] + p_in.y.v[0] * p_in.y.v[0] + p_in.z.v[0] * p_in.z.v[0];
		return
			(r2 < min_r2) ? p_in * (fix_r2 / min_r2) : // linear inner scaling
			(r2 < fix_r2) ? p_in * (fix_r2 / r2) : // this is the actual sphere inversion
			p_in;
	}
};
