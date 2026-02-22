#pragma once

#include "renderer/SceneParams.h"
#include "AnimationRenderer.h"
#include "Timeline.h"

// Returns true if any parameter was changed
bool drawFormulaPanel(FractalParams & fp);
bool drawCameraPanel(CameraParams & camera);
bool drawMaterialPanel(FractalParams & fp);
bool drawLightPanel(LightParams & light);
bool drawRenderSettingsPanel(RenderSettings & settings);
void drawStatsOverlay(int passes, int xres, int yres, float fps);

// File menu: returns true if params were loaded (caller should updateParams)
bool drawFileMenu(SceneParams & params);

// Animation rendering panel
void drawAnimationPanel(AnimationRenderer & anim_renderer, AnimationSettings & anim_settings,
	const Timeline & timeline, const SceneParams & params, int num_threads);
