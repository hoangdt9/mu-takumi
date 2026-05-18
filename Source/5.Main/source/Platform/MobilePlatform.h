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

/** Show server-select + login chrome over the cinematic/video background; ends tour overlay blocking input. */
void MU_AndroidRevealLoginServerUi();
/** If F4 06 never arrives, synthesize default connect groups and reveal UI (LAN QA). */
void MU_AndroidTickLoginSceneConnectFallback();
/** Call when entering login scene so fallback timers do not use stale WorldTime from loading. */
void MU_AndroidResetLoginSceneConnectFallback();
/** Call when C2 F4 06 server list is parsed (not the synthetic fallback list). */
void MU_AndroidNotifyWireServerListReceived();
/** Call when user taps a sub-server (before F4 03 / ReceiveServerConnect). */
void MU_AndroidNotifyServerSubPickStarted();
/** Opens LoginWin if F4 03 never arrives after sub pick (server-next / Docker LAN). */
void MU_AndroidTickLoginAfterServerPickFallback();
/** After game TCP connect: poll for C1 F1 00 (game-host may still be building in Docker). */
void MU_AndroidBeginJoinServerWait(const char* gameHost, int gamePort);
void MU_AndroidTickJoinServerWait();
void MU_AndroidResetJoinServerWait();
void MU_AndroidDismissLoginWaitMsgIfShown();
/** True after LAN connect failed and client should use 127.0.0.1 (adb reverse / Docker Desktop Mac). */
bool MU_AndroidShouldPreferLoopbackTcp();
void MU_AndroidSetPreferLoopbackTcp(bool prefer);
/** True while virtual joystick simulates MouseLButton for click-to-walk. */
bool MU_AndroidIsVirtualJoystickDrivingMouse();
/** True while the player is steering (joystick) — suppress auto-target / proximity combat. */
bool MU_AndroidShouldSuppressCombatTargeting();
#endif

#endif // defined(__ANDROID__) || defined(MU_IOS)
