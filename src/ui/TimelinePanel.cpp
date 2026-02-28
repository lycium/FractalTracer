#include "Timeline.h"
#include "imgui.h"

#include <cstdio>


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
