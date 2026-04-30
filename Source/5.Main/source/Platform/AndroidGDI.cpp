// =============================================================================
// AndroidGDI.cpp
// Android GDI text-rendering stubs built on SDL2_ttf.
// Provides CreateFont / CreateDIBSection / TextOut / GetTextExtentPoint32 etc.
// =============================================================================
#ifdef __ANDROID__

#include "AndroidGDI.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <android/log.h>
#include <fstream>
#include <limits>
#include <string.h>
#include <wchar.h>
#include <vector>
#include <string>

#define LOG_TAG "AndroidGDI"
#if defined(MU_ANDROID_DISABLE_LOG)
#define LOGI(...) ((void)0)
#define LOGE(...) ((void)0)
#else
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#endif

// ── Font paths ────────────────────────────────────────────────────────────
// Resolved relative to the working directory set by android_main.cpp
static const char* s_FontPathNormal = "Data/fonts/font_.ttf";
static const char* s_FontPathBold   = "Data/fonts/font_2.ttf";

static int s_DefaultFontSize = 14;
static bool s_TTFInited = false;

// ── TTF_Font cache (keyed by size+bold) ──────────────────────────────────
struct FontCacheEntry {
    int  size;
    bool bold;
    TTF_Font* font;
    std::vector<unsigned char> fontBytes;
};
static std::vector<FontCacheEntry> s_FontCache;

static bool ReadFontFile(const char* path, std::vector<unsigned char>& outBytes)
{
    outBytes.clear();
    if (!path || !path[0])
    {
        return false;
    }

    std::ifstream stream(path, std::ios::binary | std::ios::ate);
    if (!stream)
    {
        return false;
    }

    const std::streamsize fileSize = stream.tellg();
    if ((fileSize <= 0) || (fileSize > static_cast<std::streamsize>(std::numeric_limits<int>::max())))
    {
        return false;
    }

    outBytes.resize(static_cast<size_t>(fileSize));
    stream.seekg(0, std::ios::beg);
    if (!stream.read(reinterpret_cast<char*>(outBytes.data()), fileSize))
    {
        outBytes.clear();
        return false;
    }

    return true;
}

static TTF_Font* TryOpenFont(const char* path, int size, std::vector<unsigned char>& outFontBytes) {
    if (!ReadFontFile(path, outFontBytes))
    {
        return nullptr;
    }

    SDL_RWops* rw = SDL_RWFromConstMem(outFontBytes.data(), static_cast<int>(outFontBytes.size()));
    if (!rw)
    {
        outFontBytes.clear();
        return nullptr;
    }

    TTF_Font* f = TTF_OpenFontRW(rw, 1, size);
    if (!f)
    {
        outFontBytes.clear();
        return nullptr;
    }

    LOGI("Font OK: %s sz=%d", path, size);
    return f;
}

static TTF_Font* GetCachedFont(int size, bool bold) {
    for (auto& e : s_FontCache)
        if (e.size == size && e.bold == bold)
            return e.font;

    TTF_Font* f = nullptr;
    std::vector<unsigned char> fontBytes;

    // 1. Relative paths (via chdir to external storage)
    if (!f) f = TryOpenFont(bold ? s_FontPathBold   : s_FontPathNormal, size, fontBytes);
    if (!f) f = TryOpenFont(s_FontPathNormal, size, fontBytes);

    // 2. Absolute paths to the app data directory
    static const char* sGameFontsNormal[] = {
        "/sdcard/Android/data/com.muonline.client/files/Data/fonts/font_.ttf",
        "/storage/emulated/0/Android/data/com.muonline.client/files/Data/fonts/font_.ttf",
        "/data/user/0/com.muonline.client/files/Data/fonts/font_.ttf",
        "/data/data/com.muonline.client/files/Data/fonts/font_.ttf",
        nullptr
    };
    static const char* sGameFontsBold[] = {
        "/sdcard/Android/data/com.muonline.client/files/Data/fonts/font_2.ttf",
        "/storage/emulated/0/Android/data/com.muonline.client/files/Data/fonts/font_2.ttf",
        "/data/user/0/com.muonline.client/files/Data/fonts/font_2.ttf",
        "/data/data/com.muonline.client/files/Data/fonts/font_2.ttf",
        nullptr
    };
    const char** gameFonts = bold ? sGameFontsBold : sGameFontsNormal;
    for (int i = 0; !f && gameFonts[i]; ++i) f = TryOpenFont(gameFonts[i], size, fontBytes);
    for (int i = 0; !f && sGameFontsNormal[i]; ++i) f = TryOpenFont(sGameFontsNormal[i], size, fontBytes);

    // 3. Android system fonts fallback
    static const char* sSys[] = {
        "/system/fonts/DroidSans.ttf",
        "/system/fonts/Roboto-Regular.ttf",
        "/system/fonts/NotoSans-Regular.ttf",
        "/system/fonts/NotoSansCJK-Regular.ttc",
        "/system/fonts/Arial.ttf",
        "/system/fonts/DroidSansFallback.ttf",
        nullptr
    };
    static const char* sSysBold[] = {
        "/system/fonts/DroidSans-Bold.ttf",
        "/system/fonts/Roboto-Bold.ttf",
        "/system/fonts/NotoSans-Bold.ttf",
        nullptr
    };
    const char** arr = (bold && !f) ? sSysBold : sSys;
    for (int i = 0; !f && arr[i]; ++i) f = TryOpenFont(arr[i], size, fontBytes);
    // last resort: any sys font
    for (int i = 0; !f && sSys[i]; ++i)  f = TryOpenFont(sSys[i], size, fontBytes);

    if (!f) {
        LOGE("No font found for size=%d bold=%d", size, (int)bold);
        return nullptr;
    }

    if (bold) TTF_SetFontStyle(f, TTF_STYLE_BOLD);
    s_FontCache.push_back({size, bold, f, std::move(fontBytes)});
    return f;
}

// ── wchar_t → Uint16 conversion ──────────────────────────────────────────
// SDL_ttf SizeUNICODE/RenderUNICODE expects UTF-16 Uint16 array.
// Android wchar_t is 4 bytes (UTF-32). Convert only BMP codepoints (covers Latin, Korean, etc.)
static std::vector<Uint16> ToUint16(const wchar_t* text, int len) {
    std::vector<Uint16> out;
    out.reserve(len + 1);
    for (int i = 0; i < len && text[i]; ++i) {
        wchar_t c = text[i];
        if (c < 0xD800 || (c >= 0xE000 && c <= 0xFFFF))
            out.push_back((Uint16)c);
        else
            out.push_back(0x25A1); // □ replacement
    }
    out.push_back(0); // null terminator
    return out;
}

// ── Public API ────────────────────────────────────────────────────────────

void AndroidGDI_Init(int defaultFontSizePx) {
    s_DefaultFontSize = defaultFontSizePx;
    if (!s_TTFInited) {
        if (TTF_Init() < 0)
            LOGE("TTF_Init failed: %s", SDL_GetError());
        else {
            s_TTFInited = true;
            LOGI("SDL2_ttf initialized, defaultSize=%d", defaultFontSizePx);
        }
    }
    // Pre-load common font sizes
    GetCachedFont(defaultFontSizePx, false);
    GetCachedFont(defaultFontSizePx, true);
}

void AndroidGDI_Shutdown() {
    for (auto& e : s_FontCache)
        if (e.font) TTF_CloseFont(e.font);
    s_FontCache.clear();
    if (s_TTFInited) { TTF_Quit(); s_TTFInited = false; }
}

// ── CreateDIBSection ──────────────────────────────────────────────────────
// bmiPtr = pointer to BITMAPINFO (defined in PlatformDefs.h but we just read it raw)
struct RawBITMAPINFOHEADER { int biSize; int biWidth; int biHeight; short biPlanes; short biBitCount; };
HBITMAP AndroidCreateDIBSection(const void* bmiPtr, void** ppvBits) {
    if (!bmiPtr || !ppvBits) { if (ppvBits) *ppvBits = nullptr; return nullptr; }
    const RawBITMAPINFOHEADER* hdr = (const RawBITMAPINFOHEADER*)bmiPtr;
    int w = hdr->biWidth;
    int h = hdr->biHeight < 0 ? -hdr->biHeight : hdr->biHeight;
    int bpp = hdr->biBitCount;
    int pitch = ((w * bpp + 31) & ~31) >> 3;
    size_t sz = (size_t)pitch * (size_t)(h > 0 ? h : 1);

    AndroidBitmap* bmp = new AndroidBitmap;
    bmp->type  = ANDROID_GDI_OBJECT_BITMAP;
    bmp->data  = (uint8_t*)calloc(sz, 1);
    bmp->width  = w;
    bmp->height = h;
    bmp->pitch  = pitch;
    *ppvBits = bmp->data;
    return bmp;
}

HDC AndroidCreateCompatibleDC(HDC /*src*/) {
    AndroidDC* dc = new AndroidDC;
    dc->bmp       = nullptr;
    dc->font      = nullptr;
    dc->textColor = 0xFFFFFF00; // RGB(255,255,255) in COLORREF (0x00BBGGRR → store as 0xRRGGBB00 shifted)
    dc->bgColor   = 0x00000000;
    return dc;
}

HFONT AndroidCreateFont(int height, int weight) {
    if (height <= 0) height = s_DefaultFontSize;
    bool bold = (weight >= 600);
    AndroidFont* f = new AndroidFont;
    f->type    = ANDROID_GDI_OBJECT_FONT;
    f->ttfFont = GetCachedFont(height, bold);
    f->size    = height;
    f->bold    = bold;
    return f;
}

HGDIOBJ AndroidSelectObject(HDC hdc, HGDIOBJ obj) {
    if (!hdc || !obj) return nullptr;
    // Try to detect type: check if it's an AndroidBitmap or AndroidFont by guessing
    // We use a simple tag approach: first field of AndroidBitmap is uint8_t* (8 bytes),
    // first field of AndroidFont is void* (ttfFont). We differentiate by callers:
    // SelectObject(hdc, m_hBitmap) — obj is HBITMAP
    // SelectObject(hdc, g_hFont)   — obj is HFONT cast to HGDIOBJ
    // We can't type-check void*, so we rely on calling convention:
    // PlatformDefs.h calls AndroidSelectObject with explicit casts.
    // The callers always do SelectObject(hdc, (HGDIOBJ)bitmap) or SelectObject(hdc, (HGDIOBJ)font).
    // We'll check the 'data' pointer field: if it looks like a valid heap ptr range for a bitmap
    // vs a TTF_Font*. This is fragile but the game code always selects bitmap first, then font.
    // Better: we store a type tag.
    // For now just return nullptr (the returned HGDIOBJ isn't used by the game).
    return nullptr;
}

// AndroidSelectBitmap / AndroidSelectFont — explicit typed versions
// Called from our updated PlatformDefs.h macros

void AndroidSelectBitmap(HDC hdc, HBITMAP bmp) {
    if (hdc) hdc->bmp = bmp;
}

void AndroidSelectFont(HDC hdc, HFONT font) {
    if (hdc) hdc->font = font;
}

bool AndroidTextOut(HDC hdc, int x, int y, const wchar_t* text, int len) {
    {
        static int s_entryCount = 0;
        if (s_entryCount < 10) {
            LOGI("AndroidTextOut ENTRY len=%d hdc=%p bmp=%p text[0]=%d",
                 len, (void*)hdc,
                 (void*)(hdc ? hdc->bmp : nullptr),
                 (text && len > 0) ? (int)text[0] : -1);
            s_entryCount++;
        }
    }
    if (!hdc || !hdc->bmp || !hdc->bmp->data || !text || len <= 0) return false;

    // Determine which font to use
    AndroidFont* af = hdc->font;
    int fontSize = af ? af->size : s_DefaultFontSize;
    bool bold    = af ? af->bold : false;
    TTF_Font* font = af ? (TTF_Font*)af->ttfFont : GetCachedFont(fontSize, bold);
    if (!font) return false;

    // Convert wchar_t to Uint16 for SDL_ttf
    std::vector<Uint16> utext = ToUint16(text, len);
    if (utext.size() <= 1) return false; // empty after conversion

    // Render white text on black
    SDL_Color fg = {255, 255, 255, 255};
    SDL_Surface* surf = TTF_RenderUNICODE_Blended(font, utext.data(), fg);
    if (!surf) {
        LOGE("TTF_RenderUNICODE_Blended failed: %s", SDL_GetError());
        return false;
    }

    // surf is SDL_ARGB8888 (4 bytes/pixel, A=alpha, R, G, B from high to low)
    // DIB buffer is RGB24 — pitch = ((width*24+31)&~31)/8
    AndroidBitmap* bmp = hdc->bmp;

    // Clear the text area first (black background)
    int textW = surf->w;
    int textH = surf->h;
    for (int row = 0; row < textH; ++row) {
        int dstY = y + row;
        if (dstY < 0 || dstY >= bmp->height) continue;
        uint8_t* dstRow = bmp->data + dstY * bmp->pitch + x * 3;
        int clearW = textW;
        if (x + clearW > bmp->width) clearW = bmp->width - x;
        if (clearW > 0) memset(dstRow, 0, clearW * 3);
    }

    // Lock surface if needed
    if (SDL_MUSTLOCK(surf)) SDL_LockSurface(surf);

    // Copy surf pixels → DIB (RGB24)
    for (int row = 0; row < textH; ++row) {
        int dstY = y + row;
        if (dstY < 0 || dstY >= bmp->height) continue;

        uint8_t* srcRow = (uint8_t*)surf->pixels + row * surf->pitch;
        uint8_t* dstRow = bmp->data + dstY * bmp->pitch;

        for (int col = 0; col < textW; ++col) {
            int dstX = x + col;
            if (dstX < 0 || dstX >= bmp->width) continue;

            // SDL_ARGB8888: byte order on little-endian = B, G, R, A
            uint8_t* src = srcRow + col * 4;
            uint8_t A = src[3];

            // TTF_RenderUNICODE_Blended renders white text: R=G=B=255 always.
            // WriteText checks dst[0]==255 for "fully covered" pixels.
            // Store alpha as luminance so pure glyph pixels → 255, edges → partial.
            uint8_t* dst = dstRow + dstX * 3;
            dst[0] = A;
            dst[1] = A;
            dst[2] = A;
        }
    }

    if (SDL_MUSTLOCK(surf)) SDL_UnlockSurface(surf);

    // Debug: log surface info + max alpha found
    {
        static int s_dbgTOCount = 0;
        if (s_dbgTOCount < 5) {
            uint8_t maxA = 0;
            for (int r = 0; r < textH && maxA < 255; ++r) {
                uint8_t* row = (uint8_t*)surf->pixels + r * surf->pitch;
                for (int c = 0; c < textW && maxA < 255; ++c)
                    if (row[c*4+3] > maxA) maxA = row[c*4+3];
            }
            uint8_t* dib0 = bmp->data; // first pixel of DIB
            LOGI("TextOut surf=%dx%d fmt=0x%X maxA=%d dib[0..2]=%d,%d,%d",
                 textW, textH, surf->format->format, maxA,
                 dib0[0], dib0[1], dib0[2]);
            s_dbgTOCount++;
        }
    }

    SDL_FreeSurface(surf);
    return true;
}

bool AndroidGetTextExtentPoint32(HDC hdc, const wchar_t* text, int len, int* outW, int* outH) {
    *outW = 0; *outH = 0;
    if (!text || len <= 0) return false;

    AndroidFont* af = hdc ? hdc->font : nullptr;
    int fontSize = af ? af->size : s_DefaultFontSize;
    bool bold    = af ? af->bold : false;
    TTF_Font* font = af ? (TTF_Font*)af->ttfFont : GetCachedFont(fontSize, bold);
    if (!font) {
        // Fallback estimate
        *outW = len * fontSize / 2;
        *outH = fontSize;
        return false;
    }

    std::vector<Uint16> utext = ToUint16(text, len);
    if (utext.size() <= 1) {
        // Empty string — use dummy size of "0" character
        Uint16 zero[2] = {'0', 0};
        TTF_SizeUNICODE(font, zero, outW, outH);
        return true;
    }
    return TTF_SizeUNICODE(font, utext.data(), outW, outH) == 0;
}

void AndroidSetTextColor(HDC hdc, uint32_t colorref) {
    if (hdc) hdc->textColor = colorref;
}

void AndroidSetBkColor(HDC hdc, uint32_t colorref) {
    if (hdc) hdc->bgColor = colorref;
}

bool AndroidDeleteDC(HDC hdc) {
    if (!hdc) return false;
    delete hdc;
    return true;
}

bool AndroidDeleteObject(HGDIOBJ obj) {
    if (!obj) return false;
    const auto type = *(reinterpret_cast<const AndroidGDIObjectType*>(obj));
    if (type == ANDROID_GDI_OBJECT_BITMAP) {
        AndroidBitmap* bmp = reinterpret_cast<AndroidBitmap*>(obj);
        if (bmp->data) {
            free(bmp->data);
            bmp->data = nullptr;
        }
        delete bmp;
        return true;
    }
    if (type == ANDROID_GDI_OBJECT_FONT) {
        AndroidFont* font = reinterpret_cast<AndroidFont*>(obj);
        delete font;
        return true;
    }
    LOGE("AndroidDeleteObject: unknown object type");
    return false;
}

// ── TTF_GetError shim (in case it's not linked) ───────────────────────────
// SDL_ttf provides TTF_GetError as a macro → SDL_GetError(), no issue.

#endif // __ANDROID__

