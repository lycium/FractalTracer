#pragma once

#include <atomic>
#include <vector>
#include <algorithm>

#include "Scene.h"



struct ThreadControl
{
	const int num_passes;

	std::atomic<int> next_bucket = 0;
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
double RadicalInverse(int a, int base) noexcept
{
	const double invBase = 1.0 / base;

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


inline vec3f generateColour(int x, int y, int frame, int pass, int xres, int yres, int frames, Scene & scene) noexcept
{
	constexpr real two_pi = static_cast<real>(6.283185307179586476925286766559);
	constexpr int max_bounces = 3;
	constexpr int num_primes = 6;
	constexpr static int primes[num_primes] = { 2, 3, 5, 7, 11, 13 };

	const real aspect_ratio = xres / (real)yres;
	const real fov_deg = 80.0f;
	const real fov_rad = fov_deg * two_pi / 360; // Convert from degrees to radians
	const real sensor_width  = 2 * std::tan(fov_rad / 2);
	const real sensor_height = sensor_width / aspect_ratio;

	const vec3r cam_lookat = { 0, 0, 0 };
	const vec3r world_up = { 0, 1, 0 };

	const real hash_random    = uintToUnitReal(hash(frame * xres * yres + y * xres + x)); // Use pixel idx to randomise Halton sequence
	const real pixel_sample_x = wrap01((real)RadicalInverse(pass, primes[0]), hash_random);
	const real pixel_sample_y = wrap01((real)RadicalInverse(pass, primes[1]), hash_random);
	const real pixel_sample_t = wrap01((real)RadicalInverse(pass, primes[2]), hash_random);

	const real time  = (frames <= 0) ? 0 : two_pi * (frame + pixel_sample_t) / frames;
	const real cos_t = std::cos(time);
	const real sin_t = std::sin(time);

	const vec3r cam_pos = vec3r{ 4 * cos_t + 10 * sin_t, 5, -10 * cos_t + 4 * sin_t } * 0.25f;
	const vec3r cam_forward = normalise(cam_lookat - cam_pos);
	const vec3r cam_right = cross(world_up, cam_forward);
	const vec3r cam_up = cross(cam_forward, cam_right);

	const vec3r pixel_x = cam_right * (sensor_width / xres);
	const vec3r pixel_y = cam_up * -(sensor_height / yres);
	const vec3r pixel_v = cam_forward + (pixel_x * (x - xres * 0.5f + pixel_sample_x)) + (pixel_y * (y - yres * 0.5f + pixel_sample_y));

	Ray ray = { cam_pos, normalise(pixel_v) };
	vec3f contribution = 0;
	vec3f throughput = 1;
	int bounce = 0;
	while (true)
	{
		// Do intersection test
		const auto [nearest_hit_obj, nearest_hit_t] = scene.nearestIntersection(ray);

		// Did we hit anything? If not, return skylight colour
		if (nearest_hit_obj == nullptr)
		{
			const vec3f sky_up = vec3f{ 0.02f, 0.05f, 0.1f } * 2;
			const vec3f sky_hz = vec3f{ 0.05f, 0.07f, 0.1f } * 2;
			const vec3f sky = sky_hz + (sky_up - sky_hz) * std::max(static_cast<real>(0), ray.d.y);
			contribution += throughput * sky;
			break;
		}

		// Compute intersection position using returned nearest ray distance
		const vec3r hit_p = ray.o + ray.d * nearest_hit_t;

		// Get the normal at the intersction point from the surface we hit
		const vec3r normal = nearest_hit_obj->getNormal(hit_p);

		// Do direct lighting from a fixed point light
		{
			// Compute vector from intersection point to light
			const vec3r light_pos = { 8, 12, -6 };
			const vec3r light_vec = light_pos - hit_p;
			const real  light_ln2 = dot(light_vec, light_vec);
			const real  light_len = std::sqrt(light_ln2);
			const vec3r light_dir = light_vec * (1 / light_len);

			// Compute reflected light (simple diffuse / Lambertian) with 1/distance^2 falloff
			const real n_dot_l = dot(normal, light_dir);
			const vec3f refl_colour = nearest_hit_obj->colour * std::max(0.0f, (float)n_dot_l) / (float)light_ln2 * 420;

			// Trace shadow ray from the hit point towards the light
			const Ray shadow_ray = { hit_p, light_dir };
			const auto [shadow_nearest_hit_obj, shadow_nearest_hit_t] = scene.nearestIntersection(shadow_ray);

			// If we didn't hit anything (null hit obj or length >= length from hit point to light),
			//  add the directly reflected light to the path contribution
			if (shadow_nearest_hit_obj == nullptr || shadow_nearest_hit_t >= light_len)
				contribution += throughput * refl_colour;
		}

		if (++bounce > max_bounces)
			break;

		const real refl_sample_x = wrap01((real)RadicalInverse(pass, primes[(3 + bounce * 2 + 0) % num_primes]), hash_random);
		const real refl_sample_y = wrap01((real)RadicalInverse(pass, primes[(3 + bounce * 2 + 1) % num_primes]), hash_random);

		// Generate uniform point on sphere, see https://mathworld.wolfram.com/SpherePointPicking.html
		const real a = refl_sample_x * two_pi;
		const real s = 2 * std::sqrt(std::max(static_cast<real>(0), refl_sample_y * (1 - refl_sample_y)));
		const vec3r sphere =
		{
			std::cos(a) * s,
			std::sin(a) * s,
			1 - 2 * refl_sample_y
		};

		// Generate new cosine-weighted exitant direction
		const vec3r new_dir_cosine = normalise(normal + sphere);

		// Multiply the throughput by the surface reflection
		throughput *= nearest_hit_obj->colour;

		// Start next bounce from the hit position in the scattered ray direction
		ray.o = hit_p;
		ray.d = new_dir_cosine;
	}

	return contribution;
}


void renderThreadFunction(
	ThreadControl * const thread_control,
	vec3f * const image_HDR,
	int frame, int base_pass, int xres, int yres, int frames, const Scene * const scene_) noexcept
{
	// Make a local copy of the world for this thread, needed because it will get modified during init
	Scene scene = *scene_;

	// Get rounded up number of buckets in x and y
	constexpr int bucket_size = 32;
	const int x_buckets = (xres + bucket_size - 1) / bucket_size;
	const int y_buckets = (yres + bucket_size - 1) / bucket_size;
	const int num_buckets = x_buckets * y_buckets;
	const int num_passes = thread_control->num_passes;

	while (true)
	{
		// Get the next bucket index atomically and exit if we're done
		const int bucket = thread_control->next_bucket.fetch_add(1);
		if (bucket >= num_buckets * num_passes)
			break;

		// Get sub-pass and pixel ranges for current bucket
		const int sub_pass  = bucket / num_buckets;
		const int bucket_p  = bucket - num_buckets * sub_pass;
		const int bucket_y  = bucket_p / x_buckets;
		const int bucket_x  = bucket_p - x_buckets * bucket_y;
		const int bucket_x0 = bucket_x * bucket_size, bucket_x1 = std::min(bucket_x0 + bucket_size, xres);
		const int bucket_y0 = bucket_y * bucket_size, bucket_y1 = std::min(bucket_y0 + bucket_size, yres);

		for (int y = bucket_y0; y < bucket_y1; ++y)
		for (int x = bucket_x0; x < bucket_x1; ++x)
			image_HDR[y * xres + x] += generateColour(x, y, frame, base_pass + sub_pass, xres, yres, frames, scene);
	}
}
