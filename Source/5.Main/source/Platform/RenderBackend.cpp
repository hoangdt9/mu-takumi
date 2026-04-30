#if defined(__ANDROID__) || defined(MU_IOS)

#include "stdafx.h"

#include "Platform/MobilePlatform.h"
#include "Platform/RenderBackend.h"

#include "Platform/gl_compat.h"

#include <GLES3/gl32.h>
#include <android/log.h>

#include <algorithm>
#include <cctype>
#include <string>

namespace
{
#define RENDER_LOG_TAG "MuRender"
#if defined(MU_ANDROID_DISABLE_LOG)
#define RENDER_LOGI(...) ((void)0)
#define RENDER_LOGW(...) ((void)0)
#else
#define RENDER_LOGI(...) __android_log_print(ANDROID_LOG_INFO, RENDER_LOG_TAG, __VA_ARGS__)
#define RENDER_LOGW(...) __android_log_print(ANDROID_LOG_WARN, RENDER_LOG_TAG, __VA_ARGS__)
#endif

class OpenGLCompatBackend final : public IRenderBackend
{
public:
    const char* GetName() const override
    {
        return "OpenGLCompat";
    }

    RenderBackendType GetType() const override
    {
        return RenderBackendType::OpenGLCompat;
    }

    bool Initialize(int drawableWidth, int drawableHeight) override
    {
        GL_Compat_Init();
        glViewport(0, 0, drawableWidth, drawableHeight);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        RENDER_LOGI("OpenGLCompat initialized (drawable=%dx%d)", drawableWidth, drawableHeight);
        return true;
    }

    void OnDrawableSizeChanged(int drawableWidth, int drawableHeight) override
    {
        glViewport(0, 0, drawableWidth, drawableHeight);
    }

    void Present() override
    {
        GL_FlushPending();
    }

    RenderBackendStats GetAndResetStats() override
    {
        RenderBackendStats stats {};
        GL_GetDrawStats(&stats.drawCalls, &stats.vertices);
        GL_GetDrawPathStats(&stats.imDrawCalls,
                            &stats.vaDirectDrawCalls,
                            &stats.vaConvertedDrawCalls,
                            &stats.quadIndexedDrawCalls,
                            &stats.quadExpandedDrawCalls);
        GL_ResetDrawStats();
        return stats;
    }

    void Shutdown() override
    {
        GL_Compat_Shutdown();
    }
};

class BgfxBackend final : public IRenderBackend
{
public:
    const char* GetName() const override
    {
        return "BGFX";
    }

    RenderBackendType GetType() const override
    {
        return RenderBackendType::Bgfx;
    }

    bool Initialize(int /*drawableWidth*/, int /*drawableHeight*/) override
    {
        const void* nativeWindow = MU_MobileGetNativeWindow();
        const void* eglDisplay = MU_MobileGetEglDisplay();
        const void* eglContext = MU_MobileGetEglContext();

#if defined(MU_BGFX_SUPPORT)
        RENDER_LOGI(
            "BGFX support compiled in (nwh=%p display=%p context=%p)",
            nativeWindow,
            eglDisplay,
            eglContext);
        RENDER_LOGW(
            "BGFX runtime port is still blocked: renderer code still emits legacy gl_compat/raw-GL calls, so fallback is required until the draw path is ported");
#else
        RENDER_LOGW(
            "BGFX backend requested but MU_ENABLE_BGFX is OFF (nwh=%p display=%p context=%p); fallback is required",
            nativeWindow,
            eglDisplay,
            eglContext);
#endif
        return false;
    }

    void OnDrawableSizeChanged(int /*drawableWidth*/, int /*drawableHeight*/) override {}

    void Present() override {}

    RenderBackendStats GetAndResetStats() override
    {
        return {};
    }

    void Shutdown() override {}
};
} // namespace

RenderBackendType ParseRenderBackendType(const char* value)
{
    if (!value || !value[0])
    {
        return RenderBackendType::OpenGLCompat;
    }

    std::string normalized(value);
    std::transform(
        normalized.begin(),
        normalized.end(),
        normalized.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    if (normalized == "bgfx")
    {
        return RenderBackendType::Bgfx;
    }

    return RenderBackendType::OpenGLCompat;
}

const char* RenderBackendTypeToString(RenderBackendType type)
{
    switch (type)
    {
    case RenderBackendType::Bgfx:
        return "bgfx";
    case RenderBackendType::OpenGLCompat:
    default:
        return "opengl";
    }
}

std::unique_ptr<IRenderBackend> CreateRenderBackend(RenderBackendType type)
{
    switch (type)
    {
    case RenderBackendType::Bgfx:
        return std::make_unique<BgfxBackend>();
    case RenderBackendType::OpenGLCompat:
    default:
        return std::make_unique<OpenGLCompatBackend>();
    }
}

#endif // __ANDROID__
