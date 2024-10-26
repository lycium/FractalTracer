#pragma once

#include "scene_objects/DualDEObject.h"



// Lambdabulb formula
// Based on modifications by machina-infinitum
// Ref.: https://www.fractalforums.com/mandelbulb-renderings/lambdabulb/
struct DualLambdabulbIteration final : public IterationFunction
{
	const real phase = 1.815142;
	const real sin_phase = sin(phase);
	const real cos_phase = cos(phase);

	real power = 4; // 2-5 should look good
	DualVec3r c = { 1.035f, -0.317f, 0.013f };


	virtual void init(const DualVec3r & p_0) noexcept override final
	{

	}

	virtual void eval(const DualVec3r& p_in, DualVec3r& p_out) const noexcept override final
	{
		DualVec3r zp = triplexPow(p_in, power, phase);
		DualVec3r z_minus_zp = p_in - zp;
		p_out = triplexMult(c, z_minus_zp);
	}

	virtual real getPower() const noexcept override final { return power; }

	virtual IterationFunction * clone() const override final
	{
		return new DualLambdabulbIteration(*this);
	}

protected:
	inline DualVec3r triplexPow(const DualVec3r& z, const real & power, const Dual3r & phase) const
	{
#if 0
		// original trig version, arbitrary power
		const Dual3r r     = length(z);
		const Dual3r theta = atan2(z.y(), z.x());
		const Dual3r phi   = acos(z.z() / r);
		const Dual3r r_p     = pow(r, power);
		const Dual3r theta_p = theta * power;
		const Dual3r phi_p   = phase + phi * power;

		return DualVec3r(
			sin(phi_p) * cos(theta_p),
			sin(phi_p) * sin(theta_p),
			cos(phi_p)
		) * r_p;
#else
		// polynomial version, hardcoded to power 4

		// wxmaxima: factor(trigexpand(cos(4*atan2(y,x)))) rsp. sin
		const auto x2     = z.x() * z.x();
		const auto y2     = z.y() * z.y();
		const auto xy2    = z.x() * z.y() * 2;
		const auto y2mx2  = y2 - x2;
		const auto y2px2  = y2 + x2;
		const auto y2px22 = y2px2 * y2px2;
		const auto cos_theta = (y2mx2 - xy2) * (y2mx2 + xy2) / y2px22;
		const auto sin_theta = (xy2 * y2mx2) / y2px22 * -2;

		// wxmaxima: at(factor(expand(trigexpand(factor(trigexpand(cos(phase + 4 * acos(z / sqrt(x^2+y^2+z^2)))))))), [cos(phase) = c, sin(phase) = s]); rsp sin
		// (c*z^4-4*s*sqrt(y^2+x^2)*z^3-6*c*y^2*z^2-6*c*x^2*z^2+4*s*y^2*sqrt(y^2+x^2)*z+4*s*x^2*sqrt(y^2+x^2)*z+c*y^4+2*c*x^2*y^2+c*x^4)/(z^2+y^2+x^2)^2
		const auto z2 = z.z() * z.z();
		const auto z22 = z2 * z2;
		const auto a = z22 + y2px22 - real(6) * (y2px2 * z2);
		const auto b = sqrt(y2px2) * z.z() * (z2 - y2px2);
		// don't need to caclulate r^4 only to divide and multiply by it again
		//const auto r2 = x2 + y2px2;
		//const auto r22 = r2 * r2;
		const auto cos_phi = (cos_phase * a - (4 * sin_phase) * b);// / r22;
		const auto sin_phi = (sin_phase * a + (4 * cos_phase) * b);// / r22;

		return DualVec3r(
			sin_phi * cos_theta,
			sin_phi * sin_theta,
			cos_phi
		);// * r22;
#endif
	}

	inline DualVec3r triplexMult(const DualVec3r & z1, const DualVec3r & z2) const
	{
		const Dual3r r1 = length(z1);
		const Dual3r r2 = length(z2);
		const Dual3r a = real(1) - ((z1.z() * z2.z()) / (r1 * r2));
		return DualVec3r(
			a * (z1.x() * z2.x() - z1.y() * z2.y()),
			a * (z2.x() * z1.y() + z1.x() * z2.y()),
			r2 * z1.z() + r1 * z2.z()
		);
	}
};
