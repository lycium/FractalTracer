#include "ImGuiPanels.h"
#include "SceneSerializer.h"
#include "renderer/FormulaFactory.h"
#include "imgui.h"

#include <cstring>
#include <cmath>


static bool SliderReal(const char * label, real * v, real v_min, real v_max, const char * format = "%.4f")
{
#if USE_DOUBLE
	double dv = *v;
	bool changed = ImGui::SliderScalar(label, ImGuiDataType_Double, &dv, &v_min, &v_max, format);
	if (changed) *v = dv;
	return changed;
#else
	return ImGui::SliderFloat(label, v, v_min, v_max, format);
#endif
}

static bool DragReal(const char * label, real * v, real speed = 0.01f, real v_min = 0, real v_max = 0, const char * format = "%.4f")
{
#if USE_DOUBLE
	double dv = *v;
	bool changed = ImGui::DragScalar(label, ImGuiDataType_Double, &dv, (float)speed, &v_min, &v_max, format);
	if (changed) *v = dv;
	return changed;
#else
	return ImGui::DragFloat(label, v, speed, v_min, v_max, format);
#endif
}

static bool DragVec3r(const char * label, vec3r & v, real speed = 0.01f)
{
#if USE_DOUBLE
	double dv[3] = { v.x(), v.y(), v.z() };
	bool changed = ImGui::DragScalarN(label, ImGuiDataType_Double, dv, 3, (float)speed);
	if (changed) { v.x() = dv[0]; v.y() = dv[1]; v.z() = dv[2]; }
	return changed;
#else
	float fv[3] = { v.x(), v.y(), v.z() };
	bool changed = ImGui::DragFloat3(label, fv, speed);
	if (changed) { v.x() = fv[0]; v.y() = fv[1]; v.z() = fv[2]; }
	return changed;
#endif
}


// Draw generic formula parameters from getParams()
static bool drawFormulaParams(IterationFunction * formula, int formula_idx)
{
	bool changed = false;
	auto params = formula->getParams();

	for (auto & p : params)
	{
		// Create unique ID to avoid ImGui ID collisions between formulas
		char label[128];
		snprintf(label, sizeof(label), "%s##f%d", p.name, formula_idx);

		switch (p.type)
		{
			case ParamInfo::Real:
				changed |= SliderReal(label, (real *)p.ptr, p.min, p.max);
				break;
			case ParamInfo::Vec3r:
				changed |= DragVec3r(label, *(vec3r *)p.ptr, 0.01f);
				break;
			case ParamInfo::Bool:
				changed |= ImGui::Checkbox(label, (bool *)p.ptr);
				break;
			case ParamInfo::Real4:
				changed |= ImGui::SliderFloat4(label, (float *)p.ptr, p.min, p.max);
				break;
		}
	}

	return changed;
}


// Draw material editor for a single object
static bool drawMaterialEditor(SceneObjectDesc & obj, int obj_idx)
{
	bool changed = false;

	char label[128];
	snprintf(label, sizeof(label), "Albedo##obj%d", obj_idx);
	changed |= ImGui::ColorEdit3(label, &obj.albedo.x());

	snprintf(label, sizeof(label), "Emission##obj%d", obj_idx);
	changed |= ImGui::ColorEdit3(label, &obj.emission.x());

	snprintf(label, sizeof(label), "Use Fresnel##obj%d", obj_idx);
	changed |= ImGui::Checkbox(label, &obj.use_fresnel);

	if (obj.use_fresnel)
	{
		snprintf(label, sizeof(label), "R0 (Fresnel)##obj%d", obj_idx);
		changed |= ImGui::SliderFloat(label, &obj.r0, 0.0f, 1.0f);
	}

	if (obj.type == "fractal")
	{
		snprintf(label, sizeof(label), "Orbit Trap Colouring##obj%d", obj_idx);
		changed |= ImGui::Checkbox(label, &obj.use_orbit_trap_colouring);
	}

	return changed;
}


bool drawObjectEditor(SceneParams & params)
{
	bool changed = false;

	if (!ImGui::CollapsingHeader("Scene Objects", ImGuiTreeNodeFlags_DefaultOpen))
		return false;

	// Object list with add/remove
	if (ImGui::Button("Add Fractal"))
	{
		SceneObjectDesc obj;
		obj.name = "Fractal " + std::to_string(params.objects.size());
		setupFormulas(obj);
		params.objects.push_back(std::move(obj));
		params.selected_object = (int)params.objects.size() - 1;
		changed = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Add Sphere"))
	{
		SceneObjectDesc obj;
		obj.type = "sphere";
		obj.name = "Sphere " + std::to_string(params.objects.size());
		obj.radius = 1.0f;
		obj.albedo = { 0.9f, 0.9f, 0.9f };
		params.objects.push_back(std::move(obj));
		params.selected_object = (int)params.objects.size() - 1;
		changed = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Add Quad"))
	{
		SceneObjectDesc obj;
		obj.type = "quad";
		obj.name = "Quad " + std::to_string(params.objects.size());
		obj.albedo = { 0.7f, 0.7f, 0.7f };
		params.objects.push_back(std::move(obj));
		params.selected_object = (int)params.objects.size() - 1;
		changed = true;
	}

	// Object selector
	if (!params.objects.empty())
	{
		// Build list of object names for combo
		std::vector<const char *> obj_names;
		for (auto & obj : params.objects)
			obj_names.push_back(obj.name.c_str());

		if (params.selected_object >= (int)params.objects.size())
			params.selected_object = (int)params.objects.size() - 1;

		ImGui::Combo("Object", &params.selected_object, obj_names.data(), (int)obj_names.size());

		// Remove button
		ImGui::SameLine();
		if (ImGui::Button("Remove") && !params.objects.empty())
		{
			params.objects.erase(params.objects.begin() + params.selected_object);
			if (params.selected_object >= (int)params.objects.size())
				params.selected_object = std::max(0, (int)params.objects.size() - 1);
			changed = true;
		}
	}

	// Edit selected object
	if (params.selected_object >= 0 && params.selected_object < (int)params.objects.size())
	{
		SceneObjectDesc & obj = params.objects[params.selected_object];
		const int idx = params.selected_object;

		ImGui::Separator();

		// Name
		char name_buf[128];
		strncpy(name_buf, obj.name.c_str(), sizeof(name_buf) - 1);
		name_buf[sizeof(name_buf) - 1] = '\0';
		char name_label[32];
		snprintf(name_label, sizeof(name_label), "Name##obj%d", idx);
		if (ImGui::InputText(name_label, name_buf, sizeof(name_buf)))
			obj.name = name_buf;

		// Type selector
		const char * types[] = { "fractal", "sphere", "quad" };
		int cur_type = 0;
		if (obj.type == "sphere") cur_type = 1;
		else if (obj.type == "quad") cur_type = 2;

		char type_label[32];
		snprintf(type_label, sizeof(type_label), "Type##obj%d", idx);
		if (ImGui::Combo(type_label, &cur_type, types, 3))
		{
			obj.type = types[cur_type];
			changed = true;
		}

		// Geometry
		if (obj.type == "sphere")
		{
			char label[64];
			snprintf(label, sizeof(label), "Centre##obj%d", idx);
			changed |= DragVec3r(label, obj.position, 0.05f);
			snprintf(label, sizeof(label), "Radius##obj%d", idx);
			changed |= SliderReal(label, &obj.radius, 0.01f, 10.0f);
		}
		else if (obj.type == "quad")
		{
			char label[64];
			snprintf(label, sizeof(label), "Position##obj%d", idx);
			changed |= DragVec3r(label, obj.position, 0.05f);
			snprintf(label, sizeof(label), "U##obj%d", idx);
			changed |= DragVec3r(label, obj.quad_u, 0.05f);
			snprintf(label, sizeof(label), "V##obj%d", idx);
			changed |= DragVec3r(label, obj.quad_v, 0.05f);
		}
		else if (obj.type == "fractal")
		{
			// Formula selector
			const auto & formula_names = getFormulaNames();
			int current_formula = 0;
			for (int i = 0; i < (int)formula_names.size(); i++)
				if (obj.formula_name == formula_names[i]) { current_formula = i; break; }

			char formula_label[64];
			snprintf(formula_label, sizeof(formula_label), "Formula##obj%d", idx);
			if (ImGui::Combo(formula_label, &current_formula, formula_names.data(), (int)formula_names.size()))
			{
				obj.formula_name = formula_names[current_formula];
				setupFormulas(obj);
				changed = true;
			}

			// Common DE parameters
			char label[64];
			snprintf(label, sizeof(label), "Max Iterations##obj%d", idx);
			changed |= ImGui::SliderInt(label, &obj.max_iters, 1, 200);
			snprintf(label, sizeof(label), "Radius##obj%d", idx);
			changed |= SliderReal(label, &obj.radius, 0.1f, 10.0f);
			snprintf(label, sizeof(label), "Step Scale##obj%d", idx);
			changed |= SliderReal(label, &obj.step_scale, 0.01f, 2.0f);
			snprintf(label, sizeof(label), "Bailout Radius^2##obj%d", idx);
			changed |= SliderReal(label, &obj.bailout_radius2, 1.0f, 10000.0f, "%.1f");

			if (obj.formula_name == "hopfbrot")
			{
				snprintf(label, sizeof(label), "Scene Scale##obj%d", idx);
				changed |= SliderReal(label, &obj.scene_scale, 0.5f, 5.0f);
			}

			// Formula-specific parameters via getParams()
			if (!obj.formulas.empty())
			{
				ImGui::Separator();
				for (int fi = 0; fi < (int)obj.formulas.size(); fi++)
				{
					if (obj.formulas.size() > 1)
					{
						ImGui::Text("Formula %d", fi);
					}
					changed |= drawFormulaParams(obj.formulas[fi], idx * 100 + fi);
				}
			}
		}

		// Material
		ImGui::Separator();
		ImGui::Text("Material");
		changed |= drawMaterialEditor(obj, idx);
	}

	return changed;
}


bool drawCameraPanel(CameraParams & camera)
{
	bool changed = false;

	if (!ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
		return false;

	changed |= DragVec3r("Position", camera.position, 0.02f);
	changed |= DragVec3r("Look At", camera.look_at, 0.02f);
	changed |= SliderReal("FOV (deg)", &camera.fov_deg, 10.0f, 170.0f, "%.1f");
	changed |= SliderReal("DOF Amount", &camera.dof_amount, 0.0f, 2.0f);
	changed |= SliderReal("Lens Radius", &camera.lens_radius, 0.0f, 0.1f, "%.5f");
	changed |= SliderReal("Focal Dist Scale", &camera.focal_dist_scale, 0.01f, 2.0f);

	return changed;
}


bool drawLightPanel(LightParams & light)
{
	bool changed = false;

	if (!ImGui::CollapsingHeader("Lighting"))
		return false;

	changed |= DragVec3r("Light Position", light.light_pos, 0.1f);
	changed |= DragReal("Intensity", &light.light_intensity, 5.0f, 0.0f, 10000.0f, "%.0f");
	changed |= ImGui::ColorEdit3("Sky Up Colour", &light.sky_up_colour.x());
	changed |= ImGui::ColorEdit3("Sky Horizon Colour", &light.sky_hz_colour.x());

	return changed;
}


bool drawRenderSettingsPanel(RenderSettings & settings)
{
	bool changed = false;

	if (!ImGui::CollapsingHeader("Render Settings"))
		return false;

	changed |= ImGui::SliderInt("Max Bounces", &settings.max_bounces, 1, 32);
	changed |= ImGui::SliderInt("Target Passes", &settings.target_passes, 1, 10000);
	changed |= ImGui::SliderInt("Preview Divisor", &settings.preview_divisor, 1, 64);

	return changed;
}


bool drawStatsOverlay(int passes, int target_passes, int xres, int yres, float fps)
{
	bool refresh = false;
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(0.5f);
	if (ImGui::Begin("Stats", nullptr,
		ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav |
		ImGuiWindowFlags_NoMove))
	{
		ImGui::Text("Resolution: %d x %d", xres, yres);
		ImGui::Text("Passes: %d / %d", passes, target_passes);
		ImGui::Text("FPS: %.1f", fps);

		// Log2 sampling progress slider
		const int full_res_passes = std::max(0, passes - 4);
		const float log2_current = (full_res_passes > 0) ? std::log2((float)full_res_passes) : 0;
		const float log2_target  = (target_passes > 0)   ? std::log2((float)target_passes)   : 1;
		float progress = log2_current; // non-editable, just for display
		char label[64];
		snprintf(label, sizeof(label), "%.1f / %.1f", log2_current, log2_target);
		ImGui::SliderFloat("log2(spp)", &progress, 0, log2_target, label, ImGuiSliderFlags_NoInput);

		if (ImGui::Button("Refresh"))
			refresh = true;
	}
	ImGui::End();
	return refresh;
}


// Example scene presets
static void loadCornellBoxPreset(SceneParams & params)
{
	params.objects.clear();

	const real k = 2.0f;

	// Floor (white) — normal must face up: cross(u,v) = (0,+,0)
	{
		SceneObjectDesc q; q.type = "quad"; q.name = "Floor";
		q.position = vec3r(-k, -k, -k); q.quad_u = vec3r(0, 0, 2*k); q.quad_v = vec3r(2*k, 0, 0);
		q.albedo = { 0.7f, 0.7f, 0.7f }; q.use_fresnel = false;
		params.objects.push_back(std::move(q));
	}
	// Ceiling (white) — normal must face down: cross(u,v) = (0,-,0)
	{
		SceneObjectDesc q; q.type = "quad"; q.name = "Ceiling";
		q.position = vec3r(-k, k, k); q.quad_u = vec3r(0, 0, -2*k); q.quad_v = vec3r(2*k, 0, 0);
		q.albedo = { 0.7f, 0.7f, 0.7f }; q.use_fresnel = false;
		params.objects.push_back(std::move(q));
	}
	// Back wall (white) — normal must face forward: cross(u,v) = (0,0,-)
	{
		SceneObjectDesc q; q.type = "quad"; q.name = "Back Wall";
		q.position = vec3r(-k, -k, k); q.quad_u = vec3r(0, 2*k, 0); q.quad_v = vec3r(2*k, 0, 0);
		q.albedo = { 0.7f, 0.7f, 0.7f }; q.use_fresnel = false;
		params.objects.push_back(std::move(q));
	}
	// Left wall (red)
	{
		SceneObjectDesc q; q.type = "quad"; q.name = "Left Wall";
		q.position = vec3r(-k, -k, -k); q.quad_u = vec3r(0, 2*k, 0); q.quad_v = vec3r(0, 0, 2*k);
		q.albedo = { 0.9f, 0.1f, 0.1f }; q.use_fresnel = false;
		params.objects.push_back(std::move(q));
	}
	// Right wall (green)
	{
		SceneObjectDesc q; q.type = "quad"; q.name = "Right Wall";
		q.position = vec3r(k, -k, -k); q.quad_u = vec3r(0, 0, 2*k); q.quad_v = vec3r(0, 2*k, 0);
		q.albedo = { 0.1f, 0.9f, 0.1f }; q.use_fresnel = false;
		params.objects.push_back(std::move(q));
	}
	// Ceiling light (emissive quad) — normal must face down: cross(u,v) = (0,-,0)
	{
		const real lk = 0.5f;
		SceneObjectDesc q; q.type = "quad"; q.name = "Light";
		q.position = vec3r(-lk, k - 0.01f, lk); q.quad_u = vec3r(0, 0, -2*lk); q.quad_v = vec3r(2*lk, 0, 0);
		q.albedo = { 1, 1, 1 }; q.emission = { 15, 15, 12 };
		q.use_fresnel = false;
		params.objects.push_back(std::move(q));
	}
	// Mandelbulb fractal
	{
		SceneObjectDesc f; f.name = "Mandelbulb";
		f.formula_name = "mandelbulb";
		f.radius = 1.25f;
		f.albedo = { 0.1f, 0.3f, 0.7f };
		f.use_orbit_trap_colouring = false;
		f.use_fresnel = true;
		setupFormulas(f);
		params.objects.push_back(std::move(f));
	}

	// Camera
	params.camera.position = vec3r(0, 0, -3.5f);
	params.camera.look_at  = vec3r(0, 0, 0);
	params.camera.recompute();

	params.selected_object = 6; // Mandelbulb
}

static void loadCornellBoxAnimPreset(SceneParams & params, Timeline & timeline, AnimationSettings & anim_settings)
{
	// Load the base Cornell Box scene
	loadCornellBoxPreset(params);

	// Set up a looping camera orbit with 5 keyframes (gentle arc inside the box)
	timeline.keyframes.clear();
	timeline.duration_seconds = 3.0f;
	timeline.fps = 30;
	timeline.current_time = 0;
	timeline.playing = false;
	timeline.loop = true;

	struct KfData { float t; vec3r pos; };
	const KfData kf_data[] =
	{
		{ 0.00f, vec3r( 0.0f, 0.0f, -3.5f) },
		{ 0.75f, vec3r( 1.5f, 0.5f, -3.0f) },
		{ 1.50f, vec3r( 0.0f, 1.0f, -3.5f) },
		{ 2.25f, vec3r(-1.5f, 0.5f, -3.0f) },
		{ 3.00f, vec3r( 0.0f, 0.0f, -3.5f) },
	};

	for (const auto & kd : kf_data)
	{
		Keyframe kf;
		kf.time_seconds = kd.t;
		kf.camera = params.camera;
		kf.camera.position = kd.pos;
		kf.camera.look_at = vec3r(0, 0, 0);
		kf.camera.recompute();
		kf.objects = params.objects;
		kf.light = params.light;
		timeline.addKeyframe(kf);
	}

	// Configure animation settings for the test
	anim_settings.output_xres = 1280;
	anim_settings.output_yres = 720;
	anim_settings.target_spp = 32;
	anim_settings.output_dir = "cornell_anim";
	anim_settings.encode_mp4 = true;
}


static void loadDefaultPreset(SceneParams & params)
{
	params.objects.clear();

	SceneObjectDesc obj;
	obj.name = "AmoserSine";
	setupFormulas(obj);
	params.objects.push_back(std::move(obj));

	params.camera.position = vec3r{ 10, 5, -10 } * 0.25f;
	params.camera.look_at  = { 0, -0.125f, 0 };
	params.camera.recompute();

	params.selected_object = 0;
}


bool drawFileMenu(SceneParams & params, Timeline & timeline, AnimationSettings & anim_settings)
{
	static char save_path[256] = "scene.json";
	static char status_msg[256] = "";
	bool loaded = false;

	if (ImGui::CollapsingHeader("File"))
	{
		ImGui::InputText("Path##file", save_path, sizeof(save_path));

		if (ImGui::Button("Save Scene"))
		{
			if (saveScene(save_path, params))
				snprintf(status_msg, sizeof(status_msg), "Saved: %s", save_path);
			else
				snprintf(status_msg, sizeof(status_msg), "Save failed: %s", save_path);
		}

		ImGui::SameLine();
		if (ImGui::Button("Load Scene"))
		{
			if (loadScene(save_path, params))
			{
				snprintf(status_msg, sizeof(status_msg), "Loaded: %s", save_path);
				loaded = true;
			}
			else
				snprintf(status_msg, sizeof(status_msg), "Load failed: %s", save_path);
		}

		ImGui::Separator();
		ImGui::Text("Presets");
		if (ImGui::Button("Default Fractal"))
		{
			loadDefaultPreset(params);
			loaded = true;
			snprintf(status_msg, sizeof(status_msg), "Loaded default preset");
		}
		ImGui::SameLine();
		if (ImGui::Button("Cornell Box + Mandelbulb"))
		{
			loadCornellBoxPreset(params);
			loaded = true;
			snprintf(status_msg, sizeof(status_msg), "Loaded Cornell Box preset");
		}
		ImGui::SameLine();
		if (ImGui::Button("Cornell Box Animation"))
		{
			loadCornellBoxAnimPreset(params, timeline, anim_settings);
			loaded = true;
			snprintf(status_msg, sizeof(status_msg), "Loaded Cornell Box Animation preset (3s, 90 frames)");
		}

		if (status_msg[0])
			ImGui::TextWrapped("%s", status_msg);
	}

	return loaded;
}


void drawAnimationPanel(AnimationRenderer & anim_renderer, AnimationSettings & anim_settings,
	const Timeline & timeline, const SceneParams & params, int num_threads)
{
	if (!ImGui::CollapsingHeader("Render Animation"))
		return;

	if (anim_renderer.isRendering())
	{
		// Progress display
		const int cur_frame = anim_renderer.getCurrentFrame();
		const int tot_frames = anim_renderer.getTotalFrames();
		const int cur_pass = anim_renderer.getCurrentPass();
		const int tot_passes = anim_renderer.getTotalPasses();

		const float frame_progress = (tot_frames > 0) ? (float)cur_frame / tot_frames : 0;
		const float pass_progress = (tot_passes > 0) ? (float)cur_pass / tot_passes : 0;

		ImGui::Text("Frame %d / %d", cur_frame + 1, tot_frames);
		ImGui::ProgressBar(frame_progress, ImVec2(-1, 0), "Frames");

		ImGui::Text("Pass %d / %d", cur_pass + 1, tot_passes);
		ImGui::ProgressBar(pass_progress, ImVec2(-1, 0), "Passes");

		ImGui::TextWrapped("%s", anim_renderer.getStatusMessage().c_str());

		if (ImGui::Button("Cancel Render"))
			anim_renderer.cancel();
	}
	else
	{
		// Settings
		ImGui::SliderInt("SPP##anim", &anim_settings.target_spp, 1, 2048);
		ImGui::SliderInt("Width##anim", &anim_settings.output_xres, 320, 3840);
		ImGui::SliderInt("Height##anim", &anim_settings.output_yres, 180, 2160);

		static char output_dir[256] = "anim_output";
		ImGui::InputText("Output Dir##anim", output_dir, sizeof(output_dir));
		anim_settings.output_dir = output_dir;

		ImGui::Checkbox("Encode MP4##anim", &anim_settings.encode_mp4);

		const int total_frames = timeline.getTotalFrames();
		ImGui::Text("Timeline: %.1f s @ %d fps = %d frames", timeline.duration_seconds, timeline.fps, total_frames);
		ImGui::Text("Keyframes: %d", (int)timeline.keyframes.size());

		const bool can_render = !timeline.keyframes.empty() && total_frames > 0;

		if (!can_render)
			ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "Add at least one keyframe to render animation.");

		ImGui::BeginDisabled(!can_render);
		if (ImGui::Button("Render Animation"))
			anim_renderer.start(timeline, params, anim_settings, num_threads);
		ImGui::EndDisabled();

		// Show status from last render
		if (anim_renderer.isFinished())
			ImGui::TextWrapped("%s", anim_renderer.getStatusMessage().c_str());
	}
}
