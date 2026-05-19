#pragma once

#if defined(__ANDROID__)

#include "Utilities/Log/TakumiAndroidUiPerf.h"

/// PNG top-right HUD (balo / character / settings) over legacy main frame.
inline bool TakumiAndroid_UsePngHudOverlay()
{
#if TAKUMI_ANDROID_UI_PNG_HUD_OVERLAY
    return true;
#else
    return false;
#endif
}

/// Keep legacy bottom-right menu buttons visible (inventory / char / friend / window).
inline bool TakumiAndroid_ShouldHideLegacyMenuChrome()
{
    return false;
}

#else

inline bool TakumiAndroid_UsePngHudOverlay() { return false; }
inline bool TakumiAndroid_ShouldHideLegacyMenuChrome() { return false; }

#endif
