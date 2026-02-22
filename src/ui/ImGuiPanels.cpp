#include "ImGuiPanels.h"
#include "SceneSerializer.h"
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


bool drawFormulaPanel(FractalParams & fp)
{
	bool changed = false;

	if (!ImGui::CollapsingHeader("Fractal Formula", ImGuiTreeNodeFlags_DefaultOpen))
		return false;

	// Formula selector
	const char * formulas[] = {
		"mandalay", "amazingbox", "amazingbox_mandalay", "mandelbulb", "hopfbrot",
		"burningship4d", "lambdabulb", "octopus", "mengersponge", "cubicbulb",
		"pseudokleinian", "riemannsphere", "spheretree", "benesipine2", "sphere"
	};
	const int num_formulas = sizeof(formulas) / sizeof(formulas[0]);

	int current = 0;
	for (int i = 0; i < num_formulas; i++)
		if (fp.formula_name == formulas[i]) { current = i; break; }

	if (ImGui::Combo("Formula", &current, formulas, num_formulas))
	{
		fp.formula_name = formulas[current];
		changed = true;
	}

	// Common DE parameters
	changed |= ImGui::SliderInt("Max Iterations", &fp.max_iters, 1, 200);
	changed |= SliderReal("Radius", &fp.radius, 0.1f, 10.0f);
	changed |= SliderReal("Step Scale", &fp.step_scale, 0.01f, 2.0f);
	changed |= SliderReal("Bailout Radius^2", &fp.bailout_radius2, 1.0f, 10000.0f, "%.1f");

	// Formula-specific parameters
	const std::string & name = fp.formula_name;

	if (name == "mandalay")
	{
		ImGui::Separator();
		ImGui::Text("MandalayKIFS Parameters");
		changed |= SliderReal("Scale##mandalay", &fp.mandalay_scale, -5.0f, 5.0f);
		changed |= SliderReal("Min R2##mandalay", &fp.mandalay_min_r2, 0.0f, 2.0f);
		changed |= SliderReal("Folding Offset##mandalay", &fp.mandalay_folding_offset, 0.0f, 3.0f);
		changed |= SliderReal("Z Tower##mandalay", &fp.mandalay_z_tower, -2.0f, 2.0f);
		changed |= SliderReal("XY Tower##mandalay", &fp.mandalay_xy_tower, -2.0f, 2.0f);
		changed |= DragVec3r("Rotate##mandalay", fp.mandalay_rotate, 0.005f);
		changed |= DragVec3r("Julia C##mandalay", fp.mandalay_julia_c, 0.01f);
		changed |= ImGui::Checkbox("Julia Mode##mandalay", &fp.mandalay_julia_mode);
	}
	else if (name == "amazingbox")
	{
		ImGui::Separator();
		ImGui::Text("Amazingbox Parameters");
		changed |= SliderReal("Scale##amazingbox", &fp.amazingbox_scale, -5.0f, 5.0f);
		changed |= SliderReal("Min R2##amazingbox", &fp.amazingbox_min_r2, 0.0f, 2.0f);
		changed |= SliderReal("Fix R2##amazingbox", &fp.amazingbox_fix_r2, 0.0f, 2.0f);
		changed |= SliderReal("Fold Limit##amazingbox", &fp.amazingbox_fold_limit, 0.0f, 3.0f);
		changed |= ImGui::Checkbox("Julia Mode##amazingbox", &fp.amazingbox_julia_mode);
	}
	else if (name == "amazingbox_mandalay")
	{
		ImGui::Separator();
		ImGui::Text("Hybrid Amazingbox Parameters");
		changed |= SliderReal("AB Scale", &fp.hybrid_amazingbox_scale, -5.0f, 5.0f);
		changed |= SliderReal("AB Fold Limit", &fp.hybrid_amazingbox_fold_limit, 0.0f, 3.0f);
		changed |= SliderReal("AB Min R2", &fp.hybrid_amazingbox_min_r2, 0.0f, 2.0f);

		ImGui::Separator();
		ImGui::Text("Hybrid Mandalay Parameters");
		changed |= SliderReal("MK Scale", &fp.hybrid_mandalay_scale, -5.0f, 5.0f);
		changed |= SliderReal("MK Folding Offset", &fp.hybrid_mandalay_folding_offset, 0.0f, 3.0f);
		changed |= SliderReal("MK Z Tower", &fp.hybrid_mandalay_z_tower, -2.0f, 2.0f);
		changed |= SliderReal("MK XY Tower", &fp.hybrid_mandalay_xy_tower, -2.0f, 2.0f);
		changed |= DragVec3r("MK Rotate", fp.hybrid_mandalay_rotate, 0.005f);
		changed |= ImGui::Checkbox("MK Julia Mode", &fp.hybrid_mandalay_julia_mode);
	}
	else if (name == "lambdabulb")
	{
		ImGui::Separator();
		ImGui::Text("Lambdabulb Parameters");
		changed |= SliderReal("Power##lambdabulb", &fp.lambdabulb_power, 2.0f, 8.0f);
		changed |= DragVec3r("C##lambdabulb", fp.lambdabulb_c, 0.005f);
	}
	else if (name == "octopus")
	{
		ImGui::Separator();
		ImGui::Text("Octopus Parameters");
		changed |= SliderReal("XZ Mul##octopus", &fp.octopus_xz_mul, 0.0f, 3.0f);
		changed |= SliderReal("Sq Mul##octopus", &fp.octopus_sq_mul, 0.0f, 3.0f);
		changed |= ImGui::Checkbox("Julia Mode##octopus", &fp.octopus_julia_mode);
	}
	else if (name == "cubicbulb")
	{
		ImGui::Separator();
		ImGui::Text("Cubicbulb Parameters");
		changed |= SliderReal("Y Mul##cubicbulb", &fp.cubicbulb_y_mul, 0.0f, 6.0f);
		changed |= SliderReal("Z Mul##cubicbulb", &fp.cubicbulb_z_mul, 0.0f, 6.0f);
		changed |= SliderReal("Aux Mul##cubicbulb", &fp.cubicbulb_aux_mul, 0.0f, 3.0f);
		changed |= DragVec3r("C##cubicbulb", fp.cubicbulb_c, 0.005f);
		changed |= ImGui::Checkbox("Julia Mode##cubicbulb", &fp.cubicbulb_julia_mode);
	}
	else if (name == "mengersponge")
	{
		ImGui::Separator();
		ImGui::Text("Menger Sponge Parameters");
		changed |= SliderReal("Scale##menger", &fp.mengersponge_scale, 1.0f, 6.0f);
		changed |= DragVec3r("Scale Centre##menger", fp.mengersponge_scale_centre, 0.01f);
	}
	else if (name == "benesipine2")
	{
		ImGui::Separator();
		ImGui::Text("BenesiPine2 Parameters");
		changed |= SliderReal("Scale##benesi", &fp.benesipine2_scale, 0.5f, 5.0f);
		changed |= SliderReal("Offset##benesi", &fp.benesipine2_offset, 0.0f, 2.0f);
		changed |= ImGui::Checkbox("Julia Mode##benesi", &fp.benesipine2_julia_mode);
	}
	else if (name == "riemannsphere")
	{
		ImGui::Separator();
		ImGui::Text("Riemann Sphere Parameters");
		changed |= SliderReal("Scale##riemann", &fp.riemannsphere_scale, -3.0f, 3.0f);
		changed |= SliderReal("S Shift##riemann", &fp.riemannsphere_s_shift, -2.0f, 2.0f);
		changed |= SliderReal("T Shift##riemann", &fp.riemannsphere_t_shift, -2.0f, 2.0f);
		changed |= SliderReal("X Shift##riemann", &fp.riemannsphere_x_shift, -2.0f, 2.0f);
		changed |= SliderReal("R Shift##riemann", &fp.riemannsphere_r_shift, -2.0f, 2.0f);
		changed |= SliderReal("R Power##riemann", &fp.riemannsphere_r_pow, 1.0f, 8.0f);
	}
	else if (name == "pseudokleinian")
	{
		ImGui::Separator();
		ImGui::Text("PseudoKleinian Parameters");
		changed |= ImGui::SliderFloat4("Mins##pk", fp.pseudokleinian_mins, -2.0f, 2.0f);
		changed |= ImGui::SliderFloat4("Maxs##pk", fp.pseudokleinian_maxs, -2.0f, 2.0f);
	}
	else if (name == "hopfbrot")
	{
		ImGui::Separator();
		ImGui::Text("Hopfbrot Parameters");
		changed |= SliderReal("Scene Scale##hopf", &fp.hopfbrot_scene_scale, 0.5f, 5.0f);
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


bool drawMaterialPanel(FractalParams & fp)
{
	bool changed = false;

	if (!ImGui::CollapsingHeader("Material"))
		return false;

	changed |= ImGui::ColorEdit3("Albedo", &fp.albedo.x());
	changed |= ImGui::ColorEdit3("Emission", &fp.emission.x());
	changed |= ImGui::Checkbox("Use Fresnel", &fp.use_fresnel);
	if (fp.use_fresnel)
		changed |= ImGui::SliderFloat("R0 (Fresnel)", &fp.r0, 0.0f, 1.0f);
	changed |= ImGui::Checkbox("Orbit Trap Colouring", &fp.use_orbit_trap_colouring);

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

	return changed;
}


bool drawStatsOverlay(int passes, int xres, int yres, float fps)
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
		ImGui::Text("Passes: %d", passes);
		ImGui::Text("FPS: %.1f", fps);
		if (ImGui::Button("Refresh"))
			refresh = true;
	}
	ImGui::End();
	return refresh;
}


bool drawFileMenu(SceneParams & params)
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
