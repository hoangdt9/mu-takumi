#pragma once

// -----------------------------------------------------------------------------
// Android: logcat + g_ErrorReport cost FPS when enabled at high frequency.
// Flip these to 1 locally, rebuild the APK, reproduce, then set back to 0.
// Gradle alternative: cppFlags "-DTAKUMI_ANDROID_DEBUG_PROTOCOL=1"
// -----------------------------------------------------------------------------
#if defined(__ANDROID__)

#ifndef TAKUMI_ANDROID_DEBUG_PROTOCOL
#define TAKUMI_ANDROID_DEBUG_PROTOCOL 0
#endif

#ifndef TAKUMI_ANDROID_DEBUG_WEAR_INVENTORY
#define TAKUMI_ANDROID_DEBUG_WEAR_INVENTORY 0
#endif

#else

#define TAKUMI_ANDROID_DEBUG_PROTOCOL 0
#define TAKUMI_ANDROID_DEBUG_WEAR_INVENTORY 0

#endif
