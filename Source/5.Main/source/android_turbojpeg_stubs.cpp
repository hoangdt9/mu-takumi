// =============================================================================
// android_turbojpeg_stubs.cpp
// Real JPEG decoding on Android using stb_image (public domain).
// Implements the libjpeg-turbo (turbojpeg) API used by GlobalBitmap.cpp.
// =============================================================================

#ifdef __ANDROID__

#define STB_IMAGE_IMPLEMENTATION
// STBI_ONLY_JPEG removed — PNG support is needed for UI icon textures.
// stb_image supports JPEG + PNG + BMP + TGA + others by default.
#define STBI_NO_STDIO          // we feed raw memory buffers; file I/O done by caller
#define STBI_ASSERT(x)         // no assert needed
#include "stb_image.h"

#include "turbojpeg.h"

// Dummy handle — non-null sentinel
static int g_tjDummyHandle = 0;
static char g_tjErrorMsg[256] = "OK";

tjhandle tjInitDecompress(void) { return (tjhandle)&g_tjDummyHandle; }
tjhandle tjInitCompress(void)   { return (tjhandle)&g_tjDummyHandle; }
tjhandle tjInitTransform(void)  { return (tjhandle)&g_tjDummyHandle; }
int tjDestroy(tjhandle /*h*/)   { return 0; }
char* tjGetErrorStr(void)       { return g_tjErrorMsg; }

// ── tjDecompressHeader3 ────────────────────────────────────────────────────
// Peek at JPEG dimensions without decoding.
int tjDecompressHeader3(tjhandle /*handle*/,
                        const unsigned char* jpegBuf,
                        unsigned long jpegSize,
                        int* width, int* height,
                        int* jpegSubsamp, int* jpegColorspace)
{
    int w = 0, h = 0, comp = 0;
    if (!stbi_info_from_memory(jpegBuf, (int)jpegSize, &w, &h, &comp))
    {
        snprintf(g_tjErrorMsg, sizeof(g_tjErrorMsg), "stbi_info_from_memory failed: %s",
                 stbi_failure_reason() ? stbi_failure_reason() : "unknown");
        if (width)  *width  = 0;
        if (height) *height = 0;
        if (jpegSubsamp)    *jpegSubsamp    = 0;
        if (jpegColorspace) *jpegColorspace = 0;
        return -1;
    }
    if (width)  *width  = w;
    if (height) *height = h;
    if (jpegSubsamp)    *jpegSubsamp    = TJSAMP_444;
    if (jpegColorspace) *jpegColorspace = TJCS_RGB;
    return 0;
}

// ── tjDecompress2 ──────────────────────────────────────────────────────────
// Decode JPEG bytes into the caller-allocated dst buffer (RGB, no alpha).
// pitch=0 means tightly packed.
int tjDecompress2(tjhandle /*handle*/,
                  const unsigned char* jpegBuf,
                  unsigned long jpegSize,
                  unsigned char* dstBuf,
                  int width, int pitch, int height,
                  int /*pixelFormat*/, int /*flags*/)
{
    int w = 0, h = 0, comp = 0;
    unsigned char* data = stbi_load_from_memory(
        jpegBuf, (int)jpegSize, &w, &h, &comp, 3   // force RGB
    );
    if (!data)
    {
        snprintf(g_tjErrorMsg, sizeof(g_tjErrorMsg), "stbi_load_from_memory failed: %s",
                 stbi_failure_reason() ? stbi_failure_reason() : "unknown");
        return -1;
    }

    // Copy decoded pixels into caller's buffer.
    // pitch=0 → tightly packed (w*3 bytes per row).
    const int rowStride = (pitch > 0) ? pitch : (w * 3);
    if (rowStride == w * 3 && w == width && h == height)
    {
        memcpy(dstBuf, data, (size_t)w * h * 3);
    }
    else
    {
        const int copyW = (w < width)  ? w : width;
        const int copyH = (h < height) ? h : height;
        for (int row = 0; row < copyH; ++row)
        {
            memcpy(dstBuf + (size_t)row * rowStride,
                   data   + (size_t)row * w * 3,
                   (size_t)copyW * 3);
        }
    }
    stbi_image_free(data);
    return 0;
}

// ── Unused compress stubs ──────────────────────────────────────────────────
unsigned long tjBufSize(int width, int height, int /*jpegSubsamp*/)
{
    return (unsigned long)(width * height * 4 + 1024);
}

int tjCompress2(tjhandle /*h*/,
                const unsigned char* /*src*/,
                int /*w*/, int /*pitch*/, int /*h2*/,
                int /*fmt*/,
                unsigned char** /*jpegBuf*/, unsigned long* /*jpegSize*/,
                int /*subsamp*/, int /*qual*/, int /*flags*/)
{
    return -1;
}

#endif // __ANDROID__
