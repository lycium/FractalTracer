#pragma once

#include "scene_objects/DualDEObject.h"



// Octopus formula by Aexion (September 15, 2013)
// Ref: http://www.fractalforums.com/mandelbulb-3d/custom-formulas-and-transforms-release-t17106/msg65751/#msg65751
struct DualOctopusIteration final : public IterationFunction
{
	real xz_mul = 1.25f;
	real sq_mul = 1;
	vec3r julia_c = { 0, 0, 0 };
	bool julia_mode = true;

private:
	DualVec3r c = { 0.0f, 0.0f, 0.0f };

public:
	virtual void init(const DualVec3r & p_0) noexcept override final
	{
		if (julia_mode)
			c = DualVec3r(julia_c.x(), julia_c.y(), julia_c.z());
		else
			c = p_0;
	}

	virtual void eval(const DualVec3r & p_in, DualVec3r & p_out) const noexcept override final
	{
		p_out = DualVec3r(
			c.x() - p_in.x() * p_in.z() * xz_mul,
			c.y() - (p_in.x() * p_in.x() - p_in.z() * p_in.z()) * sq_mul,
			c.z() + p_in.y());
	}

	virtual real getPower() const noexcept override final { return 2; }

	virtual IterationFunction * clone() const override final
	{
		return new DualOctopusIteration(*this);
	}

	virtual std::vector<ParamInfo> getParams() override
	{
		return {
			{ "XZ Mul",     ParamInfo::Real, &xz_mul,     0.0f, 3.0f },
			{ "Sq Mul",     ParamInfo::Real, &sq_mul,     0.0f, 3.0f },
			{ "Julia C",    ParamInfo::Vec3r, &julia_c },
			{ "Julia Mode", ParamInfo::Bool, &julia_mode },
		};
	}
};
