#pragma once

#if defined(__ANDROID__) || defined(MU_IOS)

#include <SDL.h>

#include <string>

void MU_MobilePlatformInit();
void MU_MobilePlatformShutdown();

const Uint8* MU_MobileGetKeyboardState();
void MU_MobileSetKeyState(SDL_Scancode scancode, bool isDown);
void MU_MobileClearKeyboardState();

void MU_MobileStartTextInput();
void MU_MobileStopTextInput();
bool MU_MobileIsTextInputActive();
void MU_MobileSetTextInputRect(const SDL_Rect* rect);

std::string MU_MobileGetExternalDataPath();
std::string MU_MobileGetInternalDataPath();

const void* MU_MobileGetNativeWindow();
const void* MU_MobileGetEglDisplay();
const void* MU_MobileGetEglContext();

/** Sokol quit + Android {@code ANativeActivity_finish} so the task actually closes after fatal dialogs. */
void MU_MobileRequestExit();

#if defined(__ANDROID__)
/** Absolute UTF-8 path to {@code MOVIE_FILE_WMV} (same as PC / MovieScene). Invokes {@code MuMainNativeActivity.playLoginIntroMovie}. */
void MU_AndroidPlayLoginIntroMoviePath(const char* utf8PathAbs);
void MU_AndroidStopLoginIntroMovie();

/** Looping intro under the OpenGL login layer (see {@code MOVIE_FILE_WMV} / {@code MOVIE_FILE_MP4}). */
void MU_AndroidTryStartLoginBackgroundMovie();
void MU_AndroidStopLoginBackgroundMovie();
bool MU_AndroidIsLoginBackgroundMovieActive();
void MU_AndroidMarkLoginBackgroundMovieStarted();
void MU_AndroidMarkLoginBackgroundMovieStopped();

/** Must run on the render thread with EGL current: bind MediaPlayer to OES texture, updateTexImage, draw full viewport. */
void MU_AndroidLoginBgVideoRenderTick();
#endif

#endif // defined(__ANDROID__) || defined(MU_IOS)
