#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdint.h>
#include <cmath>
#include <vector>
#include <algorithm>

#include "maths/vec.h"

#include "renderer/Renderer.h"
#include "renderer/SceneParams.h"
#include "renderer/SceneBuilder.h"

#include "RenderController.h"
#include "ImGuiPanels.h"
#include "InteractiveCamera.h"
#include "Timeline.h"
#include "AnimationRenderer.h"

#include <SDL2/SDL.h>
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"


struct sRGBPixel
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
};


static void tonemapToPixels(std::vector<sRGBPixel> & image_LDR, const RenderOutput & output)
{
	const int xres = output.xres;
	const int yres = output.yres;
	const int passes = std::max(1, output.passes);
	const float scale = 1.0f / passes;

	const auto sRGB = [](float u) -> float
	{
		return (u <= 0.0031308f) ? 12.92f * u : 1.055f * std::pow(u, 0.416667f) - 0.055f;
	};

	image_LDR.resize(xres * yres);

	for (int y = 0; y < yres; y++)
	for (int x = 0; x < xres; x++)
	{
		const int i = y * xres + x;
		const vec3f c = output.beauty[i];
		image_LDR[i] =
		{
			(uint8_t)std::max(0.0f, std::min(255.0f, sRGB(c.x() * scale) * 256)),
			(uint8_t)std::max(0.0f, std::min(255.0f, sRGB(c.y() * scale) * 256)),
			(uint8_t)std::max(0.0f, std::min(255.0f, sRGB(c.z() * scale) * 256))
		};
	}
}


int main(int argc, char ** argv)
{
	(void)argc; (void)argv;

	// Init noise table
	{
		uint64_t v = 0;
		HilbertFibonacci(vec2i(1, 0), vec2i(0, 1), 0, noise_size, v);
	}

	const int window_w = 1280;
	const int window_h = 720;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
	{
		fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
		return 1;
	}

	SDL_Window * window = SDL_CreateWindow(
		"FractalTracer",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		window_w, window_h,
		SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

	if (!window)
	{
		fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
		SDL_Quit();
		return 1;
	}

	SDL_Renderer * sdl_renderer = SDL_CreateRenderer(window, -1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	if (!sdl_renderer)
	{
		fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 1;
	}

	// Setup Dear ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO & io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	ImGui::StyleColorsDark();

	ImGui_ImplSDL2_InitForSDLRenderer(window, sdl_renderer);
	ImGui_ImplSDLRenderer2_Init(sdl_renderer);

	// Set up render controller
	const int num_threads = std::max(1, (int)std::thread::hardware_concurrency());
	RenderController controller(num_threads, window_w, window_h);

	// Set default camera
	controller.params.camera.position = vec3r{ 10, 5, -10 } * 0.25f;
	controller.params.camera.look_at  = { 0, -0.125f, 0 };
	controller.params.camera.recompute();

	controller.updateParams(controller.params);
	controller.start();

	// Interactive camera
	InteractiveCamera interactive_cam;
	interactive_cam.initFromCamera(controller.params.camera);

	// Timeline
	Timeline timeline;

	// Animation renderer
	AnimationRenderer anim_renderer;
	AnimationSettings anim_settings;

	// Display texture
	SDL_Texture * texture = nullptr;
	int tex_w = 0, tex_h = 0;
	std::vector<sRGBPixel> display_pixels;

	Uint64 last_time = SDL_GetPerformanceCounter();
	float fps = 0;

	bool quit = false;
	while (!quit)
	{
		// Timing
		const Uint64 now = SDL_GetPerformanceCounter();
		const float dt = (float)(now - last_time) / SDL_GetPerformanceFrequency();
		last_time = now;
		fps = fps * 0.95f + (1.0f / std::max(dt, 0.001f)) * 0.05f;

		// Poll events
		bool params_changed = false;
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			ImGui_ImplSDL2_ProcessEvent(&event);

			if (event.type == SDL_QUIT) quit = true;
			if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) quit = true;

			if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED)
			{
				controller.full_xres = event.window.data1;
				controller.full_yres = event.window.data2;
				params_changed = true;
			}

			// Interactive camera mouse events
			params_changed |= interactive_cam.processEvent(event, controller.params.camera);
		}

		// Interactive camera keyboard movement
		const Uint8 * keyboard = SDL_GetKeyboardState(nullptr);
		params_changed |= interactive_cam.update(dt, keyboard, controller.params.camera);

		// Start ImGui frame
		ImGui_ImplSDLRenderer2_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		// Draw parameter panels
		ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(350, (float)window_h), ImGuiCond_FirstUseEver);
		if (ImGui::Begin("Parameters"))
		{
			params_changed |= drawFormulaPanel(controller.params.fractal);
			if (drawCameraPanel(controller.params.camera))
			{
				params_changed = true;
				interactive_cam.initFromCamera(controller.params.camera);
			}
			params_changed |= drawMaterialPanel(controller.params.fractal);
			params_changed |= drawLightPanel(controller.params.light);
			params_changed |= drawRenderSettingsPanel(controller.params.render);

			ImGui::Separator();
			ImGui::Checkbox("Orbit Mode (Tab)", &interactive_cam.orbit_mode);
			ImGui::SliderFloat("Move Speed", &interactive_cam.move_speed, 0.1f, 10.0f);
			ImGui::SliderFloat("Mouse Sensitivity", &interactive_cam.mouse_sensitivity, 0.001f, 0.01f);

			ImGui::Separator();
			if (drawFileMenu(controller.params))
			{
				params_changed = true;
				interactive_cam.initFromCamera(controller.params.camera);
			}

			ImGui::Separator();
			drawAnimationPanel(anim_renderer, anim_settings, timeline, controller.params, num_threads);
		}
		ImGui::End();

		// Stats overlay
		drawStatsOverlay(
			controller.getCompletedPasses(),
			controller.getCurrentXRes(),
			controller.getCurrentYRes(),
			fps);

		// Timeline panel
		if (drawTimelinePanel(timeline, controller.params))
		{
			params_changed = true;
			interactive_cam.initFromCamera(controller.params.camera);
		}

		// Timeline playback
		if (timeline.playing && !timeline.keyframes.empty())
		{
			timeline.current_time += dt;
			if (timeline.current_time > timeline.duration_seconds)
			{
				timeline.current_time = 0;
				timeline.playing = false;
			}
			else
			{
				SceneParams interpolated = timeline.evaluate(timeline.current_time);
				controller.params.camera = interpolated.camera;
				controller.params.fractal = interpolated.fractal;
				controller.params.light = interpolated.light;
				interactive_cam.initFromCamera(controller.params.camera);
				params_changed = true;
			}
		}

		if (params_changed)
			controller.updateParams(controller.params);

		// Tonemap current render output
		int cur_xres, cur_yres;
		{
			std::lock_guard<std::mutex> lock(controller.output_mutex);
			cur_xres = controller.output.xres;
			cur_yres = controller.output.yres;
			tonemapToPixels(display_pixels, controller.output);
		}

		// Recreate texture if resolution changed
		if (cur_xres != tex_w || cur_yres != tex_h)
		{
			if (texture) SDL_DestroyTexture(texture);
			texture = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_RGB24,
				SDL_TEXTUREACCESS_STREAMING, cur_xres, cur_yres);
			tex_w = cur_xres;
			tex_h = cur_yres;
		}

		// Upload pixels
		if (texture && !display_pixels.empty())
			SDL_UpdateTexture(texture, nullptr, display_pixels.data(), cur_xres * 3);

		// Render scene
		SDL_RenderClear(sdl_renderer);
		if (texture)
			SDL_RenderCopy(sdl_renderer, texture, nullptr, nullptr);

		// Render ImGui on top
		ImGui::Render();
		ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), sdl_renderer);

		SDL_RenderPresent(sdl_renderer);
	}

	controller.stop();

	// Cleanup
	ImGui_ImplSDLRenderer2_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	if (texture) SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(sdl_renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
