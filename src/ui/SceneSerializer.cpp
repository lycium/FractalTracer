#include "SceneSerializer.h"

#include <fstream>
#include <nlohmann/json.hpp>

#include "renderer/FormulaFactory.h"

using json = nlohmann::json;


// vec3r serialization
static json vec3rToJson(const vec3r & v) { return { v.x(), v.y(), v.z() }; }
static vec3r jsonToVec3r(const json & j) { return { (real)j[0], (real)j[1], (real)j[2] }; }

// vec3f serialization
static json vec3fToJson(const vec3f & v) { return { v.x(), v.y(), v.z() }; }
static vec3f jsonToVec3f(const json & j) { return { (float)j[0], (float)j[1], (float)j[2] }; }


// Serialize formula params generically via getParams()
static json formulaParamsToJson(IterationFunction * formula)
{
	json fp;
	if (!formula) return fp;

	for (const auto & p : formula->getParams())
	{
		switch (p.type)
		{
			case ParamInfo::Real:
				fp[p.name] = *(real *)p.ptr;
				break;
			case ParamInfo::Vec3r:
				fp[p.name] = vec3rToJson(*(vec3r *)p.ptr);
				break;
			case ParamInfo::Bool:
				fp[p.name] = *(bool *)p.ptr;
				break;
			case ParamInfo::Real4:
			{
				const real * v = (const real *)p.ptr;
				fp[p.name] = { v[0], v[1], v[2], v[3] };
				break;
			}
		}
	}
	return fp;
}


// Deserialize formula params generically via getParams()
static void jsonToFormulaParams(IterationFunction * formula, const json & fp)
{
	if (!formula || fp.empty()) return;

	for (auto & p : formula->getParams())
	{
		if (!fp.contains(p.name)) continue;

		switch (p.type)
		{
			case ParamInfo::Real:
				*(real *)p.ptr = (real)fp[p.name];
				break;
			case ParamInfo::Vec3r:
				*(vec3r *)p.ptr = jsonToVec3r(fp[p.name]);
				break;
			case ParamInfo::Bool:
				*(bool *)p.ptr = (bool)fp[p.name];
				break;
			case ParamInfo::Real4:
			{
				real * v = (real *)p.ptr;
				const auto & a = fp[p.name];
				for (int i = 0; i < 4; i++) v[i] = (real)a[i];
				break;
			}
		}
	}
}


static json objectToJson(const SceneObjectDesc & obj)
{
	json j;
	j["type"] = obj.type;
	j["name"] = obj.name;

	// Material
	j["albedo"]   = vec3fToJson(obj.albedo);
	j["emission"] = vec3fToJson(obj.emission);
	j["use_fresnel"] = obj.use_fresnel;
	j["r0"]       = obj.r0;
	j["use_orbit_trap_colouring"] = obj.use_orbit_trap_colouring;

	// Geometry
	j["position"] = vec3rToJson(obj.position);
	j["radius"]   = obj.radius;

	if (obj.type == "quad")
	{
		j["quad_u"] = vec3rToJson(obj.quad_u);
		j["quad_v"] = vec3rToJson(obj.quad_v);
	}

	if (obj.type == "fractal")
	{
		j["formula_name"]    = obj.formula_name;
		j["max_iters"]       = obj.max_iters;
		j["scene_scale"]     = obj.scene_scale;
		j["step_scale"]      = obj.step_scale;
		j["bailout_radius2"] = obj.bailout_radius2;

		// Serialize iteration_sequence
		json seq = json::array();
		for (char c : obj.iteration_sequence) seq.push_back((int)c);
		j["iteration_sequence"] = seq;

		// Serialize formula params
		json formulas_json = json::array();
		for (auto * f : obj.formulas)
			formulas_json.push_back(formulaParamsToJson(f));
		j["formula_params"] = formulas_json;
	}

	return j;
}


static SceneObjectDesc jsonToObject(const json & j)
{
	SceneObjectDesc obj;

	obj.type = j.value("type", std::string("fractal"));
	obj.name = j.value("name", std::string("Object"));

	// Material
	if (j.contains("albedo"))   obj.albedo   = jsonToVec3f(j["albedo"]);
	if (j.contains("emission")) obj.emission = jsonToVec3f(j["emission"]);
	obj.use_fresnel = j.value("use_fresnel", obj.use_fresnel);
	obj.r0          = j.value("r0",          obj.r0);
	obj.use_orbit_trap_colouring = j.value("use_orbit_trap_colouring", obj.use_orbit_trap_colouring);

	// Geometry
	if (j.contains("position")) obj.position = jsonToVec3r(j["position"]);
	obj.radius = j.value("radius", (real)obj.radius);

	if (obj.type == "quad")
	{
		if (j.contains("quad_u")) obj.quad_u = jsonToVec3r(j["quad_u"]);
		if (j.contains("quad_v")) obj.quad_v = jsonToVec3r(j["quad_v"]);
	}

	if (obj.type == "fractal")
	{
		obj.formula_name    = j.value("formula_name", obj.formula_name);
		obj.max_iters       = j.value("max_iters", obj.max_iters);
		obj.scene_scale     = j.value("scene_scale", (real)obj.scene_scale);
		obj.step_scale      = j.value("step_scale", (real)obj.step_scale);
		obj.bailout_radius2 = j.value("bailout_radius2", (real)obj.bailout_radius2);

		// Load iteration_sequence
		if (j.contains("iteration_sequence"))
		{
			obj.iteration_sequence.clear();
			for (const auto & v : j["iteration_sequence"])
				obj.iteration_sequence.push_back((char)(int)v);
		}

		// Create formula instances from formula_name
		setupFormulas(obj);

		// Restore formula params
		if (j.contains("formula_params"))
		{
			const auto & fp = j["formula_params"];
			for (size_t i = 0; i < std::min(fp.size(), obj.formulas.size()); i++)
				jsonToFormulaParams(obj.formulas[i], fp[i]);
		}
	}

	return obj;
}


bool saveScene(const std::string & path, const SceneParams & params)
{
	json j;
	j["version"] = 2;

	// Camera
	auto & cam = j["camera"];
	cam["position"]         = vec3rToJson(params.camera.position);
	cam["look_at"]          = vec3rToJson(params.camera.look_at);
	cam["world_up"]         = vec3rToJson(params.camera.world_up);
	cam["fov_deg"]          = params.camera.fov_deg;
	cam["dof_amount"]       = params.camera.dof_amount;
	cam["lens_radius"]      = params.camera.lens_radius;
	cam["focal_dist_scale"] = params.camera.focal_dist_scale;

	// Objects
	json objects_json = json::array();
	for (const auto & obj : params.objects)
		objects_json.push_back(objectToJson(obj));
	j["objects"] = objects_json;
	j["show_box"] = params.show_box;

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

	// Objects
	if (j.contains("objects"))
	{
		params.objects.clear();
		for (const auto & obj_json : j["objects"])
			params.objects.push_back(jsonToObject(obj_json));
	}
	params.show_box = j.value("show_box", false);

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
