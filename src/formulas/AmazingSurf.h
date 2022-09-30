#pragma once

#include "scene_objects/DualDEObject.h"



struct DualAmazingSurfIteration final : public IterationFunction
{
	bool julia_mode = false;
	real scale = 2;
	real min_r2 = 1.0f;
	real fold_limit = 1;
	DualVec3r rot_m1 = { 1, 0.2, 0 };
	DualVec3r rot_m2 = { 0.3, 1.2, 0 };
	DualVec3r rot_m3 = { 0.1, -0.5, 0.7 };
	DualVec3r c = { 0.0f, 0.0f, 0.0f };


	virtual void init(const DualVec3r& p_0) noexcept override final
	{
		if (!julia_mode)
			c = p_0;
		fold3 = DualVec3r(fold_limit);
	}

	virtual void eval(const DualVec3r& p_in, DualVec3r& p_out) const noexcept override final
	{
		DualVec3r p = p_in;

		//fold
		p = TGladFold(p);
		p.z() = p_in.z();//z is not folded

		//radius
		real rr = length2(p);
		real m;
		if (rr < min_r2)
		{
			m = scale / min_r2;
		}
		else if (rr < 1.0f)
		{
			m = scale / rr;
		}
		else
		{
			m = scale;
		}
		p = p * m + c;

		// Rotate
		p = DualVec3r(
			dot(p, rot_m1),
			dot(p, rot_m2),
			dot(p, rot_m3));

		p_out = p;
	}

	virtual real getPower() const noexcept override final { return 1; }

	virtual IterationFunction* clone() const override final
	{
		return new DualAmazingSurfIteration(*this);
	}

protected:
	DualVec3r fold3;

	inline DualVec3r TGladFold(const DualVec3r& p_in) const
	{
		return fabs(p_in + fold3) - fabs(p_in - fold3) - p_in;
	}

};
