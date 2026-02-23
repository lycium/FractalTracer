#pragma once

#include "renderer/SceneParams.h"
#include "AnimationRenderer.h"
#include "Timeline.h"

// Returns true if any parameter was changed
bool drawObjectEditor(SceneParams & params);
bool drawCameraPanel(CameraParams & camera);
bool drawLightPanel(LightParams & light);
bool drawRenderSettingsPanel(RenderSettings & settings);
// Returns true if the Refresh button was pressed
bool drawStatsOverlay(int passes, int target_passes, int xres, int yres, float fps);

// File menu: returns true if params were loaded (caller should updateParams)
bool drawFileMenu(SceneParams & params, Timeline & timeline, AnimationSettings & anim_settings);

// Animation rendering panel
void drawAnimationPanel(AnimationRenderer & anim_renderer, AnimationSettings & anim_settings,
	const Timeline & timeline, const SceneParams & params, int num_threads);
