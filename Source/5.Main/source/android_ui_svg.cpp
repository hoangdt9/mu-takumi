// =============================================================================
// android_ui_svg.cpp
// Rasterize UI SVGs (nanosvg / nanosvgrast, zlib license) for GLES texture upload.
// =============================================================================

#include "android_ui_svg.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <vector>

#define NANOSVG_IMPLEMENTATION
#include "nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvgrast.h"

static void FlipRgbaVertical(const int w, const int h, unsigned char* pixels)
{
    if (w <= 0 || h <= 0 || pixels == nullptr)
    {
        return;
    }

    const int stride = w * 4;
    std::vector<unsigned char> row(static_cast<size_t>(stride));
    for (int y = 0; y < h / 2; ++y)
    {
        unsigned char* a = pixels + y * stride;
        unsigned char* b = pixels + (h - 1 - y) * stride;
        std::memcpy(row.data(), a, static_cast<size_t>(stride));
        std::memcpy(a, b, static_cast<size_t>(stride));
        std::memcpy(b, row.data(), static_cast<size_t>(stride));
    }
}

bool MuAndroid_LoadSvgAssetToRgbaBottomFirst(
    const char* assetPath,
    const int maxRasterSide,
    std::vector<unsigned char>& outRgba,
    int& outW,
    int& outH)
{
    outW = 0;
    outH = 0;
    outRgba.clear();

    if (assetPath == nullptr || assetPath[0] == '\0' || maxRasterSide < 1)
    {
        return false;
    }

    std::ifstream file(assetPath, std::ios::binary | std::ios::ate);
    if (!file)
    {
        return false;
    }

    const std::streamsize size = file.tellg();
    if (size <= 0)
    {
        return false;
    }

    std::vector<char> svgBuf(static_cast<size_t>(size) + 1u);
    file.seekg(0, std::ios::beg);
    if (!file.read(svgBuf.data(), size))
    {
        return false;
    }
    svgBuf[static_cast<size_t>(size)] = '\0';

    NSVGimage* image = nsvgParse(svgBuf.data(), "px", 96.0f);
    if (image == nullptr)
    {
        return false;
    }

    const float iw = image->width;
    const float ih = image->height;
    if (iw <= 0.001f || ih <= 0.001f)
    {
        nsvgDelete(image);
        return false;
    }

    const float maxSide = static_cast<float>(maxRasterSide);
    const float scale = maxSide / std::max(iw, ih);
    const int w = std::max(1, static_cast<int>(std::ceil(iw * scale)));
    const int h = std::max(1, static_cast<int>(std::ceil(ih * scale)));

    NSVGrasterizer* rast = nsvgCreateRasterizer();
    if (rast == nullptr)
    {
        nsvgDelete(image);
        return false;
    }

    outRgba.resize(static_cast<size_t>(w) * static_cast<size_t>(h) * 4u);
    nsvgRasterize(rast, image, 0.0f, 0.0f, scale, outRgba.data(), w, h, w * 4);
    nsvgDeleteRasterizer(rast);
    nsvgDelete(image);

    FlipRgbaVertical(w, h, outRgba.data());
    outW = w;
    outH = h;
    return true;
}
