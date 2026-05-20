#pragma once

#include <vector>

// Rasterize an SVG from disk (same paths as LoadUITextureAsset / cwd game files dir).
// On success: outRgba is tightly-packed RGBA8, w*h*4 bytes, first row = bottom (matches
// stb_image + stbi_set_flip_vertically_on_load(1) / GL upload in LoadUITextureAsset).
// maxRasterSide caps the longer edge (e.g. 512).
bool MuAndroid_LoadSvgAssetToRgbaBottomFirst(
    const char* assetPath,
    int maxRasterSide,
    std::vector<unsigned char>& outRgba,
    int& outW,
    int& outH);
