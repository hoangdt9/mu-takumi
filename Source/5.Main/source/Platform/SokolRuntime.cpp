#if defined(__ANDROID__) || defined(MU_IOS)

#define SOKOL_GLES3
#define SOKOL_APP_IMPL
#include <sokol_app.h>

const void* MU_MobileGetNativeWindow()
{
#if defined(__ANDROID__)
    return _sapp.android.current.window;
#else
    return nullptr;
#endif
}

const void* MU_MobileGetEglDisplay()
{
#if defined(__ANDROID__)
    return sapp_egl_get_display();
#else
    return nullptr;
#endif
}

const void* MU_MobileGetEglContext()
{
#if defined(__ANDROID__)
    return sapp_egl_get_context();
#else
    return nullptr;
#endif
}

#endif // defined(__ANDROID__) || defined(MU_IOS)
