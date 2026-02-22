#include "SceneSerializer.h"

#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;


// vec3r serialization
static json vec3rToJson(const vec3r & v) { return { v.x(), v.y(), v.z() }; }
static vec3r jsonToVec3r(const json & j) { return { (real)j[0], (real)j[1], (real)j[2] }; }

// vec3f serialization
static json vec3fToJson(const vec3f & v) { return { v.x(), v.y(), v.z() }; }
static vec3f jsonToVec3f(const json & j) { return { (float)j[0], (float)j[1], (float)j[2] }; }


bool saveScene(const std::string & path, const SceneParams & params)
{
	json j;
	j["version"] = 1;

	// Camera
	auto & cam = j["camera"];
	cam["position"]         = vec3rToJson(params.camera.position);
	cam["look_at"]          = vec3rToJson(params.camera.look_at);
	cam["world_up"]         = vec3rToJson(params.camera.world_up);
	cam["fov_deg"]          = params.camera.fov_deg;
	cam["dof_amount"]       = params.camera.dof_amount;
	cam["lens_radius"]      = params.camera.lens_radius;
	cam["focal_dist_scale"] = params.camera.focal_dist_scale;

	// Fractal
	auto & fr = j["fractal"];
	fr["formula_name"]    = params.fractal.formula_name;
	fr["radius"]          = params.fractal.radius;
	fr["scene_scale"]     = params.fractal.scene_scale;
	fr["step_scale"]      = params.fractal.step_scale;
	fr["bailout_radius2"] = params.fractal.bailout_radius2;
	fr["max_iters"]       = params.fractal.max_iters;
	fr["show_box"]        = params.fractal.show_box;

	// Material
	auto & mat = j["material"];
	mat["albedo"]      = vec3fToJson(params.fractal.albedo);
	mat["emission"]    = vec3fToJson(params.fractal.emission);
	mat["use_fresnel"] = params.fractal.use_fresnel;
	mat["r0"]          = params.fractal.r0;
	mat["use_orbit_trap_colouring"] = params.fractal.use_orbit_trap_colouring;

	// Formula-specific params
	auto & fp = j["formula_params"];
	const auto & f = params.fractal;
	fp["mandalay_scale"]          = f.mandalay_scale;
	fp["mandalay_min_r2"]         = f.mandalay_min_r2;
	fp["mandalay_folding_offset"] = f.mandalay_folding_offset;
	fp["mandalay_z_tower"]        = f.mandalay_z_tower;
	fp["mandalay_xy_tower"]       = f.mandalay_xy_tower;
	fp["mandalay_rotate"]         = vec3rToJson(f.mandalay_rotate);
	fp["mandalay_julia_c"]        = vec3rToJson(f.mandalay_julia_c);
	fp["mandalay_julia_mode"]     = f.mandalay_julia_mode;

	fp["amazingbox_scale"]      = f.amazingbox_scale;
	fp["amazingbox_min_r2"]     = f.amazingbox_min_r2;
	fp["amazingbox_fix_r2"]     = f.amazingbox_fix_r2;
	fp["amazingbox_fold_limit"] = f.amazingbox_fold_limit;
	fp["amazingbox_julia_mode"] = f.amazingbox_julia_mode;

	fp["hybrid_amazingbox_scale"]        = f.hybrid_amazingbox_scale;
	fp["hybrid_amazingbox_fold_limit"]   = f.hybrid_amazingbox_fold_limit;
	fp["hybrid_amazingbox_min_r2"]       = f.hybrid_amazingbox_min_r2;
	fp["hybrid_mandalay_scale"]          = f.hybrid_mandalay_scale;
	fp["hybrid_mandalay_folding_offset"] = f.hybrid_mandalay_folding_offset;
	fp["hybrid_mandalay_z_tower"]        = f.hybrid_mandalay_z_tower;
	fp["hybrid_mandalay_xy_tower"]       = f.hybrid_mandalay_xy_tower;
	fp["hybrid_mandalay_rotate"]         = vec3rToJson(f.hybrid_mandalay_rotate);
	fp["hybrid_mandalay_julia_mode"]     = f.hybrid_mandalay_julia_mode;

	fp["lambdabulb_power"] = f.lambdabulb_power;
	fp["lambdabulb_c"]     = vec3rToJson(f.lambdabulb_c);

	fp["amosersine_scale"]      = f.amosersine_scale;
	fp["amosersine_julia_c"]    = vec3rToJson(f.amosersine_julia_c);
	fp["amosersine_julia_mode"] = f.amosersine_julia_mode;

	fp["octopus_xz_mul"]     = f.octopus_xz_mul;
	fp["octopus_sq_mul"]     = f.octopus_sq_mul;
	fp["octopus_julia_mode"] = f.octopus_julia_mode;

	fp["cubicbulb_y_mul"]      = f.cubicbulb_y_mul;
	fp["cubicbulb_z_mul"]      = f.cubicbulb_z_mul;
	fp["cubicbulb_aux_mul"]    = f.cubicbulb_aux_mul;
	fp["cubicbulb_c"]          = vec3rToJson(f.cubicbulb_c);
	fp["cubicbulb_julia_mode"] = f.cubicbulb_julia_mode;

	fp["benesipine2_scale"]      = f.benesipine2_scale;
	fp["benesipine2_offset"]     = f.benesipine2_offset;
	fp["benesipine2_julia_mode"] = f.benesipine2_julia_mode;

	fp["mengersponge_scale"]        = f.mengersponge_scale;
	fp["mengersponge_scale_centre"] = vec3rToJson(f.mengersponge_scale_centre);

	fp["pseudokleinian_mins"] = { f.pseudokleinian_mins[0], f.pseudokleinian_mins[1], f.pseudokleinian_mins[2], f.pseudokleinian_mins[3] };
	fp["pseudokleinian_maxs"] = { f.pseudokleinian_maxs[0], f.pseudokleinian_maxs[1], f.pseudokleinian_maxs[2], f.pseudokleinian_maxs[3] };

	fp["riemannsphere_scale"]   = f.riemannsphere_scale;
	fp["riemannsphere_s_shift"] = f.riemannsphere_s_shift;
	fp["riemannsphere_t_shift"] = f.riemannsphere_t_shift;
	fp["riemannsphere_x_shift"] = f.riemannsphere_x_shift;
	fp["riemannsphere_r_shift"] = f.riemannsphere_r_shift;
	fp["riemannsphere_r_pow"]   = f.riemannsphere_r_pow;

	fp["hopfbrot_scene_scale"] = f.hopfbrot_scene_scale;
	fp["sphere_r0"]            = f.sphere_r0;

	// Light
	auto & lt = j["light"];
	lt["light_pos"]       = vec3rToJson(params.light.light_pos);
	lt["light_intensity"] = params.light.light_intensity;
	lt["sky_up_colour"]   = vec3fToJson(params.light.sky_up_colour);
	lt["sky_hz_colour"]   = vec3fToJson(params.light.sky_hz_colour);
	lt["hdr_env_path"]    = params.light.hdr_env_path;

	// Render settings
	auto & rs = j["render"];
	rs["max_bounces"]      = params.render.max_bounces;
	rs["target_passes"]    = params.render.target_passes;
	rs["resolution_x"]     = params.render.resolution_x;
	rs["resolution_y"]     = params.render.resolution_y;
	rs["preview_divisor"]  = params.render.preview_divisor;

	std::ofstream file(path);
	if (!file.is_open()) return false;
	file << j.dump(2);
	return true;
}


// Helper to safely read json values with defaults
template<typename T>
static T get(const json & j, const std::string & key, const T & def)
{
	if (j.contains(key)) return j[key].get<T>();
	return def;
}


bool loadScene(const std::string & path, SceneParams & params)
{
	std::ifstream file(path);
	if (!file.is_open()) return false;

	json j;
	try { j = json::parse(file); }
	catch (...) { return false; }

	FractalParams defaults_fp;
	CameraParams defaults_cam;
	LightParams defaults_lt;
	RenderSettings defaults_rs;

	// Camera
	if (j.contains("camera"))
	{
		auto & cam = j["camera"];
		if (cam.contains("position"))         params.camera.position         = jsonToVec3r(cam["position"]);
		if (cam.contains("look_at"))          params.camera.look_at          = jsonToVec3r(cam["look_at"]);
		if (cam.contains("world_up"))         params.camera.world_up         = jsonToVec3r(cam["world_up"]);
		params.camera.fov_deg          = get(cam, "fov_deg",          defaults_cam.fov_deg);
		params.camera.dof_amount       = get(cam, "dof_amount",       defaults_cam.dof_amount);
		params.camera.lens_radius      = get(cam, "lens_radius",      defaults_cam.lens_radius);
		params.camera.focal_dist_scale = get(cam, "focal_dist_scale", defaults_cam.focal_dist_scale);
		params.camera.recompute();
	}

	// Fractal
	if (j.contains("fractal"))
	{
		auto & fr = j["fractal"];
		params.fractal.formula_name    = get<std::string>(fr, "formula_name", defaults_fp.formula_name);
		params.fractal.radius          = get(fr, "radius",          defaults_fp.radius);
		params.fractal.scene_scale     = get(fr, "scene_scale",     defaults_fp.scene_scale);
		params.fractal.step_scale      = get(fr, "step_scale",      defaults_fp.step_scale);
		params.fractal.bailout_radius2 = get(fr, "bailout_radius2", defaults_fp.bailout_radius2);
		params.fractal.max_iters       = get(fr, "max_iters",       defaults_fp.max_iters);
		params.fractal.show_box        = get(fr, "show_box",        defaults_fp.show_box);
	}

	// Material
	if (j.contains("material"))
	{
		auto & mat = j["material"];
		if (mat.contains("albedo"))   params.fractal.albedo   = jsonToVec3f(mat["albedo"]);
		if (mat.contains("emission")) params.fractal.emission = jsonToVec3f(mat["emission"]);
		params.fractal.use_fresnel = get(mat, "use_fresnel", defaults_fp.use_fresnel);
		params.fractal.r0          = get(mat, "r0",          defaults_fp.r0);
		params.fractal.use_orbit_trap_colouring = get(mat, "use_orbit_trap_colouring", defaults_fp.use_orbit_trap_colouring);
	}

	// Formula-specific params
	if (j.contains("formula_params"))
	{
		auto & fp = j["formula_params"];
		auto & f = params.fractal;

		f.mandalay_scale          = get(fp, "mandalay_scale",          defaults_fp.mandalay_scale);
		f.mandalay_min_r2         = get(fp, "mandalay_min_r2",         defaults_fp.mandalay_min_r2);
		f.mandalay_folding_offset = get(fp, "mandalay_folding_offset", defaults_fp.mandalay_folding_offset);
		f.mandalay_z_tower        = get(fp, "mandalay_z_tower",        defaults_fp.mandalay_z_tower);
		f.mandalay_xy_tower       = get(fp, "mandalay_xy_tower",       defaults_fp.mandalay_xy_tower);
		if (fp.contains("mandalay_rotate"))  f.mandalay_rotate = jsonToVec3r(fp["mandalay_rotate"]);
		if (fp.contains("mandalay_julia_c")) f.mandalay_julia_c = jsonToVec3r(fp["mandalay_julia_c"]);
		f.mandalay_julia_mode = get(fp, "mandalay_julia_mode", defaults_fp.mandalay_julia_mode);

		f.amazingbox_scale      = get(fp, "amazingbox_scale",      defaults_fp.amazingbox_scale);
		f.amazingbox_min_r2     = get(fp, "amazingbox_min_r2",     defaults_fp.amazingbox_min_r2);
		f.amazingbox_fix_r2     = get(fp, "amazingbox_fix_r2",     defaults_fp.amazingbox_fix_r2);
		f.amazingbox_fold_limit = get(fp, "amazingbox_fold_limit", defaults_fp.amazingbox_fold_limit);
		f.amazingbox_julia_mode = get(fp, "amazingbox_julia_mode", defaults_fp.amazingbox_julia_mode);

		f.hybrid_amazingbox_scale        = get(fp, "hybrid_amazingbox_scale",        defaults_fp.hybrid_amazingbox_scale);
		f.hybrid_amazingbox_fold_limit   = get(fp, "hybrid_amazingbox_fold_limit",   defaults_fp.hybrid_amazingbox_fold_limit);
		f.hybrid_amazingbox_min_r2       = get(fp, "hybrid_amazingbox_min_r2",       defaults_fp.hybrid_amazingbox_min_r2);
		f.hybrid_mandalay_scale          = get(fp, "hybrid_mandalay_scale",          defaults_fp.hybrid_mandalay_scale);
		f.hybrid_mandalay_folding_offset = get(fp, "hybrid_mandalay_folding_offset", defaults_fp.hybrid_mandalay_folding_offset);
		f.hybrid_mandalay_z_tower        = get(fp, "hybrid_mandalay_z_tower",        defaults_fp.hybrid_mandalay_z_tower);
		f.hybrid_mandalay_xy_tower       = get(fp, "hybrid_mandalay_xy_tower",       defaults_fp.hybrid_mandalay_xy_tower);
		if (fp.contains("hybrid_mandalay_rotate")) f.hybrid_mandalay_rotate = jsonToVec3r(fp["hybrid_mandalay_rotate"]);
		f.hybrid_mandalay_julia_mode = get(fp, "hybrid_mandalay_julia_mode", defaults_fp.hybrid_mandalay_julia_mode);

		f.lambdabulb_power = get(fp, "lambdabulb_power", defaults_fp.lambdabulb_power);
		if (fp.contains("lambdabulb_c")) f.lambdabulb_c = jsonToVec3r(fp["lambdabulb_c"]);

		f.amosersine_scale      = get(fp, "amosersine_scale",      defaults_fp.amosersine_scale);
		if (fp.contains("amosersine_julia_c")) f.amosersine_julia_c = jsonToVec3r(fp["amosersine_julia_c"]);
		f.amosersine_julia_mode = get(fp, "amosersine_julia_mode", defaults_fp.amosersine_julia_mode);

		f.octopus_xz_mul     = get(fp, "octopus_xz_mul",     defaults_fp.octopus_xz_mul);
		f.octopus_sq_mul     = get(fp, "octopus_sq_mul",     defaults_fp.octopus_sq_mul);
		f.octopus_julia_mode = get(fp, "octopus_julia_mode", defaults_fp.octopus_julia_mode);

		f.cubicbulb_y_mul      = get(fp, "cubicbulb_y_mul",      defaults_fp.cubicbulb_y_mul);
		f.cubicbulb_z_mul      = get(fp, "cubicbulb_z_mul",      defaults_fp.cubicbulb_z_mul);
		f.cubicbulb_aux_mul    = get(fp, "cubicbulb_aux_mul",    defaults_fp.cubicbulb_aux_mul);
		if (fp.contains("cubicbulb_c")) f.cubicbulb_c = jsonToVec3r(fp["cubicbulb_c"]);
		f.cubicbulb_julia_mode = get(fp, "cubicbulb_julia_mode", defaults_fp.cubicbulb_julia_mode);

		f.benesipine2_scale      = get(fp, "benesipine2_scale",      defaults_fp.benesipine2_scale);
		f.benesipine2_offset     = get(fp, "benesipine2_offset",     defaults_fp.benesipine2_offset);
		f.benesipine2_julia_mode = get(fp, "benesipine2_julia_mode", defaults_fp.benesipine2_julia_mode);

		f.mengersponge_scale = get(fp, "mengersponge_scale", defaults_fp.mengersponge_scale);
		if (fp.contains("mengersponge_scale_centre")) f.mengersponge_scale_centre = jsonToVec3r(fp["mengersponge_scale_centre"]);

		if (fp.contains("pseudokleinian_mins")) { auto & a = fp["pseudokleinian_mins"]; for (int i = 0; i < 4; i++) f.pseudokleinian_mins[i] = a[i]; }
		if (fp.contains("pseudokleinian_maxs")) { auto & a = fp["pseudokleinian_maxs"]; for (int i = 0; i < 4; i++) f.pseudokleinian_maxs[i] = a[i]; }

		f.riemannsphere_scale   = get(fp, "riemannsphere_scale",   defaults_fp.riemannsphere_scale);
		f.riemannsphere_s_shift = get(fp, "riemannsphere_s_shift", defaults_fp.riemannsphere_s_shift);
		f.riemannsphere_t_shift = get(fp, "riemannsphere_t_shift", defaults_fp.riemannsphere_t_shift);
		f.riemannsphere_x_shift = get(fp, "riemannsphere_x_shift", defaults_fp.riemannsphere_x_shift);
		f.riemannsphere_r_shift = get(fp, "riemannsphere_r_shift", defaults_fp.riemannsphere_r_shift);
		f.riemannsphere_r_pow   = get(fp, "riemannsphere_r_pow",   defaults_fp.riemannsphere_r_pow);

		f.hopfbrot_scene_scale = get(fp, "hopfbrot_scene_scale", defaults_fp.hopfbrot_scene_scale);
		f.sphere_r0            = get(fp, "sphere_r0",            defaults_fp.sphere_r0);
	}

	// Light
	if (j.contains("light"))
	{
		auto & lt = j["light"];
		if (lt.contains("light_pos"))     params.light.light_pos       = jsonToVec3r(lt["light_pos"]);
		params.light.light_intensity = get(lt, "light_intensity", defaults_lt.light_intensity);
		if (lt.contains("sky_up_colour")) params.light.sky_up_colour   = jsonToVec3f(lt["sky_up_colour"]);
		if (lt.contains("sky_hz_colour")) params.light.sky_hz_colour   = jsonToVec3f(lt["sky_hz_colour"]);
		params.light.hdr_env_path    = get<std::string>(lt, "hdr_env_path", defaults_lt.hdr_env_path);
	}

	// Render settings
	if (j.contains("render"))
	{
		auto & rs = j["render"];
		params.render.max_bounces     = get(rs, "max_bounces",     defaults_rs.max_bounces);
		params.render.target_passes   = get(rs, "target_passes",   defaults_rs.target_passes);
		params.render.resolution_x    = get(rs, "resolution_x",    defaults_rs.resolution_x);
		params.render.resolution_y    = get(rs, "resolution_y",    defaults_rs.resolution_y);
		params.render.preview_divisor = get(rs, "preview_divisor", defaults_rs.preview_divisor);
	}

	return true;
}
