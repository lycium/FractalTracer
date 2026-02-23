#include "Timeline.h"
#include "imgui.h"

#include <cmath>
#include <cstdio>


// Catmull-Rom spline interpolation for a single value
static float catmullRom(float p0, float p1, float p2, float p3, float t)
{
	const float t2 = t * t;
	const float t3 = t2 * t;
	return 0.5f * ((2.0f * p1) +
		(-p0 + p2) * t +
		(2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 +
		(-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3);
}

static vec3r interpolateVec3r(const vec3r & a, const vec3r & b, float t)
{
	return a * (1.0f - t) + b * t;
}

static vec3f interpolateVec3f(const vec3f & a, const vec3f & b, float t)
{
	return a * (1.0f - t) + b * t;
}

static vec3r catmullRomVec3r(const vec3r & p0, const vec3r & p1, const vec3r & p2, const vec3r & p3, float t)
{
	return vec3r{
		(real)catmullRom((float)p0.x(), (float)p1.x(), (float)p2.x(), (float)p3.x(), t),
		(real)catmullRom((float)p0.y(), (float)p1.y(), (float)p2.y(), (float)p3.y(), t),
		(real)catmullRom((float)p0.z(), (float)p1.z(), (float)p2.z(), (float)p3.z(), t)
	};
}


SceneParams Timeline::evaluate(float t) const
{
	if (keyframes.empty())
		return SceneParams{};

	if (keyframes.size() == 1)
	{
		SceneParams result;
		result.camera = keyframes[0].camera;
		result.objects = keyframes[0].objects;
		result.light = keyframes[0].light;
		return result;
	}

	// Clamp time
	t = std::max(keyframes.front().time_seconds, std::min(keyframes.back().time_seconds, t));

	// Find the two keyframes surrounding t
	int idx = 0;
	for (int i = 0; i < (int)keyframes.size() - 1; i++)
	{
		if (t >= keyframes[i].time_seconds && t <= keyframes[i + 1].time_seconds)
		{
			idx = i;
			break;
		}
	}

	const Keyframe & kf0 = keyframes[idx];
	const Keyframe & kf1 = keyframes[idx + 1];

	const float dt = kf1.time_seconds - kf0.time_seconds;
	const float local_t = (dt > 1e-6f) ? (t - kf0.time_seconds) / dt : 0;

	SceneParams result;

	// Objects: step interpolation (use left keyframe, switch at 0.5)
	result.objects = (local_t < 0.5f) ? kf0.objects : kf1.objects;

	// Interpolate camera
	if (kf0.interpolation_mode == 1 && keyframes.size() >= 2)
	{
		// Catmull-Rom: get phantom endpoints
		const int i0 = std::max(0, idx - 1);
		const int i3 = std::min((int)keyframes.size() - 1, idx + 2);
		const Keyframe & kfm = keyframes[i0];
		const Keyframe & kfp = keyframes[i3];

		result.camera.position = catmullRomVec3r(kfm.camera.position, kf0.camera.position, kf1.camera.position, kfp.camera.position, local_t);
		result.camera.look_at  = catmullRomVec3r(kfm.camera.look_at, kf0.camera.look_at, kf1.camera.look_at, kfp.camera.look_at, local_t);
	}
	else
	{
		result.camera.position = interpolateVec3r(kf0.camera.position, kf1.camera.position, local_t);
		result.camera.look_at  = interpolateVec3r(kf0.camera.look_at, kf1.camera.look_at, local_t);
	}
	result.camera.fov_deg = (real)catmullRom((float)kf0.camera.fov_deg, (float)kf0.camera.fov_deg, (float)kf1.camera.fov_deg, (float)kf1.camera.fov_deg, local_t);
	result.camera.world_up = kf0.camera.world_up;
	result.camera.recompute();

	// Interpolate light
	result.light.light_pos       = interpolateVec3r(kf0.light.light_pos, kf1.light.light_pos, local_t);
	result.light.light_intensity = (real)(kf0.light.light_intensity * (1 - local_t) + kf1.light.light_intensity * local_t);
	result.light.sky_up_colour   = interpolateVec3f(kf0.light.sky_up_colour, kf1.light.sky_up_colour, local_t);
	result.light.sky_hz_colour   = interpolateVec3f(kf0.light.sky_hz_colour, kf1.light.sky_hz_colour, local_t);

	return result;
}


void Timeline::addKeyframe(const Keyframe & kf)
{
	// Insert in sorted order
	auto it = std::lower_bound(keyframes.begin(), keyframes.end(), kf,
		[](const Keyframe & a, const Keyframe & b) { return a.time_seconds < b.time_seconds; });
	keyframes.insert(it, kf);
}


void Timeline::removeKeyframe(int index)
{
	if (index >= 0 && index < (int)keyframes.size())
		keyframes.erase(keyframes.begin() + index);
}


bool drawTimelinePanel(Timeline & timeline, SceneParams & params)
{
	bool changed = false;

	ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetIO().DisplaySize.y - 180), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 180), ImGuiCond_FirstUseEver);

	if (!ImGui::Begin("Timeline"))
	{
		ImGui::End();
		return false;
	}

	// Transport controls
	if (ImGui::Button(timeline.playing ? "Pause" : "Play"))
		timeline.playing = !timeline.playing;

	ImGui::SameLine();
	if (ImGui::Button("Stop"))
	{
		timeline.playing = false;
		timeline.current_time = 0;
		changed = true;
	}

	ImGui::SameLine();
	if (ImGui::Button("Set Keyframe"))
	{
		Keyframe kf;
		kf.time_seconds = timeline.current_time;
		kf.camera = params.camera;
		kf.objects = params.objects;
		kf.light = params.light;
		timeline.addKeyframe(kf);
	}

	ImGui::SameLine();
	ImGui::SliderFloat("Duration", &timeline.duration_seconds, 1.0f, 120.0f, "%.1f s");

	ImGui::SameLine();
	ImGui::SliderInt("FPS", &timeline.fps, 1, 120);

	// Time scrubber
	if (ImGui::SliderFloat("Time", &timeline.current_time, 0, timeline.duration_seconds, "%.2f s"))
		changed = true;

	// Keyframe markers visualization
	ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
	ImVec2 canvas_size = ImVec2(ImGui::GetContentRegionAvail().x, 30);
	ImDrawList * draw_list = ImGui::GetWindowDrawList();

	// Background bar
	draw_list->AddRectFilled(canvas_pos,
		ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y),
		IM_COL32(40, 40, 40, 255));

	// Keyframe markers
	int delete_idx = -1;
	for (int i = 0; i < (int)timeline.keyframes.size(); i++)
	{
		const float t = timeline.keyframes[i].time_seconds;
		const float x = canvas_pos.x + (t / timeline.duration_seconds) * canvas_size.x;
		const float y = canvas_pos.y + canvas_size.y * 0.5f;

		// Diamond marker
		draw_list->AddQuadFilled(
			ImVec2(x, y - 8), ImVec2(x + 6, y),
			ImVec2(x, y + 8), ImVec2(x - 6, y),
			IM_COL32(255, 200, 50, 255));

		// Tooltip/context on hover
		ImGui::SetCursorScreenPos(ImVec2(x - 8, canvas_pos.y));
		char id[32];
		snprintf(id, sizeof(id), "##kf%d", i);
		if (ImGui::InvisibleButton(id, ImVec2(16, canvas_size.y)))
		{
			timeline.current_time = t;
			changed = true;
		}
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text("Keyframe %d at %.2f s", i, t);
			ImGui::EndTooltip();
		}
		if (ImGui::BeginPopupContextItem())
		{
			ImGui::Text("Keyframe %d", i);
			if (ImGui::Button("Delete"))
			{
				delete_idx = i;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}

	if (delete_idx >= 0)
		timeline.removeKeyframe(delete_idx);

	// Playhead
	{
		const float x = canvas_pos.x + (timeline.current_time / timeline.duration_seconds) * canvas_size.x;
		draw_list->AddLine(
			ImVec2(x, canvas_pos.y),
			ImVec2(x, canvas_pos.y + canvas_size.y),
			IM_COL32(255, 80, 80, 255), 2.0f);
	}

	ImGui::SetCursorScreenPos(ImVec2(canvas_pos.x, canvas_pos.y + canvas_size.y + 4));

	// Info
	ImGui::Text("Keyframes: %d | Total frames: %d", (int)timeline.keyframes.size(), timeline.getTotalFrames());

	ImGui::End();

	// Evaluate timeline during playback
	if (changed && !timeline.keyframes.empty())
	{
		SceneParams interpolated = timeline.evaluate(timeline.current_time);
		params.camera = interpolated.camera;
		params.objects = interpolated.objects;
		params.light = interpolated.light;
	}

	return changed;
}
