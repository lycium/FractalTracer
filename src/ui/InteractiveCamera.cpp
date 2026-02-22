#include "InteractiveCamera.h"

#include <cmath>
#include "imgui.h"


void InteractiveCamera::initFromCamera(const CameraParams & camera)
{
	const vec3r dir = normalise(camera.look_at - camera.position);
	yaw   = (float)std::atan2(dir.x(), dir.z());
	pitch = (float)std::asin(std::max(-1.0, std::min(1.0, (double)dir.y())));
}


bool InteractiveCamera::update(float dt, const Uint8 * keyboard, CameraParams & camera)
{
	// Don't process keyboard input if ImGui wants it
	if (ImGui::GetIO().WantCaptureKeyboard)
		return false;

	bool changed = false;
	const float speed = move_speed * dt * (keyboard[SDL_SCANCODE_LSHIFT] ? fast_multiplier : 1.0f);

	camera.recompute();

	if (orbit_mode)
	{
		// WASD orbits around look_at
		const real dist = length(camera.position - camera.look_at);
		if (keyboard[SDL_SCANCODE_W]) { pitch += speed; changed = true; }
		if (keyboard[SDL_SCANCODE_S]) { pitch -= speed; changed = true; }
		if (keyboard[SDL_SCANCODE_A]) { yaw -= speed; changed = true; }
		if (keyboard[SDL_SCANCODE_D]) { yaw += speed; changed = true; }

		// Clamp pitch
		pitch = std::max(-1.5f, std::min(1.5f, pitch));

		if (changed)
		{
			camera.position = camera.look_at + vec3r{
				std::sin(yaw) * std::cos(pitch),
				std::sin(pitch),
				std::cos(yaw) * std::cos(pitch)
			} * dist;
		}
	}
	else
	{
		// Free-fly mode: WASD + Q/E
		vec3r move = { 0, 0, 0 };
		if (keyboard[SDL_SCANCODE_W]) move = move + camera.forward * speed;
		if (keyboard[SDL_SCANCODE_S]) move = move - camera.forward * speed;
		if (keyboard[SDL_SCANCODE_A]) move = move - camera.right * speed;
		if (keyboard[SDL_SCANCODE_D]) move = move + camera.right * speed;
		if (keyboard[SDL_SCANCODE_Q]) move = move - camera.up * speed;
		if (keyboard[SDL_SCANCODE_E]) move = move + camera.up * speed;

		if (length(move) > 1e-8f)
		{
			camera.position = camera.position + move;
			camera.look_at  = camera.look_at + move;
			changed = true;
		}
	}

	return changed;
}


bool InteractiveCamera::processEvent(const SDL_Event & event, CameraParams & camera)
{
	// Don't process mouse input if ImGui wants it
	if (ImGui::GetIO().WantCaptureMouse)
		return false;

	bool changed = false;

	if (event.type == SDL_MOUSEBUTTONDOWN && (event.button.button == SDL_BUTTON_LEFT || event.button.button == SDL_BUTTON_RIGHT))
	{
		mouse_captured = true;
		last_mouse_x = event.button.x;
		last_mouse_y = event.button.y;
		SDL_SetRelativeMouseMode(SDL_TRUE);
	}
	else if (event.type == SDL_MOUSEBUTTONUP && (event.button.button == SDL_BUTTON_LEFT || event.button.button == SDL_BUTTON_RIGHT))
	{
		mouse_captured = false;
		SDL_SetRelativeMouseMode(SDL_FALSE);
	}
	else if (event.type == SDL_MOUSEMOTION && mouse_captured)
	{
		const float dx = (float)event.motion.xrel * mouse_sensitivity;
		const float dy = (float)event.motion.yrel * mouse_sensitivity;

		yaw   += dx;
		pitch -= dy;
		pitch = std::max(-1.5f, std::min(1.5f, pitch));

		if (orbit_mode)
		{
			const real dist = length(camera.position - camera.look_at);
			camera.position = camera.look_at + vec3r{
				std::sin(yaw) * std::cos(pitch),
				std::sin(pitch),
				std::cos(yaw) * std::cos(pitch)
			} * dist;
		}
		else
		{
			// Update look_at from yaw/pitch relative to position
			const vec3r dir = {
				std::sin(yaw) * std::cos(pitch),
				std::sin(pitch),
				std::cos(yaw) * std::cos(pitch)
			};
			camera.look_at = camera.position + dir;
		}

		changed = true;
	}
	else if (event.type == SDL_MOUSEWHEEL)
	{
		if (orbit_mode)
		{
			// Zoom in/out by changing distance to look_at
			camera.recompute();
			const real dist = length(camera.position - camera.look_at);
			const real new_dist = dist * (1.0f - event.wheel.y * scroll_speed);
			camera.position = camera.look_at - camera.forward * new_dist;
			changed = true;
		}
	}
	else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_TAB)
	{
		if (!ImGui::GetIO().WantCaptureKeyboard)
		{
			orbit_mode = !orbit_mode;
		}
	}

	return changed;
}
