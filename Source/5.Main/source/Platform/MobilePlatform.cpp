#if defined(__ANDROID__) || defined(MU_IOS)

#include "MobilePlatform.h"

#include <sokol_app.h>

#include <algorithm>
#include <array>
#include <cstring>
#include <initializer_list>

#if defined(__ANDROID__)
#include <android/native_activity.h>
#include <jni.h>
#include <unistd.h>
#endif

namespace
{
std::array<Uint8, SDL_NUM_SCANCODES> g_keyboardState = {};
SDL_Rect g_textInputRect = {};
bool g_textInputActive = false;

bool MU_PathExists(const char* path)
{
#if defined(__ANDROID__)
    return (path != nullptr) && (path[0] != '\0') && (access(path, F_OK) == 0);
#else
    (void)path;
    return false;
#endif
}

std::string MU_GetFirstExistingPath(std::initializer_list<const char*> candidates)
{
    for (const char* candidate : candidates)
    {
        if (MU_PathExists(candidate))
        {
            return candidate;
        }
    }
    return {};
}
} // namespace

#if defined(__ANDROID__)
namespace
{
void MU_AndroidCallActivityVoidMethod(const char* methodName, const char* methodSig)
{
    const void* pActivity = sapp_android_get_native_activity();
    if (pActivity == nullptr)
    {
        return;
    }

    auto* nativeActivity = static_cast<ANativeActivity*>(const_cast<void*>(pActivity));
    JavaVM* vm = nativeActivity->vm;
    JNIEnv* env = nullptr;
    const jint getEnvResult = vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
    bool needDetach = false;
    if (getEnvResult == JNI_EDETACHED)
    {
        if (vm->AttachCurrentThread(&env, nullptr) != JNI_OK || env == nullptr)
        {
            return;
        }
        needDetach = true;
    }
    else if (getEnvResult != JNI_OK || env == nullptr)
    {
        return;
    }

    jclass clazz = env->GetObjectClass(nativeActivity->clazz);
    if (clazz != nullptr)
    {
        jmethodID mid = env->GetMethodID(clazz, methodName, methodSig);
        if (mid == nullptr)
        {
            env->ExceptionClear();
        }
        else
        {
            env->CallVoidMethod(nativeActivity->clazz, mid);
        }
        if (env->ExceptionCheck())
        {
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
        env->DeleteLocalRef(clazz);
    }

    if (needDetach)
    {
        vm->DetachCurrentThread();
    }
}
} // namespace

static void MU_AndroidSyncImeBridgeBounds(int x, int y, int w, int h)
{
    const void* pActivity = sapp_android_get_native_activity();
    if (pActivity == nullptr)
    {
        return;
    }

    auto* nativeActivity = static_cast<ANativeActivity*>(const_cast<void*>(pActivity));
    JavaVM* vm = nativeActivity->vm;
    JNIEnv* env = nullptr;
    const jint getEnvResult = vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
    bool needDetach = false;
    if (getEnvResult == JNI_EDETACHED)
    {
        if (vm->AttachCurrentThread(&env, nullptr) != JNI_OK || env == nullptr)
        {
            return;
        }
        needDetach = true;
    }
    else if (getEnvResult != JNI_OK || env == nullptr)
    {
        return;
    }

    jclass clazz = env->GetObjectClass(nativeActivity->clazz);
    if (clazz != nullptr)
    {
        jmethodID mid = env->GetMethodID(clazz, "syncImeBridgeBounds", "(IIII)V");
        if (mid == nullptr)
        {
            env->ExceptionClear();
        }
        else
        {
            env->CallVoidMethod(nativeActivity->clazz, mid, x, y, w, h);
        }
        if (env->ExceptionCheck())
        {
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
        env->DeleteLocalRef(clazz);
    }

    if (needDetach)
    {
        vm->DetachCurrentThread();
    }
}
#endif

void MU_MobilePlatformInit()
{
    MU_MobileClearKeyboardState();
    g_textInputRect = {};
    g_textInputActive = false;
}

void MU_MobilePlatformShutdown()
{
    g_textInputActive = false;
    g_textInputRect = {};
    MU_MobileClearKeyboardState();
}

const Uint8* MU_MobileGetKeyboardState()
{
    return g_keyboardState.data();
}

void MU_MobileSetKeyState(SDL_Scancode scancode, bool isDown)
{
    if ((scancode >= 0) && (static_cast<size_t>(scancode) < g_keyboardState.size()))
    {
        g_keyboardState[static_cast<size_t>(scancode)] = isDown ? 1u : 0u;
    }
}

void MU_MobileClearKeyboardState()
{
    std::fill(g_keyboardState.begin(), g_keyboardState.end(), static_cast<Uint8>(0));
}

void MU_MobileStartTextInput()
{
    g_textInputActive = true;
#if defined(__ANDROID__)
    // Sokol uses ANativeActivity_showSoftInput, which often does not show IME for this app;
    // the Java bridge view receives composition and forwards to native.
    MU_AndroidCallActivityVoidMethod("showImeBridgeKeyboard", "()V");
    {
        const SDL_Rect& r = g_textInputRect;
        const int bw = (r.w > 0) ? r.w : 1;
        const int bh = (r.h > 0) ? r.h : 1;
        MU_AndroidSyncImeBridgeBounds(r.x, r.y, bw, bh);
    }
#else
    sapp_show_keyboard(true);
#endif
}

void MU_MobileStopTextInput()
{
    g_textInputActive = false;
#if defined(__ANDROID__)
    MU_AndroidCallActivityVoidMethod("hideImeBridgeKeyboard", "()V");
#else
    sapp_show_keyboard(false);
#endif
}

bool MU_MobileIsTextInputActive()
{
    return g_textInputActive || sapp_keyboard_shown();
}

void MU_MobileSetTextInputRect(const SDL_Rect* rect)
{
    if (rect)
    {
        g_textInputRect = *rect;
#if defined(__ANDROID__)
        const int bw = (rect->w > 0) ? rect->w : 1;
        const int bh = (rect->h > 0) ? rect->h : 1;
        MU_AndroidSyncImeBridgeBounds(rect->x, rect->y, bw, bh);
#endif
    }
    else
    {
        g_textInputRect = {};
    }
}

void MU_MobileRequestExit()
{
    sapp_request_quit();
#if defined(__ANDROID__)
    const void* pActivity = sapp_android_get_native_activity();
    if (pActivity != nullptr)
    {
        ANativeActivity_finish(static_cast<ANativeActivity*>(const_cast<void*>(pActivity)));
    }
#endif
}

std::string MU_MobileGetExternalDataPath()
{
    return MU_GetFirstExistingPath({
        "/sdcard/Android/data/com.muonline.client/files",
        "/storage/emulated/0/Android/data/com.muonline.client/files"
    });
}

std::string MU_MobileGetInternalDataPath()
{
    return MU_GetFirstExistingPath({
        "/data/user/0/com.muonline.client/files",
        "/data/data/com.muonline.client/files"
    });
}

#endif // defined(__ANDROID__) || defined(MU_IOS)
