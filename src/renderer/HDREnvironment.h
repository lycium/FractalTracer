#pragma once

#include <vector>
#include <cmath>
#include <algorithm>

#include "maths/vec.h"


struct HDREnvironment
{
	int xres = 0;
	int yres = 0;
	std::vector<vec3f> data;

	bool isLoaded() const noexcept { return !data.empty(); }

	vec3f sample(const vec3r & direction) const noexcept
	{
		const real theta = std::atan2(direction.z(), direction.x()) + two_pi;
		const real phi = std::acos(std::max((real)-1, std::min((real)1, direction.y())));

		const real u = std::fmod((theta + pi_half) * (xres / two_pi), (real)xres);
		const real v = std::fmod((phi + pi) * (yres / pi), (real)yres);

		const int u0 = (int)std::floor(u), u1 = (u0 + 1 < xres) ? u0 + 1 : 0;
		const int v0 = (int)std::floor(v), v1 = (v0 + 1 < yres) ? v0 + 1 : 0;
		const float fu = (float)(u - u0);
		const float fv = (float)(v - v0);

		return
			data[v0 * xres + u0] * ((1 - fu) * (1 - fv)) +
			data[v0 * xres + u1] * ((    fu) * (1 - fv)) +
			data[v1 * xres + u0] * ((1 - fu) * (    fv)) +
			data[v1 * xres + u1] * ((    fu) * (    fv));
	}
};
