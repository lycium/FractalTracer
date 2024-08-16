#pragma once

#include "scene_objects/DualDEObject.h"

// Lambdabulb formula
// Based on modifications by machina-infinitum
// Ref.: https://www.fractalforums.com/mandelbulb-renderings/lambdabulb/
struct DualLambdabulbIteration final : public IterationFunction
{
	real power = 4;//2-5 should look good
	real phase = 1.815142;
	DualVec3r c = { 1.035f, -0.317f, 0.013f };

	virtual void init(const DualVec3r& p_0) noexcept override final
	{

	}

	virtual void eval(const DualVec3r& p_in, DualVec3r& p_out) const noexcept override final
	{
		DualVec3r zp = triplexPow(p_in, power, phase);
		DualVec3r z_minus_zp = p_in - zp;
		p_out = triplexMult(c, z_minus_zp);
	}

	virtual real getPower() const noexcept override final { return power; }

	virtual IterationFunction* clone() const override final
	{
		return new DualLambdabulbIteration(*this);
	}

protected:
	inline DualVec3r triplexPow(const DualVec3r& z, const real& power, const Dual3r& phase) const
	{
		const Dual3r r = length(z);
		const Dual3r theta = atan2(z.y(), z.x());
		const Dual3r phi = acos(z.z() / r);

		const Dual3r r_p = pow(r, power);
		const Dual3r theta_p = theta * power;
		const Dual3r phi_p = phase + phi * power;

		return DualVec3r(
			sin(phi_p) * cos(theta_p),
			sin(phi_p) * sin(theta_p),
			cos(phi_p)
		) * r_p;
	}

	inline DualVec3r triplexMult(const DualVec3r& z1, const DualVec3r& z2) const
	{
		const Dual3r r1 = length(z1);
		const Dual3r r2 = length(z2);
		const Dual3r a = Dual3r(1) - (z1.z() * z2.z()) / (r1 * r2);
		return DualVec3r(
			a * (z1.x() * z2.x() - z1.y() * z2.y()),
			a * (z2.x() * z1.y() + z1.x() * z2.y()),
			r2 * z1.z() + r1 * z2.z()
		);
	}
};
