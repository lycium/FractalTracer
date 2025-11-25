#pragma once

#include "scene_objects/AnalyticDEObject.h"
#include "scene_objects/DualDEObject.h"

#include "maths/quat.h"

DualVec4r hopf(const DualVec4r &z, int m)
{
  auto u = atan2(z.y(), z.x());
  auto v = atan2(z.w(), z.z());
  auto t = atan2(z.z() / cos(v), z.x() / cos(u));
  auto r2 = dot(z, z);
  u *= m;
  v *= m;
  t *= m;
  if (m != 2) r2 = pow(r2, 0.5f * m);
  return DualVec4r(cos(u)*cos(t), sin(u)*cos(t), cos(v)*sin(t), sin(v)*sin(t)) * r2;
}

struct Hopfbrot final : public DualDEObject
{
	virtual real getDE(const DualVec3r & p_os, vec3r & normal_os_out) noexcept override final
	{
		DualVec4r c(Dual4r(p_os.x()), Dual4r(p_os.y()), Dual4r(p_os.z()), 0);
		DualVec4r w(c);

		for (int i = 0; i < 64; i++)
		{
			const real m = length2(w);
			if (m > 256)
				break;
			w = hopf(w, 2) + c;
		}

		return getHybridDEClaude(1, 2, w, normal_os_out);
	}

	virtual SceneObject * clone() const override final
	{
		return new Hopfbrot(*this);
	}
};
