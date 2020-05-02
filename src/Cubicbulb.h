#pragma once

#include "AnalyticDEObject.h"
#include "DualDEObject.h"

/// <summary>
/// Cubicbulb formula
/// based on implementations by quasihedron and dark-beam 
/// </summary>
/// <remarks>
/// Details: https://www.deviantart.com/quasihedron/art/JIT-QH2017-CubicBulb-20170210-662776379
/// </remarks>
struct DualCubicbulbIteration final : public IterationFunction
{
	Dual3r YMul = 3.0;
	Dual3r ZMul = 3.0;
	Dual3r AuxMul = 1.0;
	DualVec3r c = { -0.5, -0.5, -0.25 };

	virtual void init(const DualVec3r& p_0) noexcept override final
	{
		//c = p_0;
	}

	virtual void eval(const DualVec3r& p_in, DualVec3r& p_out) const noexcept override final
	{
		p_out = DualVec3r(
			c.x + p_in.x * p_in.x * p_in.x - YMul * p_in.x * p_in.y * p_in.y - ZMul * p_in.x * p_in.z * p_in.z,
			c.y + -p_in.y * p_in.y * p_in.y + YMul * p_in.y * p_in.x * p_in.x - AuxMul * p_in.y * p_in.z * p_in.z,
			c.z + p_in.z * p_in.z * p_in.z - ZMul * p_in.z * p_in.x * p_in.x + AuxMul * p_in.z * p_in.y * p_in.y);
	}

	virtual IterationFunction* clone() const override
	{
		return new DualCubicbulbIteration(*this);
	}

};
