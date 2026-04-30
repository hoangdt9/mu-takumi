#pragma once

#if defined(__ANDROID__) || defined(MU_IOS)

#include <SDL.h>

#include <memory>

enum class RenderBackendType
{
    OpenGLCompat,
    Bgfx
};

struct RenderBackendStats
{
    int drawCalls = 0;
    int vertices = 0;
    int imDrawCalls = 0;
    int vaDirectDrawCalls = 0;
    int vaConvertedDrawCalls = 0;
    int quadIndexedDrawCalls = 0;
    int quadExpandedDrawCalls = 0;
};

class IRenderBackend
{
public:
    virtual ~IRenderBackend() = default;

    virtual const char* GetName() const = 0;
    virtual RenderBackendType GetType() const = 0;
    virtual bool Initialize(int drawableWidth, int drawableHeight) = 0;
    virtual void OnDrawableSizeChanged(int drawableWidth, int drawableHeight) = 0;
    virtual void Present() = 0;
    virtual RenderBackendStats GetAndResetStats() = 0;
    virtual void Shutdown() = 0;
};

RenderBackendType ParseRenderBackendType(const char* value);
const char* RenderBackendTypeToString(RenderBackendType type);
std::unique_ptr<IRenderBackend> CreateRenderBackend(RenderBackendType type);

#endif // __ANDROID__
