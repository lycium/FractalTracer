#define _CRT_SECURE_NO_WARNINGS

#include "AnimationRenderer.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../util/stb_image_write.h"

#include <cmath>
#include <cstdio>
#include <algorithm>
#include <chrono>
#include <sys/stat.h>


AnimationRenderer::~AnimationRenderer()
{
	cancel();
}


void AnimationRenderer::start(const Timeline & timeline, const SceneParams & base_params,
	const AnimationSettings & settings, int num_threads)
{
	if (rendering.load()) return;

	cancel_requested = false;
	finished = false;
	current_frame = 0;
	current_pass = 0;

	const int fps = timeline.fps;
	const int frames = (int)(timeline.duration_seconds * fps);
	total_frames = frames;
	total_passes = settings.target_spp;

	{
		std::lock_guard<std::mutex> lock(status_mutex);
		status_message = "Starting animation render...";
	}

	rendering = true;
	if (render_thread.joinable())
		render_thread.join();
	render_thread = std::thread(&AnimationRenderer::renderFunc, this,
		timeline, base_params, settings, num_threads);
}


void AnimationRenderer::cancel()
{
	cancel_requested = true;
	if (render_thread.joinable())
		render_thread.join();
}


std::string AnimationRenderer::getStatusMessage() const
{
	std::lock_guard<std::mutex> lock(status_mutex);
	return status_message;
}


struct sRGBPixelAnim { uint8_t r, g, b; };

static void tonemapFrame(std::vector<sRGBPixelAnim> & image_LDR,
	const std::vector<vec3f> & beauty, int passes, int xres, int yres)
{
	const auto sRGB = [](float u) -> float
	{
		return (u <= 0.0031308f) ? 12.92f * u : 1.055f * std::pow(u, 0.416667f) - 0.055f;
	};
	const float scale = 1.0f / std::max(1, passes);

	image_LDR.resize(xres * yres);
	for (int y = 0; y < yres; y++)
	for (int x = 0; x < xres; x++)
	{
		const int i = y * xres + x;
		const vec3f c = beauty[i];
		image_LDR[i] =
		{
			(uint8_t)std::max(0.0f, std::min(255.0f, sRGB(c.x() * scale) * 256)),
			(uint8_t)std::max(0.0f, std::min(255.0f, sRGB(c.y() * scale) * 256)),
			(uint8_t)std::max(0.0f, std::min(255.0f, sRGB(c.z() * scale) * 256))
		};
	}
}


void AnimationRenderer::renderFunc(Timeline timeline, SceneParams base_params,
	AnimationSettings settings, int num_threads)
{
	const int fps = timeline.fps;
	const int frames = (int)(timeline.duration_seconds * fps);
	const int spp = settings.target_spp;
	const int xres = settings.output_xres;
	const int yres = settings.output_yres;

	// Create output directory
#ifdef _WIN32
	_mkdir(settings.output_dir.c_str());
#else
	mkdir(settings.output_dir.c_str(), 0755);
#endif

	// Init noise table
	{
		uint64_t v = 0;
		HilbertFibonacci(vec2i(1, 0), vec2i(0, 1), 0, noise_size, v);
	}

	RenderOutput output(xres, yres);
	std::vector<std::thread> workers(num_threads);
	std::vector<sRGBPixelAnim> image_LDR;

	// Load HDR environment if needed
	HDREnvironment hdr_env;

	for (int frame = 0; frame < frames; ++frame)
	{
		if (cancel_requested.load()) break;

		current_frame = frame;
		current_pass = 0;

		// Evaluate timeline at this frame's time
		const float frame_time = (float)frame / fps;
		SceneParams frame_params = base_params;

		if (!timeline.keyframes.empty())
		{
			SceneParams interpolated = timeline.evaluate(frame_time);
			frame_params.camera = interpolated.camera;
			frame_params.objects = interpolated.objects;
			frame_params.light = interpolated.light;
		}

		frame_params.camera.recompute();

		// Build scene for this frame
		Scene scene;
		if (!buildScene(scene, frame_params))
		{
			std::lock_guard<std::mutex> lock(status_mutex);
			status_message = "Error: failed to build scene for frame " + std::to_string(frame);
			rendering = false;
			return;
		}

		// Clear output for new frame
		output.clear();

		const auto frame_start = std::chrono::steady_clock::now();

		// Render all passes for this frame
		for (int pass = 0; pass < spp; ++pass)
		{
			if (cancel_requested.load()) break;

			current_pass = pass;

			ThreadControl thread_control = { 1 };

			for (std::thread & t : workers)
			{
				t = std::thread([&, pass]()
				{
					Scene local_scene(scene);

					constexpr int bucket_size = 32;
					const int x_buckets = (xres + bucket_size - 1) / bucket_size;
					const int y_buckets = (yres + bucket_size - 1) / bucket_size;
					const int num_buckets = x_buckets * y_buckets;

					while (true)
					{
						if (cancel_requested.load()) break;

						const int bucket = thread_control.next_bucket.fetch_add(1);
						if (bucket >= num_buckets) break;

						const int bucket_y = bucket / x_buckets;
						const int bucket_x = bucket - x_buckets * bucket_y;
						const int x0 = bucket_x * bucket_size, x1 = std::min(x0 + bucket_size, xres);
						const int y0 = bucket_y * bucket_size, y1 = std::min(y0 + bucket_size, yres);

						for (int y = y0; y < y1; ++y)
						for (int x = x0; x < x1; ++x)
							render(x, y, frame, pass, frames, frame_params.camera, frame_params.light,
								frame_params.render, local_scene, output, &hdr_env);
					}
				});
			}

			for (std::thread & t : workers)
				t.join();
		}

		if (cancel_requested.load()) break;

		// Tonemap and save PNG
		tonemapFrame(image_LDR, output.beauty, spp, xres, yres);

		char filename[512];
		snprintf(filename, sizeof(filename), "%s/frame_%08d.png",
			settings.output_dir.c_str(), frame);
		stbi_write_png(filename, xres, yres, 3, image_LDR.data(), xres * 3);

		const auto frame_end = std::chrono::steady_clock::now();
		const double frame_secs = std::chrono::duration<double>(frame_end - frame_start).count();

		{
			std::lock_guard<std::mutex> lock(status_mutex);
			char msg[256];
			snprintf(msg, sizeof(msg), "Frame %d/%d rendered in %.1f s", frame + 1, frames, frame_secs);
			status_message = msg;
		}
	}

	if (cancel_requested.load())
	{
		std::lock_guard<std::mutex> lock(status_mutex);
		status_message = "Animation render cancelled.";
		rendering = false;
		return;
	}

	// Encode to MP4 with ffmpeg
	if (settings.encode_mp4)
	{
		{
			std::lock_guard<std::mutex> lock(status_mutex);
			status_message = "Encoding MP4 with ffmpeg...";
		}

		char cmd[1024];
		snprintf(cmd, sizeof(cmd),
			"ffmpeg -y -framerate %d -i %s/frame_%%08d.png -c:v libx264 -pix_fmt yuv420p -crf 18 %s/animation.mp4",
			fps, settings.output_dir.c_str(), settings.output_dir.c_str());

		const int ret = system(cmd);

		std::lock_guard<std::mutex> lock(status_mutex);
		if (ret == 0)
			status_message = "Animation complete! Saved to " + settings.output_dir + "/animation.mp4";
		else
			status_message = "Frames saved to " + settings.output_dir + "/ (ffmpeg encoding failed, is ffmpeg installed?)";
	}
	else
	{
		std::lock_guard<std::mutex> lock(status_mutex);
		status_message = "Animation complete! Frames saved to " + settings.output_dir + "/";
	}

	finished = true;
	rendering = false;
}
