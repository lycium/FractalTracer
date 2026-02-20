#pragma once

#include "scene_objects/AnalyticDEObject.h"
#include "scene_objects/DualDEObject.h"

#include "maths/quat.h"

/*
Mandelbulb, Mandelbrot, Mandelring and Hopfbrot
Oliver Knill
<https://doi.org/10.48550/arXiv.2305.17848>
<https://arxiv.org/abs/2305.17848>
*/

DualVec4r hopfTrig(const DualVec4r &z, int m)
{
	auto u = atan2(z.y(), z.x());
	auto v = atan2(z.w(), z.z());
	auto t = atan2(z.z() / cos(v), z.x() / cos(u));
	auto r2 = dot(z, z);
	u *= m;
	v *= m;
	t *= m;
	if (m != 2) r2 = pow(r2, real(0.5f * m));
	return DualVec4r(cos(u)*cos(t), sin(u)*cos(t), cos(v)*sin(t), sin(v)*sin(t)) * r2;
}

DualVec4r hopfPoly2(const DualVec4r &p)
{
	// cos(2 * atan2(y, x)) = (x^2 - y^2) / (x^2 + y^2)
	// sin(2 * atan2(y, x)) = 2 * x * y / (x^2 + y^2)
	auto x = p.x(), y = p.y(), z = p.z(), w = p.w();
	auto x2 = x * x, y2 = y * y, z2 = z * z, w2 = w * w;
	auto x2y2 = x2 + y2, z2w2 = z2 + w2;
	auto cos2u = (x2 - y2) / x2y2;
	auto sin2u = real(2) * x * y / x2y2;
	auto cos2v = (z2 - w2) / z2w2;
	auto sin2v = real(2) * z * w / z2w2;
	auto r2cos2t = x2y2 - z2w2;
	auto r2sin2t = real(2) * sqrt(x2y2 * z2w2);
	return DualVec4r(r2cos2t * cos2u, r2cos2t * sin2u, r2sin2t * cos2v, r2sin2t * sin2v);
}

DualVec4r hopf(const DualVec4r &z, int m)
{
	if (m == 2)
	{
		return hopfPoly2(z);
	}
	else
	{
		return hopfTrig(z, m);
	}
}

struct Hopfbrot final : public DualDEObject
{
	virtual real getDE(const DualVec3r & p_os, vec3r & normal_os_out) noexcept override final
	{
		const int p = 2;
		real q = 1;
		DualVec4r c(Dual4r(p_os.x()), Dual4r(p_os.y()), Dual4r(p_os.z()), Dual4r(0, 3));
		DualVec4r w(c);

		for (int i = 0; i < 65536; i++)
		{
			const real m = length2(w);
			if (m > bailout_radius2)
				return getHybridDEKnighty(p, q, w, normal_os_out);
			w = hopf(w, p) + c;
			q *= p;
		}
		// fprintf(stderr, "U");
		return getHybridDEKnighty(p, q, w, normal_os_out);
	}

	virtual SceneObject * clone() const override final
	{
		return new Hopfbrot(*this);
	}
};
