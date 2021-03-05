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
#include "../util/stb_image_write.h"

#if _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif

#include "maths/vec.h"

#include "renderer/Ray.h"
#include "renderer/Scene.h"
#include "renderer/Renderer.h"

#include "scene_objects/SimpleObjects.h"

#include "formulas/Mandelbulb.h"
#include "formulas/QuadraticJuliabulb.h"
#include "formulas/MengerSponge.h"
#include "formulas/MengerSpongeC.h"
#include "formulas/Cubicbulb.h"
#include "formulas/Amazingbox.h"
#include "formulas/Octopus.h"
#include "formulas/PseudoKleinian.h"
#include "formulas/MandalayKIFS.h"
#include "formulas/BenesiPine2.h"
#include "formulas/RiemannSphere.h"
#include "formulas/SphereTree.h"



struct sRGBPixel
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
};


void renderPasses(std::vector<std::thread> & threads, RenderOutput & output, int frame, int base_pass, int num_passes, int frames, Scene & scene) noexcept
{
	ThreadControl thread_control = { num_passes };

	for (std::thread & t : threads) t = std::thread(renderThreadFunction, &thread_control, &output, frame, base_pass, frames, &scene);
	for (std::thread & t : threads) t.join();
}


void tonemap(std::vector<sRGBPixel> & image_LDR, const std::vector<vec3f> & image_HDR, const int passes, const int xres, const int yres) noexcept
{
	const auto sRGB = [](float u) -> float { return (u <= 0.0031308f) ? 12.92f * u : 1.055f * std::pow(u, 0.416667f) - 0.055f; };
	const float scale = 1.0f / passes;

	#pragma omp parallel for
	for (int y = 0; y < yres; y++)
	for (int x = 0; x < xres; x++)
	{
		const int pixel_idx = y * xres + x;
		const vec3f pixel_colour = image_HDR[pixel_idx];

		image_LDR[pixel_idx] =
		{
			(uint8_t)std::max(0.0f, std::min(255.0f, sRGB(pixel_colour.x() * scale) * 256)),
			(uint8_t)std::max(0.0f, std::min(255.0f, sRGB(pixel_colour.y() * scale) * 256)),
			(uint8_t)std::max(0.0f, std::min(255.0f, sRGB(pixel_colour.z() * scale) * 256))
		};
	}
}


int main(int argc, char ** argv)
{
#if _WIN32
	SetPriorityClass(GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS);
#endif
#if _DEBUG
	const int num_threads = 1;
#else
	const int num_threads = (int)std::thread::hardware_concurrency();
#endif
	const bool print_timing = true;

	// Parse command line arguments
	enum { mode_progressive, mode_animation } mode = mode_progressive;
	if (argc > 1)
	{
		if (std::string(argv[1]) == "--animation")
			mode = mode_animation;
	}

	Scene scene;
	{
		const real main_sphere_rad = 1.5f;

		Sphere s;
		s.centre = { 0, 0, 0 };
		s.radius = main_sphere_rad;
		s.mat.albedo = { 0.1f, 0.1f, 0.7f };
		//scene.objects.push_back(s.clone());

		Sphere s2;
		const real bigrad = 128;
		s2.centre = { 0, -bigrad - main_sphere_rad, 0 };
		s2.radius = bigrad;
		s2.mat.albedo = vec3f{ 0.8f, 0.2f, 0.05f } * 1.0f;
		s2.mat.use_fresnel = true;

		scene.objects.push_back(s2.clone());

#if 0
		//MengerSpongeCAnalytic bulb;
		MandelbulbDual bulb;
		bulb.radius = 1.25f;
		bulb.step_scale = 1; //0.5f; //
		bulb.mat.albedo = { 0.1f, 0.3f, 0.7f };
		bulb.mat.use_fresnel = true;
		scene.objects.push_back(bulb.clone());
#else
		DualPseudoKleinianIteration pki;
		DualMandelbulbIteration mbi;
		DualTriplexMandelbulbIteration mbti;
		DualMengerSpongeCIteration msi; //msi.stc.x = 1.5f; msi.stc.y = 0.75f; msi.scale = 2.8f;
		DualCubicbulbIteration cbi;
		DualAmazingboxIteration ai; //ai.scale = 1.75f;
		DualOctopusIteration oi;
		DualBenesiPine2Iteration bp2;
		DualRiemannSphereIteration rs;
		DualMandalayKIFSIteration dki;
		DualSphereTreeIteration sti;

		std::vector<IterationFunction *> iter_funcs;
		//iter_funcs.push_back(oi.clone());
		//iter_funcs.push_back(pki.clone());
		iter_funcs.push_back(mbi.clone());
		//iter_funcs.push_back(mbti.clone());
		iter_funcs.push_back(msi.clone());
		//iter_funcs.push_back(ai.clone());
		//iter_funcs.push_back(cbi.clone());
		//iter_funcs.push_back(dki.clone());
		//iter_funcs.push_back(bp2.clone());
		iter_funcs.push_back(sti.clone());

		const std::vector<char> iter_seq = { 0 };

		const int max_iters = 64;
		GeneralDualDE hybrid(max_iters, iter_funcs, iter_seq);

		hybrid.radius = main_sphere_rad; // For Mandelbulb p8, bounding sphere has approximate radius of 1.2 or so
		hybrid.step_scale = 0.25; //1;
		hybrid.mat.albedo = { 0.1f, 0.3f, 0.7f };
		hybrid.mat.use_fresnel = true;

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

	const int image_multi  = 80;
	const int image_width  = image_multi * 16;
	const int image_height = image_multi * 9;
	const bool save_normal = false;
	const bool save_albedo = false;

	std::vector<sRGBPixel> image_LDR(image_width * image_height);
	RenderOutput output(image_width, image_height);

	std::vector<std::thread> threads(num_threads);

	const auto save_tonemapped_buffer = [&](const char * channel_name, const int frame, const int passes, const std::vector<vec3f> & buffer)
	{
		// Tonemap and convert to LDR sRGB
		tonemap(image_LDR, buffer, passes, image_width, image_height);

		// Save frame
		char filename[128];
		snprintf(filename, 128, "%s_frame_%08d.png", channel_name, frame);
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
				output.clear();

				std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

				renderPasses(threads, output, frame, 0, passes, frames, scene);

				if (print_timing)
				{
					std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
					std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
					printf("Frame took %.2f seconds to render.\n", time_span.count());
				}

				save_tonemapped_buffer("beauty", frame, passes, output.beauty);
				if (save_normal) save_tonemapped_buffer("normal", frame, passes, output.normal);
				if (save_albedo) save_tonemapped_buffer("albedo", frame, passes, output.albedo);
			}

			break;
		}

		case mode_progressive:
		{
			//const int max_passes = 2 * 3 * 5 * 7 * 11; // Set a reasonable max number of passes instead of going forever
			const int max_passes = 6; // Set something more reasonable for quick test
			printf("Progressive rendering at resolution %d x %d with doubling passes to max %d\n", image_width, image_height, max_passes);
			output.clear();

			int target_passes = 1;
			int total_passes = 0;

			std::chrono::nanoseconds total_time;

			while (total_passes < max_passes)
			{
				const auto t1 = std::chrono::steady_clock::now();

				// Note that we force num_frames to be zero since we usually don't want motion blur for stills
				const int num_passes = target_passes - total_passes;
				renderPasses(threads, output, 0, total_passes, num_passes, 0, scene);
				total_passes += num_passes;
				target_passes = std::min(target_passes * 2, max_passes);

				const auto t2 = std::chrono::steady_clock::now();
				total_time += t2 - t1;

				if (print_timing)
				{
					const auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
					printf("%d passes took %.2f seconds (%.2f seconds per pass).\n", num_passes, time_span.count(), time_span.count() / num_passes);
				}

				save_tonemapped_buffer("still_beauty", total_passes, target_passes, output.beauty);
				if (save_normal) save_tonemapped_buffer("still_normal", total_passes, total_passes, output.normal);
				if (save_albedo) save_tonemapped_buffer("still_albedo", total_passes, total_passes, output.albedo);
			}

			break;
		}
	}

	return 0;
}
