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

	while (running)
	{
		const uint64_t gen = generation.load();
		const int pass = completed_passes.load();

		// Determine sub-resolution level
		// pass 0: 1/4 res, pass 1: 1/2 res, pass 2+: full res
		int render_xres = full_xres;
		int render_yres = full_yres;
		if (pass == 0)      { render_xres /= 4; render_yres /= 4; }
		else if (pass == 1) { render_xres /= 2; render_yres /= 2; }
		render_xres = std::max(render_xres, 32);
		render_yres = std::max(render_yres, 18);

		current_xres = render_xres;
		current_yres = render_yres;

		// Resize/clear output if resolution changed
		{
			std::lock_guard<std::mutex> lock(output_mutex);
			if (output.xres != render_xres || output.yres != render_yres)
				output.resize(render_xres, render_yres);
			else if (pass <= 2)
				output.clear();
		}

		// Snapshot params and build scene
		CameraParams camera;
		LightParams light;
		RenderSettings settings;
		FractalParams fractal;
		{
			std::lock_guard<std::mutex> lock(params_mutex);
			camera = params.camera;
			light = params.light;
			settings = params.render;
			fractal = params.fractal;
		}
		camera.recompute();

		Scene render_scene;
		buildScene(render_scene, fractal);

		// Use the pass index, but for sub-resolution levels always use pass 0
		const int render_pass = (pass < 2) ? 0 : pass - 2;

		// Render one pass using bucket-based multi-threading
		ThreadControl thread_control = { 1 }; // 1 pass at a time

		for (std::thread & t : workers)
		{
			t = std::thread([&]()
			{
				const int xres = render_xres;
				const int yres = render_yres;

				Scene local_scene(render_scene);

				constexpr int bucket_size = 32;
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
					for (int x = x0; x < x1; ++x)
						render(x, y, 0, render_pass, 0, camera, light, settings, local_scene, output, &hdr_env);
				}
			});
		}

		for (std::thread & t : workers)
			t.join();

		// If generation changed during render, restart
		if (generation.load() != gen) continue;

		// Pass completed - update the pass count used for normalisation
		{
			std::lock_guard<std::mutex> lock(output_mutex);
			const int render_pass = (pass < 2) ? 0 : pass - 2;
			output.passes = render_pass + 1;
		}
		completed_passes.fetch_add(1);
	}
}
