#pragma once

#include "DualDEObject.h"



// Cubicbulb formula
// Based on implementations by quasihedron and dark-beam 
// Ref: https://www.deviantart.com/quasihedron/art/JIT-QH2017-CubicBulb-20170210-662776379
struct DualCubicbulbIteration final : public IterationFunction
{
	real y_mul = 3;
	real z_mul = 3;
	real aux_mul = 1;
	DualVec3r c = { -0.5f, -0.5f, -0.25f };
	bool julia_mode = true;


	virtual void init(const DualVec3r & p_0) noexcept override final
	{
		if (!julia_mode)
			c = p_0;
	}

	virtual void eval(const DualVec3r & p_in, DualVec3r & p_out) const noexcept override final
	{
		p_out = DualVec3r(
			 c.x + p_in.x * p_in.x * p_in.x - p_in.x * p_in.y * p_in.y * y_mul - p_in.x * p_in.z * p_in.z * z_mul,
			 c.y - p_in.y * p_in.y * p_in.y + p_in.y * p_in.x * p_in.x * y_mul - p_in.y * p_in.z * p_in.z * aux_mul,
			 c.z + p_in.z * p_in.z * p_in.z - p_in.z * p_in.x * p_in.x * z_mul + p_in.z * p_in.y * p_in.y * aux_mul);
	}

	virtual real getPower() const noexcept override final
	{ return 3;}

	virtual IterationFunction * clone() const override
	{
		return new DualCubicbulbIteration(*this);
	}
};
