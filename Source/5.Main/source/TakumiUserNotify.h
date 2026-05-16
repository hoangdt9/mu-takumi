#pragma once

// In-game modal notifications (same UI as invalid captcha / Interface::OpenMessageBox).
// Drawn every frame from RenderInfomation() so login + character scenes show alerts
// even when custom register windows are closed. MonoGame migration can reimplement this API.

void TakumiUserNotify_Draw();
bool TakumiUserNotify_IsVisible();

void TakumiUserNotify_Show(const char* caption, const char* format, ...);
void TakumiUserNotify_ShowError(const char* format, ...);
void TakumiUserNotify_ShowInfo(const char* caption, const char* format, ...);
