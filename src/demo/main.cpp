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

#define STB_IMAGE_IMPLEMENTATION
#include "../util/stb_image.h"

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
#include "renderer/HDREnvironment.h"
#include "renderer/Renderer.h"
#include "renderer/ColouringFunction.h"

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
#include "formulas/Lambdabulb.h"
#include "formulas/BurningShip4D.h"
#include "formulas/Hopfbrot.h"



struct sRGBPixel
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
};


void renderPasses(std::vector<std::thread> & threads, RenderOutput & output, int frame, int base_pass, int num_passes, int frames, Scene & scene, const HDREnvironment * hdr_env) noexcept
{
	ThreadControl thread_control = { num_passes };

	for (std::thread & t : threads) t = std::thread(renderThreadFunction, &thread_control, &output, frame, base_pass, frames, &scene, hdr_env);
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
	{
		uint64_t v = 0;
		HilbertFibonacci(vec2i(1, 0), vec2i(0, 1), 0, noise_size, v);
	}

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
	bool preview = false;
	bool box = false;
	bool save_normal = false;
	bool save_albedo = false;
	std::string formula_name = "mandalay";
	std::string hdrenv_path;
	for (int arg = 1; arg < argc; ++arg)
	{
		const std::string a = argv[arg];
		if (a == "--animation") mode = mode_animation;
		else if (a == "--preview") preview = true;
		else if (a == "--box")     box = true;
		else if (a == "--normal")  save_normal = true;
		else if (a == "--albedo")  save_albedo = true;
		else if (a == "--formula" && arg + 1 < argc) formula_name = argv[++arg];
		else if (a == "--hdrenv"  && arg + 1 < argc) hdrenv_path  = argv[++arg];
		else { fprintf(stderr, "Unknown argument: %s\nUsage: FractalTracer [--formula <name>] [--hdrenv <path>] [--animation] [--preview] [--box] [--normal] [--albedo]\n", argv[arg]); return 1; }
	}

	// Load HDR environment map if specified
	HDREnvironment hdr_env;
	if (!hdrenv_path.empty())
	{
		int channels;
		float * hdr_data = stbi_loadf(hdrenv_path.c_str(), &hdr_env.xres, &hdr_env.yres, &channels, 3);
		if (hdr_data == nullptr)
		{
			fprintf(stderr, "Failed to load HDR environment map: %s\n", hdrenv_path.c_str());
			return 1;
		}

		hdr_env.data.resize(hdr_env.xres * hdr_env.yres);
		for (int i = 0; i < hdr_env.xres * hdr_env.yres; ++i)
			hdr_env.data[i] = { hdr_data[i * 3 + 0], hdr_data[i * 3 + 1], hdr_data[i * 3 + 2] };

		stbi_image_free(hdr_data);
		printf("Loaded HDR environment map: %s (%d x %d)\n", hdrenv_path.c_str(), hdr_env.xres, hdr_env.yres);
	}

	Scene scene;
	{
		const real main_sphere_rad = 1.35f;

		Sphere s;
		s.centre = { 0, 0, 0 };
		s.radius = main_sphere_rad;
		s.mat.albedo = { 0.1f, 0.1f, 0.7f };
		//scene.objects.push_back(s.clone());

		//Sphere s2;
		//const real bigrad = 128;
		//s2.centre = { 0, -bigrad - main_sphere_rad, 0 };
		//s2.radius = bigrad;
		//s2.mat.albedo = vec3f{ 0.8f, 0.2f, 0.05f } * 1.0f;
		//s2.mat.use_fresnel = true;
		//scene.objects.push_back(s2.clone());

		//const real  quad_size = main_sphere_rad;
		//const vec3r quad_e0 = vec3r{ 0, 0, quad_size } * 2;
		//const vec3r quad_e1 = vec3r{ quad_size, 0, 0 } * 2;
		//const vec3r p0 = vec3r(0, -main_sphere_rad, 0) - quad_e0 * 0.5f - quad_e1 * 0.5f;
		//Quad quad(p0, quad_e0, quad_e1);
		//quad.mat.albedo = vec3f{ 0.8f, 0.2f, 0.05f } * 1.0f;
		//quad.mat.use_fresnel = false;
		//scene.objects.push_back(quad.clone());


		if (box)
		{
			const real k = main_sphere_rad;
			Quad q0(vec3r(-k,  k, -k), vec3r(2, 0, 0) * k, vec3r(0, 0, 2) * k); q0.mat.albedo = vec3f(0.7f, 0.7f, 0.7f); q0.mat.use_fresnel = true; scene.objects.push_back(q0.clone()); // top
			Quad q1(vec3r(-k, -k, -k), vec3r(0, 0, 2) * k, vec3r(2, 0, 0) * k); q1.mat.albedo = vec3f(0.7f, 0.7f, 0.7f); q1.mat.use_fresnel = true; scene.objects.push_back(q1.clone()); // bottom
			Quad q2(vec3r(-k, -k,  k), vec3r(0, 2, 0) * k, vec3r(2, 0, 0) * k); q2.mat.albedo = vec3f(0.7f, 0.7f, 0.7f); q2.mat.use_fresnel = true; scene.objects.push_back(q2.clone()); // back
			//Quad q3(vec3r(-k, -k, -k), vec3r(0, 2, 0) * k, vec3r(2, 0, 0) * k); scene.objects.push_back(q3); // front
			Quad q4(vec3r(-k, -k, -k), vec3r(0, 2, 0) * k, vec3r(0, 0, 2) * k); q4.mat.albedo = vec3f(0.90f, 0.2f, 0.02f); q4.mat.use_fresnel = true; scene.objects.push_back(q4.clone()); // left
			Quad q5(vec3r( k, -k, -k), vec3r(0, 0, 2) * k, vec3r(0, 2, 0) * k); q5.mat.albedo = vec3f(0.02f, 0.8f, 0.05f); q5.mat.use_fresnel = true; scene.objects.push_back(q5.clone()); // right
		}

		// Formula dispatch based on --formula flag
		if (formula_name == "sphere")
		{
			Sphere mirror;
			mirror.centre = { 0, 0, 0 };
			mirror.radius = main_sphere_rad;
			mirror.mat.albedo = { 0.9f, 0.9f, 0.9f };
			mirror.mat.use_fresnel = true;
			mirror.mat.r0 = 0.95f; // Near-perfect mirror
			scene.objects.push_back(mirror.clone());
		}
		else if (formula_name == "amazingbox_mandalay")
		{
			// Hybrid of Amazingbox and MandalayKIFS
			auto * amazingbox = new DualAmazingboxIteration;
			amazingbox->scale = -1.77f;
			amazingbox->fold_limit = 1.0f;
			amazingbox->min_r2 = 0.25f;

			auto * mandalay = new DualMandalayKIFSIteration;
			mandalay->scale = 2.8f;
			mandalay->folding_offset = 1.0f;
			mandalay->z_tower = 0.35f;
			mandalay->xy_tower = 0.2f;
			mandalay->rotate = { 0.12f, 0.08f, 0.0f };
			mandalay->julia_mode = false;

			std::vector<IterationFunction *> iter_funcs;
			iter_funcs.push_back(amazingbox);
			iter_funcs.push_back(mandalay);
			const std::vector<char> iter_seq = { 0, 0, 1 }; // 2 amazingbox per 1 mandalay

			const int max_iters = 30;
			GeneralDualDE hybrid(max_iters, iter_funcs, iter_seq);
			hybrid.radius = main_sphere_rad;
			hybrid.step_scale = 0.25;
			hybrid.mat.albedo = { 0.2f, 0.6f, 0.9f };
			hybrid.mat.use_fresnel = true;
			hybrid.mat.colouring = new OrbitTrapColouring();
			scene.objects.push_back(hybrid.clone());
		}
		else if (formula_name == "hopfbrot")
		{
			Hopfbrot bulb;
			bulb.radius = 2.0f;
			bulb.step_scale = 0.5f;
		    bulb.scene_scale = 2.0f;
			bulb.mat.albedo = { 0.1f, 0.3f, 0.7f };
			bulb.mat.use_fresnel = true;
			scene.objects.push_back(bulb.clone());
		}
		else if (formula_name == "burningship4d")
		{
			BurningShip4D bulb;
			bulb.radius = 2.0f;
			bulb.mat.albedo = { 0.1f, 0.3f, 0.7f };
			bulb.mat.use_fresnel = true;
			scene.objects.push_back(bulb.clone());
		}
		else if (formula_name == "mandelbulb")
		{
			MandelbulbDual bulb;
			bulb.radius = 1.25f;
			bulb.mat.albedo = { 0.1f, 0.3f, 0.7f };
			bulb.mat.use_fresnel = true;
			scene.objects.push_back(bulb.clone());
		}
		else
		{
			// IterationFunction-based formulas wrapped in GeneralDualDE
			IterationFunction * iter = nullptr;
			if      (formula_name == "lambdabulb")      iter = new DualLambdabulbIteration;
			else if (formula_name == "amazingbox")      iter = new DualAmazingboxIteration;
			else if (formula_name == "octopus")         iter = new DualOctopusIteration;
			else if (formula_name == "mengersponge")    iter = new DualMengerSpongeCIteration;
			else if (formula_name == "cubicbulb")       iter = new DualCubicbulbIteration;
			else if (formula_name == "pseudokleinian")  iter = new DualPseudoKleinianIteration;
			else if (formula_name == "riemannsphere")   iter = new DualRiemannSphereIteration;
			else if (formula_name == "mandalay")        iter = new DualMandalayKIFSIteration;
			else if (formula_name == "spheretree")      iter = new DualSphereTreeIteration;
			else if (formula_name == "benesipine2")     iter = new DualBenesiPine2Iteration;
			else
			{
				fprintf(stderr, "Unknown formula: %s\nAvailable formulas: amazingbox_mandalay, hopfbrot, burningship4d, mandelbulb, "
					"lambdabulb, amazingbox, octopus, mengersponge, cubicbulb, pseudokleinian, "
					"riemannsphere, mandalay, spheretree, benesipine2\n", formula_name.c_str());
				return 1;
			}

			std::vector<IterationFunction *> iter_funcs;
			iter_funcs.push_back(iter);
			const std::vector<char> iter_seq = { 0 };

			const int max_iters = 30;
			GeneralDualDE hybrid(max_iters, iter_funcs, iter_seq);
			hybrid.radius = main_sphere_rad;
			hybrid.step_scale = 0.25;
			hybrid.mat.albedo = { 0.2f, 0.6f, 0.9f };
			hybrid.mat.use_fresnel = true;
			hybrid.mat.r0 = 0.25f; // Shiny surface for strong env map reflections
			hybrid.mat.colouring = new OrbitTrapColouring();
			scene.objects.push_back(hybrid.clone());
		}
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
	const int image_div = preview ? 4 : 1;
	const int image_multi  = mode == mode_animation ? 40 : 80 * 2;
	const int image_width  = image_multi / image_div * 16;
	const int image_height = image_multi / image_div * 9;
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
			const int frames = preview ? 30 : 30 * 4;
			const int passes = preview ? 1 : 2 * 3; // 2 * 3 * 5 * 7;
			printf("Rendering %d frames at resolution %d x %d with %d passes\n", frames, image_width, image_height, passes);

			for (int frame = 0; frame < frames; ++frame)
			{
				output.clear();

				const auto t1 = std::chrono::steady_clock::now();

				renderPasses(threads, output, frame, 0, passes, frames, scene, &hdr_env);

				if (print_timing)
				{
					const auto t2 = std::chrono::steady_clock::now();
					const auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
					printf("Frame took %.2f seconds to render.\n", time_span.count());
				}

				save_tonemapped_buffer("beauty", frame, passes, output.beauty);
				if (save_normal) save_tonemapped_buffer("normal", frame, passes, output.normal);
				if (save_albedo) save_tonemapped_buffer("albedo", frame, passes, output.albedo);
			}

			// Encode PNG sequences to MP4 using ffmpeg
			const auto encode_video = [](const char * channel_name)
			{
				char cmd[512];
				snprintf(cmd, sizeof(cmd),
					"ffmpeg -y -framerate 30 -i %s_frame_%%08d.png -c:v libx264 -pix_fmt yuv420p -crf 18 %s.mp4",
					channel_name, channel_name);
				printf("Running: %s\n", cmd);
				const int ret = system(cmd);
				if (ret != 0)
					fprintf(stderr, "Warning: ffmpeg exited with code %d for channel '%s' (is ffmpeg installed?)\n", ret, channel_name);
			};

			encode_video("beauty");
			if (save_normal) encode_video("normal");
			if (save_albedo) encode_video("albedo");

			break;
		}

		case mode_progressive:
		{
			const int max_passes = 2 * 3 * 5 * 7 * 11; // Set a reasonable max number of passes instead of going forever
			printf("Progressive rendering at resolution %d x %d with doubling passes to max %d\n", image_width, image_height, max_passes);
			output.clear();

			int pass = 0;
			int target_passes = 1;
			while (pass < max_passes)
			{
				const auto t1 = std::chrono::steady_clock::now();

				// Note that we force num_frames to be zero since we usually don't want motion blur for stills
				const int num_passes = target_passes - pass;
				renderPasses(threads, output, 0, pass, num_passes, 0, scene, &hdr_env);

				if (print_timing)
				{
					const auto t2 = std::chrono::steady_clock::now();
					const auto time_span = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
					printf("%d passes took %.2f seconds (%.2f seconds per pass).\n", num_passes, time_span.count(), time_span.count() / num_passes);
				}

				save_tonemapped_buffer("beauty", 0, target_passes, output.beauty);
				if (save_normal) save_tonemapped_buffer("normal", 0, target_passes, output.normal);
				if (save_albedo) save_tonemapped_buffer("albedo", 0, target_passes, output.albedo);

				pass = target_passes;
				target_passes = std::min(target_passes << 1, max_passes);
			}

			break;
		}
	}

	return 0;
}
