#pragma once

#include "scene_objects/DualDEObject.h"



struct DualPoincareIteration final : public IterationFunction
{
	real P = 7;
	real Q = 4;

	virtual void init(const DualVec3r& p_0) noexcept override final
	{
		real x = cos(pi/P);
		real y = cos(pi/Q);
		real r2 = 1.0 / (x * x + y * y - 1.0);
		real r = sqrt(r2);
		x = r * x;
		y = r * y;
		T = DualVec3r(x, y, r);
		c = p_0;
	}

	virtual void eval(const DualVec3r& p_in, DualVec3r& p_out) const noexcept override final
	{
		DualVec3r p = p_in;
		int folds1 = p.x().v[0] < 0.0 ? 1 : 0 + p.y().v[0] < 0.0 ? 1 : 0;
		p = fabs(p);
		DualVec3r a = DualVec3r(T.x(), T.y(), 0.0);
		Dual3r k = dot(p - a, p - a);
		Dual3r r2 = T.z() * T.z();
		if (k.v[0] < r2.v[0]) {
			folds1++;
			Dual3r zmul = r2 / k;
			//scl *= zmul;
			p = (p - a) * zmul + a;
		}

		p_out = p + c;
	}

	virtual real getPower() const noexcept override final { return 1; }

	virtual IterationFunction* clone() const override final
	{
		return new DualPoincareIteration(*this);
	}

protected:
	DualVec3r c = { 0.0f, 0.0f, 0.0f };
	DualVec3r T = { 0.0f, 0.0f, 0.0f };


};
