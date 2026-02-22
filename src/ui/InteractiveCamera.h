#pragma once

#include "renderer/CameraParams.h"
#include <SDL2/SDL.h>


struct InteractiveCamera
{
	// Movement speed
	float move_speed      = 1.0f;
	float fast_multiplier = 3.0f;
	float mouse_sensitivity = 0.003f;
	float scroll_speed    = 0.1f;

	// Orbit mode
	bool orbit_mode = false;

	// Internal state
	float yaw   = 0;
	float pitch = 0;
	bool  mouse_captured = false;
	int   last_mouse_x = 0, last_mouse_y = 0;

	// Initialize yaw/pitch from current camera state
	void initFromCamera(const CameraParams & camera);

	// Process input and update camera. Returns true if camera changed.
	bool update(float dt, const Uint8 * keyboard, CameraParams & camera);

	// Process SDL events for mouse input. Returns true if camera changed.
	bool processEvent(const SDL_Event & event, CameraParams & camera);
};
