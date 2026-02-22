#pragma once

#include <string>
#include "renderer/SceneParams.h"


bool saveScene(const std::string & path, const SceneParams & params);
bool loadScene(const std::string & path, SceneParams & params);
