#pragma once

#include "DualDEObject.h"



// BenesiPine2 formula by M Benesi
// Based on Mandelbulb3D and Fragmentarium implementations
struct DualBenesiPine2Iteration final : public IterationFunction
{
	real scale = 2.5f;
	real offset = 0.75f;
	DualVec3r c = { 0.0f, 0.0f, 0.0f };
	bool julia_mode = true;


	virtual void init(const DualVec3r & p_0) noexcept override final
	{
		if (!julia_mode)
			c = p_0;
	}

	virtual void eval(const DualVec3r & p_in, DualVec3r & p_out) const noexcept override final
	{
		DualVec3r p = p_in;

		// Benesi fold transform 2
		Dual3r tx = p.x * sqrt_2_3 - p.z * sqrt_1_3;
		p.z = p.x * sqrt_1_3 + p.z * sqrt_2_3;
		p.x = tx * sqrt_1_2 - p.y * sqrt_1_2;
		p.y = tx * sqrt_1_2 + p.y * sqrt_1_2;
		p = DualVec3r(
			fabs(sqrt(p.y * p.y + p.z * p.z) - offset),
			fabs(sqrt(p.x * p.x + p.z * p.z) - offset),
			fabs(sqrt(p.x * p.x + p.y * p.y) - offset)
		);
		p = p * scale;
		tx = p.x * sqrt_1_2 + p.y * sqrt_1_2;
		p.y = -p.x * sqrt_1_2 + p.y * sqrt_1_2;
		p.x = tx * sqrt_2_3 + p.z * sqrt_1_3;
		p.z = -tx * sqrt_1_3 + p.z * sqrt_2_3;

		// Benesi pinetree
		Dual3r xt = p.x * p.x; 
		Dual3r yt = p.y * p.y; 
		Dual3r zt = p.z * p.z;
		Dual3r t = p.x / sqrt(yt + zt) * real(2);
		p_out = DualVec3r(
			c.x + xt - yt - zt,
			c.y + t * (yt - zt),
			c.z + t * p.y * p.z * real(2));
	}

	virtual real getPower() const noexcept override final { return 2; }

	virtual IterationFunction * clone() const override
	{
		return new DualBenesiPine2Iteration(*this);
	}

protected:
	const real sqrt_2_3 = 0.81649658092;//sqrt(2/3)
	const real sqrt_1_3 = 0.57735026919;//sqrt(1/3)
	const real sqrt_1_2 = 0.70710678118;//sqrt(1/2)
};
