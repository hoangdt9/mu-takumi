#pragma once

// CharacterScene.h - Character selection scene

#if !defined(__ANDROID__) && !defined(MU_IOS)
#include <windows.h>
#endif

// Character scene lifecycle
void CreateCharacterScene();
void NewMoveCharacterScene();
bool NewRenderCharacterScene(HDC hDC);

// Character management
void StartGame();
