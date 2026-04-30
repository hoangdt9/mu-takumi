#pragma once

// MainScene.h - Main game scene

#if !defined(__ANDROID__) && !defined(MU_IOS)
#include <windows.h>
#endif

#include "../ScenePerfTelemetry.h"

// Main scene lifecycle
void MoveMainScene();
bool RenderMainScene();
