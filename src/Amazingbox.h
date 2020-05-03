#pragma once

#include "DualDEObject.h"

// Amazingbox (aka Mandelbox) formula by Tglad
// Ref: http://www.fractalforums.com/amazing-box-amazing-surf-and-variations/amazing-fractal/
struct DualAmazingboxIteration final : public IterationFunction
{
	real scale = 3;
	real min_r2 = 0.5;
	real fix_r2 = 1;
	real fold_limit = 1;
	DualVec3r c = { 2.5f, 0.0f, 2.5f };
	bool julia_mode = false;

	virtual void init(const DualVec3r& p_0) noexcept override final
	{
		if(!julia_mode)
			c = p_0;
	}

	virtual void eval(const DualVec3r& p_in, DualVec3r& p_out) const noexcept override final
	{
		DualVec3r p = p_in;
		p = boxFold(p);
		p = sphereFold(p);
		p = p * scale + c;
		p_out = p;
	}

	virtual IterationFunction* clone() const override
	{
		return new DualAmazingboxIteration(*this);
	}

protected:

	inline DualVec3r boxFold(DualVec3r& const p_in) const
	{
		DualVec3r p_out = DualVec3r(
			clamp(p_in.x, -fold_limit, fold_limit) * 2.0 - p_in.x,
			clamp(p_in.y, -fold_limit, fold_limit) * 2.0 - p_in.y,
			clamp(p_in.z, -fold_limit, fold_limit) * 2.0 - p_in.z);
		return p_out;
	}

	inline DualVec3r sphereFold(DualVec3r& const p_in) const
	{
		real r2 = dot(p_in, p_in).v[0];
		if (r2 < min_r2)
			return p_in * (fix_r2 / min_r2);//linear inner scaling
		else if (r2 < fix_r2)
			return p_in * (fix_r2 / r2);//this is the actual sphere inversion
		else
			return p_in;
	}

};
