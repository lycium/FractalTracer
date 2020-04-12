#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

#include <stdint.h>
#include <stdlib.h>

#include <cmath> // For std::sqrt and so on
#include <vector>
#include <string>
#include <algorithm> // For std::pair and std::min and max

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "real.h" // Includes vec2, vec3 and Dual with float/double switching


#if USE_DOUBLE
constexpr static real ray_epsilon = 3e-9; // Ignore intersections closer than this to avoid self-intersection artifacts
constexpr static real DE_thresh = 2e-10; // Nearest intersection distance for DE objects
#else
constexpr static real ray_epsilon = 3e-4f;
constexpr static real DE_thresh = 2e-5f;
#endif



struct Ray
{
	vec3r o; // Origin
	vec3r d; // Direction normalised
};


// Base class for scene objects
struct Object
{
	virtual real  intersect(const Ray   & r) const noexcept = 0;
	virtual vec3r getNormal(const vec3r & p) const noexcept = 0;

	vec3f colour = { 0, 0, 0 };
};


// Simple sphere
struct Sphere final : public Object
{
	vec3r centre = { 0, 0, 0 };
	real  radius = 1; 


	virtual real intersect(const Ray & r) const noexcept override
	{
		const vec3r s = r.o - centre;
		const real  b = dot(s, r.d);
		const real  c = dot(s, s) - radius * radius;

		const real discriminant = b * b - c;
		if (discriminant < 0)
			return -1;

		// Compute both roots and return the nearest one that's > 0
		const real t1 = -b - std::sqrt(discriminant);
		const real t2 = -b + std::sqrt(discriminant);
		return (t1 >= 0) ? t1 : t2;
	}

	virtual vec3r getNormal(const vec3r & p) const noexcept override
	{
		return (p - centre) * (1 / radius);
	}
};


// Base class for distance estimated (DE) objects bounded by a sphere
struct DEObject : public Object
{
	vec3r centre = { 0, 0, 0 };
	real  radius = 1; 


	// Get the distance estimate for point p in object space
	virtual real getDE(const vec3r & p_os) const = 0;

	virtual real intersect(const Ray & r) const noexcept override final
	{
		const vec3r s = r.o - centre;
		const real  b = dot(s, r.d);
		const real  c = dot(s, s) - radius * radius;

		const real discriminant = b * b - c;
		if (discriminant < 0)
			return -1;

		// Compute bounding interval
		const real t1 = -b - std::sqrt(discriminant);
		const real t2 = -b + std::sqrt(discriminant);
		if (t2 <= ray_epsilon) return -1;

		// Ray could be inside bounding sphere, start from ray epsilon
		real t = std::max(ray_epsilon, t1);
		while (t < t2)
		{
			const vec3r p_os = s + r.d * t;
			const real DE = getDE(p_os);
			t += DE;

			// If we're close enough to the surface, return a valid intersection
			if (DE < DE_thresh)
				return t;
		}

		return -1; // No intersection found
	}

	virtual vec3r getNormal(const vec3r & p) const noexcept override final
	{
		const vec3r p_os = p - centre;
		const real  DE_s = 0.001f;
		const real  DE_0 = getDE(p_os);
		const vec3r grad =
		{
			getDE(p_os + vec3r{ DE_s, 0, 0}) - DE_0,
			getDE(p_os + vec3r{ 0, DE_s, 0}) - DE_0,
			getDE(p_os + vec3r{ 0, 0, DE_s}) - DE_0
		};
		return normalise(grad);
	}
};



#define USE_SCALAR_DERIV 0

struct QuadraticJuliabulb final : public DEObject
{
	virtual real getDE(const vec3r & p_os) const noexcept override final
	{
#if USE_SCALAR_DERIV
		const vec3r c = vec3r{ -1.1412f, 0.11f,  0.1513f } * 1.0f;
		vec3r z = p_os;
		real r, dr = 1;
		for (int i = 0; i < 64 * 8; i++)
		{
			const real xy_r2 = z.x * z.x + z.y * z.y;
			const real r2 = xy_r2 + z.z * z.z;
			const real scale = 1 - z.z * z.z / xy_r2;
			r = std::sqrt(r2);
			if (r > 256 * 64)
				break;

			dr = r * 2 * dr + 1;
			z = vec3r
			{
				(z.x * z.x - z.y * z.y) * scale + c.x,
				(2 * z.x * z.y) * scale + c.y,
				-2 * z.z * std::sqrt(xy_r2) + c.z
			};
		}

		return 0.0125f * std::log(r) * r / dr;
#else
		Dual3r zx(p_os.x, 0);
		Dual3r zy(p_os.y, 1);
		Dual3r zz(p_os.z, 2);

		const Dual3r cx(-1.1412f);
		const Dual3r cy( 0.11f);
		const Dual3r cz( 0.1513f);

		for (int i = 0; i < 64 * 8; i++)
		{
			const Dual3r xy_r2 = zx * zx + zy * zy;
			const Dual3r r2 = xy_r2 + zz * zz;
			const Dual3r scale = Dual3r(1) - zz * zz / xy_r2;

			const real len2 = zx.v[0] * zx.v[0] + zy.v[0] * zy.v[0] + zz.v[0] * zz.v[0];
			if (len2 > 65536 * 4096)
				break;

			const Dual3r zx_ = (zx * zx - zy * zy) * Dual3r(scale) + cx;
			const Dual3r zy_ = (Dual3r(2) * zx * zy) * Dual3r(scale) + cy;
			const Dual3r zz_ = Dual3r(-2) * zz * sqrt(xy_r2) + cz;

			zx = zx_;
			zy = zy_;
			zz = zz_;
		}

		const vec3r p  = vec3r{ zx.v[0], zy.v[0], zz.v[0] };
		const vec3r jx = vec3r{ zx.v[1], zy.v[1], zz.v[1] };
		const vec3r jy = vec3r{ zx.v[2], zy.v[2], zz.v[2] };
		const vec3r jz = vec3r{ zx.v[3], zy.v[3], zz.v[3] };
		const vec3r dr =
		{
			dot(p, jx),
			dot(p, jy),
			dot(p, jz)
		};
		return 0.05f * dot(p, p) / length(dr);
#endif
	}
};


// Inigo Quilez's distance estimator: https://www.iquilezles.org/www/articles/mandelbulb/mandelbulb.htm
struct Mandelbulb final : public DEObject
{
	virtual real getDE(const vec3r & p_os) const noexcept override final
	{
#if USE_SCALAR_DERIV
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
#else
		const Dual3r cx(p_os.x, 0);
		const Dual3r cy(p_os.y, 1);
		const Dual3r cz(p_os.z, 2);
		Dual3r wx = cx;
		Dual3r wy = cy;
		Dual3r wz = cz;

		for (int i = 0; i < 4; i++)
		{
			const Dual3r x = wx, x2 = x*x, x4 = x2*x2;
			const Dual3r y = wy, y2 = y*y, y4 = y2*y2;
			const Dual3r z = wz, z2 = z*z, z4 = z2*z2;

			const Dual3r k3 = x2 + z2;
			const Dual3r k2 = Dual3r(1) / sqrt(k3*k3*k3*k3*k3*k3*k3);
			const Dual3r k1 = x4 + y4 + z4 - Dual3r(6) * y2*z2 - Dual3r(6) * x2*y2 + Dual3r(2) * z2*x2;
			const Dual3r k4 = x2 - y2 + z2;

			wx = cx + Dual3r( 64) * x*y*z * (x2 - z2) * k4 * (x4 - Dual3r(6) * x2*z2+z4) * k1*k2;
			wy = cy + Dual3r(-16) * y2*k3*k4*k4 + k1*k1;
			wz = cz + Dual3r( -8) * y*k4 * (x4*x4 - Dual3r(28) * x4*x2*z2 + Dual3r(70) * x4*z4 - Dual3r(28) * x2*z2*z4 + z4*z4) * k1*k2;

			const real m = wx.v[0] * wx.v[0] + wy.v[0] * wy.v[0] + wz.v[0] * wz.v[0];

			// Computations above requires computing terms of size w^15.
			// The largest representable single precision float is less than 2^128.
			// (2 ** 127) ** (1 / 15) = 353.7698 =: R
			// So if m < R, the next iteration should not overflow
			if (m > 256)
				break;
		}

		// Extract the position vector and Jacobian
		const vec3r p  = vec3r{ wx.v[0], wy.v[0], wz.v[0] };
		const vec3r jx = vec3r{ wx.v[1], wy.v[1], wz.v[1] };
		const vec3r jy = vec3r{ wx.v[2], wy.v[2], wz.v[2] };
		const vec3r jz = vec3r{ wx.v[3], wy.v[3], wz.v[3] };

		const real len2 = dot(p, p);
		const real len = sqrt(len2);
		const vec3r u = p * (1 / len); // Normalise p first to avoid overflow in dot products

		// Vector-matrix norm: ||J||_u = |u.J|/|u|
		// Ref: https://fractalforums.org/fractal-image-gallery/18/burning-ship-distance-estimation/647/msg3207#msg3207
		const vec3r dr = vec3r
		{
			dot(u, jx),
			dot(u, jy),
			dot(u, jz)
		};

#if 0
		// Should work in theory, but unfortunately does not.
		// Ref: https://www.evl.uic.edu/hypercomplex/html/book/book.pdf chapter 9.6
		return 1.0f * len / length(dr);
#else
		// Modifications and explanations by claude to take into account polynomial-ness:
		const real len_dr = length(dr);

		// The basic DE formula has a log for functions that escape like |z_{n+k}| ~ |z_n|^{p^k}
		// Ref: http://linas.org/art-gallery/mderive/mderive.html
		const real de_base = len * std::log(len) / len_dr;

		// Koebe 1/4 theorem for complex-analytic functions implies that
		// the distance estimate is accurate up to a factor of 2.  Mandelbulb
		// is not complex-analytic (or even complex) but one might hope that
		// similar arguments could apply.  Divide by 2 to get the lower bound.
		const real koebe_factor = 0.5f;

#if 0
		// The distance estimate needs a log(power) factor.
		// Not sure if this is included in the derivative calculation.
		// Ref: https://fractalforums.org/fractal-mathematics-and-new-theories/28/extension-of-numerical-de-bounds-to-other-powersdimensions/3004
		const real power_factor = 0.48089834696298780245330822700063; // 1 / std::log(8.0);
#else
		const real power_factor = 1;
#endif

		// Arbitrary factor "just in case" (traditionally called "fudge factor").
		const real extra_factor = 1;
		const real de = koebe_factor * power_factor * extra_factor * de_base;

		if (std::isfinite(len_dr)) // TODO: this function is probably slow, find a replacement
		{
			// At some parts of the fractal, m can become NaN (hairs),
			// which pollutes everything downstream.
			// Calling code should deal with it.
			return de;
		}
		else
		{
			// The derivatives have overflowed to infinity
			// and then further operations on them yield NaN.
			// Assuming m is finite it might as well return 0 here.
			return 0;
		}
#endif

#endif
	}
};


class World
{
public:
	std::pair<const Object *, real> nearestIntersection(const Ray & r) const noexcept
	{
		const Object * nearest_obj = nullptr;
		real nearest_t = real_inf;

		for (const Object * const o : objects)
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


	std::vector<Object *> objects;
};


inline real sRGB(const real u)
{
	return (u <= 0.0031308f) ? 12.92f * u : 1.055f * std::pow(u, 0.416667f) - 0.055f;
}


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


inline vec3f generateColour(int x, int y, int frame, int pass, int width, int height, int frames, const World & world) noexcept
{
	const real aspect_ratio = width / (real)height;

	const real fov_deg = 80.0f;
	const real fov_rad = fov_deg * 3.14159265359f / 180; // Convert from degrees to radians
	const real sensor_width  = 2 * std::tan(fov_rad / 2);
	const real sensor_height = sensor_width / aspect_ratio;

	const vec3r cam_lookat = { 0, 0, 0 };
	const vec3r world_up = { 0, 1, 0 };

	const real hash_random    = uintToUnitReal(hash(y * width + x)); // Use pixel idx to randomise Halton sequence
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
	const real light_ln2 = dot(light_vec, light_vec);
	const real light_len = std::sqrt(light_ln2);
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


struct sRGBPixel
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
};


int main(int argc, char ** argv)
{
	std::vector<Sphere> spheres;
	{
		const real main_sphere_rad = 4;
		//Sphere s;
		//s.centre = { 0, 0, 0 };
		//s.radius = main_sphere_rad;
		//s.colour = { 0.1f, 0.3f, 0.7f };
		//spheres.push_back(s);

		Sphere s2;
		const real bigrad = 1024;
		s2.centre = { 0, -bigrad - main_sphere_rad, 0 };
		s2.radius = bigrad;
		s2.colour = { 0.6f, 0.3f, 0.2f };
		spheres.push_back(s2);
	}

	//QuadraticJuliabulb mandelbulb;
	Mandelbulb mandelbulb;
	mandelbulb.radius = 4;
	mandelbulb.colour = { 0.1f, 0.3f, 0.7f };

	// Set up the world
	World world;
	{
		for (const Sphere & s : spheres)
			world.objects.push_back((Object *)&s);

		world.objects.push_back((Object *)&mandelbulb);
	}

	const int image_multi  = 40;
	const int image_width  = image_multi * 16;
	const int image_height = image_multi * 9;
	const int frames = 30 * 8;
	const int passes = 2 * 3 * 5;
	printf("Rendering %d frames at resolution %d x %d with %d passes\n", frames, image_width, image_height, passes);


	std::vector<vec3f>     image_HDR(image_width * image_height);
	std::vector<sRGBPixel> image_LDR(image_width * image_height);

	for (int frame = 0; frame < frames; ++frame)
	{
		std::fill(image_HDR.begin(), image_HDR.end(), vec3f{ 0,0,0 });

		// Render image passes
		for (int pass = 0; pass < passes; ++pass)
		{
			#pragma omp parallel for schedule(dynamic, 1)
			for (int y = 0; y < image_height; y++)
			for (int x = 0; x < image_width;  x++)
				image_HDR[y * image_width + x] += generateColour(x, y, frame, pass, image_width, image_height, frames, world);
		}

		// Tonemap and convert to LDR sRGB
		#pragma omp parallel for
		for (int y = 0; y < image_height; y++)
		for (int x = 0; x < image_width;  x++)
		{
			const int pixel_idx = y * image_width + x;
			const float scale = 1.0f / passes;

			const vec3f pixel_colour = image_HDR[pixel_idx];
			image_LDR[pixel_idx] =
			{
				(uint8_t)std::max(0, std::min(255, (int)(sRGB(pixel_colour.x * scale) * 256))),
				(uint8_t)std::max(0, std::min(255, (int)(sRGB(pixel_colour.y * scale) * 256))),
				(uint8_t)std::max(0, std::min(255, (int)(sRGB(pixel_colour.z * scale) * 256)))
			};
		}

		// Save frame
		char filename[64];
		snprintf(filename, 64, "frame_%08d.png", frame);
		stbi_write_png(filename, image_width, image_height, 3, &image_LDR[0], image_width * 3);
		printf("Saved %s\n", filename);
	}

	return 0;
}
