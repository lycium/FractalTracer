#pragma once

#include "DualDEObject.h"



// Sphere Tree formula (WIP, doesn't work yet) by Tglad
// Ref: https://fractalforums.org/fractal-mathematics-and-new-theories/28/new-sphere-tree/3557/
struct DualSphereTreeIteration final : public IterationFunction
{
	// These should be static / constexpr...
	const real rad = 0.5f;
	const DualVec3r s0 = DualVec3r(0, 1, rad);
	const DualVec3r s1 = DualVec3r( sqrt(3.0) / 2, -0.5, rad);
	const DualVec3r s2 = DualVec3r(-sqrt(3.0) / 2, -0.5, rad);
	const DualVec3r t0 = DualVec3r(0, 1, 0);
	const DualVec3r t1 = DualVec3r( sqrt(3.0) / 2, -0.5, 0);
	const DualVec3r t2 = DualVec3r(-sqrt(3.0) / 2, -0.5, 0);
	const DualVec3r n0 = DualVec3r(1.0,0.0,0.0);
	const DualVec3r n1 = DualVec3r(-0.5, -sqrt(3.0) / 2, 0);
	const DualVec3r n2 = DualVec3r(-0.5,  sqrt(3.0) / 2, 0);
	const real inner_scale = sqrt(3.0) / (1 + sqrt(3.0));

	DualVec3r p0;

	virtual void init(const DualVec3r & p_0) noexcept override final
	{
		p0 = p_0;
	}

	virtual void eval(const DualVec3r & p_in, DualVec3r & p_out) const noexcept override final
	{
		// Change of coordinate system to Z+ up
		DualVec3r p(p_in.x(), p_in.z(), p_in.y());

		// Get a vector without the derivatives for faster distance computations
		const vec3r p_vec3 = vec3r(p.x().v[0], p.y().v[0], p.z().v[0]);

		//if (length2(p_vec3 - vec3r(0, 0, inner_scale * 0.5)) < inner_scale * inner_scale * 0.25)
		if (length(p_vec3 - vec3r(0, 0, inner_scale * 0.5)) < inner_scale * 0.5)
		{
			p_out = p_in;
			return; // Definitely inside
		}

		// Since we don't have access to the iteration count, we compare the input point to the first point (from init)
		const bool first_iter =
			p_in.x().v[0] == p0.x().v[0] &&
			p_in.y().v[0] == p0.y().v[0] &&
			p_in.z().v[0] == p0.z().v[0];
		const real maxH = (first_iter) ? -100 : 0.4;

		if (p_vec3.z() > maxH && length(p_vec3 - vec3r(0, 0, 0.5 * 1.1)) > 0.5 * 1.1)
		{
			p_out = p_in;
			return; // Definitely outside
		}

		// Sphere inversion
		p /= length2(p_vec3);
		if (p_vec3.z() >= maxH || length(p_vec3 - vec3r(0, 0, 0.5)) < 0.5)
		{
			// Stretch onto a plane at zero
			p.z() -= 1;
			p.z() = -p.z();
			p *= sqrt(3.0);
			p.z() += 1;

			// Rotate it a twelfth of a revolution
			constexpr real a = pi / 6;
			const Dual3r xx = p.x() *  cos(a) + p.y() * sin(a);
			const Dual3r yy = p.x() * -sin(a) + p.y() * cos(a);
			p.x() = xx; 
			p.y() = yy;
		}

		// Now modolu the space so we move to being in just the central hexagon, inner radius 0.5
		const Dual3r h = p.z();
		Dual3r x = dot(p, -n2) * 2 / sqrt(3.0);
		Dual3r y = dot(p, -n1) * 2 / sqrt(3.0);
		x = fmod(x, real(1));
		y = fmod(y, real(1));
		if (x.v[0] + y.v[0] > 1)
		{
			x = Dual3r(1) - x;
			y = Dual3r(1) - y;
		}
		p = t1 * x - t2 * y;

		// Fold the space to be in a kite
		const Dual3r l0 = dot(p, p);
		const Dual3r l1 = dot(p - t1, p - t1);
		const Dual3r l2 = dot(p + t2, p + t2);
		     if (l1.v[0] < l0.v[0] && l1.v[0] < l2.v[0]) p -= t1 * (dot(t1, p) * 2 - 1);
		else if (l2.v[0] < l0.v[0] && l2.v[0] < l1.v[0]) p -= t2 * (dot(p, t2) * 2 + 1);
		p.z() = h;

		// Un-rotate back to Y+ up
		p_out = { p.x(), p.z(), p.y() };
	}

	virtual real getPower() const noexcept override final { return 1; } // Knighty: Well... the DE formula for this fractal doesn't have a log()

	virtual IterationFunction * clone() const override final
	{
		return new DualSphereTreeIteration(*this);
	}
};
