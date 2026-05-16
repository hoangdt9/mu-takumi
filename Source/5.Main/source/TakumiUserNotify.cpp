#include "stdafx.h"
#include "TakumiUserNotify.h"
#include "CBInterface.h"
#include "NewUISystem.h"

void TakumiUserNotify_Draw()
{
	// g_pBCustomMenuInfo is created when custom UI Work() runs (in-game); login scene
	// may show the modal only after that subsystem is ready.
	if (g_pBCustomMenuInfo != nullptr)
	{
		gInterface.DrawMessageBox();
	}
}

bool TakumiUserNotify_IsVisible()
{
	return gInterface.Data[eWindowMessageBox].OnShow != 0;
}

void TakumiUserNotify_Show(const char* caption, const char* format, ...)
{
	char text[1024] = { 0 };
	va_list va;
	va_start(va, format);
	vsnprintf(text, sizeof(text), (format != NULL) ? format : "", va);
	va_end(va);

	gInterface.OpenMessageBox(
		const_cast<char*>(caption != NULL ? caption : ""),
		"%s",
		text);
}

void TakumiUserNotify_ShowError(const char* format, ...)
{
	char text[1024] = { 0 };
	va_list va;
	va_start(va, format);
	vsnprintf(text, sizeof(text), (format != NULL) ? format : "", va);
	va_end(va);

	gInterface.OpenMessageBox(const_cast<char*>("Error"), "%s", text);
}

void TakumiUserNotify_ShowInfo(const char* caption, const char* format, ...)
{
	char text[1024] = { 0 };
	va_list va;
	va_start(va, format);
	vsnprintf(text, sizeof(text), (format != NULL) ? format : "", va);
	va_end(va);

	gInterface.OpenMessageBox(
		const_cast<char*>(caption != NULL ? caption : "Info"),
		"%s",
		text);
}
