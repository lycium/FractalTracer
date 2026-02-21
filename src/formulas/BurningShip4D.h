#pragma once

#include "scene_objects/AnalyticDEObject.h"
#include "scene_objects/DualDEObject.h"

#include "maths/quat.h"

struct BurningShip4D final : public DualDEObject
{
	virtual real getDE(const DualVec3r & p_os, vec3r & normal_os_out) noexcept override final
	{
		DualQuat4r c(Dual4r(p_os.x()), Dual4r(p_os.y()), Dual4r(p_os.z()), 0);
		DualQuat4r w(c);

		for (int i = 0; i < 64; i++)
		{
			const real m = length2(w);
			if (m > 256)
				break;
			w = sqr(fabs(w)) + c;
		}

		return getHybridDEClaude(1, 2, w.v, normal_os_out);
	}

	virtual SceneObject * clone() const override final
	{
		return new BurningShip4D(*this);
	}
};
