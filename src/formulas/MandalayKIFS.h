#pragma once

#include "scene_objects/DualDEObject.h"
#include "maths/quat.h"



// Mandalay-KIFS formula
// There are multiple mandalay variations, this one is based on dark-beam's kifs mandalay from mandelbulb3d.
// Ref: http://www.fractalforums.com/amazing-box-amazing-surf-and-variations/'new'-fractal-type-mandalay/msg81434/#msg81434
struct DualMandalayKIFSIteration final : public IterationFunction
{
	real scale = 3;
	real min_r2 = 0;
	real folding_offset = 1;
	real z_tower = 0;
	real xy_tower = 0;
	vec3r rotate = { 0, 0, 0 }; // Euler angles in radians
	vec3r julia_c = { 0, 0, 0 };
	bool julia_mode = true;

	virtual void init(const DualVec3r& p_0) noexcept override final
	{
		if (julia_mode)
			c = DualVec3r(julia_c.x(), julia_c.y(), julia_c.z());
		else
			c = p_0;

		rotation_quat = quat<Dual4r>::from_euler(rotate.x(), rotate.y(), rotate.z());
		rotation_conj = quat<Dual4r>(rotation_quat.v.x(), rotation_quat.v.y(), rotation_quat.v.z(), -rotation_quat.v.w());
    }

	virtual void eval(const DualVec3r & p_in, DualVec3r & p_out) const noexcept override final
	{
		DualVec3r p = p_in;

		if (fabs(rotate.x()) + fabs(rotate.y()) + fabs(rotate.z()) > 0)
		{
			// Rotate using quaternions
			const quat<Dual4r> p_quat(Dual4r(p_in.x()), Dual4r(p_in.y()), Dual4r(p_in.z()), Dual4r(0));
			const quat<Dual4r> qpq = (rotation_quat * p_quat) * rotation_conj;
			p = DualVec3r(qpq.v.x(), qpq.v.y(), qpq.v.z());
		}

		p = DualVec3r(fabs(p.x()), fabs(p.y()), fabs(p.z()));

		// Kifs Octahedral fold:
		if (p.y().v[0] > p.x().v[0]) p = DualVec3r(p.y(), p.x(), p.z());
		if (p.z().v[0] > p.y().v[0]) p = DualVec3r(p.x(), p.z(), p.y());
		if (p.y().v[0] > p.x().v[0]) p = DualVec3r(p.y(), p.x(), p.z());

		// ABoxKali-like abs folding
		const Dual3r fx = p.x() - folding_offset * 2;
		const Dual3r gy = p.y() + xy_tower;

		// Edge calculations
		const DualVec3r q0(
			folding_offset - fabs(p.x() - folding_offset),
			folding_offset - fabs(p.y() - folding_offset),
			z_tower > 0 ? z_tower - fabs(p.z() - folding_offset) : z_tower + p.z()
		);

		DualVec3r q = q0;

		if (fx.v[0] > 0 &&
			fx.v[0] > p.y().v[0])
		{
			if (fx.v[0] > gy.v[0]) // Top
			{
				q.x() = q.x() + xy_tower;
				q.y() = folding_offset - fabs(xy_tower - folding_offset + p.y());
			}
			else // Edge
			{
				q.x() = -p.y();
				q.y() = folding_offset - fabs(p.x() - 3 * folding_offset);
			}
		}

		p = q;

		// Sphere folding
		const real r2 = length2(p);
		const real fold_factor = (r2 < min_r2) ? min_r2 / r2 : 1;
		p = p * clamp(fold_factor, min_r2, 1);

		// Scale and translate
		p = p * scale + c;

		p_out = p;
	}

	virtual real getPower() const noexcept override final { return 1; }

	virtual IterationFunction * clone() const override final
	{
		return new DualMandalayKIFSIteration(*this);
	}

private:
	DualVec3r c = { 0, 0, 0 };
	quat<Dual4r> rotation_quat;
	quat<Dual4r> rotation_conj;
};
