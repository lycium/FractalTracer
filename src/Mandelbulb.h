#pragma once

#include "AnalyticDEObject.h"
#include "DualDEObject.h"



// Inigo Quilez's distance estimator: https://www.iquilezles.org/www/articles/mandelbulb/mandelbulb.htm
struct MandelbulbAnalytic final : public AnalyticDEObject
{
	virtual real getDE(const vec3r & p_os) const noexcept override final
	{
		vec3r w = p_os;
		real m = dot(w, w);
		real dz = 1;

		for (int i = 0; i < 4; i++)
		{
			const real m2 = m * m;
			const real m4 = m2 * m2;
			dz = 8 * sqrt(m4 * m2 * m) * dz + 1;

			const real x = w.x, x2 = x*x, x4 = x2*x2;
			const real y = w.y, y2 = y*y, y4 = y2*y2;
			const real z = w.z, z2 = z*z, z4 = z2*z2;

			const real k3 = x2 + z2;
			const real k2 = 1 / sqrt(k3*k3*k3*k3*k3*k3*k3);
			const real k1 = x4 + y4 + z4 - 6*y2*z2 - 6*x2*y2 + 2*z2*x2;
			const real k4 = x2 - y2 + z2;

			w.x = p_os.x +  64 * x*y*z*(x2-z2)*k4*(x4 - 6 * x2*z2+z4)*k1*k2;
			w.y = p_os.y + -16 * y2*k3*k4*k4 + k1*k1;
			w.z = p_os.z +  -8 * y*k4*(x4*x4 - 28 * x4*x2*z2 + 70 * x4*z4 - 28 * x2*z2*z4 + z4*z4) * k1*k2;

			m = dot(w, w);
			if (m > 256)
				break;
		}

		return 0.25f * log(m) * sqrt(m) / dz;
	}
};


struct MandelbulbDual final : public DualDEObject
{
	virtual real getDE(const DualVec3r & p_os, vec3r & normal_os_out) const noexcept override final
	{
		const DualVec3r c(p_os);
		DualVec3r w = c;

		for (int i = 0; i < 4; i++)
		{
			const Dual3r x = w.x, x2 = x*x, x4 = x2*x2;
			const Dual3r y = w.y, y2 = y*y, y4 = y2*y2;
			const Dual3r z = w.z, z2 = z*z, z4 = z2*z2;

			const Dual3r k3 = x2 + z2;
			const Dual3r k2 = Dual3r(1) / sqrt(k3*k3*k3*k3*k3*k3*k3);
			const Dual3r k1 = x4 + y4 + z4 - Dual3r(6) * y2*z2 - Dual3r(6) * x2*y2 + Dual3r(2) * z2*x2;
			const Dual3r k4 = x2 - y2 + z2;

			w.x = c.x + Dual3r( 64) * x*y*z * (x2 - z2) * k4 * (x4 - Dual3r(6) * x2*z2+z4) * k1*k2;
			w.y = c.y + Dual3r(-16) * y2*k3*k4*k4 + k1*k1;
			w.z = c.z + Dual3r( -8) * y*k4 * (x4*x4 - Dual3r(28) * x4*x2*z2 + Dual3r(70) * x4*z4 - Dual3r(28) * x2*z2*z4 + z4*z4) * k1*k2;

			const real m = w.x.v[0] * w.x.v[0] + w.y.v[0] * w.y.v[0] + w.z.v[0] * w.z.v[0];

			// Computations above requires computing terms of size w^15.
			// The largest representable single precision float is less than 2^128.
			// (2 ** 127) ** (1 / 15) = 353.7698 =: R
			// So if m < R, the next iteration should not overflow
			if (m > 256)
				break;
		}

		return getPolynomialDE(w, normal_os_out);
	}
};
