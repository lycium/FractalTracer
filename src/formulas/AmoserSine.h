#pragma once

#include "scene_objects/DualDEObject.h"



// Amoser Sine formula by amoser
// 3D extension of the complex sine function, doubly periodic
struct DualAmoserSineIteration final : public IterationFunction
{
	real scale = 2;
	vec3r julia_c = { 0, 0, 0 };
	bool julia_mode = false;

private:
	DualVec3r c;

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
			sin(p_in.x()) * cosh(p_in.y()),
			cos(p_in.x()) * cos(p_in.z()) * sinh(p_in.y()),
			sin(p_in.z()) * cosh(p_in.y()));

		p_out = p_out * scale + c;
	}

	virtual real getPower() const noexcept override final { return 1; }

	virtual IterationFunction * clone() const override final
	{
		return new DualAmoserSineIteration(*this);
	}
};
