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

#include "Dual.h"



struct sRGBPixel
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
};


struct vec3f
{
	float x, y, z;

	inline vec3f operator+(const vec3f & rhs) const { return { x + rhs.x, y + rhs.y, z + rhs.z }; }
	inline vec3f operator-(const vec3f & rhs) const { return { x - rhs.x, y - rhs.y, z - rhs.z }; }

	inline void operator+=(const vec3f & rhs) { x += rhs.x; y += rhs.y; z += rhs.z; }
	inline void operator-=(const vec3f & rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; }

	inline vec3f operator*(const float rhs) const { return { x * rhs, y * rhs, z * rhs }; }
	inline vec3f operator/(const float rhs) const { return { x / rhs, y / rhs, z / rhs }; }

	inline void operator*=(const float rhs) { x *= rhs; y *= rhs; z *= rhs; }
	inline void operator/=(const float rhs) { x /= rhs; y /= rhs; z /= rhs; }

	inline void operator=(const vec3f & rhs) { x = rhs.x; y = rhs.y; z = rhs.z; }
};

inline float dot(const vec3f & a, const vec3f & b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline float length(const vec3f & v)               { return std::sqrt(dot(v, v)); }
inline vec3f normalise(const vec3f & v)            { return v * (1 / length(v)); }

inline vec3f cross(const vec3f & a, const vec3f & b) noexcept
{
	return
	{
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	};
}


struct Ray
{
	vec3f o; // Origin
	vec3f d; // Direction normalised
};

constexpr static float ray_epsilon = 3e-4f; // Ignore intersections closer than this to avoid self-intersection artifacts
//constexpr static float DE_thresh = 1e-5f; // Nearest intersection distance for DE objects
constexpr static float DE_thresh = 2e-5f; // Nearest intersection distance for DE objects
constexpr static float ray_inf = 1e16f; //-std::log(0.0f); // log(0) = -inf


// Base class for scene objects
struct Object
{
	virtual float intersect(const Ray   & r) const noexcept = 0;
	virtual vec3f getNormal(const vec3f & p) const noexcept = 0;

	vec3f colour = { 0, 0, 0 };
};


// Simple sphere
struct Sphere final : public Object
{
	vec3f centre = { 0, 0, 0 };
	float radius = 1; 


	virtual float intersect(const Ray & r) const noexcept override
	{
		const vec3f s = r.o - centre;
		const float b = dot(s, r.d);
		const float c = dot(s, s) - radius * radius;

		const float discriminant = b * b - c;
		if (discriminant < 0)
			return -1;

		// Compute both roots and return the nearest one that's > 0
		const float t1 = -b - std::sqrt(discriminant);
		const float t2 = -b + std::sqrt(discriminant);
		return (t1 >= 0) ? t1 : t2;
	}

	virtual vec3f getNormal(const vec3f & p) const noexcept override
	{
		return (p - centre) * (1 / radius);
	}
};


// Base class for distance estimated (DE) objects bounded by a sphere
struct DEObject : public Object
{
	vec3f centre = { 0, 0, 0 };
	float radius = 1; 


	// Get the distance estimate for point p in object space
	virtual float getDE(const vec3f & p_os) const = 0;

	virtual float intersect(const Ray & r) const noexcept override final
	{
		const vec3f s = r.o - centre;
		const float b = dot(s, r.d);
		const float c = dot(s, s) - radius * radius;

		const float discriminant = b * b - c;
		if (discriminant < 0)
			return -1;

		// Compute bounding interval
		const float t1 = -b - std::sqrt(discriminant);
		const float t2 = -b + std::sqrt(discriminant);
		if (t2 <= ray_epsilon) return -1;

		// Ray could be inside bounding sphere, start from ray epsilon
		float t = std::max(ray_epsilon, t1);
		while (t < t2)
		{
			const vec3f p_os = s + r.d * t;
			const float DE = getDE(p_os);
			t += DE;

			// If we're close enough to the surface, return a valid intersection
			if (DE < DE_thresh)
				return t;
		}

		return -1; // No intersection found
	}

	virtual vec3f getNormal(const vec3f & p) const noexcept override final
	{
		const vec3f p_os = p - centre;
		const float DE_s = 0.001f;
		const float DE_0 = getDE(p_os);
		const vec3f grad =
		{
			getDE(p_os + vec3f{ DE_s, 0, 0}) - DE_0,
			getDE(p_os + vec3f{ 0, DE_s, 0}) - DE_0,
			getDE(p_os + vec3f{ 0, 0, DE_s}) - DE_0
		};
		return normalise(grad);
	}
};


inline vec3f min(const vec3f & a, const vec3f & b) { return vec3f{ std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z) }; }
inline vec3f max(const vec3f & a, const vec3f & b) { return vec3f{ std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z) }; }
inline vec3f abs(const vec3f & p) { return { std::fabs(p.x), std::fabs(p.y), std::fabs(p.z) }; }

inline float sdSphere(const vec3f & p, float r) { return length(p) - r; }

inline float sdBox(const vec3f & p, const vec3f & b)
{
	const vec3f q = abs(p) - b;
	return length(max(q, vec3f{ 0,0,0 })) + std::min(std::max(q.x, std::max(q.y, q.z)), 0.0f);
}


#define USE_SCALAR_DERIV 0

struct QuadraticJuliabulb final : public DEObject
{
	virtual float getDE(const vec3f & p_os) const noexcept override final
	{
#if USE_SCALAR_DERIV
		const vec3f c = vec3f{ -1.1412f, 0.11f,  0.1513f } * 1.0f;
		vec3f z = p_os;
		float r, dr = 1;
		for (int i = 0; i < 64 * 8; i++)
		{
			const float xy_r2 = z.x * z.x + z.y * z.y;
			const float r2 = xy_r2 + z.z * z.z;
			const float scale = 1 - z.z * z.z / xy_r2;
			r = std::sqrt(r2);
			if (r > 256 * 64)
				break;

			dr = r * 2 * dr + 1;
			z = vec3f
			{
				(z.x * z.x - z.y * z.y) * scale + c.x,
				(2 * z.x * z.y) * scale + c.y,
				-2 * z.z * std::sqrt(xy_r2) + c.z
			};
		}

		return 0.0125f * std::log(r) * r / dr;
#else
		Dual3f zx(p_os.x, 0);
		Dual3f zy(p_os.y, 1);
		Dual3f zz(p_os.z, 2);

		const Dual3f cx(-1.1412f);
		const Dual3f cy( 0.11f);
		const Dual3f cz( 0.1513f);

		for (int i = 0; i < 64 * 8; i++)
		{
			const Dual3f xy_r2 = zx * zx + zy * zy;
			const Dual3f r2 = xy_r2 + zz * zz;
			const Dual3f scale = Dual3f(1) - zz * zz / xy_r2;

			const float len2 = zx.v[0] * zx.v[0] + zy.v[0] * zy.v[0] + zz.v[0] * zz.v[0];
			if (len2 > 65536 * 4096)
				break;

			const Dual3f zx_ = (zx * zx - zy * zy) * Dual3f(scale) + cx;
			const Dual3f zy_ = (Dual3f(2) * zx * zy) * Dual3f(scale) + cy;
			const Dual3f zz_ = Dual3f(-2) * zz * sqrt(xy_r2) + cz;

			zx = zx_;
			zy = zy_;
			zz = zz_;
		}

		const vec3f p  = vec3f{ zx.v[0], zy.v[0], zz.v[0] };
		const vec3f jx = vec3f{ zx.v[1], zy.v[1], zz.v[1] };
		const vec3f jy = vec3f{ zx.v[2], zy.v[2], zz.v[2] };
		const vec3f jz = vec3f{ zx.v[3], zy.v[3], zz.v[3] };
		const vec3f dr =
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
	virtual float getDE(const vec3f & p_os) const noexcept override final
	{
#if USE_SCALAR_DERIV
		vec3f w = p_os;
		float m = dot(w, w);
		float dz = 1;

		for (int i = 0; i < 4; i++)
		{
			const float m2 = m * m;
			const float m4 = m2 * m2;
			dz = 8 * sqrt(m4 * m2 * m) * dz + 1;

			const float x = w.x, x2 = x*x, x4 = x2*x2;
			const float y = w.y, y2 = y*y, y4 = y2*y2;
			const float z = w.z, z2 = z*z, z4 = z2*z2;

			const float k3 = x2 + z2;
			const float k2 = 1 / sqrt(k3*k3*k3*k3*k3*k3*k3);
			const float k1 = x4 + y4 + z4 - 6*y2*z2 - 6*x2*y2 + 2*z2*x2;
			const float k4 = x2 - y2 + z2;

			w.x = p_os.x +  64 * x*y*z*(x2-z2)*k4*(x4 - 6 * x2*z2+z4)*k1*k2;
			w.y = p_os.y + -16 * y2*k3*k4*k4 + k1*k1;
			w.z = p_os.z +  -8 * y*k4*(x4*x4 - 28 * x4*x2*z2 + 70 * x4*z4 - 28 * x2*z2*z4 + z4*z4) * k1*k2;

			m = dot(w, w);
			if (m > 256)
				break;
		}

		return 0.25f * log(m) * sqrt(m) / dz;
#else
		const Dual3f cx(p_os.x, 0);
		const Dual3f cy(p_os.y, 1);
		const Dual3f cz(p_os.z, 2);
		Dual3f wx = cx;
		Dual3f wy = cy;
		Dual3f wz = cz;

		for (int i = 0; i < 4; i++)
		{
			const Dual3f x = wx, x2 = x*x, x4 = x2*x2;
			const Dual3f y = wy, y2 = y*y, y4 = y2*y2;
			const Dual3f z = wz, z2 = z*z, z4 = z2*z2;

			const Dual3f k3 = x2 + z2;
			const Dual3f k2 = Dual3f(1) / sqrt(k3*k3*k3*k3*k3*k3*k3);
			const Dual3f k1 = x4 + y4 + z4 - Dual3f(6) * y2*z2 - Dual3f(6) * x2*y2 + Dual3f(2) * z2*x2;
			const Dual3f k4 = x2 - y2 + z2;

			wx = cx + Dual3f( 64) * x*y*z * (x2 - z2) * k4 * (x4 - Dual3f(6) * x2*z2+z4) * k1*k2;
			wy = cy + Dual3f(-16) * y2*k3*k4*k4 + k1*k1;
			wz = cz + Dual3f( -8) * y*k4 * (x4*x4 - Dual3f(28) * x4*x2*z2 + Dual3f(70) * x4*z4 - Dual3f(28) * x2*z2*z4 + z4*z4) * k1*k2;

			const float m = wx.v[0] * wx.v[0] + wy.v[0] * wy.v[0] + wz.v[0] * wz.v[0];
			if (m > 256)
				break;
		}

		const vec3f p  = vec3f{ wx.v[0], wy.v[0], wz.v[0] };
		const vec3f jx = vec3f{ wx.v[1], wy.v[1], wz.v[1] };
		const vec3f jy = vec3f{ wx.v[2], wy.v[2], wz.v[2] };
		const vec3f jz = vec3f{ wx.v[3], wy.v[3], wz.v[3] };

		const float len2 = dot(p, p);
		const float len = sqrt(len2);
		const vec3f u = p * (1 / len); // Normalise p first to avoid overflow in dot products

		const vec3f dr = vec3f
		{
			dot(u, jx),
			dot(u, jy),
			dot(u, jz)
		};

#if 0 // Should work in theory? See https://www.evl.uic.edu/hypercomplex/html/book/book.pdf chapter 9.6
		return 1.0f * len / length(dr);
#else // Edit by claude to take into account polynomial-ness I guess
		return 0.25f * log(len2) * len / length(dr);
#endif

#endif
	}
};


class World
{
public:
	std::pair<const Object *, float> nearestIntersection(const Ray & r) const noexcept
	{
		const Object * nearest_obj = nullptr;
		float nearest_t = ray_inf;

		for (const Object * const o : objects)
		{
			const float hit_t = o->intersect(r);
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


inline float sRGB(const float u)
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
	constexpr double DoubleOneMinusEpsilon = 0.99999999999999989;
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


inline float uintToUnitFloat(uint32_t v)
{
	// Trick from MTGP: generate an uniformly distributed single precision number in [1,2) and subtract 1
	union
	{
		uint32_t u;
		float f;
	} x;
	x.u = (v >> 9) | 0x3f800000u;
	return x.f - 1.0f;
}


inline float wrap01(float u, float v) { return (u + v < 1) ? u + v : u + v - 1; }


vec3f generateColour(int x, int y, int pass, int width, int height, const World & world) noexcept
{
	const float aspect_ratio = width / (float)height;

	const float fov_deg = 80.0f;
	const float fov_rad = fov_deg * 3.14159265359f / 180; // Convert from degrees to radians
	const float sensor_width  = 2 * std::tan(fov_rad / 2);
	const float sensor_height = sensor_width / aspect_ratio;

	const vec3f cam_pos = vec3f{ 4, 5, -10 } * 0.25f;
	const vec3f cam_lookat = { 0, 0, 0 };
	const vec3f world_up = { 0, 1, 0 };

	const vec3f cam_forward = normalise(cam_lookat - cam_pos);
	const vec3f cam_right = cross(world_up, cam_forward);
	const vec3f cam_up = cross(cam_forward, cam_right);

	const float hash_random = uintToUnitFloat(hash(y * width + x)); // Use pixel idx to randomise Halton sequence
	const float pixel_sample_x = wrap01((float)RadicalInverse<2>(pass), hash_random);
	const float pixel_sample_y = wrap01((float)RadicalInverse<3>(pass), hash_random);

	const vec3f pixel_x = cam_right * (sensor_width / width);
	const vec3f pixel_y = cam_up * -(sensor_height / height);
	const vec3f pixel_v = cam_forward + (pixel_x * (x - width * 0.5f + pixel_sample_x)) + (pixel_y * (y - height * 0.5f + pixel_sample_y));

	const Ray camera_ray = { cam_pos, normalise(pixel_v) };

	// Do intersection test
	const auto [nearest_hit_obj, nearest_hit_t] = world.nearestIntersection(camera_ray);

	// Did we hit anything? If not, immediately return black
	if (nearest_hit_obj == nullptr)
	{
		const float background = 0.0f;
		return { background, background, background };
	}

	// Compute intersection position using returned nearest ray distance
	const vec3f hit_p = camera_ray.o + camera_ray.d * nearest_hit_t;

	// Get the normal at the intersction point from the surface we hit
	const vec3f normal = nearest_hit_obj->getNormal(hit_p);

	// Compute vector from intersection point to light
	const vec3f light_pos = { 8, 12, -6 };
	const vec3f light_vec = light_pos - hit_p;
	const float light_ln2 = dot(light_vec, light_vec);
	const float light_len = std::sqrt(light_ln2);
	const vec3f light_dir = light_vec * (1 / light_len);

	// Compute reflected light (simple diffuse / Lambertian) with 1/distance^2 falloff
	const float n_dot_l = dot(normal, light_dir);
	const vec3f refl_colour = nearest_hit_obj->colour * std::max(0.0f, n_dot_l) / light_ln2 * 512;

	// Trace shadow ray from the hit point towards the light
	const Ray shadow_ray = { hit_p, light_dir };
	const auto [shadow_nearest_hit_obj, shadow_nearest_hit_t] = world.nearestIntersection(shadow_ray);

	// If we hit anything (valid hit idx and length <= length from hit point to light),
	//  we are in shadow, and no light is reflected
	if (shadow_nearest_hit_obj != nullptr && shadow_nearest_hit_t <= light_len)
		return { 0, 0, 0 };

	return refl_colour;
}


int main(int argc, char ** argv)
{
	// Some quick debug tests for Dual number class
	{
		Dual3f a = 1;
		Dual3f b = 2;
		Dual3f c = a + b;
		Dual3f f(3.14159265f / 2, 1);
		Dual3f g = sin(f);
	}

	std::vector<Sphere> spheres;
	{
		const float main_sphere_rad = 3.0f;
		//Sphere s;
		//s.centre = { 0, 0, 0 };
		//s.radius = main_sphere_rad;
		//s.colour = { 0.1f, 0.3f, 0.7f };
		//spheres.push_back(s);

		Sphere s2;
		const float bigrad = 1024;
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

	const int image_multi  = 80;
	const int image_width  = image_multi * 16;
	const int image_height = image_multi * 9;

	std::vector<vec3f>     image_HDR(image_width * image_height);
	std::vector<sRGBPixel> image_LDR(image_width * image_height);


	// Main progressive rendering loop. To take advantage of the progressive rendering on Windows,
	//  install SumatraPDF viewer and open the derp.png file. It will auto-reload on update.
	int pass = 0;
	int target_passes = 1;
	while (true)
	{
		// Render image passes
		for (; pass < target_passes; ++pass)
		{
			#pragma omp parallel for schedule(dynamic, 1)
			for (int y = 0; y < image_height; y++)
			for (int x = 0; x < image_width;  x++)
				image_HDR[y * image_width + x] += generateColour(x, y, pass, image_width, image_height, world);
		}

		// Tonemap and convert to LDR sRGB
		#pragma omp parallel for
		for (int y = 0; y < image_height; y++)
		for (int x = 0; x < image_width;  x++)
		{
			const int pixel_idx = y * image_width + x;
			const float scale = 1.0f / pass;

			const vec3f pixel_colour = image_HDR[pixel_idx];
			image_LDR[pixel_idx] =
			{
				(uint8_t)std::max(0, std::min(255, (int)(sRGB(pixel_colour.x * scale) * 256))),
				(uint8_t)std::max(0, std::min(255, (int)(sRGB(pixel_colour.y * scale) * 256))),
				(uint8_t)std::max(0, std::min(255, (int)(sRGB(pixel_colour.z * scale) * 256)))
			};
		}

		// Save image
		stbi_write_png("derp.png", image_width, image_height, 3, &image_LDR[0], image_width * 3);
		printf("saved derp.png with %d passes\n", pass);

		target_passes *= 2;
	}

	//system("derp.png");
	return 0;
}
