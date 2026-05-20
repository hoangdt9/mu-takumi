#pragma once

#if defined(__ANDROID__) || defined(MU_IOS)

#include <SDL_touch.h>

struct MobileChatPanelRect
{
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;
};

void MU_MobileChatHudInit();
void MU_MobileChatHudSyncLayout();
void MU_MobileChatHudSetShowingLines(int showingLines);
MobileChatPanelRect MU_MobileGetChatPanelRect();
MobileChatPanelRect MU_MobileGetChatResizeHandleRect();

bool MU_MobileHitTestChatPanel(float uiX, float uiY);
bool MU_MobileHitTestChatInputBar(float uiX, float uiY);
bool MU_MobileHitTestChatLogArea(float uiX, float uiY);
bool MU_MobileHitTestChatResizeHandle(float uiX, float uiY);

bool MU_MobileHandleChatResizeFingerDownAt(float uiX, float uiY, SDL_FingerID fingerId);
bool MU_MobileHandleChatResizeFingerMotionAt(float uiY, SDL_FingerID fingerId);
bool MU_MobileHandleChatResizeFingerUp(SDL_FingerID fingerId);

bool MU_MobileHandleChatUiFingerDownAt(float uiX, float uiY, SDL_FingerID fingerId);
bool MU_MobileHandleChatUiFingerMotionAt(float uiX, float uiY, SDL_FingerID fingerId);
bool MU_MobileHandleChatUiFingerUp(SDL_FingerID fingerId);
bool MU_MobileIsChatUiFingerCaptured(SDL_FingerID fingerId);
bool MU_MobileIsChatUiCapturing();

void MU_MobileToggleChatPanelCollapsed();
void MU_MobileChatHudRestoreLegacyLayout();

bool MU_MobileIsChatChannelVisible();
void MU_MobileToggleChatChannelVisible();

#endif
