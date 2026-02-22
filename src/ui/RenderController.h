#pragma once

#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

#include "renderer/Renderer.h"
#include "renderer/SceneParams.h"
#include "renderer/SceneBuilder.h"


struct RenderController
{
	RenderController(int num_threads, int xres, int yres);
	~RenderController();

	// Update scene parameters; triggers a render restart
	void updateParams(const SceneParams & new_params);

	// Reload HDR environment from current params.light.hdr_env_path
	void reloadHDREnv();

	// Start / stop the render loop
	void start();
	void stop();

	// Get current completed passes (for display)
	int getCompletedPasses() const { return completed_passes.load(); }

	// Get current generation (incremented on each param change)
	uint64_t getGeneration() const { return generation.load(); }

	// Current render output - lock output_mutex before accessing
	RenderOutput output;
	std::mutex output_mutex;

	// Current resolution being rendered (may differ from output due to sub-resolution)
	int getCurrentXRes() const { return current_xres.load(); }
	int getCurrentYRes() const { return current_yres.load(); }

	// Full target resolution (written by UI thread, read by render thread)
	std::atomic<int> full_xres, full_yres;

	// Current parameters - use updateParams() to modify and trigger restart
	SceneParams params;

	// HDR environment map
	HDREnvironment hdr_env;


private:
	void managerFunc();

	int num_threads;
	std::thread manager_thread;
	bool running = false;

	std::atomic<uint64_t> generation{0};
	std::atomic<int> completed_passes{0};
	std::atomic<int> current_xres{0};
	std::atomic<int> current_yres{0};

	std::mutex params_mutex;
};
