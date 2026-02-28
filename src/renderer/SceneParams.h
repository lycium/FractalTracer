#pragma once

#include "maths/vec.h"
#include "renderer/CameraParams.h"
#include "renderer/SceneObjectDesc.h"


struct LightParams
{
	vec3r light_pos       = { 8, 12, -6 };
	real  light_intensity = 720;

	// Procedural sky colours
	vec3f sky_up_colour = vec3f{  53, 112, 128 } * (1.0f / 255) * 0.75f;
	vec3f sky_hz_colour = vec3f{ 182, 175, 157 } * (1.0f / 255) * 0.8f;

	// HDR environment map path (empty = use procedural sky)
	std::string hdr_env_path;
};


struct RenderSettings
{
	int max_bounces     = 8;
	int target_passes   = 2310; // 2*3*5*7*11
	int resolution_x    = 1280;
	int resolution_y    = 720;
	int preview_divisor = 32;   // Sub-resolution divisor for interactive preview
};


struct SceneParams
{
	CameraParams   camera;
	std::vector<SceneObjectDesc> objects;
	LightParams    light;
	RenderSettings render;
	int  selected_object = 0;
	bool show_box = false;
};
