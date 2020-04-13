#pragma once

#include <vector>
#include <algorithm>

#include "SceneObject.h"



class Scene
{
public:
	std::pair<const SceneObject *, real> nearestIntersection(const Ray & r) const noexcept
	{
		const SceneObject * nearest_obj = nullptr;
		real nearest_t = real_inf;

		for (const SceneObject * const o : objects)
		{
			const real hit_t = o->intersect(r);
			if (hit_t > ray_epsilon && hit_t < nearest_t)
			{
				nearest_obj = o;
				nearest_t = hit_t;
			}
		}

		return { nearest_obj, nearest_t };
	}


	std::vector<SceneObject *> objects;
};


// Hash function by Thomas Wang: https://burtleburtle.net/bob/hash/integer.html
inline uint32_t hash(uint32_t x)
{
	x  = (x ^ 12345391) * 2654435769;
	x ^= (x << 6) ^ (x >> 26);
	x *= 2654435769;
	x += (x << 5) ^ (x >> 12);

	return x;
}


// From PBRT
template<int base>
double RadicalInverse(int a) noexcept
{
	constexpr double invBase = 1.0 / base;
	int reversedDigits = 0;
	double invBaseN = 1;
	while (a)
	{
		const int next  = a / base;
		const int digit = a - base * next;
		reversedDigits = reversedDigits * base + digit;
		invBaseN *= invBase;
		a = next;
	}
	return std::min(reversedDigits * invBaseN, DoubleOneMinusEpsilon);
}


inline real uintToUnitReal(uint32_t v)
{
#if USE_DOUBLE
	constexpr double uint32_double_scale = 1.0 / (1ull << 32);
	return v * uint32_double_scale;
#else
	// Trick from MTGP: generate an uniformly distributed single precision number in [1,2) and subtract 1
	union
	{
		uint32_t u;
		float f;
	} x;
	x.u = (v >> 9) | 0x3f800000u;
	return x.f - 1.0f;
#endif
}


inline real wrap01(real u, real v) { return (u + v < 1) ? u + v : u + v - 1; }


inline vec3f generateColour(int x, int y, int frame, int pass, int width, int height, int frames, const Scene & world) noexcept
{
	const real aspect_ratio = width / (real)height;

	const real fov_deg = 80.0f;
	const real fov_rad = fov_deg * 3.14159265359f / 180; // Convert from degrees to radians
	const real sensor_width  = 2 * std::tan(fov_rad / 2);
	const real sensor_height = sensor_width / aspect_ratio;

	const vec3r cam_lookat = { 0, 0, 0 };
	const vec3r world_up = { 0, 1, 0 };

	const real hash_random    = uintToUnitReal(hash(frame * width * height + y * width + x)); // Use pixel idx to randomise Halton sequence
	const real pixel_sample_x = wrap01((real)RadicalInverse<2>(pass), hash_random);
	const real pixel_sample_y = wrap01((real)RadicalInverse<3>(pass), hash_random);
	const real pixel_sample_t = wrap01((real)RadicalInverse<5>(pass), hash_random);

	const real time  = 2 * 3.14159265359f * (frame + pixel_sample_t) / frames;
	const real cos_t = std::cos(time);
	const real sin_t = std::sin(time);

	const vec3r cam_pos = vec3r{ 4 * cos_t + 10 * sin_t, 5, -10 * cos_t + 4 * sin_t } * 0.25f;
	const vec3r cam_forward = normalise(cam_lookat - cam_pos);
	const vec3r cam_right = cross(world_up, cam_forward);
	const vec3r cam_up = cross(cam_forward, cam_right);

	const vec3r pixel_x = cam_right * (sensor_width / width);
	const vec3r pixel_y = cam_up * -(sensor_height / height);
	const vec3r pixel_v = cam_forward + (pixel_x * (x - width * 0.5f + pixel_sample_x)) + (pixel_y * (y - height * 0.5f + pixel_sample_y));

	const Ray camera_ray = { cam_pos, normalise(pixel_v) };

	// Do intersection test
	const auto [nearest_hit_obj, nearest_hit_t] = world.nearestIntersection(camera_ray);

	// Did we hit anything? If not, immediately return black
	if (nearest_hit_obj == nullptr)
	{
		const real background = 0.0f;
		return { background, background, background };
	}

	// Compute intersection position using returned nearest ray distance
	const vec3r hit_p = camera_ray.o + camera_ray.d * nearest_hit_t;

	// Get the normal at the intersction point from the surface we hit
	const vec3r normal = nearest_hit_obj->getNormal(hit_p);

	// Compute vector from intersection point to light
	const vec3r light_pos = { 8, 12, -6 };
	const vec3r light_vec = light_pos - hit_p;
	const real  light_ln2 = dot(light_vec, light_vec);
	const real  light_len = std::sqrt(light_ln2);
	const vec3r light_dir = light_vec * (1 / light_len);

	// Compute reflected light (simple diffuse / Lambertian) with 1/distance^2 falloff
	const real n_dot_l = dot(normal, light_dir);
	const vec3f refl_colour = nearest_hit_obj->colour * std::max(0.0f, (float)n_dot_l) / (float)light_ln2 * 512;

	// Trace shadow ray from the hit point towards the light
	const Ray shadow_ray = { hit_p, light_dir };
	const auto [shadow_nearest_hit_obj, shadow_nearest_hit_t] = world.nearestIntersection(shadow_ray);

	// If we hit anything (valid hit idx and length <= length from hit point to light),
	//  we are in shadow, and no light is reflected
	if (shadow_nearest_hit_obj != nullptr && shadow_nearest_hit_t <= light_len)
		return { 0, 0, 0 };

	return refl_colour;
}
