#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

#include <stdint.h>
#include <stdlib.h>

#include <cmath> // For std::sqrt and so on
#include <chrono> // For timing
#include <vector>
#include <string>
#include <thread>
#include <algorithm> // For std::pair and std::min and max

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#if _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif

#include "vec2.h"
#include "vec3.h"

#include "Ray.h"
#include "Scene.h"
#include "Renderer.h"

#include "SimpleObjects.h"
#include "Mandelbulb.h"
#include "QuadraticJuliabulb.h"
#include "MengerSponge.h"
#include "Cubicbulb.h"
#include "Amazingbox.h"
#include "Octopus.h"
#include "RiemannSphere.h"



inline float sRGB(const float u)
{
	return (u <= 0.0031308f) ? 12.92f * u : 1.055f * std::pow(u, 0.416667f) - 0.055f;
}


struct sRGBPixel
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
};


void renderPasses(
	std::vector<std::thread> & threads, std::vector<vec3f> & image_HDR,
	const int frame, const int base_pass, int num_passes, const int xres, const int yres, const int frames, Scene & scene) noexcept
{
	ThreadControl thread_control = { num_passes };

	for (std::thread & t : threads) t = std::thread(renderThreadFunction, &thread_control, &image_HDR[0], frame, base_pass, xres, yres, frames, &scene);
	for (std::thread & t : threads) t.join();
}


void tonemap(std::vector<sRGBPixel> & image_LDR, const std::vector<vec3f> & image_HDR, const int passes, const int xres, const int yres) noexcept
{
	const float scale = 1.0f / passes;

	#pragma omp parallel for
	for (int y = 0; y < yres; y++)
	for (int x = 0; x < xres; x++)
	{
		const int pixel_idx = y * xres + x;
		const vec3f pixel_colour = image_HDR[pixel_idx];

		image_LDR[pixel_idx] =
		{
			(uint8_t)std::max(0, std::min(255, (int)(sRGB(pixel_colour.x * scale) * 256))),
			(uint8_t)std::max(0, std::min(255, (int)(sRGB(pixel_colour.y * scale) * 256))),
			(uint8_t)std::max(0, std::min(255, (int)(sRGB(pixel_colour.z * scale) * 256)))
		};
	}
}


int main(int argc, char ** argv)
{
#if _WIN32
	SetPriorityClass(GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS);
#endif
	const int  num_threads = (int)std::thread::hardware_concurrency();
	const bool time_frames = false;

	// Parse command line arguments
	enum { mode_progressive, mode_animation } mode = mode_progressive;
	if (argc > 1)
	{
		if (std::string(argv[1]) == "--animation")
			mode = mode_animation;
	}

	Scene scene;
	{
		const real main_sphere_rad = 2.0f;

		Sphere s;
		s.centre = { 0, 0, 0 };
		s.radius = main_sphere_rad;
		s.mat.albedo = { 0.1f, 0.3f, 0.7f };
		//scene.objects.push_back(s.clone());

		Sphere s2;
		const real bigrad = 128;
		s2.centre = { 0, -bigrad - main_sphere_rad, 0 };
		s2.radius = bigrad;
		s2.mat.albedo = vec3f{ 0.8f, 0.2f, 0.05f } * 1.0f;
		s2.mat.use_fresnel = true;

		scene.objects.push_back(s2.clone());

#if 0
		MandelbulbDual bulb;
		bulb.radius = 1.5f;
		bulb.step_scale = 0.5f; //1;
		bulb.mat.albedo = { 0.1f, 0.3f, 0.7f };
		bulb.mat.use_fresnel = true;
		scene.objects.push_back(bulb.clone());
#else
		/*DualMandelbulbIteration mbi;
		DualMengerSpongeIteration msi;
		DualCubicbulbIteration cbi;
		DualAmazingboxIteration ai;
		DualOctopusIteration oi;*/
		DualRiemannSphereIteration rsi;

		GeneralDualDE hybrid;
		hybrid.radius = 2;
		hybrid.step_scale = 0.5;
		hybrid.mat.albedo = { 0.1f, 0.3f, 0.7f };
		hybrid.mat.use_fresnel = true;
		hybrid.max_iters = 16;

		hybrid.sequence = { 0 };

		hybrid.funcs.push_back(rsi.clone());
		//hybrid.funcs.push_back(oi.clone());
		//hybrid.funcs.push_back(mbi.clone());
		//hybrid.funcs.push_back(msi.clone());
		//hybrid.funcs.push_back(cbi.clone());

		scene.objects.push_back(hybrid.clone());
#endif

		// Test adding sphere lights
		const int num_sphere_lights = 0;//1 << 5;
		for (int i = 0; i < num_sphere_lights; ++i)
		{
			const real offset = 0.61803398874989484820458683436564f;
			const real refl_sample_x = wrap1r(i / (real)num_sphere_lights, offset);
			//const real refl_sample_x = wrap1r((real)RadicalInverse(i, 3), offset);
			const real refl_sample_y = wrap1r((real)RadicalInverse(i, 2), offset);

			// Generate uniform point on sphere, see https://mathworld.wolfram.com/SpherePointPicking.html
			const real a = refl_sample_x * two_pi;
			const real s = 2 * std::sqrt(std::max(static_cast<real>(0), refl_sample_y * (1 - refl_sample_y)));
			const vec3r sphere =
			{
				std::cos(a) * s,
				std::sin(a) * s,
				1 - 2 * refl_sample_y
			};

			Sphere sp;
			sp.centre = sphere * 1.0f;
			sp.radius = 0.05f;
			sp.mat.albedo = 0.0f;
			sp.mat.emission = 4;
			scene.objects.push_back(sp.clone());
		}
	}

	const int image_multi  = 50;
	const int image_width  = image_multi * 16;
	const int image_height = image_multi * 9;

	std::vector<vec3f>     image_HDR(image_width * image_height);
	std::vector<sRGBPixel> image_LDR(image_width * image_height);

	std::vector<std::thread> threads(num_threads);

	const auto save_tonemapped_frame = [&](int frame, int passes)
	{
		// Tonemap and convert to LDR sRGB
		tonemap(image_LDR, image_HDR, passes, image_width, image_height);

		// Save frame
		char filename[64];
		snprintf(filename, 64, "frame_%08d.png", frame);
		stbi_write_png(filename, image_width, image_height, 3, &image_LDR[0], image_width * 3);
		printf("Saved %s with %d passes\n", filename, passes);
	};

	switch (mode)
	{
		case mode_animation:
		{
			const int frames = 30 * 4;
			const int passes = 2 * 3 * 5 * 7;
			printf("Rendering %d frames at resolution %d x %d with %d passes\n", frames, image_width, image_height, passes);

			for (int frame = 0; frame < frames; ++frame)
			{
				std::fill(image_HDR.begin(), image_HDR.end(), vec3f{ 0,0,0 });

				std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

				renderPasses(threads, image_HDR, frame, 0, passes, image_width, image_height, frames, scene);

				if (time_frames)
				{
					std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
					std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
					printf("Frame took %.3f seconds to render.\n", time_span.count());
				}

				save_tonemapped_frame(frame, passes);
			}

			break;
		}

		case mode_progressive:
		{
			const int max_passes = 2 * 3 * 5 * 7 * 11; // Set a reasonable max number of passes instead of going forever
			printf("Progressive rendering at resolution %d x %d with doubling passes to max %d\n", image_width, image_height, max_passes);
			std::fill(image_HDR.begin(), image_HDR.end(), vec3f{ 0,0,0 });

			int pass = 0;
			int target_passes = 1;
			while (pass < max_passes)
			{
				// Note that we force num_frames to be zero since we usually don't want motion blur for stills
				renderPasses(threads, image_HDR, 0, pass, target_passes - pass, image_width, image_height, 0, scene);

				save_tonemapped_frame(0, target_passes);

				pass = target_passes;
				target_passes = std::min(target_passes << 1, max_passes);
			}

			break;
		}
	}

	return 0;
}
