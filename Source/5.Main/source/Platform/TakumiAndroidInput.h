#pragma once

#if defined(__ANDROID__)

#include <SDL_events.h>

/// Pending one-frame inventory right-click (book/gem use). Peek without consuming.
bool TakumiAndroid_PeekInventoryUsePress();

/// Consume the pending inventory use press (returns true once).
bool TakumiAndroid_ConsumeInventoryUsePress();

/// Drop pending press without acting (e.g. no item under cursor).
void TakumiAndroid_CancelInventoryUsePress();

/// Pulse VK_RBUTTON / MouseRButton + key state for UI that reads mouse/keyboard this frame.
void TakumiAndroid_PulseRightClick();

/// Run inventory use handling immediately after SDL events (before ZzzInterface hotkey path).
void TakumiAndroid_ProcessInventoryUseFrame();

/// Deferred single-tap melee (waits past double-tap window so double-tap does not melee first).
void TakumiAndroid_ProcessWorldSkillFrame();

bool TakumiAndroid_HandleInventoryTouchDown(const SDL_TouchFingerEvent& touch);
bool TakumiAndroid_HandleInventoryTouchMove(const SDL_TouchFingerEvent& touch);
/// Returns true when long-press or double-tap "use item" consumed the touch.
bool TakumiAndroid_HandleInventoryTouchUp(const SDL_TouchFingerEvent& touch);

/// World combat: long-press / double-tap on monster = PC right-click skill (hotbar CurrentSkill).
bool TakumiAndroid_HandleWorldSkillTouchDown(const SDL_TouchFingerEvent& touch);
bool TakumiAndroid_HandleWorldSkillTouchMove(const SDL_TouchFingerEvent& touch);
/// Returns true when skill gesture consumed the touch (suppress LMB release attack).
bool TakumiAndroid_HandleWorldSkillTouchUp(const SDL_TouchFingerEvent& touch);

#else

inline bool TakumiAndroid_PeekInventoryUsePress() { return false; }
inline bool TakumiAndroid_ConsumeInventoryUsePress() { return false; }
inline void TakumiAndroid_CancelInventoryUsePress() {}
inline void TakumiAndroid_PulseRightClick() {}
inline void TakumiAndroid_ProcessInventoryUseFrame() {}
inline void TakumiAndroid_ProcessWorldSkillFrame() {}
inline bool TakumiAndroid_HandleInventoryTouchDown(const void*) { return false; }
inline bool TakumiAndroid_HandleInventoryTouchMove(const void*) { return false; }
inline bool TakumiAndroid_HandleInventoryTouchUp(const void*) { return false; }
inline bool TakumiAndroid_HandleWorldSkillTouchDown(const void*) { return false; }
inline bool TakumiAndroid_HandleWorldSkillTouchMove(const void*) { return false; }
inline bool TakumiAndroid_HandleWorldSkillTouchUp(const void*) { return false; }

#endif
