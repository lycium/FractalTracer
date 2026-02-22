#pragma once

#include <atomic>
#include <mutex>
#include <thread>
#include <vector>
#include <string>

#include "renderer/Renderer.h"
#include "renderer/SceneParams.h"
#include "renderer/SceneBuilder.h"
#include "Timeline.h"


struct AnimationSettings
{
	int target_spp = 64;
	int output_xres = 1920;
	int output_yres = 1080;
	std::string output_dir = "anim_output";
	bool encode_mp4 = true;
};


struct AnimationRenderer
{
	// Start rendering animation in background
	void start(const Timeline & timeline, const SceneParams & base_params,
		const AnimationSettings & settings, int num_threads);

	// Cancel rendering
	void cancel();

	// Is rendering in progress?
	bool isRendering() const { return rendering.load(); }

	// Progress info
	int getCurrentFrame() const { return current_frame.load(); }
	int getTotalFrames() const { return total_frames.load(); }
	int getCurrentPass() const { return current_pass.load(); }
	int getTotalPasses() const { return total_passes.load(); }
	std::string getStatusMessage() const;

	// Did rendering complete successfully?
	bool isFinished() const { return finished.load(); }

	~AnimationRenderer();

private:
	void renderFunc(Timeline timeline, SceneParams base_params,
		AnimationSettings settings, int num_threads);

	std::thread render_thread;
	std::atomic<bool> rendering{false};
	std::atomic<bool> cancel_requested{false};
	std::atomic<bool> finished{false};
	std::atomic<int> current_frame{0};
	std::atomic<int> total_frames{0};
	std::atomic<int> current_pass{0};
	std::atomic<int> total_passes{0};
	mutable std::mutex status_mutex;
	std::string status_message;
};
