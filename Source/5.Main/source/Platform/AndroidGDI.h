#pragma once
// =============================================================================
// AndroidGDI.h
// Android-side GDI stub types and declarations.
// Replaces void* handles with real structs so TextOut/GetTextExtentPoint32 work.
// Implemented in AndroidGDI.cpp using SDL2_ttf.
// =============================================================================
#if defined(__ANDROID__) || defined(MU_IOS)

#include <stdint.h>
#include <stdlib.h>

// ── Internal GDI handle structs ───────────────────────────────────────────

enum AndroidGDIObjectType : uint32_t {
    ANDROID_GDI_OBJECT_UNKNOWN = 0,
    ANDROID_GDI_OBJECT_BITMAP  = 1,
    ANDROID_GDI_OBJECT_FONT    = 2
};

struct AndroidBitmap {
    AndroidGDIObjectType type;
    uint8_t* data;      // RGB24 pixel data (calloc'd)
    int      width;
    int      height;
    int      pitch;     // bytes per scanline = ((width*24+31)&~31)/8
};

struct AndroidFont {
    AndroidGDIObjectType type;
    void*  ttfFont;     // TTF_Font*
    int    size;        // requested pixel height
    bool   bold;
};

struct AndroidDC {
    AndroidBitmap* bmp;            // currently selected bitmap
    AndroidFont*   font;           // currently selected font
    uint32_t       textColor;      // 0xRRGGBB00  (matches COLORREF RGB() on Windows)
    uint32_t       bgColor;
};

// ── Typedefs that replace void* in the Android build ─────────────────────
// These MUST match the typedefs in PlatformDefs.h.
// We change PlatformDefs.h to use these instead.

typedef AndroidDC*     HDC;
typedef AndroidBitmap* HBITMAP;
typedef AndroidFont*   HFONT;
typedef void*          HGDIOBJ;   // generic GDI object (HFONT or HBITMAP)
typedef void*          HBRUSH;
typedef void*          HPEN;
typedef void*          HINSTANCE;
typedef void*          HMENU;

// ── Function declarations (defined in AndroidGDI.cpp) ────────────────────

// Font initialisation — call once from android_main after working dir is set
void  AndroidGDI_Init(int defaultFontSizePx);
void  AndroidGDI_Shutdown();

// GDI implementations
HBITMAP AndroidCreateDIBSection(const void* bmiPtr, void** ppvBits);
HDC     AndroidCreateCompatibleDC(HDC src);
HFONT   AndroidCreateFont(int height, int weight);   // weight: FW_NORMAL=400, FW_SEMIBOLD=600, FW_BOLD=700
HGDIOBJ AndroidSelectObject(HDC hdc, HGDIOBJ obj);
void    AndroidSelectBitmap(HDC hdc, HBITMAP bmp);
void    AndroidSelectFont(HDC hdc, HFONT font);
bool    AndroidTextOut(HDC hdc, int x, int y, const wchar_t* text, int len);
bool    AndroidGetTextExtentPoint32(HDC hdc, const wchar_t* text, int len, int* outW, int* outH);
void    AndroidSetTextColor(HDC hdc, uint32_t colorref);
void    AndroidSetBkColor(HDC hdc, uint32_t colorref);
bool    AndroidDeleteDC(HDC hdc);
bool    AndroidDeleteObject(HGDIOBJ obj);

#endif // __ANDROID__
