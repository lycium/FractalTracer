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
#include "renderer/SceneBuilder.h"
#include "ui/SceneSerializer.h"



struct sRGBPixel
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
};


void renderPasses(
	std::vector<std::thread> & threads, RenderOutput & output,
	int frame, int base_pass, int num_passes, int frames,
	const CameraParams & camera, const LightParams & light, const RenderSettings & settings,
	Scene & scene, const HDREnvironment * hdr_env) noexcept
{
	ThreadControl thread_control = { num_passes };

	for (std::thread & t : threads) t = std::thread(renderThreadFunction, &thread_control, &output, frame, base_pass, frames, &camera, &light, &settings, &scene, hdr_env);
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
	bool save_normal = false;
	bool save_albedo = false;
	std::string scene_path;

	SceneParams params;

	for (int arg = 1; arg < argc; ++arg)
	{
		const std::string a = argv[arg];
		if (a == "--animation") mode = mode_animation;
		else if (a == "--preview") preview = true;
		else if (a == "--box")     params.fractal.show_box = true;
		else if (a == "--normal")  save_normal = true;
		else if (a == "--albedo")  save_albedo = true;
		else if (a == "--formula" && arg + 1 < argc) params.fractal.formula_name = argv[++arg];
		else if (a == "--hdrenv"  && arg + 1 < argc) params.light.hdr_env_path   = argv[++arg];
		else if (a == "--scene"  && arg + 1 < argc)  scene_path = argv[++arg];
		else { fprintf(stderr, "Unknown argument: %s\nUsage: FractalTracer [--scene <path>] [--formula <name>] [--hdrenv <path>] [--animation] [--preview] [--box] [--normal] [--albedo]\n", argv[arg]); return 1; }
	}

	// Load scene file if specified (overrides defaults, CLI args can override further)
	if (!scene_path.empty())
	{
		if (!loadScene(scene_path, params))
		{
			fprintf(stderr, "Failed to load scene: %s\n", scene_path.c_str());
			return 1;
		}
		printf("Loaded scene: %s\n", scene_path.c_str());
	}

	// Set formula-specific defaults that differ from the struct defaults
	const std::string & formula_name = params.fractal.formula_name;
	if (formula_name == "hopfbrot")
	{
		params.fractal.radius     = 2.0f;
		params.fractal.step_scale = 0.5f;
		params.fractal.albedo     = { 0.1f, 0.3f, 0.7f };
		params.fractal.use_orbit_trap_colouring = false;
	}
	else if (formula_name == "burningship4d")
	{
		params.fractal.radius = 2.0f;
		params.fractal.albedo = { 0.1f, 0.3f, 0.7f };
		params.fractal.use_orbit_trap_colouring = false;
	}
	else if (formula_name == "mandelbulb")
	{
		params.fractal.radius = 1.25f;
		params.fractal.albedo = { 0.1f, 0.3f, 0.7f };
		params.fractal.use_orbit_trap_colouring = false;
	}

	// Load HDR environment map if specified
	HDREnvironment hdr_env;
	if (!params.light.hdr_env_path.empty())
	{
		int channels;
		float * hdr_data = stbi_loadf(params.light.hdr_env_path.c_str(), &hdr_env.xres, &hdr_env.yres, &channels, 3);
		if (hdr_data == nullptr)
		{
			fprintf(stderr, "Failed to load HDR environment map: %s\n", params.light.hdr_env_path.c_str());
			return 1;
		}

		hdr_env.data.resize(hdr_env.xres * hdr_env.yres);
		for (int i = 0; i < hdr_env.xres * hdr_env.yres; ++i)
			hdr_env.data[i] = { hdr_data[i * 3 + 0], hdr_data[i * 3 + 1], hdr_data[i * 3 + 2] };

		stbi_image_free(hdr_data);
		printf("Loaded HDR environment map: %s (%d x %d)\n", params.light.hdr_env_path.c_str(), hdr_env.xres, hdr_env.yres);
	}

	// Build scene from fractal params
	Scene scene;
	if (!buildScene(scene, params.fractal))
	{
		fprintf(stderr, "Unknown formula: %s\nAvailable formulas: amosersine, sphere, amazingbox_mandalay, hopfbrot, burningship4d, mandelbulb, "
			"lambdabulb, amazingbox, octopus, mengersponge, cubicbulb, pseudokleinian, "
			"riemannsphere, mandalay, spheretree, benesipine2\n", params.fractal.formula_name.c_str());
		return 1;
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
			const int passes = preview ? 1 : 2 * 3;
			printf("Rendering %d frames at resolution %d x %d with %d passes\n", frames, image_width, image_height, passes);

			for (int frame = 0; frame < frames; ++frame)
			{
				output.clear();

				// Compute camera orbit for this frame
				const real time  = (frames <= 0) ? 0 : two_pi * frame / frames;
				const real cos_t = std::cos(time);
				const real sin_t = std::sin(time);
				params.camera.position = vec3r{ 4 * cos_t + 10 * sin_t, 5, -10 * cos_t + 4 * sin_t } * 0.25f;
				params.camera.look_at  = { 0, -0.125f, 0 };
				params.camera.recompute();

				const auto t1 = std::chrono::steady_clock::now();

				renderPasses(threads, output, frame, 0, passes, frames,
					params.camera, params.light, params.render, scene, &hdr_env);

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
			// Set up static camera for progressive mode
			params.camera.position = vec3r{ 10, 5, -10 } * 0.25f;
			params.camera.look_at  = { 0, -0.125f, 0 };
			params.camera.recompute();

			const int max_passes = params.render.target_passes;
			printf("Progressive rendering at resolution %d x %d with doubling passes to max %d\n", image_width, image_height, max_passes);
			output.clear();

			int pass = 0;
			int target_passes = 1;
			while (pass < max_passes)
			{
				const auto t1 = std::chrono::steady_clock::now();

				// Note that we force num_frames to be zero since we usually don't want motion blur for stills
				const int num_passes = target_passes - pass;
				renderPasses(threads, output, 0, pass, num_passes, 0,
					params.camera, params.light, params.render, scene, &hdr_env);

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
