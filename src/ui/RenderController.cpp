#include "RenderController.h"

#include <algorithm>
#include <chrono>


RenderController::RenderController(int num_threads_, int xres, int yres)
	: output(xres, yres), full_xres(xres), full_yres(yres), num_threads(num_threads_)
{
	current_xres = xres;
	current_yres = yres;
}

RenderController::~RenderController()
{
	stop();
}

void RenderController::updateParams(const SceneParams & new_params)
{
	{
		std::lock_guard<std::mutex> lock(params_mutex);
		params = new_params;
	}

	// Signal restart - scene will be rebuilt by the manager thread
	completed_passes = 0;
	generation.fetch_add(1);
}

void RenderController::reloadHDREnv()
{
	// Placeholder - HDR loading requires stb_image which is only available
	// in translation units that define STB_IMAGE_IMPLEMENTATION
}

void RenderController::start()
{
	if (running) return;
	running = true;
	manager_thread = std::thread(&RenderController::managerFunc, this);
}

void RenderController::stop()
{
	running = false;
	if (manager_thread.joinable())
		manager_thread.join();
}

void RenderController::managerFunc()
{
	std::vector<std::thread> workers(num_threads);

	// Cached scene and params - only rebuilt when generation changes
	uint64_t scene_gen = UINT64_MAX; // Force initial build
	Scene render_scene;
	CameraParams camera;
	LightParams light;
	RenderSettings settings;

	// Local render buffer - output is only updated with completed frames
	RenderOutput render_buf(1, 1);

	while (running)
	{
		const uint64_t gen = generation.load();
		const int pass = completed_passes.load();

		// Only snapshot params and rebuild scene when generation changes
		if (gen != scene_gen)
		{
			FractalParams fractal;
			{
				std::lock_guard<std::mutex> lock(params_mutex);
				camera = params.camera;
				light = params.light;
				settings = params.render;
				fractal = params.fractal;
			}
			camera.recompute();
			buildScene(render_scene, fractal);
			scene_gen = gen;
		}

		// Progressive sub-resolution preview: each pass halves the divisor
		// e.g. divisor=32: pass 0 @ /32, pass 1 @ /16, ... pass 5 @ /1 (full res), pass 6+ accumulate
		const int divisor = std::max(1, settings.preview_divisor);
		const int cur_divisor = std::max(1, divisor >> pass);
		const int target_xres = full_xres.load();
		const int target_yres = full_yres.load();
		int render_xres = std::max(1, target_xres / cur_divisor);
		int render_yres = std::max(1, target_yres / cur_divisor);

		// Count sub-resolution levels (passes before full res)
		int sub_res_count = 0;
		for (int d = divisor; d > 1; d >>= 1) sub_res_count++;

		current_xres = render_xres;
		current_yres = render_yres;

		// Prepare render buffer
		if (render_buf.xres != render_xres || render_buf.yres != render_yres)
			render_buf.resize(render_xres, render_yres);
		else if (pass <= sub_res_count)
			render_buf.clear();

		// Sub-res passes use render_pass 0, full-res passes accumulate from 0
		const int render_pass = (pass < sub_res_count) ? 0 : pass - sub_res_count;

		// Render one pass using bucket-based multi-threading
		ThreadControl thread_control = { 1 }; // 1 pass at a time

		for (std::thread & t : workers)
		{
			t = std::thread([&]()
			{
				const int xres = render_xres;
				const int yres = render_yres;

				Scene local_scene(render_scene);

				constexpr int bucket_size = 64;
				const int x_buckets = (xres + bucket_size - 1) / bucket_size;
				const int y_buckets = (yres + bucket_size - 1) / bucket_size;
				const int num_buckets = x_buckets * y_buckets;

				while (true)
				{
					if (generation.load() != gen) break; // Cancelled

					const int bucket = thread_control.next_bucket.fetch_add(1);
					if (bucket >= num_buckets) break;

					const int bucket_y = bucket / x_buckets;
					const int bucket_x = bucket - x_buckets * bucket_y;
					const int x0 = bucket_x * bucket_size, x1 = std::min(x0 + bucket_size, xres);
					const int y0 = bucket_y * bucket_size, y1 = std::min(y0 + bucket_size, yres);

					for (int y = y0; y < y1; ++y)
					{
						if (generation.load() != gen) break; // Check per-row for faster cancellation
						for (int x = x0; x < x1; ++x)
							render(x, y, 0, render_pass, 0, camera, light, settings, local_scene, render_buf, &hdr_env);
					}
				}
			});
		}

		for (std::thread & t : workers)
			t.join();

		// If generation changed during render, restart
		if (generation.load() != gen) continue;

		// Pass completed - publish to display output
		{
			std::lock_guard<std::mutex> lock(output_mutex);
			if (output.xres != render_xres || output.yres != render_yres)
				output.resize(render_xres, render_yres);
			const size_t n = (size_t)render_xres * render_yres;
			memcpy((void *)output.beauty.data(), render_buf.beauty.data(), n * sizeof(vec3f));
			memcpy((void *)output.normal.data(), render_buf.normal.data(), n * sizeof(vec3f));
			memcpy((void *)output.albedo.data(), render_buf.albedo.data(), n * sizeof(vec3f));
			output.passes = render_pass + 1;
		}
		completed_passes.fetch_add(1);
	}
}
