#pragma once

#include <vector>
#include <algorithm>
#include "renderer/SceneParams.h"


struct Keyframe
{
	float time_seconds = 0;
	CameraParams camera;
	FractalParams fractal;
	LightParams light;
	int interpolation_mode = 1; // 0=linear, 1=catmull-rom
};


struct Timeline
{
	std::vector<Keyframe> keyframes;
	float duration_seconds = 10.0f;
	float current_time = 0;
	bool playing = false;
	int fps = 30;

	// Evaluate the timeline at time t, interpolating between keyframes
	SceneParams evaluate(float t) const;

	// Add a keyframe (inserted in sorted order by time)
	void addKeyframe(const Keyframe & kf);

	// Remove a keyframe by index
	void removeKeyframe(int index);

	// Get total number of frames
	int getTotalFrames() const { return (int)(duration_seconds * fps); }
};


// Timeline UI panel. Returns true if scene params changed (during playback or scrubbing).
bool drawTimelinePanel(Timeline & timeline, SceneParams & params);
