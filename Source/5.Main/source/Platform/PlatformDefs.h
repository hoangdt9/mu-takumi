#pragma once
// =============================================================================
// Platform/PlatformDefs.h
// Windows type definitions and API stubs for Android (NDK / Clang / POSIX).
// Included automatically from stdafx.h when __ANDROID__ is defined.
// =============================================================================

#if defined(__ANDROID__) || defined(MU_IOS)

#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <cwchar>
#include <cstdio>
#include <cassert>
#include <cmath>
#include <climits>
#include <ctime>
#include <cerrno>
#include <string>
#include <unordered_map>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <SDL.h>
#include <android/log.h>
#include "MobilePlatform.h"
#include "MobileTime.h"

#if defined(MU_ANDROID_DISABLE_LOG)
#ifdef __android_log_print
#undef __android_log_print
#endif
#define __android_log_print(...) (0)
#endif

// ── Basic Windows integer types ────────────────────────────────────────────
typedef uint8_t         BYTE;
typedef uint8_t         UCHAR;
typedef int16_t         SHORT;
typedef uint16_t        WORD;
typedef uint16_t        USHORT;
typedef uint32_t        DWORD;
typedef int32_t         LONG;
typedef uint32_t        ULONG;
typedef int32_t         INT;
typedef uint32_t        UINT;
typedef int64_t         LONGLONG;
typedef uint64_t        ULONGLONG;
typedef intptr_t        LONG_PTR;
typedef uintptr_t       ULONG_PTR;
typedef uintptr_t       DWORD_PTR;
typedef uintptr_t       UINT_PTR;
typedef intptr_t        INT_PTR;
typedef int             BOOL;
typedef float           FLOAT;
#ifndef FAR
#define FAR
#endif
#ifndef PASCAL
#define PASCAL
#endif

// ── Message parameter types ────────────────────────────────────────────────
typedef long            HRESULT;
#define S_OK            ((HRESULT)0L)
#define S_FALSE         ((HRESULT)1L)
#define E_FAIL          ((HRESULT)0x80004005L)
#define SUCCEEDED(hr)   (((HRESULT)(hr)) >= 0)
#define FAILED(hr)      (((HRESULT)(hr)) < 0)
typedef intptr_t        LRESULT;
typedef uintptr_t       WPARAM;
typedef intptr_t        LPARAM;

// ── Handle types ──────────────────────────────────────────────────────────
// HDC/HBITMAP/HFONT are real structs on Android (for text rendering).
// Include AndroidGDI.h which defines the struct types.
#include "AndroidGDI.h"

typedef void*           HANDLE;
typedef void*           HWND;
// HDC, HBITMAP, HFONT, HGDIOBJ, HBRUSH, HMENU defined by AndroidGDI.h
typedef void*           HGLRC;
typedef void*           HMODULE;
typedef void*           HICON;
typedef void*           HCURSOR;
typedef void*           HIMC;
typedef void*           HINTERNET;  // WinInet handle stub
typedef void*           HPEN;
typedef void*           HHOOK;
inline BOOL UnhookWindowsHookEx(HHOOK) { return 1; }
inline LRESULT CallNextHookEx(HHOOK, int, WPARAM, LPARAM) { return 0; }
typedef void*           HRSRC;
typedef int FREE_IMAGE_FORMAT;
typedef void FIBITMAP;
#define FIF_JPEG 2
inline FREE_IMAGE_FORMAT FreeImage_GetFileType(const char*, int = 0) { return 0; }
inline FIBITMAP* FreeImage_Load(FREE_IMAGE_FORMAT, const char*, int = 0) { return nullptr; }
inline void FreeImage_Unload(FIBITMAP*) {}
inline BOOL FreeImage_Save(FREE_IMAGE_FORMAT, FIBITMAP*, const char*, int = 0) { return 0; }
typedef struct tagMOUSEHOOKSTRUCTEX { struct { LONG x, y; } pt; HWND hwnd; UINT wHitTestCode; ULONG_PTR dwExtraInfo; DWORD mouseData; } MOUSEHOOKSTRUCTEX, *PMOUSEHOOKSTRUCTEX;
typedef int             SOCKET;
typedef struct sockaddr SOCKADDR;
typedef struct sockaddr_in SOCKADDR_IN;
inline int AndroidGetPeerNameCompat(SOCKET s, SOCKADDR* addr, int* len) { socklen_t sockLen = len ? (socklen_t)*len : 0; int result = getpeername(s, addr, len ? &sockLen : nullptr); if (len) *len = (int)sockLen; return result; }
#define getpeername(s,a,l) AndroidGetPeerNameCompat((s),(SOCKADDR*)(a),(int*)(l))
typedef void*           HKEY;
#define HKEY_CLASSES_ROOT      ((HKEY)(uintptr_t)0x80000000)
#define HKEY_CURRENT_USER      ((HKEY)(uintptr_t)0x80000001)
#define HKEY_LOCAL_MACHINE     ((HKEY)(uintptr_t)0x80000002)
#define HKEY_USERS             ((HKEY)(uintptr_t)0x80000003)
#define HKEY_CURRENT_CONFIG    ((HKEY)(uintptr_t)0x80000005)

#define REG_DWORD              4
#define REG_SZ                 1
#define REG_EXPAND_SZ          2
#define REG_OPTION_NON_VOLATILE 0
#define KEY_ALL_ACCESS         0xF003F
#define ERROR_SUCCESS          0L
#define ERROR_ALREADY_EXISTS   183L
#define THREAD_PRIORITY_NORMAL 0
#define THREAD_PRIORITY_ABOVE_NORMAL 1
#define HCBT_ACTIVATE 5
#define HCBT_DESTROYWND 4
#define HWND_TOPMOST ((HWND)(uintptr_t)-1)
#define SWP_HIDEWINDOW 0x0080
typedef DWORD (*LPTHREAD_START_ROUTINE)(void*);

// Window procedure callback
typedef LONG_PTR (*WNDPROC)(HWND, UINT, WPARAM, LPARAM);

// WinInet stubs (wininet.h not available on Android)
typedef WORD   INTERNET_PORT;
typedef void (*INTERNET_STATUS_CALLBACK)(HINTERNET, DWORD_PTR, DWORD, void*, DWORD);
#define INTERNET_FLAG_PASSIVE               0x08000000
#define INTERNET_SERVICE_FTP                1
#define INTERNET_DEFAULT_FTP_PORT           21
#define INTERNET_MAX_URL_LENGTH             2084
#define INTERNET_MAX_USER_NAME_LENGTH       128
#define INTERNET_MAX_PASSWORD_LENGTH        128
#define INTERNET_MAX_PATH_LENGTH            2048
#define INTERNET_MAX_HOST_NAME_LENGTH       256
#define INTERNET_OPEN_TYPE_DIRECT           1
#define INTERNET_OPEN_TYPE_PRECONFIG        0
#define INTERNET_FLAG_RELOAD                0x80000000
#define INTERNET_FLAG_NO_CACHE_WRITE        0x04000000
#define FTP_TRANSFER_TYPE_BINARY            0x00000002
#define INTERNET_ERROR_BASE                 12000

// ── Pointer types ─────────────────────────────────────────────────────────
typedef char*           LPSTR;
typedef const char*     LPCSTR;
typedef wchar_t*        LPWSTR;
typedef const wchar_t*  LPCWSTR;
typedef char*           LPTSTR;
typedef const char*     LPCTSTR;
typedef void*           LPVOID;
typedef void*           PVOID;
typedef const void*     LPCVOID;

// ── Boolean constants ──────────────────────────────────────────────────────
#ifndef TRUE
#  define TRUE  1
#  define FALSE 0
#endif
#ifndef NULL
#  define NULL nullptr
#endif

// ── Path / sizes ───────────────────────────────────────────────────────────
#ifndef MAX_PATH
#  define MAX_PATH 260
#endif
#define _MAX_PATH MAX_PATH

// ── Calling conventions (empty on Android) ────────────────────────────────
#define CALLBACK
#define WINAPI
#define APIENTRY
#define CDECL
#define __cdecl
#define _cdecl
#define CONST const

// ── SAL annotations (Windows Source Annotation Language → no-op) ──────────
#ifndef IN
#  define IN
#endif
#ifndef OUT
#  define OUT
#endif
#ifndef OPTIONAL
#  define OPTIONAL
#endif
#ifndef INOUT
#  define INOUT
#endif

// ── DEFINE_ENUM_FLAG_OPERATORS (Windows winnt.h macro → provide for Android) ─
// Enables bitwise operators on strongly-typed enums
#ifndef DEFINE_ENUM_FLAG_OPERATORS
#define DEFINE_ENUM_FLAG_OPERATORS(ENUMTYPE) \
    extern "C++" { \
    inline ENUMTYPE  operator| (ENUMTYPE  a, ENUMTYPE  b) { return (ENUMTYPE)((__underlying_type(ENUMTYPE))a|(__underlying_type(ENUMTYPE))b); } \
    inline ENUMTYPE& operator|=(ENUMTYPE& a, ENUMTYPE  b) { return (ENUMTYPE&)((__underlying_type(ENUMTYPE)&)a|=(__underlying_type(ENUMTYPE))b); } \
    inline ENUMTYPE  operator& (ENUMTYPE  a, ENUMTYPE  b) { return (ENUMTYPE)((__underlying_type(ENUMTYPE))a&(__underlying_type(ENUMTYPE))b); } \
    inline ENUMTYPE& operator&=(ENUMTYPE& a, ENUMTYPE  b) { return (ENUMTYPE&)((__underlying_type(ENUMTYPE)&)a&=(__underlying_type(ENUMTYPE))b); } \
    inline ENUMTYPE  operator~ (ENUMTYPE  a)               { return (ENUMTYPE)(~(__underlying_type(ENUMTYPE))a); } \
    inline ENUMTYPE  operator^ (ENUMTYPE  a, ENUMTYPE  b) { return (ENUMTYPE)((__underlying_type(ENUMTYPE))a^(__underlying_type(ENUMTYPE))b); } \
    inline ENUMTYPE& operator^=(ENUMTYPE& a, ENUMTYPE  b) { return (ENUMTYPE&)((__underlying_type(ENUMTYPE)&)a^=(__underlying_type(ENUMTYPE))b); } \
    }
#endif

// ── Array size helper ─────────────────────────────────────────────────────
#ifndef _countof
#  define _countof(arr) (sizeof(arr)/sizeof((arr)[0]))
#endif

// ── Bit manipulation macros ────────────────────────────────────────────────
#define LOWORD(l)      ((WORD)(((DWORD_PTR)(l)) & 0xffff))
#define HIWORD(l)      ((WORD)((((DWORD_PTR)(l)) >> 16) & 0xffff))
#define LOBYTE(w)      ((BYTE)((w) & 0xff))
#define HIBYTE(w)      ((BYTE)(((w) >> 8) & 0xff))
#define MAKELONG(a,b)  ((LONG)(((WORD)((DWORD_PTR)(a)&0xffff))|(((DWORD)((WORD)((DWORD_PTR)(b)&0xffff)))<<16)))
#define MAKEWORD(a,b)  ((WORD)(((BYTE)((a)&0xff))|(((WORD)((BYTE)((b)&0xff)))<<8)))
#define MAKELPARAM(l,h) ((LPARAM)(DWORD)MAKELONG(l,h))
#define MAKEWPARAM(l,h) ((WPARAM)(DWORD)MAKELONG(l,h))

// ── Color macro ────────────────────────────────────────────────────────────
#define RGB(r,g,b) ((DWORD)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))

// ── Memory macros ─────────────────────────────────────────────────────────
#define ZeroMemory(ptr,sz)      memset((ptr),0,(sz))
#define CopyMemory(dst,src,sz)  memcpy((dst),(src),(sz))
#define FillMemory(ptr,sz,val)  memset((ptr),(val),(sz))

// ── Path component sizes (MSVC CRT) ──────────────────────────────────────
#ifndef _MAX_PATH
#  define _MAX_PATH  260
#endif
#ifndef _MAX_DRIVE
#  define _MAX_DRIVE   3
#endif
#ifndef _MAX_DIR
#  define _MAX_DIR   256
#endif
#ifndef _MAX_FNAME
#  define _MAX_FNAME 256
#endif
#ifndef _MAX_EXT
#  define _MAX_EXT    16
#endif

// _wsplitpath: split wide-char path into components (MSVC CRT → implement inline)
inline void _wsplitpath(const wchar_t* path,
                        wchar_t* drive, wchar_t* dir,
                        wchar_t* fname, wchar_t* ext)
{
    if (drive) drive[0] = L'\0';
    if (dir)   dir[0]   = L'\0';
    if (fname) fname[0] = L'\0';
    if (ext)   ext[0]   = L'\0';
    if (!path || !path[0]) return;

    // Locate last '/' or '\'
    const wchar_t* lastSlash = nullptr;
    for (const wchar_t* p = path; *p; ++p)
        if (*p == L'/' || *p == L'\\') lastSlash = p;

    // Locate last '.'
    const wchar_t* dot = nullptr;
    const wchar_t* start = lastSlash ? lastSlash + 1 : path;
    for (const wchar_t* p = start; *p; ++p)
        if (*p == L'.') dot = p;

    // dir: everything up to (and including) last slash
    if (dir && lastSlash) {
        const wchar_t* dirEnd = lastSlash + 1;
        size_t len = dirEnd - path;
        wcsncpy(dir, path, len);
        dir[len] = L'\0';
    }

    // fname: from after last slash to last dot (or end)
    if (fname) {
        const wchar_t* nameEnd = dot ? dot : (path + wcslen(path));
        if (nameEnd > start) {
            size_t len = nameEnd - start;
            wcsncpy(fname, start, len);
            fname[len] = L'\0';
        }
    }

    // ext: from last dot to end
    if (ext && dot) {
        wcscpy(ext, dot);
    }
}

// ── Wide-char case-insensitive compare (wcsicmp without underscore) ────────
#ifndef wcsicmp
#  define wcsicmp wcscasecmp
#endif

// ── Safe string functions → standard C ────────────────────────────────────
// Handle both 2-arg (dst,src) and 3-arg (dst,size,src) MSVC forms
#define _wcscpy_s_2(d,s)        wcscpy((d),(s))
#define _wcscpy_s_3(d,n,s)      wcscpy((d),(s))
#define _wcscat_s_2(d,s)        wcscat((d),(s))
#define _wcscat_s_3(d,n,s)      wcscat((d),(s))
#define _va_pick3(_1,_2,_3,X,...) X
#define wcscpy_s(...) _va_pick3(__VA_ARGS__, _wcscpy_s_3, _wcscpy_s_2, ~)(__VA_ARGS__)
#define wcscat_s(...) _va_pick3(__VA_ARGS__, _wcscat_s_3, _wcscat_s_2, ~)(__VA_ARGS__)
inline int _wcsncpy_s_impl(wchar_t* dst, size_t dstsz, const wchar_t* src, size_t cnt) {
    if (!dst || dstsz == 0) return EINVAL;
    size_t n = (cnt == (size_t)-1 /*_TRUNCATE*/) ? dstsz - 1 : (cnt < dstsz - 1 ? cnt : dstsz - 1);
    wcsncpy(dst, src, n);
    dst[n] = L'\0';
    return 0;
}
// Handle both 3-arg (dst,src,cnt) and 4-arg (dst,size,src,cnt) MSVC forms
#define _wcsncpy_s_3(d,s,n)      (wcsncpy((d),(s),(n)),((d)[(n)]=L'\0'),0)
#define _wcsncpy_s_4(d,sz,s,n)   _wcsncpy_s_impl((d),(sz),(s),(n))
// Pick 5th argument: for 4-arg call the 5th is _wcsncpy_s_4; for 3-arg call it's _wcsncpy_s_3
#define _PICK5(_1,_2,_3,_4,_5,...) _5
#define wcsncpy_s(...) _PICK5(__VA_ARGS__, _wcsncpy_s_4, _wcsncpy_s_3, ~, ~)(__VA_ARGS__)
#define _strcpy_s_2(d,s)                  strcpy((d),(s))
#define _strcpy_s_3(d,n,s)                strcpy((d),(s))
#define strcpy_s(...)                     _va_pick3(__VA_ARGS__, _strcpy_s_3, _strcpy_s_2, ~)(__VA_ARGS__)
#define _strncpy_s_3(dst,src,cnt)         strncpy((dst),(src),(cnt))
#define _strncpy_s_4(dst,dstsz,src,cnt)   strncpy((dst),(src),(cnt))
#define strncpy_s(...)                    _PICK5(__VA_ARGS__, _strncpy_s_4, _strncpy_s_3, ~, ~)(__VA_ARGS__)
inline int fopen_s(FILE** file, const char* path, const char* mode) { if (!file) return EINVAL; *file = fopen(path, mode); return *file ? 0 : errno; }
#define _tcslen strlen
inline int memcpy_s(void* dst, size_t dstsz, const void* src, size_t cnt) { if (!dst || !src || cnt > dstsz) return EINVAL; memcpy(dst, src, cnt); return 0; }
inline int sprintf_s(char* buf, size_t sz, const char* fmt, ...) { va_list args; va_start(args, fmt); int result = vsnprintf(buf, sz, fmt, args); va_end(args); return result; }
inline int sprintf_s(char* buf, const char* fmt, ...) { va_list args; va_start(args, fmt); int result = vsnprintf(buf, 4096, fmt, args); va_end(args); return result; }
inline int wvsprintf(char* buf, const char* fmt, va_list args) { return vsnprintf(buf, 4096, fmt, args); }
inline int wvsprintf(wchar_t* buf, const wchar_t* fmt, va_list args) { return vswprintf(buf, 4096, fmt, args); }
#define _vsprintf_s_3(buf,fmt,args)       vsnprintf((buf),4096,(fmt),(args))
#define _vsprintf_s_4(buf,sz,fmt,args)    vsnprintf((buf),(sz),(fmt),(args))
#define vsprintf_s(...)                   _PICK5(__VA_ARGS__, _vsprintf_s_4, _vsprintf_s_3, ~, ~)(__VA_ARGS__)
#define swprintf_s(buf,sz,...)            swprintf((buf),(sz),__VA_ARGS__)
#define sscanf_s                           sscanf
#define _snprintf_s(buf,sz,cnt,...)       snprintf((buf),(sz),__VA_ARGS__)
#define _snwprintf_s(buf,sz,cnt,...)      swprintf((buf),(sz),__VA_ARGS__)
#define _wcsicmp(a,b)                     wcscasecmp((a),(b))
#define _stricmp(a,b)                     strcasecmp((a),(b))
#define stricmp(a,b)                      strcasecmp((a),(b))
inline char* _strupr(char* s) { if (!s) return s; for (char* p=s; *p; ++p) *p=(char)toupper((unsigned char)*p); return s; }
#define _strnicmp(a,b,n)                  strncasecmp((a),(b),(n))
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#define strnicmp(a,b,n) strncasecmp((a),(b),(n))
#define _atoi64(s)                        atoll(s)
#define _wfopen(path,mode)                _wfopen_android((path),(mode))
#define _vsntprintf(buf,sz,fmt,args)      vsnprintf((buf),(sz),(fmt),(args))
#define _stprintf                         sprintf

#ifndef FILE_ATTRIBUTE_ARCHIVE
#define FILE_ATTRIBUTE_ARCHIVE 0x00000020
#endif

inline int wsprintf(char* buf, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int result = vsnprintf(buf, 4096, fmt, args);
    va_end(args);
    return result;
}

// wfopen helper: convert wchar path to char on Android (\ → / for Linux)
inline FILE* _wfopen_android(const wchar_t* path, const wchar_t* mode) {
    char cpath[MAX_PATH], cmode[16];
    wcstombs(cpath, path, MAX_PATH);
    for (char* p = cpath; *p; ++p) if (*p == '\\') *p = '/';
    wcstombs(cmode, mode, 16);
    FILE* f = fopen(cpath, cmode);
    if (!f && cmode[0] == 'r') {
        char resolved[MAX_PATH] = {};
        const char* seg = cpath;
        bool ok = true;
        while (ok && *seg) {
            const char* slash = strchr(seg, '/');
            size_t seglen = slash ? (size_t)(slash - seg) : strlen(seg);
            char component[256] = {};
            strncpy(component, seg, seglen);
            char trypath[MAX_PATH];
            snprintf(trypath, MAX_PATH, "%s/%s", resolved[0] ? resolved : ".", component);
            struct stat st_buf;
            if (stat(trypath, &st_buf) == 0) {
                if (resolved[0]) strncat(resolved, "/", MAX_PATH - strlen(resolved) - 1);
                strncat(resolved, component, MAX_PATH - strlen(resolved) - 1);
            } else {
                DIR* d = opendir(resolved[0] ? resolved : ".");
                bool found = false;
                if (d) {
                    char lc[256]; strncpy(lc, component, 255); lc[255]=0;
                    for (char* q = lc; *q; ++q) *q = (char)tolower((unsigned char)*q);
                    struct dirent* ent;
                    while ((ent = readdir(d)) != NULL) {
                        char elc[256]; strncpy(elc, ent->d_name, 255); elc[255]=0;
                        for (char* q = elc; *q; ++q) *q = (char)tolower((unsigned char)*q);
                        if (strcmp(elc, lc) == 0) {
                            if (resolved[0]) strncat(resolved, "/", MAX_PATH - strlen(resolved) - 1);
                            strncat(resolved, ent->d_name, MAX_PATH - strlen(resolved) - 1);
                            found = true; break;
                        }
                    }
                    closedir(d);
                }
                if (!found) ok = false;
            }
            seg += seglen + (slash ? 1 : 0);
        }
        if (ok && resolved[0]) f = fopen(resolved, cmode);
    }
    return f;
}

// ── Time ───────────────────────────────────────────────────────────────────
#include <SDL.h>
inline DWORD    GetTickCount()    { return static_cast<DWORD>(MU_MobileGetTicks()); }
inline uint64_t GetTickCount64()  { return static_cast<uint64_t>(MU_MobileGetTicks()); }
inline DWORD    timeGetTime()     { return static_cast<DWORD>(MU_MobileGetTicks()); }  // winmm
inline void     Sleep(DWORD ms)   { MU_MobileSleep(static_cast<uint32_t>(ms)); }

// ── MBCS functions (not available on Android — use stubs) ──────────────────
// _mbclen: length of multibyte character at ptr (simple UTF-8 heuristic)
inline int _mbclen(const unsigned char* ptr) {
    if (!ptr || *ptr == 0) return 1;
    // ASCII (single byte)
    if (*ptr < 0x80) return 1;
    // 2-byte UTF-8 lead (0xC0–0xDF)
    if (*ptr < 0xE0) return 2;
    // 3-byte UTF-8 lead (0xE0–0xEF)
    if (*ptr < 0xF0) return 3;
    // 4-byte UTF-8 lead
    return 4;
}

// ── Wide string functions (MSVC-specific → POSIX equivalents) ─────────────
// wcstok_s → wcstok (POSIX, same semantics)
inline wchar_t* wcstok_s(wchar_t* str, const wchar_t* delim, wchar_t** ctx) {
    return wcstok(str, delim, ctx);
}
// _wtoi → wide string to int
inline int _wtoi(const wchar_t* s) { return (int)wcstol(s, nullptr, 10); }
// _wtol → wide string to long
inline long _wtol(const wchar_t* s) { return wcstol(s, nullptr, 10); }
// _wtof → wide string to double
inline double _wtof(const wchar_t* s) { return wcstod(s, nullptr); }
// _itow → int to wide string (MSVC CRT)
inline wchar_t* _itow(int val, wchar_t* buf, int radix) {
    if (radix == 10) swprintf(buf, 32, L"%d", val);
    else if (radix == 16) swprintf(buf, 32, L"%x", val);
    else swprintf(buf, 32, L"%d", val);
    return buf;
}

// ── GLvoid / VOID ─────────────────────────────────────────────────────────
typedef void GLvoid;
#define VOID void

// ── POINT, RECT, SIZE (Windows GDI structs) ───────────────────────────────
struct POINT  { LONG x, y; };
struct RECT   { LONG left, top, right, bottom; };
struct SIZE   { LONG cx, cy; };
typedef SIZE* LPSIZE;

// ── TCHAR (maps to wchar_t for Unicode) ───────────────────────────────────
typedef char TCHAR;
#define _T(x) x
#define TEXT(x) x

// ── __int64 (MSVC intrinsic, maps to int64_t) ─────────────────────────────
typedef int64_t  __int64;
typedef uint64_t __uint64;
typedef int32_t  __int32;
typedef int16_t  __int16;
typedef int8_t   __int8;

// ── Additional pointer types ───────────────────────────────────────────────
typedef DWORD*   LPDWORD;
typedef WORD*    LPWORD;
typedef LONG*    LPLONG;
typedef BYTE*    LPBYTE;
typedef BYTE*    PBYTE;

#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif
#ifndef SOCKET_ERROR
#define SOCKET_ERROR (-1)
#endif
#ifndef WSAEWOULDBLOCK
#define WSAEWOULDBLOCK EWOULDBLOCK
#endif
inline int WSAGetLastError() { return errno; }
inline int closesocket(SOCKET s) { return close(s); }
typedef BOOL*    LPBOOL;
typedef void*    LPOVERLAPPED;  // simplified stub

// ── Windows code pages ────────────────────────────────────────────────────
#define CP_ACP    0
#define CP_UTF7   65000
#define CP_UTF8   65001

// WideCharToMultiByte / MultiByteToWideChar stubs (use iconv/wcstombs on Android)
#define WC_NO_BEST_FIT_CHARS 0x00000400
#define MB_PRECOMPOSED       0x00000001
inline int WideCharToMultiByte(UINT cp, DWORD, LPCWSTR src, int srcLen, LPSTR dst, int dstLen, LPCSTR, LPBOOL) {
    if (!dst || !dstLen) return (int)(wcslen(src) * 4 + 1);
    int n = wcstombs(dst, src, dstLen - 1);
    if (n >= 0) dst[n] = '\0';
    return n >= 0 ? n : 0;
}
inline int MultiByteToWideChar(UINT, DWORD, LPCSTR src, int, LPWSTR dst, int dstLen) {
    if (!dst || !dstLen) return (int)(strlen(src) + 1);
    int n = mbstowcs(dst, src, dstLen - 1);
    if (n >= 0) dst[n] = L'\0';
    return n >= 0 ? n : 0;
}

// ── Windows console color constants (no-op on Android) ────────────────────
#define FOREGROUND_BLUE      0x0001
#define FOREGROUND_GREEN     0x0002
#define FOREGROUND_RED       0x0004
#define FOREGROUND_INTENSITY 0x0008
#define BACKGROUND_BLUE      0x0010
#define BACKGROUND_GREEN     0x0020
#define BACKGROUND_RED       0x0040
#define BACKGROUND_INTENSITY 0x0080

// ── Windows message constants ─────────────────────────────────────────────
#ifndef WM_USER
#define WM_USER           0x0400
#endif
#define WM_QUIT           0x0012
#define WM_DESTROY        0x0002
#define WM_CLOSE          0x0010
#define WM_ACTIVATE       0x0006
#define WM_TIMER          0x0113
#define WM_PAINT          0x000F
#define WM_MOUSEMOVE      0x0200
#define WM_LBUTTONDOWN    0x0201
#define WM_LBUTTONUP      0x0202
#define WM_RBUTTONDOWN    0x0204
#define WM_RBUTTONUP      0x0205
#define WM_MBUTTONDOWN    0x0207
#define WM_MBUTTONUP      0x0208
#define WM_MOUSEWHEEL     0x020A
#define WM_LBUTTONDBLCLK  0x0203
#define WM_CHAR           0x0102
#define WM_KEYDOWN        0x0100
#define WM_KEYUP          0x0101
#define WM_SETCURSOR      0x0020
#define WM_ERASEBKGND     0x0014
#define WM_SYSKEYDOWN     0x0104
#define WM_CTLCOLOREDIT   0x0133
#define WM_IME_NOTIFY     0x0282
#ifndef WM_RECEIVE_BUFFER
#define WM_RECEIVE_BUFFER (WM_USER + 2)
#endif
#ifndef WM_NPROTECT_EXIT_TWO
#define WM_NPROTECT_EXIT_TWO (WM_USER + 10001)
#endif
#define IMN_SETCONVERSIONMODE 0x0001
#define IMN_SETSENTENCEMODE   0x0002

// ShowWindow commands
#define SW_SHOW    5
#define SW_HIDE    0
#define SW_NORMAL  1

// MessageBox flags
#define MB_OK              0x0000L
#define MB_OKCANCEL        0x0001L
#define MB_YESNOCANCEL     0x0003L
#define MB_YESNO           0x0004L
#define MB_ICONINFORMATION 0x0040L
#define MB_ICONEXCLAMATION 0x0030L
#define MB_ICONERROR       0x0010L
#define MB_ICONQUESTION    0x0020L
// MessageBox return values
#define IDOK     1
#define IDCANCEL 2
#define IDYES    6
#define IDNO     7

// WA_xxx (WM_ACTIVATE)
#define WA_INACTIVE    0
#define WA_ACTIVE      1
#define WA_CLICKACTIVE 2

// Virtual keys
#define VK_LBUTTON 0x01
#define VK_RBUTTON 0x02
#define VK_MBUTTON 0x04
#define VK_BACK    0x08
#define VK_TAB     0x09
#define VK_RETURN  0x0D
#define VK_SHIFT   0x10
#define VK_CONTROL 0x11
#define VK_MENU    0x12
#define VK_ESCAPE  0x1B
#define VK_SPACE   0x20
#define VK_PRIOR   0x21
#define VK_NEXT    0x22
#define VK_END     0x23
#define VK_HOME    0x24
#define VK_LEFT    0x25
#define VK_UP      0x26
#define VK_RIGHT   0x27
#define VK_DOWN    0x28
#define VK_INSERT  0x2D
#define VK_DELETE  0x2E
#define VK_F1      0x70
#define VK_F2      0x71
#define VK_F3      0x72
#define VK_F4      0x73
#define VK_F5      0x74
#define VK_F6      0x75
#define VK_F7      0x76
#define VK_F8      0x77
#define VK_F9      0x78
#define VK_F10     0x79
#define VK_F11     0x7A
#define VK_F12     0x7B

// Wheel delta
#define WHEEL_DELTA 120

// File access flags
#define GENERIC_READ           0x80000000L
#define GENERIC_WRITE          0x40000000L
#define FILE_SHARE_READ        0x00000001
#define OPEN_EXISTING          3
#define FILE_ATTRIBUTE_NORMAL  0x00000080
#define INVALID_HANDLE_VALUE   ((HANDLE)(intptr_t)-1)
#ifndef CREATE_NEW
#define CREATE_NEW             1
#endif
#ifndef CREATE_ALWAYS
#define CREATE_ALWAYS          2
#endif
#ifndef OPEN_ALWAYS
#define OPEN_ALWAYS            4
#endif
#ifndef TRUNCATE_EXISTING
#define TRUNCATE_EXISTING      5
#endif

static constexpr intptr_t kAndroidMaxFdHandleValue = 0x100000;
static constexpr uint32_t kAndroidThreadHandleMagic = 0x54485244; // 'THRD'

struct AndroidThreadHandle {
    uint32_t        magic;
    pthread_t       thread;
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    bool            completed;
    bool            joined;
    bool            closeRequested;
    unsigned        result;
};

struct AndroidBeginThreadArgs {
    unsigned(__stdcall* fn)(void*);
    void*               arg;
    AndroidThreadHandle* handle;
};

inline bool AndroidIsFdHandle(HANDLE h) {
    const intptr_t v = reinterpret_cast<intptr_t>(h);
    return v > 0 && v <= kAndroidMaxFdHandleValue;
}

inline int AndroidHandleToFd(HANDLE h) {
    return static_cast<int>(reinterpret_cast<intptr_t>(h)) - 1;
}

inline bool AndroidIsThreadHandle(HANDLE h) {
    if (!h || h == INVALID_HANDLE_VALUE || AndroidIsFdHandle(h)) {
        return false;
    }
    const auto* handle = reinterpret_cast<const AndroidThreadHandle*>(h);
    return handle->magic == kAndroidThreadHandleMagic;
}

inline void AndroidDestroyThreadHandle(AndroidThreadHandle* handle) {
    if (!handle) {
        return;
    }
    handle->magic = 0;
    pthread_cond_destroy(&handle->cond);
    pthread_mutex_destroy(&handle->mutex);
    delete handle;
}

inline void* AndroidBeginThreadProc(void* rawArgs) {
    AndroidBeginThreadArgs* args = static_cast<AndroidBeginThreadArgs*>(rawArgs);
    unsigned result = 0;
    if (args && args->fn) {
        result = args->fn(args->arg);
    }

    AndroidThreadHandle* handle = args ? args->handle : nullptr;
    delete args;
    if (!handle) {
        return nullptr;
    }

    bool releaseHandle = false;
    pthread_mutex_lock(&handle->mutex);
    handle->result = result;
    handle->completed = true;
    releaseHandle = handle->closeRequested;
    pthread_cond_broadcast(&handle->cond);
    pthread_mutex_unlock(&handle->mutex);

    if (releaseHandle) {
        AndroidDestroyThreadHandle(handle);
    }
    return nullptr;
}

// SPI flags
#define SPI_SCREENSAVERRUNNING   0x0061
#define SPI_GETSCREENSAVETIMEOUT 0x000E
#define SPI_SETSCREENSAVETIMEOUT 0x000F

// System metrics
#define SM_CXSCREEN 0
#define SM_CYSCREEN 1

// IME constants
#define IME_CMODE_ALPHANUMERIC 0x0000
#define IME_SMODE_NONE         0x0000
#ifndef IME_CONVERSIONMODE
#define IME_CONVERSIONMODE     0x0001
#endif
#ifndef IME_SENTENCEMODE
#define IME_SENTENCEMODE       0x0002
#endif

// ── Stub DEVMODE struct ────────────────────────────────────────────────────
struct DEVMODE {
    wchar_t dmDeviceName[32];
    WORD    dmSize;
    DWORD   dmPelsWidth;
    DWORD   dmPelsHeight;
    DWORD   dmBitsPerPel;
    DWORD   dmDisplayFrequency;
};

// ── Stub VS_FIXEDFILEINFO ──────────────────────────────────────────────────
struct VS_FIXEDFILEINFO {
    DWORD dwFileVersionMS;
    DWORD dwFileVersionLS;
};

// ── Stub PAINTSTRUCT ───────────────────────────────────────────────────────
struct PAINTSTRUCT { HDC hdc; };

// ── Stub _EXCEPTION_POINTERS ───────────────────────────────────────────────
struct _EXCEPTION_POINTERS {};

// ── Stub Windows API functions ─────────────────────────────────────────────
#define ANDROID_TAG "MuMain"
#if defined(MU_ANDROID_DISABLE_LOG)
#define ALOGI(...) ((void)0)
#define ALOGE(...) ((void)0)
#define ALOGW(...) ((void)0)
#else
#define ALOGI(...) __android_log_print(ANDROID_LOG_INFO,  ANDROID_TAG, __VA_ARGS__)
#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, ANDROID_TAG, __VA_ARGS__)
#define ALOGW(...) __android_log_print(ANDROID_LOG_WARN,  ANDROID_TAG, __VA_ARGS__)
#endif

// Live input/window state comes from core client globals.
extern int MouseX;
extern int MouseY;
extern bool MouseLButton;
extern bool MouseRButton;
extern bool MouseMButton;
extern float g_fScreenRate_x;
extern float g_fScreenRate_y;
extern bool g_bWndActive;
extern bool g_bEnterPressed;
extern HWND g_hWnd;

struct AndroidStubEditControl
{
    std::wstring Text;
    int Limit = 512;
    int SelectionStart = 0;
    int SelectionEnd = 0;
    bool Visible = false;
    bool IsPassword = false;
    int ScrollPosition = 0;
    LONG_PTR UserData = 0;
    WNDPROC WindowProc = nullptr;
};

inline std::unordered_map<HWND, AndroidStubEditControl>& AndroidEditControls()
{
    static std::unordered_map<HWND, AndroidStubEditControl> controls;
    return controls;
}

inline HWND& AndroidFocusedWindow()
{
    static HWND focused = nullptr;
    return focused;
}

inline int AndroidClampInt(int value, int minValue, int maxValue)
{
    return (value < minValue) ? minValue : (value > maxValue ? maxValue : value);
}

inline AndroidStubEditControl* AndroidGetEditControl(HWND handle)
{
    const auto it = AndroidEditControls().find(handle);
    return it == AndroidEditControls().end() ? nullptr : &it->second;
}

inline bool AndroidIsEditControl(HWND handle)
{
    return AndroidGetEditControl(handle) != nullptr;
}

inline void AndroidNormalizeSelection(AndroidStubEditControl& control)
{
    const int maxPos = static_cast<int>(control.Text.length());
    control.SelectionStart = AndroidClampInt(control.SelectionStart, 0, maxPos);
    control.SelectionEnd = AndroidClampInt(control.SelectionEnd, 0, maxPos);
    if (control.SelectionStart > control.SelectionEnd)
    {
        std::swap(control.SelectionStart, control.SelectionEnd);
    }
}

inline void _splitpath(const char* path,
                       char* drive, char* dir,
                       char* fname, char* ext)
{
    if (drive) drive[0] = '\0';
    if (dir) dir[0] = '\0';
    if (fname) fname[0] = '\0';
    if (ext) ext[0] = '\0';
    if (!path) return;

    const char* slash1 = strrchr(path, '/');
    const char* slash2 = strrchr(path, '\\');
    const char* slash = slash1 > slash2 ? slash1 : slash2;
    const char* start = slash ? slash + 1 : path;
    const char* dot = strrchr(start, '.');

    if (dir && slash) {
        size_t len = (slash - path) + 1;
        strncpy(dir, path, len);
        dir[len] = '\0';
    }

    if (fname) {
        const char* nameEnd = dot ? dot : (path + strlen(path));
        if (nameEnd > start) {
            size_t len = nameEnd - start;
            strncpy(fname, start, len);
            fname[len] = '\0';
        }
    }

    if (ext && dot) {
        strcpy(ext, dot);
    }
}

inline LRESULT AndroidDefaultEditWndProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
{
    constexpr UINT kEmSetLimitText = 0x00C5;
    constexpr UINT kEmSetSel = 0x00B1;
    constexpr UINT kEmGetSel = 0x00B0;
    constexpr UINT kEmReplaceSel = 0x00C2;
    constexpr UINT kEmGetLineCount = 0x00BA;
    constexpr UINT kEmSetPasswordChar = 0x00CC;
    constexpr UINT kEmScroll = 0x00B5;
    constexpr UINT kEmLineScroll = 0x00B6;
    constexpr UINT kWmSetFont = 0x0030;

    auto* control = AndroidGetEditControl(handle);
    if (!control)
    {
        return 0;
    }

    switch (message)
    {
    case WM_CHAR:
    {
        AndroidNormalizeSelection(*control);

        const wchar_t ch = static_cast<wchar_t>(wParam);
        if (ch == VK_BACK)
        {
            if (control->SelectionStart != control->SelectionEnd)
            {
                control->Text.erase(
                    static_cast<std::wstring::size_type>(control->SelectionStart),
                    static_cast<std::wstring::size_type>(control->SelectionEnd - control->SelectionStart));
                control->SelectionEnd = control->SelectionStart;
            }
            else if (control->SelectionStart > 0)
            {
                control->Text.erase(static_cast<std::wstring::size_type>(control->SelectionStart - 1), 1);
                control->SelectionStart--;
                control->SelectionEnd = control->SelectionStart;
            }

            return 0;
        }

        if (ch < 0x20)
        {
            return 0;
        }

        const int currentLength = static_cast<int>(control->Text.length());
        const int selectedLength = control->SelectionEnd - control->SelectionStart;
        const int nextLength = currentLength - selectedLength + 1;
        if (control->Limit > 0 && nextLength > control->Limit)
        {
            return 0;
        }

        control->Text.replace(
            static_cast<std::wstring::size_type>(control->SelectionStart),
            static_cast<std::wstring::size_type>(selectedLength),
            std::wstring(1, ch));

        control->SelectionStart++;
        control->SelectionEnd = control->SelectionStart;
        return 0;
    }
    case kEmSetLimitText:
    {
        control->Limit = static_cast<int>(wParam);
        if (control->Limit < 0)
        {
            control->Limit = 512;
        }
        if (control->Limit > 0 && static_cast<int>(control->Text.length()) > control->Limit)
        {
            control->Text.resize(static_cast<std::wstring::size_type>(control->Limit));
        }
        AndroidNormalizeSelection(*control);
        return 0;
    }
    case kEmSetSel:
    {
        int start = static_cast<int>(static_cast<intptr_t>(wParam));
        int end = static_cast<int>(static_cast<intptr_t>(lParam));
        const int length = static_cast<int>(control->Text.length());

        if (start == -1 && end == -1)
        {
            control->SelectionStart = 0;
            control->SelectionEnd = length;
            return 0;
        }

        if (start == -2 && end == -1)
        {
            control->SelectionStart = length;
            control->SelectionEnd = length;
            return 0;
        }

        if (start < 0)
        {
            start = 0;
        }
        if (end < 0)
        {
            end = start;
        }

        control->SelectionStart = AndroidClampInt(start, 0, length);
        control->SelectionEnd = AndroidClampInt(end, 0, length);
        AndroidNormalizeSelection(*control);
        return 0;
    }
    case kEmGetSel:
        AndroidNormalizeSelection(*control);
        return MAKELONG(control->SelectionStart, control->SelectionEnd);
    case kEmReplaceSel:
    {
        const wchar_t* replacement = reinterpret_cast<const wchar_t*>(lParam);
        if (!replacement)
        {
            return 0;
        }

        AndroidNormalizeSelection(*control);
        const int selectedLength = control->SelectionEnd - control->SelectionStart;
        const int replacementLength = static_cast<int>(wcslen(replacement));
        const int nextLength = static_cast<int>(control->Text.length()) - selectedLength + replacementLength;

        if (control->Limit > 0 && nextLength > control->Limit)
        {
            return 0;
        }

        control->Text.replace(
            static_cast<std::wstring::size_type>(control->SelectionStart),
            static_cast<std::wstring::size_type>(selectedLength),
            replacement);
        control->SelectionStart += replacementLength;
        control->SelectionEnd = control->SelectionStart;
        return 0;
    }
    case kEmGetLineCount:
    {
        int lineCount = 1;
        for (const wchar_t c : control->Text)
        {
            if (c == L'\n')
            {
                lineCount++;
            }
        }
        return lineCount;
    }
    case kEmSetPasswordChar:
        control->IsPassword = (wParam != 0);
        return 0;
    case kEmScroll:
    case kEmLineScroll:
        return 0;
    case WM_ERASEBKGND:
    {
        HDC hdc = reinterpret_cast<HDC>(wParam);
        if (hdc && hdc->bmp && hdc->bmp->data && hdc->bmp->pitch > 0 && hdc->bmp->height > 0)
        {
            std::memset(
                hdc->bmp->data,
                0,
                static_cast<std::size_t>(hdc->bmp->pitch) * static_cast<std::size_t>(hdc->bmp->height));
        }
        return 1;
    }
    case WM_PAINT:
    {
        HDC hdc = reinterpret_cast<HDC>(wParam);
        if (!hdc)
        {
            return 0;
        }

        std::wstring textToPaint = control->Text;
        if (control->IsPassword)
        {
            textToPaint.assign(control->Text.length(), L'*');
        }

        if (!textToPaint.empty())
        {
            AndroidTextOut(
                hdc,
                1,
                0,
                textToPaint.c_str(),
                static_cast<int>(textToPaint.length()));
        }

        return 0;
    }
    case kWmSetFont:
        return 0;
    default:
        return 0;
    }
}

inline bool AndroidHasFocusedTextInput()
{
    return AndroidIsEditControl(AndroidFocusedWindow());
}

inline bool AndroidInjectCharToFocusedTextInput(wchar_t character)
{
    const HWND focused = AndroidFocusedWindow();
    auto* control = AndroidGetEditControl(focused);
    if (!control)
    {
        return false;
    }

    WNDPROC proc = control->WindowProc ? control->WindowProc : AndroidDefaultEditWndProc;
    proc(focused, WM_CHAR, static_cast<WPARAM>(character), 0);
    return true;
}

inline bool AndroidInjectUtf8ToFocusedTextInput(const char* textUtf8)
{
    if (!textUtf8 || !*textUtf8)
    {
        return false;
    }

    bool handled = false;
    const unsigned char* cursor = reinterpret_cast<const unsigned char*>(textUtf8);
    while (*cursor)
    {
        if ((*cursor & 0x80u) == 0)
        {
            const wchar_t ch = static_cast<wchar_t>(*cursor);
            if (ch == L'\b' || ch == 0x7Fu)
            {
                handled |= AndroidInjectCharToFocusedTextInput(VK_BACK);
            }
            else if (ch == L'\r' || ch == L'\n')
            {
                handled |= AndroidInjectCharToFocusedTextInput(VK_RETURN);
            }
            else if (ch >= 0x20)
            {
                handled |= AndroidInjectCharToFocusedTextInput(ch);
            }
            cursor++;
            continue;
        }

        // Skip multi-byte sequences for now. Login/password are expected ASCII.
        if ((*cursor & 0xE0u) == 0xC0u) { cursor += 2; }
        else if ((*cursor & 0xF0u) == 0xE0u) { cursor += 3; }
        else if ((*cursor & 0xF8u) == 0xF0u) { cursor += 4; }
        else { cursor++; }
    }

    return handled;
}

#ifndef SDL_StartTextInput
#define SDL_StartTextInput() MU_MobileStartTextInput()
#endif

#ifndef SDL_StopTextInput
#define SDL_StopTextInput() MU_MobileStopTextInput()
#endif

#ifndef SDL_IsTextInputActive
#define SDL_IsTextInputActive() (MU_MobileIsTextInputActive() ? SDL_TRUE : SDL_FALSE)
#endif

#ifndef SDL_SetTextInputRect
#define SDL_SetTextInputRect(rect) MU_MobileSetTextInputRect(rect)
#endif

inline int   MessageBox(HWND, LPCSTR txt, LPCSTR cap, UINT) {
    ALOGW("MessageBox [%s]: %s", cap ? cap : "", txt ? txt : "");
    return IDOK;
}
inline BOOL GetTextExtentPoint32W(HDC hdc, LPCWSTR text, int len, SIZE* size) { int w = 0; int h = 0; bool ok = AndroidGetTextExtentPoint32(hdc, text, len, &w, &h); if (size) { size->cx = w; size->cy = h; } return ok ? TRUE : FALSE; }
inline BOOL TextOutW(HDC hdc, int x, int y, LPCWSTR text, int len) { return AndroidTextOut(hdc, x, y, text, len); }
inline int   MessageBox(HWND, LPCWSTR txt, LPCWSTR cap, UINT) {
    ALOGW("MessageBox [%ls]: %ls", cap, txt);
    return IDOK;
}
inline void  ShowCursor(BOOL)                       {}
inline void  SetCapture(HWND)                       {}
inline void  ReleaseCapture()                       {}
inline void  PostQuitMessage(int)                   {}
inline BOOL  DestroyWindow(HWND handle)
{
    auto* control = AndroidGetEditControl(handle);
    if (!control)
    {
        return TRUE;
    }

    if (AndroidFocusedWindow() == handle)
    {
        AndroidFocusedWindow() = nullptr;
        MU_MobileStopTextInput();
    }

    AndroidEditControls().erase(handle);
    delete static_cast<int*>(handle);
    return TRUE;
}
inline void  UpdateWindow(HWND)                     {}
inline void  ShowWindow(HWND handle, int command)
{
    auto* control = AndroidGetEditControl(handle);
    if (!control)
    {
        return;
    }

    control->Visible = (command != SW_HIDE);
}
inline void  SetForegroundWindow(HWND)              {}
inline HWND  SetFocus(HWND handle)
{
    const HWND previous = AndroidFocusedWindow();
    AndroidFocusedWindow() = handle;

    const bool currentIsEdit = AndroidIsEditControl(AndroidFocusedWindow());
    if (currentIsEdit)
    {
        if (!MU_MobileIsTextInputActive())
        {
            MU_MobileStartTextInput();
        }
    }
    else if (MU_MobileIsTextInputActive())
    {
        MU_MobileStopTextInput();
    }

    if (handle && !currentIsEdit)
    {
        g_hWnd = handle;
    }

    return previous ? previous : g_hWnd;
}
inline HWND  GetFocus()
{
    if (!g_bWndActive)
    {
        return nullptr;
    }

    if (AndroidFocusedWindow())
    {
        return AndroidFocusedWindow();
    }

    return g_hWnd ? g_hWnd : reinterpret_cast<HWND>(0x1);
}
inline HWND  FindWindow(LPCWSTR, LPCWSTR)           { return nullptr; }
inline BOOL  SendMessage(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
{
    auto* control = AndroidGetEditControl(handle);
    if (!control)
    {
        return FALSE;
    }

    WNDPROC proc = control->WindowProc ? control->WindowProc : AndroidDefaultEditWndProc;
    return static_cast<BOOL>(proc(handle, message, wParam, lParam));
}
inline BOOL  PostMessage(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
{
    return SendMessage(handle, message, wParam, lParam);
}
inline void  SetTimer(HWND, UINT_PTR id, UINT ms, void*)  {}
inline void  KillTimer(HWND, UINT_PTR id)              {}
// DeleteObject defined later with proper AndroidGDI implementation
inline HGDIOBJ GetStockObject(int)                  { return nullptr; }
inline void  ShellExecute(HWND,LPCWSTR,LPCWSTR,LPCWSTR,LPCWSTR,int) {}
inline void  ShellExecute(HWND,LPCSTR,LPCSTR,LPCSTR,LPCSTR,int) {}
inline HFONT CreateFontA(int h,int,int,int,int weight,DWORD,DWORD,DWORD,DWORD,DWORD,DWORD,DWORD,DWORD,LPCSTR) { return AndroidCreateFont(h, weight); }
inline DWORD GetLastError()                         { return 0; }
inline void  SystemParametersInfo(UINT,UINT,void*,UINT) {}
inline LONG  ChangeDisplaySettings(DEVMODE*, DWORD) { return 0; }
inline BOOL  EnumDisplaySettings(LPCWSTR, DWORD, DEVMODE*) { return FALSE; }
inline int   GetSystemMetrics(int)                  { return 0; }
inline SHORT GetAsyncKeyState(int key)
{
    auto toShort = [](bool isDown) -> SHORT { return isDown ? static_cast<SHORT>(0x8000) : static_cast<SHORT>(0); };

    switch (key)
    {
    case VK_LBUTTON: return toShort(MouseLButton);
    case VK_RBUTTON: return toShort(MouseRButton);
    case VK_MBUTTON: return toShort(MouseMButton);
    default:
        break;
    }

    const Uint8* keyboardState = MU_MobileGetKeyboardState();
    if (!keyboardState)
    {
        return 0;
    }

    auto keyDown = [&](SDL_Scancode scancode) -> SHORT
    {
        return (scancode != SDL_SCANCODE_UNKNOWN && keyboardState[scancode] != 0)
            ? static_cast<SHORT>(0x8000)
            : static_cast<SHORT>(0);
    };

    switch (key)
    {
    case VK_BACK:    return keyDown(SDL_SCANCODE_BACKSPACE);
    case VK_TAB:     return keyDown(SDL_SCANCODE_TAB);
    case VK_RETURN:
        return toShort(g_bEnterPressed || (keyboardState[SDL_SCANCODE_RETURN] != 0));
    case VK_SHIFT:   return toShort(keyboardState[SDL_SCANCODE_LSHIFT] || keyboardState[SDL_SCANCODE_RSHIFT]);
    case VK_CONTROL: return toShort(keyboardState[SDL_SCANCODE_LCTRL] || keyboardState[SDL_SCANCODE_RCTRL]);
    case VK_MENU:    return toShort(keyboardState[SDL_SCANCODE_LALT] || keyboardState[SDL_SCANCODE_RALT]);
    case VK_ESCAPE:  return keyDown(SDL_SCANCODE_ESCAPE);
    case VK_SPACE:   return keyDown(SDL_SCANCODE_SPACE);
    case VK_PRIOR:   return keyDown(SDL_SCANCODE_PAGEUP);
    case VK_NEXT:    return keyDown(SDL_SCANCODE_PAGEDOWN);
    case VK_END:     return keyDown(SDL_SCANCODE_END);
    case VK_HOME:    return keyDown(SDL_SCANCODE_HOME);
    case VK_LEFT:    return keyDown(SDL_SCANCODE_LEFT);
    case VK_UP:      return keyDown(SDL_SCANCODE_UP);
    case VK_RIGHT:   return keyDown(SDL_SCANCODE_RIGHT);
    case VK_DOWN:    return keyDown(SDL_SCANCODE_DOWN);
    case VK_INSERT:  return keyDown(SDL_SCANCODE_INSERT);
    case VK_DELETE:  return keyDown(SDL_SCANCODE_DELETE);
    case VK_F1:      return keyDown(SDL_SCANCODE_F1);
    case VK_F2:      return keyDown(SDL_SCANCODE_F2);
    case VK_F3:      return keyDown(SDL_SCANCODE_F3);
    case VK_F4:      return keyDown(SDL_SCANCODE_F4);
    case VK_F5:      return keyDown(SDL_SCANCODE_F5);
    case VK_F6:      return keyDown(SDL_SCANCODE_F6);
    case VK_F7:      return keyDown(SDL_SCANCODE_F7);
    case VK_F8:      return keyDown(SDL_SCANCODE_F8);
    case VK_F9:      return keyDown(SDL_SCANCODE_F9);
    case VK_F10:     return keyDown(SDL_SCANCODE_F10);
    case VK_F11:     return keyDown(SDL_SCANCODE_F11);
    case VK_F12:     return keyDown(SDL_SCANCODE_F12);
    default:
        break;
    }

    if (key >= 'A' && key <= 'Z')
    {
        return keyDown(static_cast<SDL_Scancode>(SDL_SCANCODE_A + (key - 'A')));
    }

    if (key >= '0' && key <= '9')
    {
        return keyDown(static_cast<SDL_Scancode>(SDL_SCANCODE_0 + (key - '0')));
    }

    return 0;
}
inline LRESULT DefWindowProc(HWND,UINT,WPARAM,LPARAM) { return 0; }

// ── GDI font constants ────────────────────────────────────────────────────
#define FW_DONTCARE         0
#define FW_THIN             100
#define FW_NORMAL           400
#define FW_BOLD             700
#define FW_BLACK            900
#define DEFAULT_CHARSET     1
#define ANSI_CHARSET        0
#define OUT_DEFAULT_PRECIS  0
#define CLIP_DEFAULT_PRECIS 0
#define DEFAULT_QUALITY     0
#define NONANTIALIASED_QUALITY 3
#define ANTIALIASED_QUALITY 4
#define DEFAULT_PITCH       0
#define FF_DONTCARE         0
#define FF_ROMAN            0x10
#define FF_SWISS            0x20
#define FF_MODERN           0x30
#define FIXED_PITCH         1
#define VARIABLE_PITCH      2
#define MB_ICONSTOP         0x0010L
// Font creation — routes to AndroidGDI (uses SDL2_ttf)
inline HFONT CreateFont(int height,int,int,int,int weight,int,int,int,int,int,int,int,int,LPCSTR) { return AndroidCreateFont(height, weight); }
inline HFONT CreateFont(int height,int,int,int,int weight,int,int,int,int,int,int,int,int,LPCWSTR) {
    return AndroidCreateFont(height, weight);
}

// GDI text measurement
inline BOOL GetTextExtentPoint32(HDC hdc, LPCWSTR text, int len, SIZE* sz) {
    int w = 0, h = 0;
    AndroidGetTextExtentPoint32(hdc, text, len, &w, &h);
    if (sz) { sz->cx = w; sz->cy = h; }
    return TRUE;
}
inline BOOL GetTextExtentPoint32(HDC, LPCSTR, int, SIZE* sz) { if (sz) { sz->cx = 0; sz->cy = 0; } return FALSE; }

// RECT intersection (Windows GDI)
inline BOOL IntersectRect(RECT* dst, const RECT* a, const RECT* b) {
    dst->left   = a->left   > b->left   ? a->left   : b->left;
    dst->top    = a->top    > b->top    ? a->top    : b->top;
    dst->right  = a->right  < b->right  ? a->right  : b->right;
    dst->bottom = a->bottom < b->bottom ? a->bottom : b->bottom;
    return (dst->left < dst->right && dst->top < dst->bottom) ? TRUE : FALSE;
}

// GDI painting stubs
inline HDC   BeginPaint(HWND, PAINTSTRUCT* ps)      { if(ps) memset(ps,0,sizeof(*ps)); return nullptr; }
inline void  EndPaint(HWND, const PAINTSTRUCT*)      {}
inline BOOL  SetBkColor(HDC hdc, DWORD c)    { AndroidSetBkColor(hdc, c);   return TRUE; }
inline BOOL  SetTextColor(HDC hdc, DWORD c)  { AndroidSetTextColor(hdc, c); return TRUE; }

// WGL stubs → SDL_GL handles context on Android
inline BOOL  wglMakeCurrent(HDC, HGLRC)             { return TRUE; }
inline BOOL  wglDeleteContext(HGLRC)                { return TRUE; }
inline HDC   GetDC(HWND)                            { return nullptr; }
inline BOOL  ReleaseDC(HWND, HDC)                   { return TRUE; }

// Path helper: convert wide path → UTF-8 with \ → / (must be before any user)
inline void AndroidConvertPath(const wchar_t* wpath, char* cpath, size_t n) {
    if (!cpath || n == 0) {
        return;
    }

    cpath[0] = '\0';
    if (!wpath) {
        return;
    }

    size_t converted = wcstombs(cpath, wpath, n - 1);
    if (converted == (size_t)-1) {
        size_t i = 0;
        while (wpath[i] != 0 && i < (n - 1)) {
            const wchar_t ch = wpath[i];
            cpath[i] = (ch >= 0 && ch <= 0x7f) ? static_cast<char>(ch) : '_';
            ++i;
        }
        cpath[i] = '\0';
    } else {
        cpath[n - 1] = '\0';
    }

    for (char* p = cpath; *p; ++p) {
        if (*p == '\\') {
            *p = '/';
        }
    }
}
// File I/O → POSIX
inline HANDLE CreateFile(LPCSTR lpPath, DWORD desiredAccess, DWORD, void*, DWORD creationDisposition, DWORD, HANDLE) {
    if (!lpPath) {
        return INVALID_HANDLE_VALUE;
    }

    const bool wantRead = (desiredAccess & GENERIC_READ) != 0;
    const bool wantWrite = (desiredAccess & GENERIC_WRITE) != 0;
    int openFlags = 0;

    if (wantRead && wantWrite) openFlags |= O_RDWR;
    else if (wantWrite) openFlags |= O_WRONLY;
    else openFlags |= O_RDONLY;

    switch (creationDisposition) {
    case CREATE_NEW: openFlags |= O_CREAT | O_EXCL; break;
    case CREATE_ALWAYS: openFlags |= O_CREAT | O_TRUNC; break;
    case OPEN_ALWAYS: openFlags |= O_CREAT; break;
    case TRUNCATE_EXISTING: openFlags |= O_TRUNC; break;
    default: break;
    }

    char cpath[MAX_PATH];
    strncpy(cpath, lpPath, MAX_PATH - 1);
    cpath[MAX_PATH - 1] = '\0';
    for (char* p = cpath; *p; ++p) if (*p == '\\') *p = '/';

    int fd = open(cpath, openFlags, 0666);
    if (fd < 0) return INVALID_HANDLE_VALUE;
    return reinterpret_cast<HANDLE>(static_cast<intptr_t>(fd + 1));
}

inline HANDLE CreateFile(LPCWSTR lpPath, DWORD desiredAccess, DWORD, void*, DWORD creationDisposition, DWORD, HANDLE) {
    if (!lpPath) {
        return INVALID_HANDLE_VALUE;
    }

    char cpath[MAX_PATH];
    AndroidConvertPath(lpPath, cpath, MAX_PATH);

    const bool wantRead = (desiredAccess & GENERIC_READ) != 0;
    const bool wantWrite = (desiredAccess & GENERIC_WRITE) != 0;
    int openFlags = 0;

    if (wantRead && wantWrite) {
        openFlags |= O_RDWR;
    } else if (wantWrite) {
        openFlags |= O_WRONLY;
    } else {
        openFlags |= O_RDONLY;
    }

    switch (creationDisposition) {
    case CREATE_NEW:
        openFlags |= O_CREAT | O_EXCL;
        break;
    case CREATE_ALWAYS:
        openFlags |= O_CREAT | O_TRUNC;
        if (!wantRead && !wantWrite) {
            openFlags &= ~O_RDONLY;
            openFlags |= O_WRONLY;
        }
        break;
    case OPEN_ALWAYS:
        openFlags |= O_CREAT;
        break;
    case TRUNCATE_EXISTING:
        openFlags |= O_TRUNC;
        if (!wantRead && !wantWrite) {
            openFlags &= ~O_RDONLY;
            openFlags |= O_WRONLY;
        }
        break;
    case OPEN_EXISTING:
    default:
        break;
    }

    int fd = open(cpath, openFlags, 0666);
    if (fd < 0) return INVALID_HANDLE_VALUE;
    return reinterpret_cast<HANDLE>(static_cast<intptr_t>(fd + 1));
}
inline DWORD GetFileSize(HANDLE hFile, DWORD* lpHigh) {
    if (!AndroidIsFdHandle(hFile)) {
        return 0;
    }
    int fd = AndroidHandleToFd(hFile);
    struct stat st{};
    if (fstat(fd, &st) != 0) return 0;
    if (lpHigh) *lpHigh = 0;
    return (DWORD)st.st_size;
}
inline BOOL ReadFile(HANDLE hFile, void* buf, DWORD toRead, DWORD* pRead, void*) {
    if (pRead) {
        *pRead = 0;
    }
    if (!AndroidIsFdHandle(hFile) || (!buf && toRead > 0)) {
        return FALSE;
    }
    int fd = AndroidHandleToFd(hFile);
    ssize_t n = ::read(fd, buf, toRead);
    if (n < 0) {
        return FALSE;
    }
    if (pRead) {
        *pRead = static_cast<DWORD>(n);
    }
    return TRUE;
}
inline BOOL CloseHandle(HANDLE hFile) {
    if (!hFile || hFile == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    if (AndroidIsFdHandle(hFile)) {
        return (close(AndroidHandleToFd(hFile)) == 0) ? TRUE : FALSE;
    }

    if (AndroidIsThreadHandle(hFile)) {
        auto* handle = reinterpret_cast<AndroidThreadHandle*>(hFile);
        bool releaseNow = false;
        pthread_mutex_lock(&handle->mutex);
        if (handle->completed) {
            releaseNow = true;
        } else {
            handle->closeRequested = true;
        }
        pthread_mutex_unlock(&handle->mutex);

        if (releaseNow) {
            AndroidDestroyThreadHandle(handle);
        }
        return TRUE;
    }

    return FALSE;
}

// Version info → not available on Android
inline DWORD GetFileVersionInfoSize(LPCWSTR, DWORD*) { return 0; }
inline BOOL  GetFileVersionInfo(LPCWSTR, DWORD, DWORD, void*) { return FALSE; }
inline BOOL  VerQueryValue(void*, LPCWSTR, LPVOID*, UINT*) { return FALSE; }

// IME stubs
inline HIMC  ImmGetContext(HWND)                      { return nullptr; }
inline BOOL  ImmSetConversionStatus(HIMC, DWORD, DWORD) { return FALSE; }
inline BOOL  ImmReleaseContext(HWND, HIMC)            { return FALSE; }
// SaveIMEStatus is defined in UIControls.cpp (not a Windows API stub)
inline void  CheckTextInputBoxIME(DWORD)              {}

// Network → POSIX sockets (already available on Android via <sys/socket.h>)
// WinSock2 compatibility
#define WSADATA int
inline int WSAStartup(WORD, void*) { return 0; }
inline int WSACleanup()            { return 0; }

// Crypto (DPAPI) → stub (replace with OpenSSL/botan if needed)
inline BOOL CryptProtectData(void*,void*,void*,void*,void*,DWORD,void*)   { return FALSE; }
inline BOOL CryptUnprotectData(void*,void*,void*,void*,void*,DWORD,void*) { return FALSE; }

// SetEnterPressed is defined in Scenes/SceneCommon.cpp (not a Windows API stub)

// ── Debug macros (crtdbg.h is Windows-only) ───────────────────────────────
#ifndef _ASSERT
#  define _ASSERT(expr) assert(expr)
#endif
#ifndef _ASSERTE
#  define _ASSERTE(expr) assert(expr)
#endif

// ── Size type ──────────────────────────────────────────────────────────────
typedef size_t SIZE_T;
// -- Timing / memory protection / socket event compatibility -----------------
typedef UINT MMRESULT;
typedef struct _LARGE_INTEGER { long long QuadPart; } LARGE_INTEGER;
typedef struct tagTIMECAPS { UINT wPeriodMin; UINT wPeriodMax; } TIMECAPS, *LPTIMECAPS;
#define TIMERR_NOCANDO 97
inline BOOL QueryPerformanceFrequency(LARGE_INTEGER* li) { if (!li) return FALSE; li->QuadPart = static_cast<long long>(MU_MobilePerfFrequency()); return TRUE; }
inline BOOL QueryPerformanceCounter(LARGE_INTEGER* li) { if (!li) return FALSE; li->QuadPart = static_cast<long long>(MU_MobilePerfNow()); return TRUE; }
inline MMRESULT timeGetDevCaps(TIMECAPS* caps, UINT) { if (caps) { caps->wPeriodMin = 1; caps->wPeriodMax = 1; } return 0; }
inline MMRESULT timeBeginPeriod(UINT) { return 0; }
inline MMRESULT timeEndPeriod(UINT) { return 0; }
#define PAGE_EXECUTE_READWRITE 0x40
inline BOOL VirtualProtect(void*, SIZE_T, DWORD, DWORD* oldProtect) { if (oldProtect) *oldProtect = PAGE_EXECUTE_READWRITE; return TRUE; }
#define FD_CONNECT 0x0010
#define FD_CLOSE   0x0020
#define FD_READ    0x0001
#define FD_WRITE   0x0002
inline int _ultoa_s(unsigned long value, char* buffer, size_t bufferCount, int radix) { if (!buffer || bufferCount == 0) return EINVAL; int written = (radix == 16) ? snprintf(buffer, bufferCount, "%lx", value) : snprintf(buffer, bufferCount, "%lu", value); if (written < 0 || (size_t)written >= bufferCount) { buffer[0] = '\0'; return ERANGE; } return 0; }
inline int _ultoa_s(unsigned long value, char* buffer, int radix) { return _ultoa_s(value, buffer, 34, radix); }
#ifndef GL_CURRENT_COLOR
#define GL_CURRENT_COLOR 0x0B00
#endif
inline void glColor4fv(const float* v) { if (v) { } }
inline void glVertex4fv(const float*) {}

// ── lstrlen / lstrcmpi / lstrcmp / lstrcpy ────────────────────────────────
#ifdef lstrlen
#undef lstrlen
#endif
inline int lstrlen(const char* s) { return s ? (int)strlen(s) : 0; }
inline int lstrlen(const wchar_t* s) { return s ? (int)wcslen(s) : 0; }
#define lstrcmpi(a,b)   wcscasecmp((a),(b))
#define lstrcmp(a,b)    wcscmp((a),(b))
#define lstrcpy(d,s)    wcscpy((d),(s))

// ── Cursor / mouse API stubs ──────────────────────────────────────────────
inline void SetCursorPos(int, int) {}
inline BOOL GetCursorPos(POINT* p)
{
    if (!p)
    {
        return FALSE;
    }

    const float safeScaleX = (g_fScreenRate_x > 0.0f) ? g_fScreenRate_x : 1.0f;
    const float safeScaleY = (g_fScreenRate_y > 0.0f) ? g_fScreenRate_y : 1.0f;

    p->x = static_cast<LONG>(std::lround(static_cast<float>(MouseX) * safeScaleX));
    p->y = static_cast<LONG>(std::lround(static_cast<float>(MouseY) * safeScaleY));
    return TRUE;
}
inline BOOL ScreenToClient(HWND, POINT*) { return TRUE; }   // no-op: coords already in client space on Android
inline BOOL ClientToScreen(HWND, POINT*) { return TRUE; }
inline DWORD GetDoubleClickTime() { return 500; }            // 500 ms default
inline HWND GetActiveWindow() { return g_bWndActive ? (g_hWnd ? g_hWnd : reinterpret_cast<HWND>(0x1)) : nullptr; }
inline HWND GetForegroundWindow() { return GetActiveWindow(); }
inline HWND GetDesktopWindow() { return reinterpret_cast<HWND>(0x1); }
inline BOOL GetWindowRect(HWND, RECT* rc) { if (rc) { rc->left = 0; rc->top = 0; rc->right = 640; rc->bottom = 480; } return TRUE; }

// ── SYSTEMTIME + GetLocalTime ─────────────────────────────────────────────
struct SYSTEMTIME {
    WORD wYear, wMonth, wDayOfWeek, wDay;
    WORD wHour, wMinute, wSecond, wMilliseconds;
};
inline void GetLocalTime(SYSTEMTIME* st) {
    time_t t = time(nullptr);
    struct tm* ti = localtime(&t);
    st->wYear        = (WORD)(ti->tm_year + 1900);
    st->wMonth       = (WORD)(ti->tm_mon + 1);
    st->wDay         = (WORD)(ti->tm_mday);
    st->wHour        = (WORD)(ti->tm_hour);
    st->wMinute      = (WORD)(ti->tm_min);
    st->wSecond      = (WORD)(ti->tm_sec);
    st->wMilliseconds = 0;
    st->wDayOfWeek   = (WORD)(ti->tm_wday);
}

// ── FILETIME (used by GetProcessTimes) ────────────────────────────────────
struct FILETIME { DWORD dwLowDateTime, dwHighDateTime; };

// ── SYSTEM_INFO + GetSystemInfo ───────────────────────────────────────────
struct SYSTEM_INFO { DWORD dwNumberOfProcessors; DWORD dwPageSize; };
inline void GetSystemInfo(SYSTEM_INFO* si) {
    si->dwNumberOfProcessors = 1;
    si->dwPageSize = 4096;
}
inline HANDLE GetCurrentProcess() { return INVALID_HANDLE_VALUE; }
inline BOOL GetProcessTimes(HANDLE, FILETIME*, FILETIME*, FILETIME*, FILETIME*) { return FALSE; }

// ── MEMORYSTATUS + GlobalMemoryStatus ────────────────────────────────────
struct MEMORYSTATUS {
    DWORD  dwLength;
    SIZE_T dwTotalPhys;
    SIZE_T dwAvailPhys;
    SIZE_T dwTotalVirtual;
    SIZE_T dwAvailVirtual;
};
inline void GlobalMemoryStatus(MEMORYSTATUS* ms) {
    ms->dwTotalPhys = 0; ms->dwAvailPhys = 0;
    ms->dwTotalVirtual = 0; ms->dwAvailVirtual = 0;
}

// ── HKL + keyboard layout stubs ───────────────────────────────────────────
typedef void* HKL;
inline HKL  GetKeyboardLayout(DWORD)                     { return nullptr; }
inline UINT ImmGetDescription(HKL, LPWSTR buf, UINT)     { if(buf) buf[0]=0; return 0; }
inline UINT ImmGetIMEFileName(HKL, LPWSTR buf, UINT)     { if(buf) buf[0]=0; return 0; }
inline BOOL GetKeyboardLayoutName(LPWSTR buf)            { if(buf) buf[0]=0; return FALSE; }
// ImmGetConversionStatus stub
inline BOOL ImmGetConversionStatus(HIMC, DWORD* conv, DWORD* sent) {
    if (conv) *conv = 0;
    if (sent) *sent = 0;
    return FALSE;
}
// IME conversion mode constants
#ifndef IME_CMODE_NATIVE
#  define IME_CMODE_NATIVE     0x0001
#endif
#ifndef IME_CMODE_HANGEUL
#  define IME_CMODE_HANGEUL    0x0001
#endif
#ifndef IME_CMODE_FULLSHAPE
#  define IME_CMODE_FULLSHAPE  0x0008
#endif
#ifndef IME_CMODE_ROMAN
#  define IME_CMODE_ROMAN      0x0010
#endif

// ── Additional Windows helpers ────────────────────────────────────────────
// GetSystemDirectory → Android has no system directory
inline UINT GetSystemDirectory(LPWSTR buf, UINT) { if(buf) buf[0]=0; return 0; }
// DeleteFile
inline BOOL DeleteFile(LPCWSTR path) {
    if (!path) return FALSE;
    char cpath[MAX_PATH] = {};
    AndroidConvertPath(path, cpath, MAX_PATH);
    return (::unlink(cpath) == 0) ? TRUE : FALSE;
}
inline BOOL DeleteFileA(LPCSTR path) { return path ? (::unlink(path) == 0 ? TRUE : FALSE) : FALSE; }
// SetFilePointer
#define FILE_BEGIN   0
#define FILE_CURRENT 1
#define FILE_END     2
inline DWORD SetFilePointer(HANDLE hFile, LONG dist, LONG*, DWORD method) {
    if (!AndroidIsFdHandle(hFile)) {
        return static_cast<DWORD>(-1);
    }
    int fd = AndroidHandleToFd(hFile);
    int whence = (method == FILE_BEGIN) ? SEEK_SET : (method == FILE_END) ? SEEK_END : SEEK_CUR;
    off_t pos = lseek(fd, dist, whence);
    if (pos < 0) {
        return static_cast<DWORD>(-1);
    }
    return static_cast<DWORD>(pos);
}
// WriteFile (POSIX write)
inline BOOL WriteFile(HANDLE hFile, const void* buf, DWORD toWrite, DWORD* pWritten, void*) {
    if (pWritten) {
        *pWritten = 0;
    }
    if (!AndroidIsFdHandle(hFile) || (!buf && toWrite > 0)) {
        return FALSE;
    }
    int fd = AndroidHandleToFd(hFile);
    ssize_t n = ::write(fd, buf, toWrite);
    if (n < 0) {
        return FALSE;
    }
    if (pWritten) {
        *pWritten = static_cast<DWORD>(n);
    }
    return n >= 0 ? TRUE : FALSE;
}
// OPEN_ALWAYS for CreateFile
#ifndef OPEN_ALWAYS
#  define OPEN_ALWAYS 4
#endif

// ── __forceinline (MSVC intrinsic → GCC/Clang always_inline) ─────────────
#ifndef __forceinline
#  define __forceinline __attribute__((always_inline)) inline
#endif

// ── _snwprintf → swprintf with size ──────────────────────────────────────
#define _snwprintf(buf,n,...) swprintf((buf),(n),__VA_ARGS__)

// ── GDI bitmap types (stub) ───────────────────────────────────────────────
typedef struct tagRGBQUAD { BYTE rgbBlue, rgbGreen, rgbRed, rgbReserved; } RGBQUAD;
typedef struct tagPALETTEENTRY { BYTE peRed, peGreen, peBlue, peFlags; } PALETTEENTRY;
struct BITMAPINFOHEADER {
    DWORD biSize, biWidth; LONG biHeight;
    WORD  biPlanes, biBitCount; DWORD biCompression, biSizeImage;
    LONG  biXPelsPerMeter, biYPelsPerMeter; DWORD biClrUsed, biClrImportant;
};
struct BITMAPINFO {
    BITMAPINFOHEADER bmiHeader;
    RGBQUAD          bmiColors[1];
};
#define BI_RGB       0L
#define DIB_RGB_COLORS 0
// GDI functions — route to AndroidGDI.cpp (real SDL2_ttf-based implementation)
inline HBITMAP CreateDIBSection(HDC, const BITMAPINFO* bmi, UINT, void** ppvBits, HANDLE, DWORD) {
    return AndroidCreateDIBSection(bmi, ppvBits);
}
inline HDC     CreateCompatibleDC(HDC src)                          { return AndroidCreateCompatibleDC(src); }
// SelectObject overloads — C++ resolves the correct AndroidSelectBitmap/Font call
inline HGDIOBJ SelectObject(HDC hdc, HBITMAP bmp)  { AndroidSelectBitmap(hdc, bmp); return (HGDIOBJ)bmp; }
inline HGDIOBJ SelectObject(HDC hdc, HFONT   font) { AndroidSelectFont(hdc, font);  return (HGDIOBJ)font; }
// HGDIOBJ = void* — template fallback for brushes/pens/other objects
template<typename T>
inline HGDIOBJ SelectObject(HDC, T* h)             { return (HGDIOBJ)h; }
inline BOOL    DeleteDC(HDC hdc)                   { return AndroidDeleteDC(hdc)  ? TRUE : FALSE; }
inline BOOL    DeleteObject(HGDIOBJ obj)            { return AndroidDeleteObject(obj) ? TRUE : FALSE; }
inline BOOL    TextOut(HDC hdc, int x, int y, LPCWSTR text, int len) {
    return AndroidTextOut(hdc, x, y, text, len) ? TRUE : FALSE;
}
inline BOOL    TextOut(HDC hdc, int x, int y, LPCSTR text, int len) {
    if (!text) return FALSE;
    wchar_t wide[2048] = {};
    mbstowcs(wide, text, sizeof(wide) / sizeof(wide[0]) - 1);
    return TextOut(hdc, x, y, wide, len);
}
// GWLP_WNDPROC + SetWindowLongPtrW (edit box subclassing)
#define GWLP_WNDPROC (-4)
#define GWL_WNDPROC GWLP_WNDPROC
#define GWLP_USERDATA (-21)
#define GWL_USERDATA GWLP_USERDATA
inline LONG_PTR SetWindowLongPtrW(HWND handle, int index, LONG_PTR value)
{
    auto* control = AndroidGetEditControl(handle);
    if (!control)
    {
        return 0;
    }

    if (index == GWLP_WNDPROC)
    {
        const LONG_PTR previous = reinterpret_cast<LONG_PTR>(
            control->WindowProc ? control->WindowProc : AndroidDefaultEditWndProc);
        control->WindowProc = reinterpret_cast<WNDPROC>(value);
        return previous;
    }

    if (index == GWLP_USERDATA)
    {
        const LONG_PTR previous = control->UserData;
        control->UserData = value;
        return previous;
    }

    return 0;
}
inline LONG_PTR GetWindowLongPtrW(HWND handle, int index)
{
    auto* control = AndroidGetEditControl(handle);
    if (!control)
    {
        return 0;
    }

    if (index == GWLP_WNDPROC)
    {
        return reinterpret_cast<LONG_PTR>(control->WindowProc ? control->WindowProc : AndroidDefaultEditWndProc);
    }

    if (index == GWLP_USERDATA)
    {
        return control->UserData;
    }

    return 0;
}
#define SetWindowLongPtr SetWindowLongPtrW
#define GetWindowLongPtr GetWindowLongPtrW
inline LONG_PTR SetWindowLongW(HWND handle, int index, LONG_PTR value) { return SetWindowLongPtrW(handle, index, value); }
inline LONG_PTR GetWindowLongW(HWND handle, int index) { return GetWindowLongPtrW(handle, index); }

// ── INT64 typedef (used by game code) ────────────────────────────────────
typedef int64_t  INT64;
typedef uint64_t UINT64;

// ── OutputDebugString stubs ───────────────────────────────────────────────
inline void OutputDebugStringA(const char* s)    { ALOGI("%s", s); }
inline void OutputDebugStringW(const wchar_t* s) { ALOGW("%ls", s); }
#define OutputDebugString(s) OutputDebugStringW(s)

// ── VK_SNAPSHOT (Print Screen key) ───────────────────────────────────────
#define VK_SNAPSHOT   0x2C
#define VK_NUMPAD0    0x60
#define VK_NUMPAD1    0x61
#define VK_NUMPAD2    0x62
#define VK_NUMPAD3    0x63
#define VK_NUMPAD4    0x64
#define VK_NUMPAD5    0x65
#define VK_NUMPAD6    0x66
#define VK_NUMPAD7    0x67
#define VK_NUMPAD8    0x68
#define VK_NUMPAD9    0x69
#define VK_MULTIPLY   0x6A
#define VK_ADD        0x6B
#define VK_SUBTRACT   0x6D
#define VK_DECIMAL    0x6E
#define VK_DIVIDE     0x6F
#define VK_CAPITAL    0x14
#define VK_NUMLOCK    0x90
#define VK_SCROLL     0x91
#define VK_LSHIFT     0xA0
#define VK_RSHIFT     0xA1
#define VK_LCONTROL   0xA2
#define VK_RCONTROL   0xA3
#define VK_LMENU      0xA4
#define VK_RMENU      0xA5

// ── SetRect / PtInRect / OffsetRect / EqualRect (Windows GDI) ─────────────
inline void SetRect(RECT* r, LONG l, LONG t, LONG ri, LONG b) {
    r->left=l; r->top=t; r->right=ri; r->bottom=b;
}
inline BOOL PtInRect(const RECT* r, POINT pt) {
    return (pt.x >= r->left && pt.x < r->right &&
            pt.y >= r->top  && pt.y < r->bottom) ? TRUE : FALSE;
}
inline void OffsetRect(RECT* r, LONG dx, LONG dy) {
    r->left+=dx; r->right+=dx; r->top+=dy; r->bottom+=dy;
}
inline BOOL EqualRect(const RECT* a, const RECT* b) {
    return (a->left==b->left && a->top==b->top &&
            a->right==b->right && a->bottom==b->bottom) ? TRUE : FALSE;
}
inline BOOL IsRectEmpty(const RECT* r) {
    return (r->left>=r->right || r->top>=r->bottom) ? TRUE : FALSE;
}
inline void UnionRect(RECT* dst, const RECT* a, const RECT* b) {
    dst->left   = a->left   < b->left   ? a->left   : b->left;
    dst->top    = a->top    < b->top    ? a->top    : b->top;
    dst->right  = a->right  > b->right  ? a->right  : b->right;
    dst->bottom = a->bottom > b->bottom ? a->bottom : b->bottom;
}

// ── IsBadReadPtr → always return FALSE (Android has no GPF probing) ──────
inline BOOL IsBadReadPtr(const void*, size_t) { return FALSE; }

// ── GetCommandLine stub (returns empty argv-style string) ─────────────────
inline char* GetCommandLine() { static char cmd[] = ""; return cmd; }

// ── MCI constants (mmsystem.h multimedia) ────────────────────────────────
#define MCI_SEQ_MAPPER   0xFFFFL

// ── FreeLibrary/LoadLibrary stubs (not needed on Android) ────────────────
typedef void (*FARPROC)();
inline BOOL FreeLibrary(HMODULE) { return TRUE; }
inline HMODULE LoadLibrary(LPCWSTR) { return nullptr; }

// ── GetCurrentThreadId / GetCurrentProcessId / GetWindowThreadProcessId ──
inline DWORD GetCurrentThreadId() { return 0; }
inline DWORD GetCurrentProcessId() { return 0; }
inline DWORD GetWindowThreadProcessId(HWND, DWORD*) { return 0; }
// EnumChildWindows stub
typedef BOOL (CALLBACK* WNDENUMPROC)(HWND, LPARAM);
inline BOOL EnumChildWindows(HWND, WNDENUMPROC, LPARAM) { return FALSE; }

// ── GetClassName stub ──────────────────────────────────────────────────────
inline int GetClassName(HWND, LPWSTR buf, int) { if(buf) buf[0]=0; return 0; }

// ── IsWindowVisible / GetSystemMenu / DrawMenuBar / RemoveMenu ────────────
inline BOOL IsWindowVisible(HWND handle)
{
    auto* control = AndroidGetEditControl(handle);
    if (control)
    {
        return control->Visible ? TRUE : FALSE;
    }

    return handle ? TRUE : FALSE;
}
inline HMENU GetSystemMenu(HWND, BOOL) { return nullptr; }
inline BOOL DrawMenuBar(HWND) { return FALSE; }
#define SC_CLOSE        0xF060L
#define MF_BYCOMMAND    0x00000000L
inline BOOL RemoveMenu(HMENU, UINT, UINT) { return FALSE; }
// Console API stubs
inline BOOL SetConsoleTitle(LPCWSTR) { return FALSE; }
inline DWORD GetConsoleTitle(LPWSTR buf, DWORD) { if(buf) buf[0]=0; return 0; }
inline BOOL AllocConsole() { return FALSE; }
inline BOOL FreeConsole() { return FALSE; }
inline HANDLE GetStdHandle(DWORD) { return INVALID_HANDLE_VALUE; }
inline BOOL SetConsoleMode(HANDLE, DWORD) { return FALSE; }
inline void SetLastError(DWORD) {}
// Console mode constants
#define STD_INPUT_HANDLE    ((DWORD)-10)
#define STD_OUTPUT_HANDLE   ((DWORD)-11)
#define STD_ERROR_HANDLE    ((DWORD)-12)
#define ENABLE_LINE_INPUT          0x0002
#define ENABLE_ECHO_INPUT          0x0004
#define ENABLE_PROCESSED_INPUT     0x0001
#define ENABLE_PROCESSED_OUTPUT    0x0001
#define ENABLE_WRAP_AT_EOL_OUTPUT  0x0002
// CONSOLE structs (stubs)
struct COORD { SHORT X, Y; };
struct SMALL_RECT { SHORT Left, Top, Right, Bottom; };
struct CONSOLE_SCREEN_BUFFER_INFO {
    COORD dwSize;
    COORD dwCursorPosition;
    WORD  wAttributes;
};
struct CHAR_INFO { wchar_t Char; WORD Attributes; };
inline BOOL GetConsoleScreenBufferInfo(HANDLE, CONSOLE_SCREEN_BUFFER_INFO* p) {
    if(p) memset(p,0,sizeof(*p)); return FALSE;
}
inline BOOL FillConsoleOutputCharacter(HANDLE,TCHAR,DWORD,COORD,DWORD*) { return FALSE; }
inline BOOL FillConsoleOutputAttribute(HANDLE,WORD,DWORD,COORD,DWORD*) { return FALSE; }
inline BOOL SetConsoleCursorPosition(HANDLE,COORD) { return FALSE; }
inline BOOL SetConsoleTextAttribute(HANDLE,WORD) { return FALSE; }
inline BOOL ReadConsoleOutput(HANDLE,CHAR_INFO*,COORD,COORD,SMALL_RECT*) { return FALSE; }
#define ERROR_SUCCESS 0L

// ── u_int64 (BSD typedef, not on Android) ────────────────────────────────
typedef uint64_t u_int64;
typedef uint32_t u_int32;
typedef uint16_t u_int16;
typedef uint8_t  u_int8;

// ── wcsnicmp → wcsncasecmp ────────────────────────────────────────────────
#define wcsnicmp(a,b,n)  wcsncasecmp((a),(b),(n))
#define _wcsnicmp(a,b,n) wcsncasecmp((a),(b),(n))

// ── Clipboard stubs ───────────────────────────────────────────────────────
typedef void* HGLOBAL;
#define CF_TEXT       1
#define CF_UNICODETEXT 13
inline BOOL   OpenClipboard(HWND)               { return FALSE; }
inline BOOL   CloseClipboard()                  { return FALSE; }
inline BOOL   EmptyClipboard()                  { return FALSE; }
inline HANDLE SetClipboardData(UINT, HANDLE)    { return nullptr; }
inline HANDLE GetClipboardData(UINT)            { return nullptr; }
inline HGLOBAL GlobalAlloc(UINT, size_t)        { return nullptr; }
inline void*  GlobalLock(HGLOBAL)               { return nullptr; }
inline BOOL   GlobalUnlock(HGLOBAL)             { return TRUE; }
inline HGLOBAL GlobalFree(HGLOBAL h)            { return h; }
#define GMEM_MOVEABLE 0x0002

// ── IME composition / window message constants ────────────────────────────
#define WM_IME_COMPOSITION        0x010F
#define WM_IME_STARTCOMPOSITION   0x010D
#define WM_IME_ENDCOMPOSITION     0x010E
#define WM_IME_NOTIFY             0x0282
#define IMN_SETOPENSTATUS         0x0008
#define GCS_RESULTSTR             0x0800
// COMPOSITIONFORM struct
struct COMPOSITIONFORM { DWORD dwStyle; POINT ptCurrentPos; RECT rcArea; };
#define CFS_FORCE_POSITION  0x0020
#define CFS_DEFAULT         0x0000
inline BOOL ImmSetCompositionWindow(HIMC, COMPOSITIONFORM*) { return FALSE; }
inline BOOL ImmGetCompositionWindow(HIMC, COMPOSITIONFORM* p) { if(p) memset(p,0,sizeof(*p)); return FALSE; }
inline LONG ImmGetCompositionStringW(HIMC, DWORD, void* buf, DWORD n) { if(buf&&n>0) memset(buf,0,n); return 0; }
#define ImmGetCompositionString ImmGetCompositionStringW

// ── CallWindowProcW / GetWindowText / GetCaretPos ─────────────────────────
inline LRESULT CallWindowProcW(WNDPROC proc, HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (proc)
    {
        return proc(handle, message, wParam, lParam);
    }

    return AndroidDefaultEditWndProc(handle, message, wParam, lParam);
}
#define CallWindowProc CallWindowProcW
inline int  GetWindowTextW(HWND handle, LPWSTR buffer, int bufferLength)
{
    if (!buffer || bufferLength <= 0)
    {
        return 0;
    }

    auto* control = AndroidGetEditControl(handle);
    if (!control)
    {
        buffer[0] = 0;
        return 0;
    }

    const int copyLength = AndroidClampInt(static_cast<int>(control->Text.length()), 0, bufferLength - 1);
    if (copyLength > 0)
    {
        std::wmemcpy(buffer, control->Text.c_str(), static_cast<std::size_t>(copyLength));
    }
    buffer[copyLength] = 0;
    return copyLength;
}
inline int GetWindowTextA(HWND handle, LPSTR buffer, int bufferLength) { if (!buffer || bufferLength <= 0) return 0; wchar_t wide[2048] = {}; int len = GetWindowTextW(handle, wide, (int)(sizeof(wide)/sizeof(wide[0]))); wcstombs(buffer, wide, bufferLength - 1); buffer[bufferLength - 1] = 0; return len; }
#define GetWindowText GetWindowTextA
inline BOOL GetCaretPos(POINT* p) { if(p){p->x=0;p->y=0;} return FALSE; }

// ── Edit control / window messages ───────────────────────────────────────
#define EM_SETLIMITTEXT  0x00C5
#define EM_SETSEL        0x00B1
#define EM_GETSEL        0x00B0
#define EM_REPLACESEL    0x00C2
#define EM_GETLINECOUNT  0x00BA
#define EM_SETPASSWORDCHAR 0x00CC
// Window style flags
#define ES_PASSWORD      0x0020L
#define ES_MULTILINE     0x0004L
#define ES_LEFT          0x0000L
#define ES_AUTOHSCROLL   0x0080L
#define ES_WANTRETURN    0x1000L
#define WS_CHILD         0x40000000L
#define WS_VISIBLE       0x10000000L
#define WS_BORDER        0x00800000L
#define WS_VSCROLL       0x00200000L
// SetWindowPos flags
#define SWP_NOMOVE       0x0002
#define SWP_NOZORDER     0x0004
#define SWP_NOSIZE       0x0001
#define SWP_SHOWWINDOW   0x0040
#define HWND_TOP         ((HWND)0)
inline BOOL SetWindowPos(HWND handle, HWND, int, int, int, int, UINT flags)
{
    auto* control = AndroidGetEditControl(handle);
    if (!control)
    {
        return FALSE;
    }

    if (flags & SWP_SHOWWINDOW)
    {
        control->Visible = true;
    }

    return TRUE;
}
// CreateWindowEx / CreateWindowW stubs for edit boxes
inline HWND CreateWindowExW(DWORD, LPCWSTR, LPCWSTR, DWORD style, int, int, int, int, HWND, HMENU, HINSTANCE, void*)
{
    auto* token = new int(1);
    const HWND handle = reinterpret_cast<HWND>(token);

    AndroidStubEditControl control;
    control.Visible = (style & WS_VISIBLE) != 0;
    control.IsPassword = (style & 0x0020L) != 0; // ES_PASSWORD
    control.WindowProc = AndroidDefaultEditWndProc;

    AndroidEditControls().emplace(handle, std::move(control));
    return handle;
}
#define CreateWindowEx CreateWindowExW
#define CreateWindowW(cls,wnd,style,x,y,w,h,parent,menu,inst,par) CreateWindowExW(0,cls,wnd,style,x,y,w,h,parent,menu,inst,par)
#define CreateWindow CreateWindowW
// SendMessageW / PostMessageW / SetWindowTextW aliases
#define SendMessageW    SendMessage
#define PostMessageW    PostMessage
inline BOOL SetWindowTextW(HWND handle, LPCWSTR text)
{
    auto* control = AndroidGetEditControl(handle);
    if (!control)
    {
        return FALSE;
    }

    control->Text = text ? text : L"";
    if (control->Limit > 0 && static_cast<int>(control->Text.length()) > control->Limit)
    {
        control->Text.resize(static_cast<std::wstring::size_type>(control->Limit));
    }

    control->SelectionStart = static_cast<int>(control->Text.length());
    control->SelectionEnd = control->SelectionStart;
    return TRUE;
}
inline BOOL SetWindowTextA(HWND handle, LPCSTR text) { return SetWindowTextW(handle, text ? std::wstring(text, text + strlen(text)).c_str() : L""); }
#define SetWindowText SetWindowTextA
// Scrollbar / edit messages
#define SB_VERT         1
#define SB_LINEUP       0
#define SB_LINEDOWN     1
#define SB_PAGEUP       2
#define SB_PAGEDOWN     3
#define WM_SETFONT      0x0030
#define EM_SCROLL       0x00B5
#define ES_AUTOVSCROLL  0x0040L
#define ES_AUTOHSCROLL  0x0080L
// Scroll position API
inline int  GetScrollPos(HWND, int)             { return 0; }
inline int  SetScrollPos(HWND, int, int, BOOL)  { return 0; }
#define EM_LINESCROLL   0x00B6
// IME string flags
#define GCS_COMPSTR     0x0008
#define GCS_COMPATTR    0x0010
#define GCS_RESULTSTR   0x0800
// itoa / _itoa → sprintf-based
inline char* itoa(int v, char* buf, int base) {
    if (base==16) sprintf(buf, "%x", v);
    else sprintf(buf, "%d", v);
    return buf;
}
#define _itoa(v,buf,base) itoa((v),(buf),(base))
// RegisterClassEx stub
struct WNDCLASSEX { UINT cbSize; UINT style; WNDPROC lpfnWndProc; int cbClsExtra; int cbWndExtra; HINSTANCE hInstance; HICON hIcon; HCURSOR hCursor; HBRUSH hbrBackground; LPCWSTR lpszMenuName; LPCWSTR lpszClassName; HICON hIconSm; };
typedef WORD ATOM;
inline ATOM RegisterClassExW(const WNDCLASSEX*) { return 0; }
#define RegisterClassEx RegisterClassExW

// ── 2-arg swprintf shim (MSVC allows swprintf without size; POSIX requires it)
// Provides global-namespace swprintf(buf, fmt, ...) that adds a default size.
// std::swprintf(buf, n, fmt, ...) calls are unaffected (qualified lookup).
#include <cstdarg>
inline int swprintf(wchar_t* buf, const wchar_t* fmt, ...) {
    va_list va;
    va_start(va, fmt);
    int n = vswprintf(buf, 1024, fmt, va);
    va_end(va);
    return n;
}

#include <chrono>
#include <thread>
#include <wctype.h>

// ── strsafe.h replacements ────────────────────────────────────────────────
// StringCchCopy(W) — safe copy with char count limit
inline long StringCchCopyW(wchar_t* dst, size_t cchDst, const wchar_t* src) {
    if (!dst || cchDst == 0) return 0x80070057L; // STRSAFE_E_INVALID_PARAMETER
    wcsncpy(dst, src, cchDst - 1);
    dst[cchDst - 1] = L'\0';
    return 0L; // S_OK
}
inline long StringCchCopyA(char* dst, size_t cchDst, const char* src) {
    if (!dst || cchDst == 0) return 0x80070057L;
    strncpy(dst, src, cchDst - 1);
    dst[cchDst - 1] = '\0';
    return 0L;
}
#define StringCchCopy StringCchCopyW

// ── RtlSecureZeroMemory → memset (no speculative-execution zeroing needed on Android) ─
inline void* RtlSecureZeroMemory(void* ptr, size_t sz) {
    return memset(ptr, 0, sz);
}

// ── MessageBoxW alias ─────────────────────────────────────────────────────
#define MessageBoxW(hwnd,txt,cap,flags) MessageBox((hwnd),(txt),(cap),(flags))

// ── ExitProcess → exit() on Android ──────────────────────────────────────
inline void ExitProcess(UINT code) { exit((int)code); }

// ── CFS_POINT (IME composition position style) ────────────────────────────
#ifndef CFS_POINT
#  define CFS_POINT 0x0002
#endif

// ── ImmGetDefaultIMEWnd stub ──────────────────────────────────────────────
inline HWND ImmGetDefaultIMEWnd(HWND) { return nullptr; }

// ── WM_IME_CONTROL / IMC_SETCOMPOSITIONWINDOW ─────────────────────────────
#ifndef WM_IME_CONTROL
#  define WM_IME_CONTROL           0x0283
#endif
#ifndef IMC_SETCOMPOSITIONWINDOW
#  define IMC_SETCOMPOSITIONWINDOW 0x000C
#endif

// ── IME sentence mode constants ────────────────────────────────────────────
#ifndef IME_SMODE_AUTOMATIC
#  define IME_SMODE_AUTOMATIC      0x0004
#endif
#ifndef IME_SMODE_PHRASEPREDICT
#  define IME_SMODE_PHRASEPREDICT  0x0008
#endif
#ifndef IME_SMODE_CONVERSATION
#  define IME_SMODE_CONVERSATION   0x0010
#endif

// ── _TRUNCATE sentinel (MSVC safe-string "truncate" mode) ────────────────
#ifndef _TRUNCATE
#  define _TRUNCATE ((size_t)-1)
#endif

// ── _wcsupr → wide-char to uppercase in-place ────────────────────────────
inline wchar_t* _wcsupr(wchar_t* s) {
    for (wchar_t* p = s; *p; ++p) *p = (wchar_t)towupper((wint_t)*p);
    return s;
}

// ── _tzset → POSIX tzset ──────────────────────────────────────────────────
#ifndef _tzset
#  define _tzset() tzset()
#endif

// ── errno_t (MSVC safe-string return type) ────────────────────────────────
#ifndef _ERRNO_T_DEFINED
#  define _ERRNO_T_DEFINED
typedef int errno_t;
#endif

// ── _wfopen_s → POSIX fopen via wcstombs ─────────────────────────────────
inline errno_t _wfopen_s(FILE** pFile, const wchar_t* path, const wchar_t* mode) {
    if (!pFile) return EINVAL;
    char cpath[MAX_PATH], cmode[16];
    wcstombs(cpath, path, MAX_PATH);
    for (char* p = cpath; *p; ++p) if (*p == '\\') *p = '/';
    wcstombs(cmode, mode, 16);
    *pFile = fopen(cpath, cmode);
    return (*pFile) ? 0 : errno;
}

// ── BITMAPFILEHEADER (GDI bitmap file header) ─────────────────────────────
#pragma pack(push, 1)
struct BITMAPFILEHEADER {
    WORD  bfType;
    DWORD bfSize;
    WORD  bfReserved1;
    WORD  bfReserved2;
    DWORD bfOffBits;
};
#pragma pack(pop)

// ── GetCurrentDirectory → returns empty string on Android ────────────────
inline DWORD GetCurrentDirectoryW(DWORD nBufLen, LPWSTR lpBuffer) {
    char cwd[MAX_PATH] = {};
    if (!getcwd(cwd, sizeof(cwd))) {
        if (lpBuffer && nBufLen > 0) {
            lpBuffer[0] = L'\0';
        }
        return 0;
    }

    for (char* p = cwd; *p; ++p) {
        if (*p == '/') {
            *p = '\\';
        }
    }

    const size_t len = strlen(cwd);
    if (!lpBuffer || nBufLen == 0) {
        return static_cast<DWORD>(len);
    }
    if (len + 1 > nBufLen) {
        lpBuffer[0] = L'\0';
        return static_cast<DWORD>(len + 1);
    }

    for (size_t i = 0; i < len; ++i) {
        lpBuffer[i] = static_cast<unsigned char>(cwd[i]);
    }
    lpBuffer[len] = L'\0';
    return static_cast<DWORD>(len);
}
#define GetCurrentDirectory GetCurrentDirectoryW

// ── _wtoi64 → wide string to int64 ───────────────────────────────────────
inline long long _wtoi64(const wchar_t* s) { return (long long)wcstoll(s, nullptr, 10); }

// ── WIN32_FIND_DATA (filesystem search) ──────────────────────────────────
struct _WIN32_FIND_DATAW {
    DWORD dwFileAttributes;
    FILETIME ftCreationTime, ftLastAccessTime, ftLastWriteTime;
    DWORD nFileSizeHigh, nFileSizeLow;
    DWORD dwReserved0, dwReserved1;
    wchar_t cFileName[MAX_PATH];
    wchar_t cAlternateFileName[14];
};
typedef _WIN32_FIND_DATAW WIN32_FIND_DATAW;
typedef WIN32_FIND_DATAW WIN32_FIND_DATA;

// ── _beginthreadex → POSIX pthread ───────────────────────────────────────
inline uintptr_t _beginthreadex(void*, unsigned, unsigned(__stdcall* fn)(void*), void* arg, unsigned, unsigned* pThreadId) {
    if (!fn) {
        return reinterpret_cast<uintptr_t>(INVALID_HANDLE_VALUE);
    }

    auto* handle = new AndroidThreadHandle();
    handle->magic = kAndroidThreadHandleMagic;
    handle->completed = false;
    handle->joined = false;
    handle->closeRequested = false;
    handle->result = 0;
    if (pthread_mutex_init(&handle->mutex, nullptr) != 0) {
        delete handle;
        return reinterpret_cast<uintptr_t>(INVALID_HANDLE_VALUE);
    }
    if (pthread_cond_init(&handle->cond, nullptr) != 0) {
        pthread_mutex_destroy(&handle->mutex);
        delete handle;
        return reinterpret_cast<uintptr_t>(INVALID_HANDLE_VALUE);
    }

    auto* args = new AndroidBeginThreadArgs();
    args->fn = fn;
    args->arg = arg;
    args->handle = handle;

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    int rc = pthread_create(&handle->thread, &attr, AndroidBeginThreadProc, args);
    pthread_attr_destroy(&attr);
    if (rc != 0) {
        delete args;
        AndroidDestroyThreadHandle(handle);
        return reinterpret_cast<uintptr_t>(INVALID_HANDLE_VALUE);
    }

    if (pThreadId) {
        uintptr_t threadBits = 0;
        const size_t copySize = (sizeof(threadBits) < sizeof(handle->thread)) ? sizeof(threadBits) : sizeof(handle->thread);
        memcpy(&threadBits, &handle->thread, copySize);
        *pThreadId = static_cast<unsigned>(threadBits & UINT_MAX);
    }
    return reinterpret_cast<uintptr_t>(handle);
}
// ── Thread wait constants ────────────────────────────────────────────────
#define WAIT_TIMEOUT    258L
#define WAIT_OBJECT_0   0L
#define INFINITE        0xFFFFFFFF
// ── WaitForSingleObject stub ──────────────────────────────────────────────
inline DWORD WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds) {
    if (!AndroidIsThreadHandle(hHandle)) {
        return static_cast<DWORD>(-1);
    }

    auto* handle = reinterpret_cast<AndroidThreadHandle*>(hHandle);
    pthread_mutex_lock(&handle->mutex);

    if (dwMilliseconds == 0) {
        DWORD result = handle->completed ? WAIT_OBJECT_0 : WAIT_TIMEOUT;
        pthread_mutex_unlock(&handle->mutex);
        return result;
    }

    if (dwMilliseconds == INFINITE) {
        while (!handle->completed) {
            pthread_cond_wait(&handle->cond, &handle->mutex);
        }
        pthread_mutex_unlock(&handle->mutex);
        return WAIT_OBJECT_0;
    }

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += static_cast<time_t>(dwMilliseconds / 1000);
    ts.tv_nsec += static_cast<long>((dwMilliseconds % 1000) * 1000000L);
    if (ts.tv_nsec >= 1000000000L) {
        ts.tv_sec += 1;
        ts.tv_nsec -= 1000000000L;
    }

    while (!handle->completed) {
        int rc = pthread_cond_timedwait(&handle->cond, &handle->mutex, &ts);
        if (rc == ETIMEDOUT && !handle->completed) {
            pthread_mutex_unlock(&handle->mutex);
            return WAIT_TIMEOUT;
        }
    }

    pthread_mutex_unlock(&handle->mutex);
    return WAIT_OBJECT_0;
}

// ── File attribute constants ────────────────────────────────────────────
#define FILE_ATTRIBUTE_READONLY     0x00000001
#define FILE_ATTRIBUTE_HIDDEN       0x00000002
#define FILE_ATTRIBUTE_DIRECTORY    0x00000010
#define INVALID_FILE_ATTRIBUTES     ((DWORD)-1)
#ifndef CREATE_ALWAYS
#define CREATE_ALWAYS               2
#endif
#ifndef CREATE_NEW
#define CREATE_NEW                  1
#endif
#ifndef TRUNCATE_EXISTING
#define TRUNCATE_EXISTING           5
#endif
// ── GetFileAttributes → POSIX stat ────────────────────────────────────────
inline DWORD GetFileAttributesW(LPCWSTR path) {
    char cpath[MAX_PATH]; AndroidConvertPath(path, cpath, MAX_PATH);
    struct stat st;
    if (stat(cpath, &st) != 0) return INVALID_FILE_ATTRIBUTES;
    DWORD attr = 0;
    if (S_ISDIR(st.st_mode)) attr |= FILE_ATTRIBUTE_DIRECTORY;
    return attr ? attr : FILE_ATTRIBUTE_NORMAL;
}
#define GetFileAttributes GetFileAttributesW
// ── SetFileAttributes → no-op on Android ────────────────────────────────
inline BOOL SetFileAttributesW(LPCWSTR, DWORD) { return FALSE; }
#define SetFileAttributes SetFileAttributesW

// ── GetModuleFileName → returns empty on Android ─────────────────────────
inline DWORD GetModuleFileNameW(HMODULE, LPWSTR buf, DWORD n) {
    char cwd[MAX_PATH] = {};
    if (!getcwd(cwd, sizeof(cwd))) {
        if (buf && n > 0) {
            buf[0] = L'\0';
        }
        return 0;
    }

    for (char* p = cwd; *p; ++p) {
        if (*p == '/') {
            *p = '\\';
        }
    }

    char modulePath[MAX_PATH] = {};
    snprintf(modulePath, sizeof(modulePath), "%s\\MuMain.exe", cwd);

    const size_t len = strlen(modulePath);
    if (!buf || n == 0) {
        return static_cast<DWORD>(len);
    }
    if (len + 1 > n) {
        buf[0] = L'\0';
        return static_cast<DWORD>(len + 1);
    }

    for (size_t i = 0; i < len; ++i) {
        buf[i] = static_cast<unsigned char>(modulePath[i]);
    }
    buf[len] = L'\0';
    return static_cast<DWORD>(len);
}
#define GetModuleFileName GetModuleFileNameA

// ── INI file API → no-op stubs (Android uses SDL prefs or json) ──────────
inline UINT GetPrivateProfileIntW(LPCWSTR, LPCWSTR, INT def, LPCWSTR) { return (UINT)def; }
inline UINT GetPrivateProfileIntA(LPCSTR, LPCSTR, INT def, LPCSTR) { return (UINT)def; }
inline BOOL WritePrivateProfileStringW(LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR) { return FALSE; }
inline DWORD GetPrivateProfileStringA(LPCSTR, LPCSTR key, LPCSTR def, LPSTR buf, DWORD n, LPCSTR) { if (!buf || n == 0) return 0; strncpy(buf, def ? def : "", n - 1); buf[n - 1] = 0; return (DWORD)strlen(buf); }
inline DWORD GetPrivateProfileStringW(LPCWSTR, LPCWSTR key, LPCWSTR def, LPWSTR buf, DWORD n, LPCWSTR) {
    if (buf && n > 0 && def) wcsncpy(buf, def, n-1), buf[n-1]=L'\0';
    return def ? (DWORD)wcslen(def) : 0;
}
#define GetPrivateProfileInt    GetPrivateProfileIntA
#define WritePrivateProfileString WritePrivateProfileStringW
#define GetPrivateProfileString GetPrivateProfileStringA

// ── DATA_BLOB (DPAPI) stub ────────────────────────────────────────────────
struct DATA_BLOB { DWORD cbData; BYTE* pbData; };

// ── CreateDirectory → POSIX mkdir ────────────────────────────────────────
#include <sys/stat.h>
inline BOOL CreateDirectoryA(LPCSTR path, void*) { if (!path) return FALSE; return (::mkdir(path, 0755) == 0 || errno == EEXIST) ? TRUE : FALSE; }
inline BOOL CreateDirectoryW(LPCWSTR path, void*) {
    char cpath[MAX_PATH]; wcstombs(cpath, path, MAX_PATH);
    return (mkdir(cpath, 0755) == 0) ? TRUE : FALSE;
}
#ifndef CreateDirectory
#  define CreateDirectory CreateDirectoryW
#endif

// ── StringCchLength / StringCchLengthA ────────────────────────────────────
inline long StringCchLengthW(const wchar_t* psz, size_t cchMax, size_t* pcch) {
    if (!psz) return 0x80070057L;
    size_t len = wcsnlen(psz, cchMax);
    if (pcch) *pcch = len;
    return 0L;
}
inline long StringCchLengthA(const char* psz, size_t cchMax, size_t* pcch) {
    if (!psz) return 0x80070057L;
    size_t len = strnlen(psz, cchMax);
    if (pcch) *pcch = len;
    return 0L;
}
#define StringCchLength StringCchLengthW

// ── strsafe StringCchPrintf / StringCchVPrintf ────────────────────────────
#include <cstdarg>
inline long StringCchPrintfW(wchar_t* dst, size_t cchDst, const wchar_t* fmt, ...) {
    va_list va;
    va_start(va, fmt);
    int n = vswprintf(dst, cchDst, fmt, va);
    va_end(va);
    if (dst && cchDst > 0) dst[cchDst - 1] = L'\0';
    return (n >= 0) ? 0L : 0x80070057L;
}
#define StringCchPrintf StringCchPrintfW

inline long StringCchVPrintfW(wchar_t* dst, size_t cchDst, const wchar_t* fmt, va_list ap) {
    int n = vswprintf(dst, cchDst, fmt, ap);
    if (dst && cchDst > 0) dst[cchDst - 1] = L'\0';
    return (n >= 0) ? 0L : 0x80070057L;
}
#define StringCchVPrintf StringCchVPrintfW

// ── URLDownloadToFile → stub (no WinInet on Android) ─────────────────────
#define CBGAMEGUARD(...) ((void)0)
inline HRESULT URLDownloadToFile(void*, LPCWSTR, LPCWSTR, DWORD, void*) { return E_FAIL; }

// ── CORECLR / DotNet bridge stubs (Android: no .NET runtime) ─────────────
#define CORECLR_DELEGATE_CALLTYPE
inline void* muLoadClientLibraryHandle()
{
    static void* handle = []() -> void*
    {
        // Expected when bundled in jniLibs and loaded by Android runtime.
        void* loaded = dlopen("libMUnique.Client.Library.so", RTLD_NOW | RTLD_GLOBAL);
        if (!loaded)
        {
            // Fallback name without prefix for custom loaders.
            loaded = dlopen("MUnique.Client.Library.so", RTLD_NOW | RTLD_GLOBAL);
        }
        return loaded;
    }();

    return handle;
}
inline void* munique_client_library_handle = muLoadClientLibraryHandle();
inline void* symLoad(void* libraryHandle, const char* symbolName)
{
    if (!symbolName)
    {
        return nullptr;
    }

    if (libraryHandle)
    {
        return dlsym(libraryHandle, symbolName);
    }

    // Android fallback when managed bridge .so is not present:
    // resolve symbols exported by libmain.so itself.
    return dlsym(RTLD_DEFAULT, symbolName);
}

inline UINT WinExec(LPCSTR, UINT) { return 0; }
inline HANDLE CreateMutex(void*, BOOL, LPCSTR) { return (HANDLE)1; }
inline HWND FindWindow(LPCSTR, LPCSTR) { return nullptr; }
inline DWORD GetModuleFileNameA(HMODULE, LPSTR out, DWORD size) { if (out && size) { out[0]=0; } return 0; }
inline HMODULE LoadLibraryA(LPCSTR) { return nullptr; }
inline HMODULE LoadLibraryW(LPCWSTR) { return nullptr; }
inline FARPROC GetProcAddress(HMODULE, LPCSTR) { return nullptr; }
#define LoadLibrary LoadLibraryA
inline LONG RegCreateKeyEx(HKEY, const char*, DWORD, void*, DWORD, DWORD, void*, HKEY* outKey, DWORD* disp) { if (outKey) *outKey = (HKEY)1; if (disp) *disp = 0; return ERROR_SUCCESS; }
inline LONG RegQueryValueEx(HKEY, const char*, DWORD*, DWORD* type, LPBYTE data, DWORD* size) { if (type) *type = REG_DWORD; if (data && size && *size >= sizeof(DWORD)) { *(DWORD*)data = 0; } return ERROR_SUCCESS; }
inline LONG RegSetValueEx(HKEY, const char*, DWORD, DWORD, const BYTE*, DWORD) { return ERROR_SUCCESS; }
inline LONG RegDeleteKey(HKEY, const char*) { return ERROR_SUCCESS; }
inline LONG RegOpenKeyEx(HKEY, const char*, DWORD, DWORD, HKEY* outKey) { if (outKey) *outKey = (HKEY)1; return ERROR_SUCCESS; }
inline LONG RegDeleteValue(HKEY, const char*) { return ERROR_SUCCESS; }
inline LONG RegCloseKey(HKEY) { return ERROR_SUCCESS; }
inline BOOL SetProcessWorkingSetSize(HANDLE, SIZE_T, SIZE_T) { return TRUE; }
inline BOOL SetThreadPriority(HANDLE, int) { return TRUE; }
inline SHORT GetKeyState(int vkey) { return (HIBYTE(GetAsyncKeyState(vkey)) & 0x80) ? (SHORT)0x8000 : 0; }
inline HANDLE CreateThread(void*, SIZE_T, LPTHREAD_START_ROUTINE start, void* param, DWORD, DWORD* threadId) { unsigned id = 0; uintptr_t handle = _beginthreadex(nullptr, 0, reinterpret_cast<unsigned(__stdcall*)(void*)>(start), param, 0, &id); if (threadId) *threadId = id; return reinterpret_cast<HANDLE>(handle); }
namespace SEASON3B
{
    int TextDraw(HFONT font, int PosX, int PosY, DWORD color, DWORD bkcolor, int Width, int Height, BYTE Align, LPCTSTR Text, ...);
}
inline int TextDraw(HFONT font, int PosX, int PosY, DWORD color, DWORD bkcolor, int Width, int Height, int Align, const char* Text, ...)
{
    if (Text == nullptr)
    {
        return 0;
    }

    char buffer[1024] = {};
    va_list args;
    va_start(args, Text);
    vsnprintf(buffer, sizeof(buffer), Text, args);
    va_end(args);

    return SEASON3B::TextDraw(font, PosX, PosY, color, bkcolor, Width, Height, static_cast<BYTE>(Align), "%s", buffer);
}
inline BOOL LoadWaveFile(const char*, int = 0) { return TRUE; }
inline BOOL LoadWaveFile(int, const char*, int = 0, bool = false) { return TRUE; }
inline int CreateMessageBox(const char*, ...) { return 0; }
inline int CreateMessageBox(int, const char*, ...) { return 0; }

inline void AndroidNormalizePathA(const char* path, char* normalized, size_t normalizedSize)
{
    if (!normalized || normalizedSize == 0) {
        return;
    }

    normalized[0] = '\0';
    if (!path) {
        return;
    }

    strncpy(normalized, path, normalizedSize - 1);
    normalized[normalizedSize - 1] = '\0';
    for (char* p = normalized; *p; ++p) {
        if (*p == '\\') {
            *p = '/';
        }
    }
}

inline bool AndroidAppendPathComponent(char* path, size_t pathSize, const char* component)
{
    if (!path || !component || pathSize == 0) {
        return false;
    }

    const size_t pathLen = strlen(path);
    const size_t componentLen = strlen(component);
    const bool needsSlash = pathLen > 0 && strcmp(path, "/") != 0;
    const size_t required = pathLen + (needsSlash ? 1 : 0) + componentLen + 1;
    if (required > pathSize) {
        return false;
    }

    if (needsSlash) {
        strcat(path, "/");
    }
    strcat(path, component);
    return true;
}

inline bool AndroidResolveExistingPathA(const char* path, char* resolved, size_t resolvedSize)
{
    if (!path || !resolved || resolvedSize == 0) {
        return false;
    }

    char normalized[MAX_PATH];
    AndroidNormalizePathA(path, normalized, sizeof(normalized));
    if (normalized[0] == '\0') {
        return false;
    }

    resolved[0] = '\0';
    const char* segment = normalized;
    if (*segment == '/') {
        strncpy(resolved, "/", resolvedSize - 1);
        resolved[resolvedSize - 1] = '\0';
        while (*segment == '/') {
            ++segment;
        }
    }

    while (*segment) {
        while (*segment == '/') {
            ++segment;
        }
        if (!*segment) {
            break;
        }

        const char* slash = strchr(segment, '/');
        const size_t segmentLen = slash ? static_cast<size_t>(slash - segment) : strlen(segment);
        if (segmentLen == 0 || segmentLen >= 256) {
            return false;
        }

        char component[256];
        strncpy(component, segment, segmentLen);
        component[segmentLen] = '\0';

        char direct[MAX_PATH];
        direct[0] = '\0';
        if (resolved[0]) {
            strncpy(direct, resolved, sizeof(direct) - 1);
            direct[sizeof(direct) - 1] = '\0';
        }
        if (!AndroidAppendPathComponent(direct, sizeof(direct), component)) {
            return false;
        }

        struct stat st_buf;
        if (stat(direct, &st_buf) == 0) {
            strncpy(resolved, direct, resolvedSize - 1);
            resolved[resolvedSize - 1] = '\0';
            segment += segmentLen + (slash ? 1 : 0);
            continue;
        }

        const char* directoryPath = resolved[0] ? resolved : ".";
        DIR* directory = opendir(directoryPath);
        if (!directory) {
            return false;
        }

        bool found = false;
        char componentLower[256];
        strncpy(componentLower, component, sizeof(componentLower) - 1);
        componentLower[sizeof(componentLower) - 1] = '\0';
        for (char* q = componentLower; *q; ++q) {
            *q = static_cast<char>(tolower(static_cast<unsigned char>(*q)));
        }

        struct dirent* entry;
        while ((entry = readdir(directory)) != nullptr) {
            char entryLower[256];
            strncpy(entryLower, entry->d_name, sizeof(entryLower) - 1);
            entryLower[sizeof(entryLower) - 1] = '\0';
            for (char* q = entryLower; *q; ++q) {
                *q = static_cast<char>(tolower(static_cast<unsigned char>(*q)));
            }
            if (strcmp(entryLower, componentLower) == 0) {
                if (!AndroidAppendPathComponent(resolved, resolvedSize, entry->d_name)) {
                    closedir(directory);
                    return false;
                }
                found = true;
                break;
            }
        }
        closedir(directory);

        if (!found) {
            return false;
        }

        segment += segmentLen + (slash ? 1 : 0);
    }

    return resolved[0] != '\0';
}

inline FILE* AndroidFopen(const char* path, const char* mode)
{
    char normalized[MAX_PATH];
    AndroidNormalizePathA(path, normalized, sizeof(normalized));

    FILE* file = std::fopen(normalized[0] ? normalized : path, mode);
    if (file || !mode || mode[0] != 'r') {
        return file;
    }

    char resolved[MAX_PATH];
    if (AndroidResolveExistingPathA(normalized, resolved, sizeof(resolved))) {
        file = std::fopen(resolved, mode);
    }
    return file;
}

#ifndef MU_ANDROID_NO_FOPEN_WRAP
#define fopen(path, mode) AndroidFopen((path), (mode))
#endif
#endif // __ANDROID__
















