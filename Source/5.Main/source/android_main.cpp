// =============================================================================
// android_main.cpp
// sokol_app entry point for Android â€” replaces Winmain.cpp on Android platform.
//
// Mapping tá»« Windows â†’ Android:
//   WinMain()              â†’ sokol_main()/sapp callbacks
//   CreateWindow / WGL     â†’ sokol_app EGL/GLES3 context
//   WndProc / PeekMessage  â†’ sapp_event callbacks
//   wzAudio + DirectSound  â†’ SDL_mixer
//   SetTimer()             â†’ SDL_AddTimer / std::thread
//   wglSwapBuffers()       â†’ sokol_app frame present
//   HWND/HDC/HGLRC         â†’ nullptr stubs (PlatformDefs.h)
// =============================================================================

#ifdef __ANDROID__

#include "stdafx.h"
#include "MobilePlatform.h"
#include "GameConfigConstants.h"
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#define TAKUMI_ANDROID_MAIN_UNDEF_MINMAX 1

#include <unistd.h>
#include <fcntl.h>

// Android: game data is under /Android/data/<applicationId>/files (see PreloadActivity).
// applicationId may differ from the Java namespace (e.g. flavor `preloadDatafresh` → `.dataredl`).
// Resolve at runtime from /proc/self/cmdline (first NUL-terminated field is typically the package name).
static char g_androidHostPackage[256];

static void EnsureAndroidHostPackageInitialized()
{
    if (g_androidHostPackage[0] != '\0')
    {
        return;
    }

    const char* fallback = "com.muonline.client";
    char buf[384];
    std::memset(buf, 0, sizeof(buf));
    const int fd = open("/proc/self/cmdline", O_RDONLY | O_CLOEXEC);
    if (fd < 0)
    {
        std::strncpy(g_androidHostPackage, fallback, sizeof(g_androidHostPackage) - 1);
        g_androidHostPackage[sizeof(g_androidHostPackage) - 1] = '\0';
        return;
    }

    const ssize_t n = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    if (n <= 0)
    {
        std::strncpy(g_androidHostPackage, fallback, sizeof(g_androidHostPackage) - 1);
        g_androidHostPackage[sizeof(g_androidHostPackage) - 1] = '\0';
        return;
    }

    buf[n] = '\0';
    size_t i = 0;
    while (i < static_cast<size_t>(n) && buf[i] != '\0' && i + 1 < sizeof(g_androidHostPackage))
    {
        g_androidHostPackage[i] = buf[i];
        ++i;
    }
    g_androidHostPackage[i] = '\0';

    if (i == 0 || std::strncmp(g_androidHostPackage, "com.", 4) != 0)
    {
        std::strncpy(g_androidHostPackage, fallback, sizeof(g_androidHostPackage) - 1);
        g_androidHostPackage[sizeof(g_androidHostPackage) - 1] = '\0';
    }
}

// â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
// Set working directory BEFORE any C++ global constructors run.
// Priority 101 = runs before default C++ constructors (priority 65535).
// Game data lives in the app's external files dir on sdcard so it can be
// pushed via adb push without root access.
// â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
#if defined(__ANDROID__)
__attribute__((constructor(101)))
static void android_set_data_dir_early()
{
    EnsureAndroidHostPackageInitialized();

    char path[512];
    std::snprintf(
        path,
        sizeof(path),
        "/sdcard/Android/data/%s/files",
        g_androidHostPackage);
    int r1 = chdir(path);
#if !defined(MU_ANDROID_DISABLE_LOG)
    __android_log_print(
        ANDROID_LOG_INFO,
        "MuMain",
        "early chdir(%s) = %d (errno=%d) pkg=%s",
        path,
        r1,
        errno,
        g_androidHostPackage);
#endif
    if (r1 == 0)
    {
        return;
    }

    std::snprintf(
        path,
        sizeof(path),
        "/storage/emulated/0/Android/data/%s/files",
        g_androidHostPackage);
    r1 = chdir(path);
#if !defined(MU_ANDROID_DISABLE_LOG)
    __android_log_print(ANDROID_LOG_INFO, "MuMain", "early chdir(%s) = %d (errno=%d)", path, r1, errno);
#endif
    if (r1 == 0)
    {
        return;
    }

    std::snprintf(path, sizeof(path), "/data/user/0/%s/files", g_androidHostPackage);
    r1 = chdir(path);
#if !defined(MU_ANDROID_DISABLE_LOG)
    __android_log_print(ANDROID_LOG_INFO, "MuMain", "early chdir(%s) = %d (errno=%d)", path, r1, errno);
#endif
    if (r1 == 0)
    {
        return;
    }

    std::snprintf(path, sizeof(path), "/data/data/%s/files", g_androidHostPackage);
    const int r2 = chdir(path);
#if !defined(MU_ANDROID_DISABLE_LOG)
    __android_log_print(ANDROID_LOG_INFO, "MuMain", "fallback chdir(%s) = %d (errno=%d)", path, r2, errno);
#endif
}
#endif

#include <SDL.h>
#include <SDL_mixer.h>
#include <sokol_app.h>
#include <android/input.h>
#include <android/log.h>
#include <android/keycodes.h>
#include <jni.h>
#include <sys/system_properties.h>
#include <algorithm>
#include <array>
#include <cctype>
#include <cstdarg>
#include <cmath>
#include <cstring>
#include <fstream>
#include <filesystem>
#include <memory>
#include <mutex>
#include <limits>
#include <string>
#include <string_view>
#include <thread>
#include <chrono>
#include <unordered_map>
#include <vector>

// Game systems
#include "GameConfig/GameConfig.h"
#include "GameConfig/MuLanDefaults.h"
#include "ZzzOpenglUtil.h"
#include "ZzzTexture.h"
#include "ZzzOpenData.h"
#include "ZzzScene.h"
#include "ScenePerfTelemetry.h"
#include "WSclient.h"

#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzAI.h"
#include "ZzzCharacter.h"
#include "ZzzEffect.h"
#include "CharacterManager.h"
#include "SkillManager.h"
#include "ZzzInterface.h"
#include "ZzzInventory.h"
#include "ZzzLodTerrain.h"
#include "NewUIMainFrameWindow.h"
#include "NewUIMyInventory.h"
#include "NewUIInventoryCtrl.h"
#include "NewUISystem.h"
#include "NewUIMessageBox.h"
#include "Translation/i18n.h"
#include "Time/Timer.h"
#include "UIMng.h"
#include "UIManager.h"
#include "UIMapName.h"
#include "_enum.h"

float GetAdaptiveEffectSpawnScale();
bool ShouldThrottleAdaptiveEffectSpawn(int kind, int type, vec3_t Position, int SubType, float Scale, OBJECT* Owner);
#include "w_MapHeaders.h"
#include "w_PetProcess.h"
#include "Input.h"
#include "NewUIMuHelper.h"
#include "Platform/AndroidGDI.h"
#include "Platform/RenderBackend.h"
#include "Platform/gl_compat.h"
#include "android/AndroidNetwork.h"
#include "android/SimpleModulusCrypt.h"

// stb_image â€” implementation is in android_turbojpeg_stubs.cpp; only declare here.
#include "stb_image.h"

#define LOG_TAG "MuMain"
#if defined(MU_ANDROID_DISABLE_LOG)
#define LOGI(...) ((void)0)
#define LOGE(...) ((void)0)
#define LOGW(...) ((void)0)
#else
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,  LOG_TAG, __VA_ARGS__)
#endif

#if defined(MU_ANDROID_PERF_LOG)
static void PerfLogInfo(const char* fmt, ...)
{
    if (fmt == nullptr || fmt[0] == '\0')
    {
        return;
    }

    char buffer[1536];
    va_list args;
    va_start(args, fmt);
    std::vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    buffer[sizeof(buffer) - 1] = '\0';
    __android_log_write(ANDROID_LOG_INFO, LOG_TAG, buffer);
}
#define PERF_LOGI(...) PerfLogInfo(__VA_ARGS__)
#else
#define PERF_LOGI(...) ((void)0)
#endif

extern float g_fScreenRate_x;
extern float g_fScreenRate_y;
extern int DisplayWinCDepthBox;
extern int DisplayWin;
extern int DisplayHeight;
extern int DisplayWinMid;
extern int DisplayWinExt;
extern int DisplayHeightExt;
extern int DisplayWinReal;
extern BOOL g_bGameServerConnected;
extern BYTE g_byPacketSerialSend;
extern bool First;
extern int FirstTime;
extern char* g_lpszMp3[NUM_MUSIC];

namespace
{
constexpr Sint32 kSdlUserEventLoginIntroMovieFinished = 9001;
}

static int g_AndroidSafeInsetBottomPx = 56;

static void UpdateAndroidScreenMetrics(int screenW, int screenH)
{
    WindowWidth = static_cast<unsigned int>(screenW);
    WindowHeight = static_cast<unsigned int>(screenH);
    g_fScreenRate_x = static_cast<float>(WindowWidth) / 640.0f;
    g_fScreenRate_y = static_cast<float>(WindowHeight) / 480.0f;
    // Reserve nav bar + gesture area (devices often report 63–130px bottom inset).
    g_AndroidSafeInsetBottomPx = std::clamp(
        static_cast<int>(static_cast<float>(screenH) * 0.058f),
        48,
        140);

    DisplayWin = 640;
    DisplayHeight = 480;
    DisplayWinMid = 320;
    DisplayWinExt = 0;
    DisplayWinReal = 640;
    DisplayWinCDepthBox = 0;
    DisplayHeightExt = 0;
}

static std::string ReadAndroidSystemProperty(const char* key)
{
    if (!key || !key[0])
    {
        return {};
    }

    char value[PROP_VALUE_MAX] = {};
    const int length = __system_property_get(key, value);
    if (length <= 0)
    {
        return {};
    }

    return std::string(value, static_cast<std::size_t>(length));
}

static void SetWorkingDirectoryToMobileDataRoot()
{
    EnsureAndroidHostPackageInitialized();

    char roots[4][512] = {};
    std::snprintf(
        roots[0],
        sizeof(roots[0]),
        "/sdcard/Android/data/%s/files",
        g_androidHostPackage);
    std::snprintf(
        roots[1],
        sizeof(roots[1]),
        "/storage/emulated/0/Android/data/%s/files",
        g_androidHostPackage);
    std::snprintf(roots[2], sizeof(roots[2]), "/data/user/0/%s/files", g_androidHostPackage);
    std::snprintf(roots[3], sizeof(roots[3]), "/data/data/%s/files", g_androidHostPackage);

    const char* kMobileDataRoots[] = { roots[0], roots[1], roots[2], roots[3] };

    for (const char* workingDir : kMobileDataRoots)
    {
        if ((workingDir != nullptr) && (workingDir[0] != '\0') && (chdir(workingDir) == 0))
        {
            LOGI("Working dir set to: %s", workingDir);
            return;
        }
    }

    LOGW("Failed to resolve a writable working directory for this mobile platform");
}

static void InitializeTakumiProtectState()
{
    static bool initialized = false;
    if (initialized)
    {
        return;
    }

    initialized = true;

    auto applyFallbackMainInfo = []()
    {
        std::memset(&gProtect.m_MainInfo, 0, sizeof(gProtect.m_MainInfo));
        gProtect.m_MainInfo.GSPortMin = MuLanDefaults::kDefaultGameShardPortMin;
        gProtect.m_MainInfo.GSPortMax = 55999;
        std::strcpy(gProtect.m_MainInfo.CustomerName, "takumi12");
        std::strcpy(gProtect.m_MainInfo.IpAddress, CfgDefaults::CfgDefaultServerIpNarrow);
        gProtect.m_MainInfo.IpAddressPort = CfgDefaults::CfgDefaultServerPort;
        std::strcpy(gProtect.m_MainInfo.ClientVersion, "1.04.05");
        std::strcpy(gProtect.m_MainInfo.ClientSerial, "TbYehR2hFUPBKgZj");
        gProtect.LoadEncDec();
        LOGW(
            "Protect fallback active: gsPorts=%u-%u server=%s:%u serial=%s",
            static_cast<unsigned int>(gProtect.m_MainInfo.GSPortMin),
            static_cast<unsigned int>(gProtect.m_MainInfo.GSPortMax),
            gProtect.m_MainInfo.IpAddress,
            static_cast<unsigned int>(gProtect.m_MainInfo.IpAddressPort),
            gProtect.m_MainInfo.ClientSerial);
    };

#if defined(__ANDROID__)
    applyFallbackMainInfo();
    return;
#endif

    std::ifstream file(".\\Data\\Local\\CBGetMain.bin", std::ios::binary);
    if (!file)
    {
        LOGE("Open failed for Data\\Local\\CBGetMain.bin");
        applyFallbackMainInfo();
        return;
    }

    MAIN_FILE_INFO mainInfo {};
    file.read(reinterpret_cast<char*>(&mainInfo), sizeof(mainInfo));
    if (file.gcount() != static_cast<std::streamsize>(sizeof(mainInfo)))
    {
        LOGE(
            "Read failed for Data\\Local\\CBGetMain.bin size=%lld expected=%zu",
            static_cast<long long>(file.gcount()),
            sizeof(mainInfo));
        applyFallbackMainInfo();
        return;
    }

    for (int n = 0; n < static_cast<int>(sizeof(MAIN_FILE_INFO)); ++n)
    {
        reinterpret_cast<BYTE*>(&mainInfo)[n] -= static_cast<BYTE>(0x95 ^ HIBYTE(n));
        reinterpret_cast<BYTE*>(&mainInfo)[n] ^= static_cast<BYTE>(0xCA ^ LOBYTE(n));
    }

    std::memcpy(&gProtect.m_MainInfo, &mainInfo, sizeof(MAIN_FILE_INFO));
    gProtect.LoadEncDec();

    LOGI(
        "Protect loaded: gsPorts=%u-%u server=%s:%u clientVersion=%s serial=%s",
        static_cast<unsigned int>(gProtect.m_MainInfo.GSPortMin),
        static_cast<unsigned int>(gProtect.m_MainInfo.GSPortMax),
        gProtect.m_MainInfo.IpAddress,
        static_cast<unsigned int>(gProtect.m_MainInfo.IpAddressPort),
        gProtect.m_MainInfo.ClientVersion,
        gProtect.m_MainInfo.ClientSerial);
}

extern CSimpleModulus g_SimpleModulusCS;
extern CSimpleModulus g_SimpleModulusSC;

static void InitializeTakumiPacketKeys()
{
    const BOOL encLoaded = g_SimpleModulusCS.LoadEncryptionKey((char*)"Data/Enc1.dat");
    const BOOL decLoaded = g_SimpleModulusSC.LoadDecryptionKey((char*)"Data/Dec2.dat");

    LOGI(
        "SimpleModulus init: enc=%d dec=%d",
        encLoaded ? 1 : 0,
        decLoaded ? 1 : 0);
}

static bool ContainsNoCase(const std::string& haystack, const char* needle)
{
    if (!needle || !needle[0] || haystack.empty())
    {
        return false;
    }

    std::string lowerHaystack(haystack);
    std::transform(
        lowerHaystack.begin(),
        lowerHaystack.end(),
        lowerHaystack.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    std::string lowerNeedle(needle);
    std::transform(
        lowerNeedle.begin(),
        lowerNeedle.end(),
        lowerNeedle.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    return lowerHaystack.find(lowerNeedle) != std::string::npos;
}

static bool IsLikelyAndroidEmulator()
{
    const std::string kernelQemu = ReadAndroidSystemProperty("ro.kernel.qemu");
    if (kernelQemu == "1")
    {
        return true;
    }

    // LDPlayer service marker observed on local test images.
    const std::string ldInitService = ReadAndroidSystemProperty("init.svc.ldinit");
    if (!ldInitService.empty())
    {
        return true;
    }

    const std::string hardware = ReadAndroidSystemProperty("ro.hardware");
    const std::string product = ReadAndroidSystemProperty("ro.product.device");
    const std::string model = ReadAndroidSystemProperty("ro.product.model");
    const std::string manufacturer = ReadAndroidSystemProperty("ro.product.manufacturer");
    const std::string abi = ReadAndroidSystemProperty("ro.product.cpu.abi");

    return ContainsNoCase(hardware, "ranchu")
        || ContainsNoCase(hardware, "goldfish")
        || ContainsNoCase(hardware, "vbox")
        || ContainsNoCase(product, "emulator")
        || ContainsNoCase(product, "simulator")
        || ContainsNoCase(product, "vbox")
        || ContainsNoCase(model, "emulator")
        || ContainsNoCase(model, "ldplayer")
        || ContainsNoCase(model, "android sdk built for")
        || ContainsNoCase(manufacturer, "genymotion")
        || ContainsNoCase(manufacturer, "netease")
        || ContainsNoCase(abi, "x86");
}

// Forward-declare camera variables at file scope so they're accessible
// from the anonymous namespace below.
extern float CameraDistance;
extern float CameraDistanceTarget;
extern float g_androidZoomOverride;   // defined in CameraUtility.cpp
extern float CameraAngle[3];

// =============================================================================
// Globals (defined here on Android â€” in Winmain.cpp on Windows)
// =============================================================================

// Stub handles â€” referenced by existing code but unused on Android
HWND      g_hWnd      = nullptr;
HINSTANCE g_hInst     = nullptr;
HDC       g_hDC       = nullptr;
HGLRC     g_hRC       = nullptr;
HFONT     g_hFont     = nullptr;
HFONT     g_hFontBold = nullptr;
HFONT     g_hFontBig  = nullptr;
HFONT     g_hFixFont  = nullptr;

CTimer*   g_pTimer    = new CTimer();
bool      Destroy     = false;
bool      ActiveIME   = false;
bool      g_bWndActive = true;
static bool g_AndroidGameInitialized = false;
static bool g_AndroidQuitRequested = false;
static int  g_DrawableWidth = 1280;
static int  g_DrawableHeight = 720;

BYTE*             RendomMemoryDump        = nullptr;
ITEM_ATTRIBUTE*   ItemAttRibuteMemoryDump = nullptr;
CHARACTER*        CharacterMemoryDump     = nullptr;

int         RandomTable[100];
CErrorReport g_ErrorReport;

BOOL g_bMinimizedEnabled    = FALSE;
int  g_iScreenSaverOldValue = 0;
BOOL g_bUseWindowMode       = FALSE;   // Always fullscreen on Android
BOOL g_bUseFullscreenMode   = TRUE;

char m_Username[11]  = {};
char m_Password[21]  = {};
char m_Version[11]   = "2.04d";
char m_ExeVersion[11]= "1.00";
int     m_SoundOnOff    = 1;
int     m_MusicOnOff    = 1;
int     m_Resolution    = 0;
int     m_nColorDepth   = 0;
int     m_RememberMe    = 0;

char g_aszMLSelection[MAX_LANGUAGE_NAME_LENGTH] = {};
int     g_iRenderTextType = 0;

char Mp3FileName[256]   = {};
CMultiLanguage* pMultiLanguage  = nullptr;
// g_dwTopWindow defined in UIControls.cpp
CUIManager* g_pUIManager        = nullptr;
CUIMapName* g_pUIMapName        = nullptr;

CUIMercenaryInputBox* g_pMercenaryInputBox  = nullptr;
CUITextInputBox*      g_pSingleTextInputBox  = nullptr;
CUITextInputBox*      g_pSinglePasswdInputBox = nullptr;
int  g_iChatInputType = 1;

// â”€â”€ Custom standalone character-name input (bypasses Android edit-control stub) â”€â”€
bool     g_charNameInputActive = false;
wchar_t  g_charNameBuf[11]     = {};
int      g_charNameLen         = 0;
// g_bIMEBlock defined in UIControls.cpp

int Time_Effect = 0;
bool  ashies      = false;
int   weather     = 0;
double CPU_AVG    = 0.0;
int    g_MaxMessagePerCycle = -1;

int g_iInactiveTime  = 0;
int g_iNoMouseTime   = 0;
int g_iInactiveWarning = 0;
int g_iMousePopPosition_x = 0;
int g_iMousePopPosition_y = 0;

BOOL g_bInactiveTimeChecked = FALSE;

// Symbols defined in Winmain.cpp on Windows â€” stub here on Android
bool g_bEnterPressed = false;
static SDL_FingerID g_primaryTouchFinger = -1;
static bool g_seenFingerInput = false;
static bool g_pendingImeEnterTextInput = false;

// Double-tap tracking â€” replaces Windows WM_LBUTTONDBLCLK on Android
// Record last primary finger-up so next finger-down can detect double-tap.
static uint32_t s_doubleTapLastUpMs = 0;
static float    s_doubleTapLastUpNX = -1.0f;   // normalized 0..1
static float    s_doubleTapLastUpNY = -1.0f;
constexpr uint32_t kDoubleTapMaxMs   = 320;     // max ms between taps
constexpr float    kDoubleTapMaxDist = 0.07f;   // max normalized distance
static std::unique_ptr<IRenderBackend> g_RenderBackend;
extern int ActionTarget;
extern int TargetX;
extern int TargetY;
extern int Attacking;
bool CheckTile(CHARACTER* c, OBJECT* o, float Range);
void SetPlayerAttack(CHARACTER* c);

namespace
{
constexpr bool kUseLegacyMainHud = true;
constexpr int kVirtualAttackButton = 0;
constexpr int kVirtualSkillButtonBase = 1;
constexpr int kVirtualAttackSkillSlot = 0;
constexpr int kVirtualVisibleSkillButtonCount = 4;
constexpr int kVirtualSkillSlotCount = 1 + kVirtualVisibleSkillButtonCount;
constexpr int kVirtualUtilityButtonCount = 4;
constexpr int kVirtualUtilityButtonChat = 3;
constexpr int kVirtualZoomButtonMinus = 0;
constexpr int kVirtualZoomButtonPlus = 1;
constexpr uint32_t kVirtualMiniMapButtonCooldownMs = 220;
constexpr uint32_t kVirtualAttackRepeatMs = 280;
constexpr uint32_t kVirtualProximityAttackMs = 150;
constexpr uint32_t kVirtualMeleeAttackSendMs = 160;
constexpr float kVirtualJoystickCombatMoveStrength = 0.12f;
constexpr uint32_t kVirtualUtilityButtonCooldownMs = 200;
constexpr uint32_t kVirtualSkillAssignLongPressMs = 480;
constexpr uint32_t kVirtualAssignModeTimeoutMs = 9000;
constexpr uint32_t kVirtualAssignTapDebounceMs = 160;
constexpr const char* kVirtualSkillSlotsPath = "Data/Local/android_touch_skill_slots.cfg";
// Bottom of virtual-pad touch/draw zone (extends below legacy action bar so the
// joystick ring is not clipped at the lower corners).
constexpr float kVirtualPadInputMaxY = 468.0f;
constexpr float kInventoryWindowWidth = 190.0f;
constexpr float kInventoryWindowHeight = 429.0f;
constexpr float kVirtualAutoAcquireMaxDistance = 10.0f;
constexpr float kVirtualJoystickDefaultCenterX = 88.0f;
constexpr float kVirtualJoystickDefaultCenterY = 368.0f;
constexpr float kVirtualJoystickHudClearanceUi = 8.0f;
constexpr float kVirtualJoystickRadius = 48.0f;
constexpr float kVirtualJoystickDeadZone = 8.0f;
constexpr float kVirtualJoystickKnobRadius = 18.0f;
constexpr float kVirtualJoystickMouseMinRadius = 32.0f;
constexpr float kVirtualJoystickMouseMaxRadius = 120.0f;
// Invisible touch zone (bottom-left) — wider than the drawn pad for easier grabs.
constexpr float kVirtualJoystickDynamicAreaMinY = 248.0f;
constexpr float kVirtualJoystickDynamicAreaMaxX = 248.0f;
constexpr float kVirtualJoystickOuterRenderW = 88.0f;
constexpr float kVirtualJoystickOuterRenderH = 88.0f;
// Soft glow + ring PNG scale vs outerW/H (touch/hitbox unchanged). Larger = bigger faint circle.
constexpr float kJoystickRingDrawDiameterScale = 1.52f;
// Outer soft circle radius = layoutDiameter/2 * this; ring texture diameter ~= 2 * that (nearly full glow).
constexpr float kJoystickRingSoftOuterRadiusMul = 1.08f;
constexpr float kVirtualJoystickVisualExtentUi =
    kVirtualJoystickOuterRenderW * 0.5f * kJoystickRingDrawDiameterScale * kJoystickRingSoftOuterRadiusMul * 1.06f;
constexpr float kJoystickKnobDrawDiameterScale = 1.18f;
constexpr float kVirtualJoystickKnobRenderW = 40.0f;
constexpr float kVirtualJoystickKnobRenderH = 40.0f;
constexpr float kVirtualJoystickIdleVisualScale = 0.68f;
constexpr float kVirtualJoystickIdleRingAlpha = 0.30f;
constexpr float kVirtualJoystickIdleKnobAlpha = 0.24f;
constexpr float kVirtualJoystickActiveRingAlpha = 0.70f;
constexpr float kVirtualJoystickActiveKnobAlpha = 0.86f;
constexpr SDL_FingerID kPcMouseJoystickFingerId = static_cast<SDL_FingerID>(-2);
constexpr bool kShowVirtualAttackButton = !kUseLegacyMainHud;
constexpr bool kShowVirtualSkillButtons = !kUseLegacyMainHud;

struct VirtualButtonLayout
{
    float cx;
    float cy;
    float radius;
};

constexpr float kVirtualAttackButtonCx = 588.0f;
constexpr float kVirtualAttackButtonCy = 396.0f;
constexpr float kVirtualAttackButtonRadius = 27.0f;
constexpr float kVirtualSkillButtonRadius = 14.5f;
struct VirtualUiOffset
{
    float x;
    float y;
};
constexpr std::array<VirtualUiOffset, kVirtualVisibleSkillButtonCount> kVirtualSkillCenters = {
    // Four compact skill slots orbit the attack button, evenly spaced like the
    // reference mobile cluster and kept away from the right screen edge.
    VirtualUiOffset{ 544.0f, 426.0f },
    VirtualUiOffset{ 544.0f, 366.0f },
    VirtualUiOffset{ 600.0f, 342.0f },
    VirtualUiOffset{ 634.0f, 396.0f },
};

const std::array<VirtualButtonLayout, 1 + kVirtualVisibleSkillButtonCount> kVirtualButtons = []()
{
    std::array<VirtualButtonLayout, 1 + kVirtualVisibleSkillButtonCount> buttons{};
    buttons[kVirtualAttackButton] = {
        kVirtualAttackButtonCx,
        kVirtualAttackButtonCy,
        kVirtualAttackButtonRadius
    };

    for (int i = 0; i < kVirtualVisibleSkillButtonCount; ++i)
    {
        buttons[kVirtualSkillButtonBase + i] = {
            kVirtualSkillCenters[i].x,
            kVirtualSkillCenters[i].y,
            kVirtualSkillButtonRadius
        };
    }

    return buttons;
}();

// â”€â”€ Consumable potion slots (restored stable layout) â”€â”€
constexpr int kVirtualConsumableSlotCount = 3;
constexpr std::array<VirtualButtonLayout, kVirtualConsumableSlotCount> kVirtualConsumableSlots = {
    VirtualButtonLayout{ 472.0f, 334.0f, 15.0f }, // consumable slot 0
    VirtualButtonLayout{ 472.0f, 388.0f, 15.0f }, // consumable slot 1
    VirtualButtonLayout{ 472.0f, 442.0f, 15.0f }, // consumable slot 2
};

struct VirtualConsumableSlot {
    int itemType  = -1;   // -1 = empty
    int itemLevel = 0;
};
std::array<VirtualConsumableSlot, kVirtualConsumableSlotCount> g_virtualConsumableSlots{};

constexpr float kTopRightButtonY = 12.0f;
constexpr float kTopRightButtonSize = 37.0f; // +3 px for all top-right icon buttons
constexpr float kTopRightButtonGap = 6.0f;
constexpr float kTopRightButtonMarginRight = 8.0f;
constexpr float kTopRightPanelGap = 8.0f;
constexpr float kCompactMiniMapPanelWidth = 86.0f;
constexpr float kCompactMiniMapPanelHeight = 86.0f;
constexpr float kCompactMiniMapPanelGapToIcons = 10.0f;
constexpr float kVirtualChatQuickButtonCx = 305.0f;
constexpr float kVirtualChatQuickButtonCy = 413.0f;
constexpr float kVirtualChatQuickButtonRadius = 18.0f;

// â”€â”€ UI icon texture cache (loaded once from assets/ui/*.png) â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
struct UITexture
{
    GLuint id = 0;
    int w = 0;
    int h = 0;
    // Alpha-weighted center in GL texcoords (0..1); used so art center lands on draw anchor.
    float contentCenterU = 0.5f;
    float contentCenterV = 0.5f;
    // Max visible span vs quad width on aspect-correct draw (bbox/w); <1 => upscale to hit target diameter.
    float contentVisibleFrac = 1.0f;
};
static UITexture g_uiTex_map;
static UITexture g_uiTex_minimap;
static UITexture g_uiTex_attack;
static UITexture g_uiTex_skillbox;
static UITexture g_uiTex_skillline;
static UITexture g_uiTex_joystick1;
static UITexture g_uiTex_joystick2;
static UITexture g_uiTex_balo;
static UITexture g_uiTex_character;
static UITexture g_uiTex_setting;
static bool g_uiTexturesLoaded = false;

constexpr float kSkillLineU = 157.0f / 677.0f;
constexpr float kSkillLineV = 3.0f / 369.0f;
constexpr float kSkillLineUW = 363.0f / 677.0f;
constexpr float kSkillLineVH = 363.0f / 369.0f;
// joystick1.png / joystick2.png are standalone full textures (see LoadUITextureAsset).
// Draw with a pixel quad matching tex w:h so full UV (0,0)-(1,1) is not squashed (avoids
// oval distortion and avoids center-UV crops that can clip one side if art is not centered).

struct AndroidUiRect
{
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;
};

AndroidUiRect GetTopRightStackButtonRect(int stackIndex)
{
    return {
        640.0f - kTopRightButtonMarginRight - kTopRightButtonSize,
        kTopRightButtonY + static_cast<float>(stackIndex) * (kTopRightButtonSize + kTopRightButtonGap),
        kTopRightButtonSize,
        kTopRightButtonSize
    };
}

bool IsMiniMapPanelVisible()
{
    return g_pNewUISystem != nullptr
        && g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MINI_MAP);
}

AndroidUiRect GetMiniMapButtonRect()
{
    return GetTopRightStackButtonRect(0);
}

AndroidUiRect GetMapButtonRect()
{
    return GetTopRightStackButtonRect(1);
}

AndroidUiRect GetCompactMiniMapRect()
{
    const AndroidUiRect mapButton = GetMapButtonRect();
    const float x = std::clamp(
        mapButton.x - kCompactMiniMapPanelWidth - kCompactMiniMapPanelGapToIcons,
        6.0f,
        640.0f - kCompactMiniMapPanelWidth - 6.0f);
    const float y = std::clamp(
        mapButton.y + 2.0f,
        6.0f,
        480.0f - kCompactMiniMapPanelHeight - 6.0f);

    return { x, y, kCompactMiniMapPanelWidth, kCompactMiniMapPanelHeight };
}

AndroidUiRect GetVirtualUtilityButtonRect(int button)
{
    if (button < 0 || button >= kVirtualUtilityButtonCount)
    {
        return {};
    }

    if (button == kVirtualUtilityButtonChat)
    {
        return {
            kVirtualChatQuickButtonCx - kVirtualChatQuickButtonRadius,
            kVirtualChatQuickButtonCy - kVirtualChatQuickButtonRadius,
            kVirtualChatQuickButtonRadius * 2.0f,
            kVirtualChatQuickButtonRadius * 2.0f
        };
    }

    return GetTopRightStackButtonRect(2 + button);
}

float GetAndroidCompactMiniMapTopYInternal()
{
    return GetCompactMiniMapRect().y;
}

float GetAndroidCompactMiniMapLeftXInternal()
{
    return GetCompactMiniMapRect().x;
}

bool GetAndroidMoveMapWindowPositionInternal(int panelWidth, int panelHeight, int* outX, int* outY)
{
    if (outX == nullptr || outY == nullptr)
    {
        return false;
    }

    const int marginX = 12;
    const int topReserved = 72;
    const int bottomReserved = 66;
    const int preferredY = 78;
    const int centeredX = (640 - panelWidth) / 2;
    const int maxX = std::max(marginX, 640 - panelWidth - marginX);
    const int minY = topReserved;
    const int maxY = std::max(minY, 480 - panelHeight - bottomReserved);

    *outX = std::clamp(centeredX, marginX, maxX);
    *outY = std::clamp(preferredY, minY, maxY);
    return true;
}

// â”€â”€ Bottom HUD â€” HP/MP/AG/EXP bars (1/3 screen width, numbers on bar) â”€â”€â”€â”€â”€â”€â”€
// Bars occupy the left 1/3 of the screen (â‰ˆ213 virtual units), stacked at bottom.
// Numbers are drawn centered directly on each bar. No blending â€” solid opaque.
constexpr float kHudStripY      = 432.0f;   // top of the HUD strip (virtual y)
constexpr float kLegacyMainFrameBarTopY = 480.0f - 51.0f; // CNewUIMainFrameWindow::RenderFrame
constexpr float kHudBarH        =  11.0f;   // height of one bar (tall enough for numbers)
constexpr float kHudBarGap      =   2.0f;   // vertical gap between bars
constexpr float kHudBarLeft     =   4.0f;   // bar left edge (virtual x)
constexpr float kHudBarRight    = 218.0f;   // bar right edge  (â‰ˆ 1/3 of 640)
// Number center X = center of the bar
constexpr float kHudNumCenterX  = (kHudBarLeft + kHudBarRight) * 0.5f;  // â‰ˆ 111

// Disabled: the original MU mainframe skill box is back. Keep only joystick
// custom on mobile and do not draw an extra Android-only skill box on top.
constexpr bool kShowVirtualCurrentSkillBox = false;
constexpr float kVirtualCurrentSkillBoxX = kHudBarRight + 4.0f;
constexpr float kVirtualCurrentSkillBoxY = 431.0f;
constexpr float kVirtualCurrentSkillBoxW = 32.0f;
constexpr float kVirtualCurrentSkillBoxH = 38.0f;
constexpr float kVirtualHudChatBoxX = 226.0f;
constexpr float kVirtualHudChatBoxY = 400.0f;
constexpr float kVirtualHudChatBoxW = 170.0f;
constexpr float kVirtualHudChatBoxH = 52.0f;

// â”€â”€ Zoom +/- buttons: top-center beside the level badge â”€â”€
constexpr float kZoomButtonW = 44.0f;
constexpr float kZoomButtonH = 28.0f;
constexpr float kZoomButtonY = 28.0f;
constexpr float kZoomButtonGap = 8.0f;
constexpr float kZoomAnchorCenterX = 318.0f;
constexpr float kZoomMinusX = kZoomAnchorCenterX - kZoomButtonGap * 0.5f - kZoomButtonW;
constexpr float kZoomPlusX  = kZoomAnchorCenterX + kZoomButtonGap * 0.5f;
constexpr float kZoomStep = 100.0f;   // distance per tap
constexpr float kZoomMin  = 800.0f;
constexpr float kZoomMax  = 1600.0f;
constexpr uint32_t kZoomCooldownMs = 180;
constexpr float kMainFrameItemHotKeyX = 0.0f;
constexpr float kMainFrameItemHotKeyY = 430.0f;
constexpr float kMainFrameItemHotKeyW = 38.0f;
constexpr float kMainFrameItemHotKeyH = 38.0f;

struct ActiveVirtualTouch
{
    SDL_FingerID fingerId = static_cast<SDL_FingerID>(-1);
    int button = -1;
    uint32_t downMs = 0;
    uint32_t lastRepeatMs = 0;
};

struct ActiveVirtualJoystick
{
    SDL_FingerID fingerId = static_cast<SDL_FingerID>(-1);
    float centerX = kVirtualJoystickDefaultCenterX;
    float centerY = kVirtualJoystickDefaultCenterY;
    float thumbOffsetX = 0.0f;
    float thumbOffsetY = 0.0f;
    float moveDirX = 0.0f;
    float moveDirY = 0.0f;
    float moveStrength = 0.0f;
};

struct ActiveVirtualPickerTouch
{
    SDL_FingerID fingerId = static_cast<SDL_FingerID>(-1);
    int skillIndex = -1;
};

std::array<ActiveVirtualTouch, 4> g_activeVirtualTouches{};
std::array<int, kVirtualSkillSlotCount> g_virtualSkillSlots = []()
{
    std::array<int, kVirtualSkillSlotCount> slots{};
    slots.fill(-1);
    return slots;
}();
std::array<int, kVirtualSkillSlotCount> g_virtualSkillTypes{};
ActiveVirtualJoystick g_virtualJoystick{};
ActiveVirtualPickerTouch g_virtualPickerTouch{};
bool g_virtualJoystickDrivingMouse = false;
bool g_joystickPcMouseCaptured = false;
bool g_virtualSkillSlotsLoaded = false;
bool g_virtualSkillSlotsDirty = false;
bool g_virtualAssignModeActive = false;
int g_virtualAssignSkillIndex = -1;
uint32_t g_virtualAssignModeUntilMs = 0;
bool g_virtualLastSkillPickerOpen = false;
int g_virtualAssignPickerSkillIndex = -1;
bool g_virtualAssignConsumedForPickerSkill = false;
bool g_virtualAssignConsumedForPickerSession = false;
uint32_t g_virtualLastAssignTapMs = 0;
uint32_t g_virtualLastMiniMapTapMs = 0;
uint32_t g_virtualLastZoomTapMs = 0;
uint32_t g_virtualLastUtilityTapMs = 0;
bool g_virtualHudChatPinned = false;

int GetVirtualHotKeyBySlot(int slot)
{
    switch (slot)
    {
    case 1: return SEASON3B::HOTKEY_Q;
    case 2: return SEASON3B::HOTKEY_W;
    case 3: return SEASON3B::HOTKEY_E;
    case 4: return SEASON3B::HOTKEY_R;
    default: return SEASON3B::HOTKEY_Q;
    }
}

float GetVirtualButtonHitRadius(int button)
{
    if (button < 0 || button >= static_cast<int>(kVirtualButtons.size()))
    {
        return 0.0f;
    }

    const VirtualButtonLayout& layout = kVirtualButtons[button];
    if (button == kVirtualAttackButton)
    {
        return layout.radius + 5.0f;
    }

    // Keep the radial skill slots easy to tap without letting neighboring slots
    // steal input from each other.
    return layout.radius + 9.0f;
}

bool IsWithinVirtualAutoAcquireRange(int characterIndex)
{
    if (characterIndex < 0
        || characterIndex >= MAX_CHARACTERS_CLIENT
        || Hero == nullptr
        || CharactersClient == nullptr)
    {
        return false;
    }

    const CHARACTER* c = &CharactersClient[characterIndex];
    const float dx = static_cast<float>(c->PositionX - Hero->PositionX);
    const float dy = static_cast<float>(c->PositionY - Hero->PositionY);
    const float dist2 = (dx * dx) + (dy * dy);
    const float maxDist = kVirtualAutoAcquireMaxDistance;
    return dist2 <= (maxDist * maxDist);
}

bool IsVirtualPadAvailable()
{
    return SceneFlag == MAIN_SCENE
        && Hero != nullptr
        && CharacterAttribute != nullptr
        && g_pMainFrame != nullptr
        && !AndroidHasFocusedTextInput();
}

void SyncVirtualHudChatBox()
{
    if (g_pNewUISystem == nullptr || g_pChatInputBox == nullptr)
    {
        return;
    }

    if (SceneFlag != MAIN_SCENE)
    {
        return;
    }

    g_pChatInputBox->SetWndPos(
        static_cast<int>(std::lround(kVirtualHudChatBoxX)),
        static_cast<int>(std::lround(kVirtualHudChatBoxY)));

    if (!g_virtualHudChatPinned)
    {
        return;
    }

    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHATINPUTBOX))
    {
        return;
    }

    // Keep compact chat panel visible in HUD without stealing gameplay focus.
    g_pNewUISystem->Show(SEASON3B::INTERFACE_CHATINPUTBOX);
    SetFocus(g_hWnd ? g_hWnd : reinterpret_cast<HWND>(0x1));
}

bool HitTestVirtualHudChatBox(float uiX, float uiY)
{
    if (SceneFlag != MAIN_SCENE
        || g_pNewUISystem == nullptr
        || !g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHATINPUTBOX))
    {
        return false;
    }

    return uiX >= kVirtualHudChatBoxX
        && uiX <= (kVirtualHudChatBoxX + kVirtualHudChatBoxW)
        && uiY >= kVirtualHudChatBoxY
        && uiY <= (kVirtualHudChatBoxY + kVirtualHudChatBoxH);
}

bool FocusVirtualChatInputAt(float uiX, float uiY)
{
    if (SceneFlag != MAIN_SCENE
        || g_pNewUISystem == nullptr
        || g_pChatInputBox == nullptr
        || !g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHATINPUTBOX))
    {
        return false;
    }

    auto hitInput = [uiX, uiY](CUITextInputBox* input)
    {
        return input != nullptr
            && input->GetState() == UISTATE_NORMAL
            && uiX >= input->GetPosition_x()
            && uiX <= (input->GetPosition_x() + input->GetWidth())
            && uiY >= input->GetPosition_y()
            && uiY <= (input->GetPosition_y() + input->GetHeight());
    };

    if (hitInput(g_pChatInputBox->m_pWhsprIDInputBox))
    {
        g_pChatInputBox->m_pWhsprIDInputBox->GiveFocus(TRUE);
        return true;
    }

    if (hitInput(g_pChatInputBox->m_pChatInputBox))
    {
        g_pChatInputBox->m_pChatInputBox->GiveFocus(FALSE);
        return true;
    }

    if (HitTestVirtualHudChatBox(uiX, uiY) && g_pChatInputBox->m_pChatInputBox != nullptr)
    {
        g_pChatInputBox->m_pChatInputBox->GiveFocus(FALSE);
        return true;
    }

    return false;
}

void ToggleVirtualChatInputBox()
{
    if (g_pNewUISystem == nullptr || g_pChatInputBox == nullptr)
    {
        return;
    }

    const bool isVisible = g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHATINPUTBOX);

    if (!isVisible)
    {
        g_pNewUISystem->Show(SEASON3B::INTERFACE_CHATINPUTBOX);
        if (g_pChatInputBox->m_pChatInputBox != nullptr)
        {
            g_pChatInputBox->m_pChatInputBox->GiveFocus(TRUE);
        }
        return;
    }

    if (g_pChatInputBox->HaveFocus())
    {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_CHATINPUTBOX);
        return;
    }

    if (g_pChatInputBox->m_pChatInputBox != nullptr)
    {
        g_pChatInputBox->m_pChatInputBox->GiveFocus(FALSE);
    }
}

bool IsValidSkillIndex(int skillIndex)
{
    if (CharacterAttribute == nullptr)
    {
        return false;
    }

    if (skillIndex < 0 || skillIndex >= MAX_MAGIC)
    {
        return false;
    }

    const int skillType = CharacterAttribute->Skill[skillIndex];
    return skillType > 0 && skillType < MAX_SKILLS;
}

int FindSkillIndexByType(int skillType)
{
    if (CharacterAttribute == nullptr || skillType <= 0 || skillType >= MAX_SKILLS)
    {
        return -1;
    }

    for (int i = 0; i < MAX_MAGIC; ++i)
    {
        if (CharacterAttribute->Skill[i] == skillType)
        {
            return i;
        }
    }

    return -1;
}

bool IsAssignableVirtualSkillIndex(int skillIndex)
{
    // The custom mobile skill rings stay disabled; if they are ever re-enabled,
    // keep index 0 out so they only store explicit skill-list entries.
    return skillIndex > 0
        && skillIndex <= static_cast<int>(std::numeric_limits<BYTE>::max())
        && IsValidSkillIndex(skillIndex);
}

int GetSkillTypeFromIndex(int skillIndex)
{
    if (!IsValidSkillIndex(skillIndex))
    {
        return 0;
    }

    const int skillType = CharacterAttribute->Skill[skillIndex];
    return (skillType > 0 && skillType < MAX_SKILLS) ? skillType : 0;
}

int GetHeroCharacterIndex();
void SyncVirtualSlotsToMainFrame();
void SaveVirtualSkillSlots();
void ClearVirtualPickerTouch();

void SanitizeVirtualSkillSlots()
{
    for (int& slot : g_virtualSkillSlots)
    {
        if (!IsAssignableVirtualSkillIndex(slot))
        {
            slot = -1;
        }
    }
}

void RefreshVirtualSkillTypesFromSlots()
{
    for (int i = 0; i < kVirtualSkillSlotCount; ++i)
    {
        g_virtualSkillTypes[i] = GetSkillTypeFromIndex(g_virtualSkillSlots[i]);
    }
}

std::string BuildVirtualSkillArrayString(const std::array<int, kVirtualSkillSlotCount>& values)
{
    std::string text;
    text.reserve(2 + kVirtualSkillSlotCount * 8);
    text.push_back('[');
    for (int i = 0; i < kVirtualSkillSlotCount; ++i)
    {
        if (i > 0)
        {
            text.push_back(',');
        }
        text += std::to_string(values[i]);
    }
    text.push_back(']');
    return text;
}

bool IsVirtualSkillTypeAssigned(int skillType)
{
    if (skillType <= 0)
    {
        return false;
    }

    for (int slot = 0; slot < kVirtualSkillSlotCount; ++slot)
    {
        if (!IsAssignableVirtualSkillIndex(g_virtualSkillSlots[slot]))
        {
            continue;
        }

        if (GetSkillTypeFromIndex(g_virtualSkillSlots[slot]) == skillType)
        {
            return true;
        }
    }

    return false;
}

bool HasAnyAssignedVirtualSkillSlot()
{
    for (int slot = 0; slot < kVirtualSkillSlotCount; ++slot)
    {
        if (IsAssignableVirtualSkillIndex(g_virtualSkillSlots[slot]))
        {
            return true;
        }
    }

    return false;
}

int FindFirstAssignableSkillIndex()
{
    if (IsAssignableVirtualSkillIndex(Hero != nullptr ? Hero->CurrentSkill : -1))
    {
        return static_cast<int>(Hero->CurrentSkill);
    }

    for (int skillIndex = 1; skillIndex < MAX_MAGIC; ++skillIndex)
    {
        if (IsAssignableVirtualSkillIndex(skillIndex))
        {
            return skillIndex;
        }
    }

    return -1;
}

bool AutoFillVirtualSkillSlotsFromCharacter()
{
    if (CharacterAttribute == nullptr)
    {
        return false;
    }

    bool changed = false;
    int nextSlot = 0;

    for (int skillIndex = 1; skillIndex < MAX_MAGIC && nextSlot < kVirtualSkillSlotCount; ++skillIndex)
    {
        if (!IsAssignableVirtualSkillIndex(skillIndex))
        {
            continue;
        }

        while (nextSlot < kVirtualSkillSlotCount && IsAssignableVirtualSkillIndex(g_virtualSkillSlots[nextSlot]))
        {
            ++nextSlot;
        }

        if (nextSlot >= kVirtualSkillSlotCount)
        {
            break;
        }

        g_virtualSkillSlots[nextSlot] = skillIndex;
        g_virtualSkillTypes[nextSlot] = GetSkillTypeFromIndex(skillIndex);
        changed = true;
        ++nextSlot;
    }

    return changed;
}

bool ResolveVirtualSkillSlotsFromTypes()
{
    bool changed = false;
    for (int i = 0; i < kVirtualSkillSlotCount; ++i)
    {
        if (IsAssignableVirtualSkillIndex(g_virtualSkillSlots[i]))
        {
            const int resolvedType = GetSkillTypeFromIndex(g_virtualSkillSlots[i]);
            if (resolvedType > 0 && g_virtualSkillTypes[i] != resolvedType)
            {
                g_virtualSkillTypes[i] = resolvedType;
                changed = true;
            }
            continue;
        }

        if (g_virtualSkillTypes[i] <= 0)
        {
            continue;
        }

        const int resolvedIndex = FindSkillIndexByType(g_virtualSkillTypes[i]);
        if (!IsAssignableVirtualSkillIndex(resolvedIndex))
        {
            continue;
        }

        g_virtualSkillSlots[i] = resolvedIndex;
        changed = true;
    }

    if (changed)
    {
        SyncVirtualSlotsToMainFrame();
        if (g_virtualSkillSlotsLoaded)
        {
            g_virtualSkillSlotsDirty = true;
            SaveVirtualSkillSlots();
        }
    }

    return changed;
}

void SyncVirtualSlotsToMainFrame()
{
    // Keep virtual slots independent from the legacy hotkey bar (1..0 / WER/Q).
    // We intentionally do not write these bindings back to the default UI hotkey system.
}

void LoadVirtualSkillSlots()
{
    if (g_virtualSkillSlotsLoaded)
    {
        return;
    }

    for (int slot = 0; slot < kVirtualSkillSlotCount; ++slot)
    {
        g_virtualSkillSlots[slot] = -1;
        g_virtualSkillTypes[slot] = 0;
    }

    int loadedVersion = 0;
    std::ifstream in(kVirtualSkillSlotsPath, std::ios::in);
    if (in.good())
    {
        int version = 0;
        if (in >> version)
        {
            if (version == 1 || version == 2)
            {
                int slot0 = -1;
                int slot1 = -1;
                int slot2 = -1;
                if (in >> slot0 >> slot1 >> slot2)
                {
                    loadedVersion = version;
                    if (version == 2)
                    {
                        g_virtualSkillTypes[1] = slot0;
                        g_virtualSkillTypes[2] = slot1;
                        g_virtualSkillTypes[3] = slot2;
                    }
                    else
                    {
                        // Backward compatibility: v1 stored skill indices directly.
                        g_virtualSkillSlots[1] = slot0;
                        g_virtualSkillSlots[2] = slot1;
                        g_virtualSkillSlots[3] = slot2;
                    }
                }
            }
            else if (version == 3)
            {
                int savedCount = 0;
                if ((in >> savedCount) && savedCount > 0)
                {
                    bool readOk = true;
                    for (int i = 0; i < savedCount; ++i)
                    {
                        int savedType = 0;
                        if (!(in >> savedType))
                        {
                            readOk = false;
                            break;
                        }

                        if (i < kVirtualSkillSlotCount)
                        {
                            g_virtualSkillTypes[i] = savedType;
                        }
                    }

                    if (readOk)
                    {
                        loadedVersion = version;
                    }
                }
            }
            else if (version >= 4)
            {
                int savedCount = 0;
                if ((in >> savedCount) && savedCount > 0)
                {
                    bool readOk = true;
                    for (int i = 0; i < savedCount; ++i)
                    {
                        int savedIndex = -1;
                        if (!(in >> savedIndex))
                        {
                            readOk = false;
                            break;
                        }

                        if (i < kVirtualSkillSlotCount)
                        {
                            g_virtualSkillSlots[i] = savedIndex;
                        }
                    }

                    if (readOk)
                    {
                        loadedVersion = version;
                    }
                }
            }
        }
    }

    SanitizeVirtualSkillSlots();
    bool resolvedChanged = false;
    if (loadedVersion < 4)
    {
        if (loadedVersion == 1)
        {
            RefreshVirtualSkillTypesFromSlots();
        }

        resolvedChanged = ResolveVirtualSkillSlotsFromTypes();
    }
    else
    {
        RefreshVirtualSkillTypesFromSlots();
    }
    bool autoFilled = false;
    if (!HasAnyAssignedVirtualSkillSlot())
    {
        autoFilled = AutoFillVirtualSkillSlotsFromCharacter();
    }
    SyncVirtualSlotsToMainFrame();
    g_virtualSkillSlotsLoaded = true;
    g_virtualSkillSlotsDirty = resolvedChanged || autoFilled || loadedVersion < 4;
    if (g_virtualSkillSlotsDirty)
    {
        SaveVirtualSkillSlots();
    }

    const std::string slotText = BuildVirtualSkillArrayString(g_virtualSkillSlots);
    const std::string typeText = BuildVirtualSkillArrayString(g_virtualSkillTypes);
    LOGI(
        "VirtualPad: slots loaded version=%d idx=%s type=%s path=%s",
        loadedVersion,
        slotText.c_str(),
        typeText.c_str(),
        kVirtualSkillSlotsPath);
}

void SaveVirtualSkillSlots()
{
    if (!g_virtualSkillSlotsLoaded || !g_virtualSkillSlotsDirty)
    {
        return;
    }

    SanitizeVirtualSkillSlots();

    std::error_code ec;
    const std::filesystem::path savePath(kVirtualSkillSlotsPath);
    if (savePath.has_parent_path())
    {
        std::filesystem::create_directories(savePath.parent_path(), ec);
    }

    std::ofstream out(savePath, std::ios::out | std::ios::trunc);
    if (!out.good())
    {
        LOGW("VirtualPad: failed to save slots at '%s'", kVirtualSkillSlotsPath);
        return;
    }

    RefreshVirtualSkillTypesFromSlots();
    out << "4 " << kVirtualSkillSlotCount;
    for (int slot = 0; slot < kVirtualSkillSlotCount; ++slot)
    {
        out << ' ' << g_virtualSkillSlots[slot];
    }
    out << '\n';
    g_virtualSkillSlotsDirty = false;
    const std::string slotText = BuildVirtualSkillArrayString(g_virtualSkillSlots);
    LOGI("VirtualPad: slots saved idx=%s", slotText.c_str());
}

void SetVirtualSkillSlot(int slot, int skillIndex)
{
    if (slot < 0 || slot >= kVirtualSkillSlotCount)
    {
        return;
    }

    if (!IsAssignableVirtualSkillIndex(skillIndex))
    {
        const int skillType = (IsValidSkillIndex(skillIndex) && CharacterAttribute != nullptr)
            ? CharacterAttribute->Skill[skillIndex]
            : -1;
        LOGI(
            "VirtualPad: slot%d assign rejected skillIndex=%d skillType=%d",
            slot,
            skillIndex,
            skillType);
        return;
    }

    g_virtualSkillSlots[slot] = skillIndex;
    g_virtualSkillTypes[slot] = GetSkillTypeFromIndex(skillIndex);
    SyncVirtualSlotsToMainFrame();
    g_virtualSkillSlotsDirty = true;
    SaveVirtualSkillSlots();

    const std::string slotText = BuildVirtualSkillArrayString(g_virtualSkillSlots);
    LOGI(
        "VirtualPad: slot%d set to skillIndex=%d skillType=%d slots=%s",
        slot,
        skillIndex,
        CharacterAttribute->Skill[skillIndex],
        slotText.c_str());
}

void DeactivateVirtualAssignMode(const char* reason)
{
    if (!g_virtualAssignModeActive)
    {
        return;
    }

    LOGI(
        "VirtualPad: assign mode OFF skillIndex=%d reason=%s",
        g_virtualAssignSkillIndex,
        (reason != nullptr) ? reason : "n/a");
    g_virtualAssignModeActive = false;
    g_virtualAssignSkillIndex = -1;
    g_virtualAssignModeUntilMs = 0;
}

void ActivateVirtualAssignMode(int skillIndex, const char* reason)
{
    if (!IsAssignableVirtualSkillIndex(skillIndex))
    {
        return;
    }

    const uint32_t nowMs = MU_MobileGetTicks();
    if (g_virtualAssignModeActive && g_virtualAssignSkillIndex == skillIndex)
    {
        g_virtualAssignModeUntilMs = nowMs + kVirtualAssignModeTimeoutMs;
        return;
    }

    g_virtualAssignModeActive = true;
    g_virtualAssignSkillIndex = skillIndex;
    g_virtualAssignModeUntilMs = nowMs + kVirtualAssignModeTimeoutMs;
    LOGI(
        "VirtualPad: assign mode ON skillIndex=%d skillType=%d reason=%s",
        skillIndex,
        CharacterAttribute->Skill[skillIndex],
        (reason != nullptr) ? reason : "n/a");
}

bool IsVirtualAssignModeActive()
{
    if (!g_virtualAssignModeActive)
    {
        return false;
    }

    if (!IsAssignableVirtualSkillIndex(g_virtualAssignSkillIndex))
    {
        DeactivateVirtualAssignMode("invalid-skill");
        return false;
    }

    const uint32_t nowMs = MU_MobileGetTicks();
    if (nowMs > g_virtualAssignModeUntilMs)
    {
        DeactivateVirtualAssignMode("timeout");
        return false;
    }

    return true;
}

int GetPendingVirtualAssignSkillIndex(uint32_t nowMs)
{
    (void)nowMs;

    if (IsVirtualAssignModeActive())
    {
        return g_virtualAssignSkillIndex;
    }

    // Keep the last picker-selected skill pending until the player assigns it
    // to one of the 4 slots or explicitly reopens the picker.
    if (IsAssignableVirtualSkillIndex(g_virtualAssignPickerSkillIndex))
    {
        return g_virtualAssignPickerSkillIndex;
    }

    return -1;
}

void UpdateVirtualAssignMode()
{
    if (g_pSkillList == nullptr || Hero == nullptr || CharacterAttribute == nullptr)
    {
        DeactivateVirtualAssignMode("missing-context");
        g_virtualLastSkillPickerOpen = false;
        g_virtualAssignPickerSkillIndex = -1;
        g_virtualAssignConsumedForPickerSkill = false;
        g_virtualAssignConsumedForPickerSession = false;
        ClearVirtualPickerTouch();
        return;
    }

    const bool pickerOpen = g_pSkillList->IsSkillPickerOpen();
    const int pickedSkill = g_pSkillList->GetAndroidTouchAssignSkillIndex();

    if (pickerOpen)
    {
        if (!g_virtualLastSkillPickerOpen)
        {
            g_virtualAssignPickerSkillIndex = -1;
            g_virtualAssignConsumedForPickerSkill = false;
            g_virtualAssignConsumedForPickerSession = false;
        }

        if (pickedSkill != g_virtualAssignPickerSkillIndex)
        {
            g_virtualAssignPickerSkillIndex = pickedSkill;
            g_virtualAssignConsumedForPickerSkill = false;
            g_virtualAssignConsumedForPickerSession = false;
        }

        if (IsAssignableVirtualSkillIndex(pickedSkill))
        {
            ActivateVirtualAssignMode(pickedSkill, "picker-open");
        }
        else
        {
            DeactivateVirtualAssignMode("picker-await-selection");
        }
    }
    else
    {
        g_virtualAssignConsumedForPickerSession = false;
        if (g_virtualLastSkillPickerOpen
            && IsAssignableVirtualSkillIndex(pickedSkill))
        {
            // Allow one quick assignment after closing picker.
            g_virtualAssignPickerSkillIndex = pickedSkill;
            g_virtualAssignConsumedForPickerSkill = false;
            ActivateVirtualAssignMode(pickedSkill, "picker-closed");
        }
    }

    g_virtualLastSkillPickerOpen = pickerOpen;
}

void TouchToVirtualUi(const SDL_TouchFingerEvent& touch, float& outX, float& outY)
{
    const float nx = std::clamp(touch.x, 0.0f, 1.0f);
    const float ny = std::clamp(touch.y, 0.0f, 1.0f);
    outX = nx * 640.0f;
    outY = ny * 480.0f;
}

bool IsTouchOverInventoryWindow(float uiX, float uiY)
{
    if (g_pNewUISystem == nullptr
        || g_pMyInventory == nullptr
        || !g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INVENTORY))
    {
        return false;
    }

    const POINT& pos = g_pMyInventory->GetPos();
    const float left = static_cast<float>(pos.x);
    const float top = static_cast<float>(pos.y);
    const float right = left + kInventoryWindowWidth;
    const float bottom = top + kInventoryWindowHeight;
    return uiX >= left && uiX <= right && uiY >= top && uiY <= bottom;
}

bool IsVirtualJoystickCaptured(SDL_FingerID fingerId)
{
    return g_virtualJoystick.fingerId != static_cast<SDL_FingerID>(-1)
        && g_virtualJoystick.fingerId == fingerId;
}

bool IsVirtualPickerTouchCaptured(SDL_FingerID fingerId)
{
    return g_virtualPickerTouch.fingerId != static_cast<SDL_FingerID>(-1)
        && g_virtualPickerTouch.fingerId == fingerId;
}

void ClearVirtualPickerTouch()
{
    g_virtualPickerTouch.fingerId = static_cast<SDL_FingerID>(-1);
    g_virtualPickerTouch.skillIndex = -1;
}

bool HandleVirtualPickerFingerDown(const SDL_TouchFingerEvent& touch)
{
    if (kUseLegacyMainHud)
    {
        return false;
    }

    if (g_pSkillList == nullptr || !g_pSkillList->IsSkillPickerOpen())
    {
        return false;
    }

    float uiX = 0.0f;
    float uiY = 0.0f;
    TouchToVirtualUi(touch, uiX, uiY);

    const int skillIndex = g_pSkillList->HitTestAndroidTouchSkillPicker(uiX, uiY);
    if (skillIndex < 0)
    {
        return false;
    }

    g_virtualPickerTouch.fingerId = touch.fingerId;
    g_virtualPickerTouch.skillIndex = skillIndex;
    return true;
}

bool HandleVirtualPickerFingerMotion(const SDL_TouchFingerEvent& touch)
{
    if (kUseLegacyMainHud)
    {
        return false;
    }

    return IsVirtualPickerTouchCaptured(touch.fingerId);
}

bool HandleVirtualPickerFingerUp(const SDL_TouchFingerEvent& touch)
{
    if (kUseLegacyMainHud)
    {
        return false;
    }

    if (!IsVirtualPickerTouchCaptured(touch.fingerId))
    {
        return false;
    }

    float uiX = 0.0f;
    float uiY = 0.0f;
    TouchToVirtualUi(touch, uiX, uiY);

    const int releasedSkill = (g_pSkillList != nullptr)
        ? g_pSkillList->HitTestAndroidTouchSkillPicker(uiX, uiY)
        : -1;
    const int chosenSkill = (releasedSkill == g_virtualPickerTouch.skillIndex)
        ? releasedSkill
        : g_virtualPickerTouch.skillIndex;

    if (g_pSkillList != nullptr && chosenSkill >= 0)
    {
        int previousSkillType = 0;
        if (Hero != nullptr && CharacterAttribute != nullptr)
        {
            if (Hero->CurrentSkill >= AT_PET_COMMAND_DEFAULT && Hero->CurrentSkill < AT_PET_COMMAND_END)
            {
                previousSkillType = Hero->CurrentSkill;
            }
            else if (Hero->CurrentSkill >= 0 && Hero->CurrentSkill < MAX_MAGIC)
            {
                previousSkillType = CharacterAttribute->Skill[Hero->CurrentSkill];
            }
        }

        if (previousSkillType > 0)
        {
            g_pSkillList->SetHeroPriorSkill(static_cast<BYTE>(previousSkillType));
        }

        if (Hero != nullptr)
        {
            Hero->CurrentSkill = static_cast<BYTE>(chosenSkill);
        }

        g_pSkillList->SetAndroidTouchAssignSkillIndex(chosenSkill);
        g_pSkillList->SetSkillPickerOpen(false);

        if (IsAssignableVirtualSkillIndex(chosenSkill))
        {
            g_virtualAssignPickerSkillIndex = chosenSkill;
            g_virtualAssignConsumedForPickerSkill = false;
            g_virtualAssignConsumedForPickerSession = false;
            ActivateVirtualAssignMode(chosenSkill, "picker-touch");
        }
        else
        {
            g_virtualAssignPickerSkillIndex = -1;
            DeactivateVirtualAssignMode("picker-touch-nonassignable");
        }

        UpdateVirtualAssignMode();
    }

    ClearVirtualPickerTouch();
    return true;
}

bool IsInsideVirtualJoystickDynamicArea(float uiX, float uiY)
{
    if (!IsVirtualPadAvailable())
    {
        return false;
    }

    if (uiY >= kVirtualPadInputMaxY)
    {
        return false;
    }

    if (uiY < kVirtualJoystickDynamicAreaMinY)
    {
        return false;
    }

    if (uiX < 0.0f || uiX > kVirtualJoystickDynamicAreaMaxX)
    {
        return false;
    }

    if (IsTouchOverInventoryWindow(uiX, uiY))
    {
        return false;
    }

    return true;
}

float ClampVirtualJoystickCenterX(float uiX)
{
    const float minX = kVirtualJoystickRadius + 8.0f;
    const float maxX = std::max(minX, kVirtualJoystickDynamicAreaMaxX - kVirtualJoystickRadius - 8.0f);
    return std::clamp(uiX, minX, maxX);
}

float GetVirtualUiScaleY();
float GetVirtualUiBottomSafeInsetUi();
float GetVirtualJoystickMaxCenterY();

float GetVirtualUiBottomSafeInsetUi()
{
    const int insetPx = std::max(g_AndroidSafeInsetBottomPx, 0);
    return static_cast<float>(insetPx) / std::max(GetVirtualUiScaleY(), 0.001f);
}

float GetVirtualJoystickMaxCenterY()
{
    const float bottomInsetUi = GetVirtualUiBottomSafeInsetUi();
    const float maxRadius = kVirtualJoystickVisualExtentUi;
    const float aboveNav = 480.0f - bottomInsetUi - maxRadius - 8.0f;
    const float aboveHud = kLegacyMainFrameBarTopY - kVirtualJoystickHudClearanceUi - maxRadius;
    return std::floor(std::min(aboveHud, aboveNav));
}

float ClampVirtualJoystickCenterY(float uiY)
{
    const float minY = kVirtualJoystickDynamicAreaMinY + kVirtualJoystickRadius + 8.0f;
    const float maxYByTouch = kVirtualPadInputMaxY - kVirtualJoystickRadius - 8.0f;
    const float maxY = std::min(maxYByTouch, GetVirtualJoystickMaxCenterY());
    return std::clamp(uiY, minY, std::max(minY, maxY));
}

float GetVirtualJoystickRenderCenterX()
{
    return std::round((g_virtualJoystick.fingerId != static_cast<SDL_FingerID>(-1))
        ? g_virtualJoystick.centerX
        : kVirtualJoystickDefaultCenterX);
}

float GetVirtualJoystickRenderCenterY()
{
    return std::round((g_virtualJoystick.fingerId != static_cast<SDL_FingerID>(-1))
        ? g_virtualJoystick.centerY
        : kVirtualJoystickDefaultCenterY);
}

void InterruptVirtualCombatForMovement()
{
    Attacking = -1;

    if (Hero != nullptr)
    {
        if (Hero->MovementType == MOVEMENT_ATTACK)
        {
            Hero->MovementType = MOVEMENT_MOVE;
        }

        Hero->AttackTime = 0;
    }

    SelectedCharacter = -1;
}

void ReleaseVirtualJoystickMouseDrive()
{
    if (!g_virtualJoystickDrivingMouse)
    {
        return;
    }

    g_virtualJoystickDrivingMouse = false;
    MouseLButtonPush = false;
    MouseLButton = false;
    MouseLButtonPop = false;
    CancelHeroClickMove(true);
}

void ClearVirtualJoystick()
{
    g_virtualJoystick = ActiveVirtualJoystick{};
    ReleaseVirtualJoystickMouseDrive();
}

bool HitTestVirtualJoystick(float uiX, float uiY)
{
    return IsInsideVirtualJoystickDynamicArea(uiX, uiY);
}

void UpdateVirtualJoystickByUi(float uiX, float uiY)
{
    const float dx = uiX - g_virtualJoystick.centerX;
    const float dy = uiY - g_virtualJoystick.centerY;
    const float dist = std::sqrt((dx * dx) + (dy * dy));

    float dirX = 0.0f;
    float dirY = 0.0f;
    if (dist > 0.0001f)
    {
        const float invDist = 1.0f / dist;
        dirX = dx * invDist;
        dirY = dy * invDist;
    }

    const float clampedDist = std::min(dist, kVirtualJoystickRadius);
    g_virtualJoystick.thumbOffsetX = dirX * clampedDist;
    g_virtualJoystick.thumbOffsetY = dirY * clampedDist;

    const float effectiveDist = std::max(clampedDist - kVirtualJoystickDeadZone, 0.0f);
    const float activeRange = std::max(kVirtualJoystickRadius - kVirtualJoystickDeadZone, 1.0f);
    const float strength = std::clamp(effectiveDist / activeRange, 0.0f, 1.0f);

    g_virtualJoystick.moveStrength = strength;
    if (strength > 0.001f)
    {
        g_virtualJoystick.moveDirX = dirX;
        // Touch Y grows downward; movement Y should grow upward.
        g_virtualJoystick.moveDirY = -dirY;
    }
    else
    {
        g_virtualJoystick.moveDirX = 0.0f;
        g_virtualJoystick.moveDirY = 0.0f;
    }
}

void StartVirtualJoystick(SDL_FingerID fingerId, float uiX, float uiY)
{
    g_virtualJoystick = ActiveVirtualJoystick{};
    g_virtualJoystick.fingerId = fingerId;
    g_virtualJoystick.centerX = ClampVirtualJoystickCenterX(uiX);
    g_virtualJoystick.centerY = ClampVirtualJoystickCenterY(uiY);
    UpdateVirtualJoystickByUi(uiX, uiY);
}

bool HandleVirtualJoystickFingerDown(const SDL_TouchFingerEvent& touch)
{
    float uiX = 0.0f;
    float uiY = 0.0f;
    TouchToVirtualUi(touch, uiX, uiY);

    if (!HitTestVirtualJoystick(uiX, uiY))
    {
        return false;
    }

    if (g_virtualJoystick.fingerId != static_cast<SDL_FingerID>(-1)
        && g_virtualJoystick.fingerId != touch.fingerId)
    {
        return true;
    }

    StartVirtualJoystick(touch.fingerId, uiX, uiY);
    return true;
}

bool HandleVirtualJoystickFingerMotion(const SDL_TouchFingerEvent& touch)
{
    if (!IsVirtualJoystickCaptured(touch.fingerId))
    {
        return false;
    }

    float uiX = 0.0f;
    float uiY = 0.0f;
    TouchToVirtualUi(touch, uiX, uiY);
    UpdateVirtualJoystickByUi(uiX, uiY);
    return true;
}

bool HandleVirtualJoystickFingerUp(const SDL_TouchFingerEvent& touch)
{
    if (!IsVirtualJoystickCaptured(touch.fingerId))
    {
        return false;
    }

    ClearVirtualJoystick();
    return true;
}

void ApplyVirtualJoystickAim()
{
    if (g_virtualJoystick.fingerId == static_cast<SDL_FingerID>(-1))
    {
        return;
    }

    float dirX = g_virtualJoystick.moveDirX;
    float dirY = g_virtualJoystick.moveDirY;
    if (g_virtualJoystick.moveStrength <= 0.001f)
    {
        const float thumbX = g_virtualJoystick.thumbOffsetX;
        const float thumbY = -g_virtualJoystick.thumbOffsetY;
        const float thumbLen = std::sqrt((thumbX * thumbX) + (thumbY * thumbY));
        if (thumbLen <= 2.0f)
        {
            return;
        }

        dirX = thumbX / thumbLen;
        dirY = thumbY / thumbLen;
    }

    const float aimRadius = kVirtualJoystickMouseMinRadius + 12.0f;
    MouseX = std::clamp(
        static_cast<int>(320.0f + dirX * aimRadius),
        0,
        640);
    MouseY = std::clamp(
        static_cast<int>(180.0f - dirY * aimRadius),
        0,
        480);
    g_iNoMouseTime = 0;
}

void ApplyVirtualJoystickMovement()
{
    if (g_virtualJoystick.fingerId == static_cast<SDL_FingerID>(-1))
    {
        ReleaseVirtualJoystickMouseDrive();
        return;
    }

    if (!IsVirtualPadAvailable())
    {
        ClearVirtualJoystick();
        return;
    }

    if (g_virtualJoystick.moveStrength <= 0.001f)
    {
        ReleaseVirtualJoystickMouseDrive();
        ApplyVirtualJoystickAim();
        return;
    }

    if (g_virtualJoystick.moveStrength <= kVirtualJoystickCombatMoveStrength)
    {
        ReleaseVirtualJoystickMouseDrive();
        ApplyVirtualJoystickAim();
        return;
    }

    InterruptVirtualCombatForMovement();

    const float driveRadius = kVirtualJoystickMouseMinRadius
        + (kVirtualJoystickMouseMaxRadius - kVirtualJoystickMouseMinRadius) * g_virtualJoystick.moveStrength;
    const int targetMouseX = std::clamp(
        static_cast<int>(320.0f + g_virtualJoystick.moveDirX * driveRadius),
        0,
        640);
    const int targetMouseY = std::clamp(
        static_cast<int>(180.0f - g_virtualJoystick.moveDirY * driveRadius),
        0,
        480);

    MouseX = targetMouseX;
    MouseY = targetMouseY;
    g_iNoMouseTime = 0;

    MouseLButtonPop = false;
    if (!MouseLButton)
    {
        MouseLButtonPush = true;
        MouseLButton = true;
    }
    else
    {
        MouseLButtonPush = false;
    }

    g_virtualJoystickDrivingMouse = true;
}

bool IsMiniMapToggleAvailable()
{
    return SceneFlag == MAIN_SCENE
        && g_pNewUISystem != nullptr
        && !AndroidHasFocusedTextInput();
}

// â”€â”€ Map button hit test â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
bool HitTestMapButton(float uiX, float uiY)
{
    if (SceneFlag != MAIN_SCENE || g_pNewUISystem == nullptr || AndroidHasFocusedTextInput())
        return false;

    const AndroidUiRect rect = GetMapButtonRect();
    return uiX >= rect.x && uiX <= (rect.x + rect.w)
        && uiY >= rect.y && uiY <= (rect.y + rect.h);
}

void ToggleMapListByVirtualButton()
{
    if (g_pNewUISystem == nullptr)
        return;
    g_pNewUISystem->Toggle(SEASON3B::INTERFACE_MOVEMAP);
    LOGI("VirtualPad: map list toggled");
}

bool HitTestMiniMapToggleButton(float uiX, float uiY)
{
    if (!IsMiniMapToggleAvailable())
    {
        return false;
    }

    const AndroidUiRect rect = GetMiniMapButtonRect();
    return uiX >= rect.x
        && uiX <= (rect.x + rect.w)
        && uiY >= rect.y
        && uiY <= (rect.y + rect.h);
}

bool IsVirtualUtilityButtonsAvailable();

bool HitTestVirtualChatUtilityButton(float uiX, float uiY)
{
    if (!IsVirtualUtilityButtonsAvailable())
    {
        return false;
    }

    const AndroidUiRect rect = GetVirtualUtilityButtonRect(kVirtualUtilityButtonChat);
    return uiX >= rect.x
        && uiX <= (rect.x + rect.w)
        && uiY >= rect.y
        && uiY <= (rect.y + rect.h);
}

bool IsVirtualUtilityButtonsAvailable()
{
    return SceneFlag == MAIN_SCENE
        && g_pNewUISystem != nullptr
        && !AndroidHasFocusedTextInput();
}

int HitTestVirtualUtilityButton(float uiX, float uiY)
{
    if (!IsVirtualUtilityButtonsAvailable())
    {
        return -1;
    }

    for (int i = 0; i < kVirtualUtilityButtonCount; ++i)
    {
        const AndroidUiRect rect = GetVirtualUtilityButtonRect(i);
        if (uiX >= rect.x && uiX <= (rect.x + rect.w)
            && uiY >= rect.y && uiY <= (rect.y + rect.h))
        {
            return i;
        }
    }

    return -1;
}

bool IsVirtualUtilityButtonActive(int button)
{
    if (g_pNewUISystem == nullptr)
    {
        return false;
    }

    switch (button)
    {
    case 0: return g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INVENTORY);
    case 1: return g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHARACTER);
    case 2: return g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_OPTION);
    case 3: return g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHATINPUTBOX);
    default: return false;
    }
}

void ToggleVirtualUtilityButton(int button)
{
    if (g_pNewUISystem == nullptr)
    {
        return;
    }

    switch (button)
    {
    case 0:
        g_pNewUISystem->Toggle(SEASON3B::INTERFACE_INVENTORY);
        LOGI("VirtualPad: utility toggle -> inventory");
        break;
    case 1:
        g_pNewUISystem->Toggle(SEASON3B::INTERFACE_CHARACTER);
        LOGI("VirtualPad: utility toggle -> character");
        break;
    case 2:
        g_pNewUISystem->Toggle(SEASON3B::INTERFACE_OPTION);
        LOGI("VirtualPad: utility toggle -> option");
        break;
    case 3:
        ToggleVirtualChatInputBox();
        LOGI("VirtualPad: utility toggle -> chat");
        break;
    default:
        break;
    }
}

void ToggleMiniMapByVirtualButton()
{
    if (g_pNewUISystem == nullptr)
    {
        return;
    }

    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MINI_MAP))
    {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_MINI_MAP);
        LOGI("VirtualPad: minimap toggle -> hide");
    }
    else
    {
        g_pNewUISystem->Toggle(SEASON3B::INTERFACE_MINI_MAP);
        LOGI("VirtualPad: minimap toggle -> show");
    }
}

int HitTestVirtualZoomButton(float uiX, float uiY)
{
    if (!IsVirtualPadAvailable())
    {
        return -1;
    }

    if (uiY < kZoomButtonY || uiY > (kZoomButtonY + kZoomButtonH))
    {
        return -1;
    }

    if (uiX >= kZoomMinusX && uiX <= (kZoomMinusX + kZoomButtonW))
    {
        return kVirtualZoomButtonMinus;
    }

    if (uiX >= kZoomPlusX && uiX <= (kZoomPlusX + kZoomButtonW))
    {
        return kVirtualZoomButtonPlus;
    }

    return -1;
}

bool HandleVirtualZoomButtonTap(int button)
{
    if (button != kVirtualZoomButtonMinus && button != kVirtualZoomButtonPlus)
    {
        return false;
    }

    const uint32_t nowMs = MU_MobileGetTicks();
    if ((nowMs - g_virtualLastZoomTapMs) < kZoomCooldownMs)
    {
        return true;
    }

    float currentZoom = g_androidZoomOverride > 0.0f
        ? g_androidZoomOverride
        : CameraDistanceTarget;
    if (currentZoom <= 0.0f)
    {
        currentZoom = 1200.0f;
    }

    currentZoom = std::clamp(currentZoom, kZoomMin, kZoomMax);
    const float delta = (button == kVirtualZoomButtonMinus) ? kZoomStep : -kZoomStep;
    const float nextZoom = std::clamp(currentZoom + delta, kZoomMin, kZoomMax);

    g_virtualLastZoomTapMs = nowMs;
    g_androidZoomOverride = nextZoom;
    CameraDistance = nextZoom;
    CameraDistanceTarget = nextZoom;

    LOGI(
        "VirtualPad: zoom tap button=%s current=%.1f next=%.1f",
        button == kVirtualZoomButtonMinus ? "minus" : "plus",
        currentZoom,
        nextZoom);
    return true;
}

bool HitTestVirtualCurrentSkillBox(float uiX, float uiY);
void ToggleVirtualSkillPickerByTouch();

bool HandleVirtualTopControlTap(float uiX, float uiY)
{
    const uint32_t nowMs = MU_MobileGetTicks();

    if (HitTestMiniMapToggleButton(uiX, uiY))
    {
        if ((nowMs - g_virtualLastMiniMapTapMs) >= kVirtualMiniMapButtonCooldownMs)
        {
            g_virtualLastMiniMapTapMs = nowMs;
            ToggleMiniMapByVirtualButton();
        }

        return true;
    }

    if (HitTestMapButton(uiX, uiY))
    {
        if ((nowMs - g_virtualLastMiniMapTapMs) >= kVirtualMiniMapButtonCooldownMs)
        {
            g_virtualLastMiniMapTapMs = nowMs;
            ToggleMapListByVirtualButton();
        }

        return true;
    }

    if (HitTestVirtualChatUtilityButton(uiX, uiY))
    {
        if ((nowMs - g_virtualLastUtilityTapMs) >= kVirtualUtilityButtonCooldownMs)
        {
            g_virtualLastUtilityTapMs = nowMs;
            ToggleVirtualChatInputBox();
        }

        return true;
    }

    if (kUseLegacyMainHud)
    {
        return false;
    }

    if (HitTestVirtualCurrentSkillBox(uiX, uiY))
    {
        ToggleVirtualSkillPickerByTouch();
        return true;
    }

    return false;
}

bool HitTestVirtualCurrentSkillBox(float uiX, float uiY)
{
    if (!kShowVirtualCurrentSkillBox)
    {
        return false;
    }

    if (!IsVirtualPadAvailable())
    {
        return false;
    }

    return uiX >= kVirtualCurrentSkillBoxX
        && uiX <= (kVirtualCurrentSkillBoxX + kVirtualCurrentSkillBoxW)
        && uiY >= kVirtualCurrentSkillBoxY
        && uiY <= (kVirtualCurrentSkillBoxY + kVirtualCurrentSkillBoxH);
}

void ToggleVirtualSkillPickerByTouch()
{
    if (g_pSkillList == nullptr || CharacterAttribute == nullptr)
    {
        return;
    }

    if (CharacterAttribute->SkillNumber <= 0 && CharacterAttribute->SkillMasterNumber <= 0)
    {
        return;
    }

    if (!g_pSkillList->IsSkillPickerOpen())
    {
        // Each picker open starts a fresh "pick one skill -> assign one slot" flow.
        g_pSkillList->SetAndroidTouchAssignSkillIndex(-1);
        g_virtualAssignPickerSkillIndex = -1;
        DeactivateVirtualAssignMode("picker-reset");
    }
    ClearVirtualPickerTouch();

    g_pSkillList->ToggleSkillPicker();
    UpdateVirtualAssignMode();
}

int HitTestVirtualButton(float uiX, float uiY)
{
    if (!IsVirtualPadAvailable())
    {
        return -1;
    }

    int bestButton = -1;
    float bestNormDist = 1000.0f;

    for (int i = 0; i < static_cast<int>(kVirtualButtons.size()); ++i)
    {
        const float dx = uiX - kVirtualButtons[i].cx;
        const float dy = uiY - kVirtualButtons[i].cy;
        const float hitRadius = GetVirtualButtonHitRadius(i);
        const float r2 = hitRadius * hitRadius;
        const float d2 = (dx * dx) + (dy * dy);
        if (d2 <= r2)
        {
            const float norm = d2 / std::max(r2, 1.0f);
            if (norm < bestNormDist)
            {
                bestNormDist = norm;
                bestButton = i;
            }
        }
    }

    // Keep bottom action bar touches for original UI unless the touch landed
    // directly on one of the virtual combat buttons above.
    if (uiY >= kVirtualPadInputMaxY && bestButton < 0)
    {
        return -1;
    }

    return bestButton;
}

// â”€â”€ Consumable slot helpers â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€

// Returns the lineal slot position (y*8 + x + offset) needed by SendRequestUse/FindItem.
// Uses g_pMyInventory which is the authoritative item store (NOT the legacy Inventory[] array).
int FindConsumableInInventory(int itemType, int itemLevel)
{
    if (itemType < 0 || g_pMyInventory == nullptr)
        return -1;
    const short sType = static_cast<short>(itemType);
    // Try exact level match first; fall back to any-level (-1) for robustness.
    int idx = g_pMyInventory->FindItemReverseIndex(sType, itemLevel);
    if (idx < 0)
        idx = g_pMyInventory->FindItemReverseIndex(sType, -1);
    return idx;
}

int HitTestVirtualConsumableSlot(float uiX, float uiY)
{
    if (!IsVirtualPadAvailable())
        return -1;
    // Use a larger touch radius than the visual radius so fat fingers can tap easily.
    constexpr float kTouchRadius = 24.0f;  // visual radius is 14; this gives ~50% more hit area
    for (int i = 0; i < kVirtualConsumableSlotCount; ++i)
    {
        const float dx = uiX - kVirtualConsumableSlots[i].cx;
        const float dy = uiY - kVirtualConsumableSlots[i].cy;
        if ((dx * dx + dy * dy) <= (kTouchRadius * kTouchRadius))
            return i;
    }
    if (uiY >= kVirtualPadInputMaxY)
        return -1;
    return -1;
}

bool TryBindPickedItemToVirtualConsumableSlot(int slot)
{
    if (slot < 0 || slot >= kVirtualConsumableSlotCount)
    {
        return false;
    }

    auto* picked = SEASON3B::CNewUIInventoryCtrl::GetPickedItem();
    if (picked == nullptr)
    {
        return false;
    }

    ITEM* pickedItem = picked->GetItem();
    if (pickedItem == nullptr)
    {
        return false;
    }

    const bool isBindableConsumable = false;
    if (!isBindableConsumable)
    {
        return false;
    }

    // Keep one binding per item type/level across the 3 consumable slots.
    for (int i = 0; i < kVirtualConsumableSlotCount; ++i)
    {
        if (i == slot)
        {
            continue;
        }

        if (g_virtualConsumableSlots[i].itemType == pickedItem->Type
            && g_virtualConsumableSlots[i].itemLevel == pickedItem->Level)
        {
            g_virtualConsumableSlots[i] = VirtualConsumableSlot{};
        }
    }

    g_virtualConsumableSlots[slot].itemType = pickedItem->Type;
    g_virtualConsumableSlots[slot].itemLevel = pickedItem->Level;

    SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();

    MouseLButton = false;
    MouseLButtonPush = false;
    MouseLButtonPop = false;
    MouseLButtonDBClick = false;
    return true;
}

void UseVirtualConsumableSlot(int slot)
{
    if (slot < 0 || slot >= kVirtualConsumableSlotCount)
        return;
    const VirtualConsumableSlot& cs = g_virtualConsumableSlots[slot];
    if (cs.itemType < 0)
        return;
    const int idx = FindConsumableInInventory(cs.itemType, cs.itemLevel);
    if (idx >= 0)
        (void)idx;
    else
        g_virtualConsumableSlots[slot] = VirtualConsumableSlot{};  // item gone â€” clear slot
}

int FindActiveVirtualTouchSlot(SDL_FingerID fingerId)
{
    for (int i = 0; i < static_cast<int>(g_activeVirtualTouches.size()); ++i)
    {
        if (g_activeVirtualTouches[i].fingerId == fingerId)
        {
            return i;
        }
    }

    return -1;
}

int AcquireActiveVirtualTouchSlot(SDL_FingerID fingerId)
{
    const int existing = FindActiveVirtualTouchSlot(fingerId);
    if (existing >= 0)
    {
        return existing;
    }

    for (int i = 0; i < static_cast<int>(g_activeVirtualTouches.size()); ++i)
    {
        if (g_activeVirtualTouches[i].fingerId == static_cast<SDL_FingerID>(-1))
        {
            return i;
        }
    }

    return -1;
}

void ClearActiveVirtualTouchSlot(int slot)
{
    if (slot < 0 || slot >= static_cast<int>(g_activeVirtualTouches.size()))
    {
        return;
    }

    g_activeVirtualTouches[slot] = ActiveVirtualTouch{};
}

bool IsValidAutoCombatTarget(int characterIndex)
{
    if (!IsVirtualPadAvailable()
        || CharactersClient == nullptr
        || characterIndex < 0
        || characterIndex >= MAX_CHARACTERS_CLIENT)
    {
        return false;
    }

    // Reject self-attack: check both by pointer AND by index.
    // Hero pointer can move to a random slot (HeroIndex = rand()),
    // so we must use GetHeroCharacterIndex() as the authoritative check.
    const int heroIdx = GetHeroCharacterIndex();
    CHARACTER* target = &CharactersClient[characterIndex];
    if (target == nullptr || target == Hero || characterIndex == heroIdx)
    {
        return false;
    }

    if (target->Dead > 0
        || !target->Object.Live
        || target->Object.HiddenMesh == -2
        || target->Object.Alpha <= 0.05f)
    {
        return false;
    }

    const int kind = target->Object.Kind;
    if (kind != KIND_MONSTER && kind != KIND_PLAYER)
    {
        return false;
    }

    return true;
}

bool IsTargetAttackable(int characterIndex)
{
    if (!IsValidAutoCombatTarget(characterIndex))
    {
        return false;
    }

    const int previousSelection = SelectedCharacter;
    SelectedCharacter = characterIndex;
    const bool canAttack = CheckAttack();
    SelectedCharacter = previousSelection;
    return canAttack;
}

int FindNearestAttackableTarget()
{
    if (!IsVirtualPadAvailable() || CharactersClient == nullptr)
    {
        return -1;
    }

    int bestIndex = -1;
    float bestDist2 = std::numeric_limits<float>::max();
    bool bestIsMonster = false;

    for (int i = 0; i < MAX_CHARACTERS_CLIENT; ++i)
    {
        CHARACTER* c = &CharactersClient[i];
        if (!IsTargetAttackable(i))
        {
            continue;
        }

        const bool isMonster = (c->Object.Kind == KIND_MONSTER);
        const float dx = static_cast<float>(c->PositionX - Hero->PositionX);
        const float dy = static_cast<float>(c->PositionY - Hero->PositionY);
        const float dist2 = (dx * dx) + (dy * dy);

        if (bestIndex < 0
            || (!bestIsMonster && isMonster)
            || (bestIsMonster == isMonster && dist2 < bestDist2))
        {
            bestIndex = i;
            bestDist2 = dist2;
            bestIsMonster = isMonster;
        }
    }

    return bestIndex;
}

int FindNearestAttackableMonsterTarget(bool limitToAcquireRange)
{
    if (!IsVirtualPadAvailable() || CharactersClient == nullptr)
    {
        return -1;
    }

    int bestIndex = -1;
    float bestDist2 = std::numeric_limits<float>::max();

    for (int i = 0; i < MAX_CHARACTERS_CLIENT; ++i)
    {
        CHARACTER* c = &CharactersClient[i];
        if (c == nullptr || c->Object.Kind != KIND_MONSTER)
        {
            continue;
        }
        if (limitToAcquireRange && !IsWithinVirtualAutoAcquireRange(i))
        {
            continue;
        }
        if (!IsTargetAttackable(i))
        {
            continue;
        }

        const float dx = static_cast<float>(c->PositionX - Hero->PositionX);
        const float dy = static_cast<float>(c->PositionY - Hero->PositionY);
        const float dist2 = (dx * dx) + (dy * dy);

        if (bestIndex < 0 || dist2 < bestDist2)
        {
            bestIndex = i;
            bestDist2 = dist2;
        }
    }

    return bestIndex;
}

int FindNearestVisibleMonsterTarget(bool requireAttackable)
{
    if (!IsVirtualPadAvailable() || CharactersClient == nullptr)
    {
        return -1;
    }

    int bestIndex = -1;
    float bestDist2 = std::numeric_limits<float>::max();

    for (int i = 0; i < MAX_CHARACTERS_CLIENT; ++i)
    {
        CHARACTER* c = &CharactersClient[i];
        if (c == nullptr
            || c->Object.Kind != KIND_MONSTER
            || !c->Object.Visible)
        {
            continue;
        }

        if (requireAttackable)
        {
            if (!IsTargetAttackable(i))
            {
                continue;
            }
        }
        else if (!IsValidAutoCombatTarget(i))
        {
            continue;
        }

        const float dx = static_cast<float>(c->PositionX - Hero->PositionX);
        const float dy = static_cast<float>(c->PositionY - Hero->PositionY);
        const float dist2 = (dx * dx) + (dy * dy);

        if (bestIndex < 0 || dist2 < bestDist2)
        {
            bestIndex = i;
            bestDist2 = dist2;
        }
    }

    return bestIndex;
}

int FindNearestMonsterTarget(bool limitToAcquireRange)
{
    if (!IsVirtualPadAvailable() || CharactersClient == nullptr)
    {
        return -1;
    }

    int bestIndex = -1;
    float bestDist2 = std::numeric_limits<float>::max();

    for (int i = 0; i < MAX_CHARACTERS_CLIENT; ++i)
    {
        CHARACTER* c = &CharactersClient[i];
        if (!IsValidAutoCombatTarget(i) || c->Object.Kind != KIND_MONSTER)
        {
            continue;
        }
        if (limitToAcquireRange && !IsWithinVirtualAutoAcquireRange(i))
        {
            continue;
        }

        const float dx = static_cast<float>(c->PositionX - Hero->PositionX);
        const float dy = static_cast<float>(c->PositionY - Hero->PositionY);
        const float dist2 = (dx * dx) + (dy * dy);

        if (bestIndex < 0 || dist2 < bestDist2)
        {
            bestIndex = i;
            bestDist2 = dist2;
        }
    }

    return bestIndex;
}

int GetHeroCharacterIndex()
{
    if (!IsVirtualPadAvailable() || CharactersClient == nullptr || Hero == nullptr)
    {
        return -1;
    }

    if (Hero < &CharactersClient[0] || Hero >= (&CharactersClient[0] + MAX_CHARACTERS_CLIENT))
    {
        return -1;
    }

    return static_cast<int>(Hero - &CharactersClient[0]);
}

void EnsureCombatTarget()
{
    if (!IsVirtualPadAvailable())
    {
        return;
    }

    if (IsTargetAttackable(SelectedCharacter))
    {
        return;
    }

    const int nearest = FindNearestAttackableTarget();
    if (nearest >= 0)
    {
        SelectedCharacter = nearest;
    }
    else
    {
        SelectedCharacter = -1;
    }
}

void EnsureOffensiveSkillTarget()
{
    if (!IsVirtualPadAvailable() || CharactersClient == nullptr)
    {
        return;
    }

    const int heroIdx = GetHeroCharacterIndex();
    if (heroIdx >= 0 && SelectedCharacter == heroIdx)
    {
        SelectedCharacter = -1;
    }

    const int nearestVisibleAttackableMonster = FindNearestVisibleMonsterTarget(true);
    if (nearestVisibleAttackableMonster >= 0)
    {
        SelectedCharacter = nearestVisibleAttackableMonster;
        return;
    }

    const int nearestVisibleMonster = FindNearestVisibleMonsterTarget(false);
    if (nearestVisibleMonster >= 0)
    {
        SelectedCharacter = nearestVisibleMonster;
        return;
    }

    const int nearestAttackableMonster = FindNearestAttackableMonsterTarget(false);
    if (nearestAttackableMonster >= 0)
    {
        SelectedCharacter = nearestAttackableMonster;
        return;
    }

    const int nearestMonster = FindNearestMonsterTarget(false);
    if (nearestMonster >= 0)
    {
        SelectedCharacter = nearestMonster;
        return;
    }

    EnsureCombatTarget();
}

void RefreshSelectedCharacterAtMouse()
{
    const int previousTarget = SelectedCharacter;

    if (!IsVirtualPadAvailable()
        || Hero == nullptr
        || CharactersClient == nullptr
        || MouseOnWindow
        || g_pNewUISystem == nullptr
        || g_pNewUISystem->CheckMouseUse())
    {
        SelectedCharacter = -1;
        return;
    }

    const int heroIdx = GetHeroCharacterIndex();
    if (heroIdx >= 0 && SelectedCharacter == heroIdx)
    {
        SelectedCharacter = -1;
    }

    int candidate = SelectCharacter(KIND_MONSTER | KIND_EDIT);
    if (candidate == -1)
    {
        candidate = SelectCharacter(KIND_PLAYER);
    }

    if (candidate < 0 || !IsTargetAttackable(candidate))
    {
        if (IsTargetAttackable(previousTarget)
            && IsWithinVirtualAutoAcquireRange(previousTarget))
        {
            SelectedCharacter = previousTarget;
        }
        else
        {
            SelectedCharacter = -1;
        }
        return;
    }

    SelectedCharacter = candidate;
}

void EnsureNormalAttackTarget()
{
    if (!IsVirtualPadAvailable())
    {
        return;
    }

    const int heroIdx = GetHeroCharacterIndex();
    if (heroIdx >= 0 && SelectedCharacter == heroIdx)
    {
        SelectedCharacter = -1;
    }

    if (IsTargetAttackable(SelectedCharacter)
        && IsWithinVirtualAutoAcquireRange(SelectedCharacter))
    {
        return;
    }

    RefreshSelectedCharacterAtMouse();
}

bool IsSupportOrSelfSkill(ActionSkillType skillType)
{
    return IsCorrectSkillType_Buff(skillType) || IsCorrectSkillType_FrendlySkill(skillType);
}

static bool TrySendHeroMeleeAttackPacket(int targetIndex)
{
    if (targetIndex < 0
        || targetIndex >= MAX_CHARACTERS_CLIENT
        || Hero == nullptr
        || CharactersClient == nullptr)
    {
        return false;
    }

    CHARACTER* c = Hero;
    OBJECT* o = &c->Object;
    float range = 1.8f;
    const int rightWeapon = CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type;
    if (rightWeapon >= ITEM_SPEAR && rightWeapon < ITEM_SPEAR + MAX_ITEM_INDEX)
    {
        range = 2.2f;
    }

    if (gCharacterManager.GetEquipedBowType() != BOWTYPE_NONE)
    {
        range = 6.0f;
    }

    if (!CheckTile(c, o, range))
    {
        return false;
    }

    TargetX = static_cast<int>(CharactersClient[targetIndex].Object.Position[0] / TERRAIN_SCALE);
    TargetY = static_cast<int>(CharactersClient[targetIndex].Object.Position[1] / TERRAIN_SCALE);
    SetPlayerAttack(c);
    c->AttackTime = 1;
    VectorCopy(CharactersClient[targetIndex].Object.Position, c->TargetPosition);
    o->Angle[2] = CreateAngle(
        o->Position[0],
        o->Position[1],
        c->TargetPosition[0],
        c->TargetPosition[1]);
    LetHeroStop();
    c->Movement = false;
    c->TargetCharacter = targetIndex;
    c->Skill = 0;
    const int dir = static_cast<int>(((BYTE)((Hero->Object.Angle[2] + 22.5f) / 360.f * 8.f + 1.f)) % 8);
    static uint32_t s_lastMeleeAttackSendMs = 0;
    const uint32_t nowMs = MU_MobileGetTicks();
    if (s_lastMeleeAttackSendMs != 0 && (nowMs - s_lastMeleeAttackSendMs) < kVirtualMeleeAttackSendMs)
    {
        // In range but on send cooldown — do not fall through to walk.
        return true;
    }

    s_lastMeleeAttackSendMs = nowMs;
    TakumiSendMeleeAttack(CharactersClient[targetIndex].Key, static_cast<BYTE>(dir));
    return true;
}

bool TriggerVirtualNormalAutoAttack()
{
    if (!IsVirtualPadAvailable()
        || Hero == nullptr
        || CharactersClient == nullptr
        || SelectedCharacter < 0
        || SelectedCharacter >= MAX_CHARACTERS_CLIENT
        || !IsValidAutoCombatTarget(SelectedCharacter)
        || !CheckAttack())
    {
        return false;
    }

    // Snapshot SelectedCharacter into a local to guard against the global
    // being modified by another code path (network thread, UI, etc.)
    // between validation above and the actual memory access below.
    const int localTarget = SelectedCharacter;

    // Double-check: reject self-attack by index (Hero can be at any random
    // slot after ReceiveJoinMapServer sets HeroIndex = rand()).
    const int heroIdx = GetHeroCharacterIndex();
    if (localTarget < 0 || localTarget >= MAX_CHARACTERS_CLIENT || localTarget == heroIdx)
    {
        return false;
    }

    // Re-validate target is still alive right before accessing its data.
    // A monster can die between IsValidAutoCombatTarget() and this point.
    const CHARACTER& targetChar = CharactersClient[localTarget];
    if (targetChar.Dead > 0 || !targetChar.Object.Live)
    {
        return false;
    }

    CHARACTER* c = Hero;
    OBJECT* o = &c->Object;

    Attacking = 1;
    c->MovementType = MOVEMENT_ATTACK;
    ActionTarget = localTarget;
    TargetX = static_cast<int>(CharactersClient[ActionTarget].Object.Position[0] / TERRAIN_SCALE);
    TargetY = static_cast<int>(CharactersClient[ActionTarget].Object.Position[1] / TERRAIN_SCALE);

    if (!CheckWall(c->PositionX, c->PositionY, TargetX, TargetY))
    {
        return false;
    }

    if (TrySendHeroMeleeAttackPacket(localTarget))
    {
        Action(c, o, true);
        return true;
    }

    if (!PathFinding2(c->PositionX, c->PositionY, TargetX, TargetY, &c->Path))
    {
        if (!CheckArrow())
        {
            return false;
        }
        Action(c, o, true);
        return true;
    }

    const bool rangedAutoAttack =
        (gCharacterManager.GetEquipedBowType() != BOWTYPE_NONE)
        || (c->MonsterIndex == 0);

    if (rangedAutoAttack)
    {
        if (!CheckArrow())
        {
            return false;
        }
        Action(c, o, true);
    }
    else
    {
        SendMove(c, o);
    }

    return true;
}

void TriggerVirtualCombat(bool useNormalAttack, int skillSlot)
{
    if (!IsVirtualPadAvailable() || Hero->Dead > 0)
    {
        return;
    }

    static uint32_t s_lastVirtualCombatLog = 0;
    const uint32_t nowMs = MU_MobileGetTicks();

    if (useNormalAttack)
    {
        EnsureNormalAttackTarget();

        // Reject hero index â€” after ReceiveJoinMapServer, HeroIndex is random
        // so SelectedCharacter could accidentally equal it.
        const int heroIdx = GetHeroCharacterIndex();
        if (SelectedCharacter == heroIdx && heroIdx >= 0)
        {
            SelectedCharacter = -1;
        }

        const int selectedBeforeAttack = SelectedCharacter;
        if (selectedBeforeAttack < 0)
        {
            LOGI(
                "VirtualPad: fire skipped mode=normal reason=no-target");
            return;
        }

        const bool triggered = TriggerVirtualNormalAutoAttack();
        if ((nowMs - s_lastVirtualCombatLog) > 150)
        {
            s_lastVirtualCombatLog = nowMs;
            LOGI(
                "VirtualPad: fire mode=normal target=%d triggered=%d",
                selectedBeforeAttack,
                triggered ? 1 : 0);
        }
        return;
    }

    LoadVirtualSkillSlots();

    if (skillSlot < 0 || skillSlot >= kVirtualSkillSlotCount)
    {
        return;
    }

    if (!IsAssignableVirtualSkillIndex(g_virtualSkillSlots[skillSlot]))
    {
        const int currentSkillType = (IsValidSkillIndex(Hero->CurrentSkill) && CharacterAttribute != nullptr)
            ? CharacterAttribute->Skill[Hero->CurrentSkill]
            : -1;
        LOGI(
            "VirtualPad: slot%d empty; currentSkill=%d skillType=%d",
            skillSlot,
            Hero->CurrentSkill,
            currentSkillType);
        return;
    }

    const int previousSkillIndex = Hero->CurrentSkill;
    Hero->CurrentSkill = static_cast<BYTE>(g_virtualSkillSlots[skillSlot]);
    const int rawSkillType = CharacterAttribute->Skill[Hero->CurrentSkill];
    if (rawSkillType <= 0 || rawSkillType >= MAX_SKILLS)
    {
        LOGW("VirtualPad: invalid skillType=%d skillIndex=%d", rawSkillType, Hero->CurrentSkill);
        Hero->CurrentSkill = static_cast<BYTE>(previousSkillIndex);
        return;
    }

    const ActionSkillType skillType = static_cast<ActionSkillType>(rawSkillType);
    const bool supportSkill = IsSupportOrSelfSkill(skillType);
    if (!supportSkill)
    {
        EnsureOffensiveSkillTarget();
    }
    else
    {
        // Buff/friendly skills are safer when explicitly bound to self target.
        SelectedCharacter = GetHeroCharacterIndex();
    }

    const int selectedBeforeAttack = SelectedCharacter;
    if (supportSkill && selectedBeforeAttack < 0)
    {
        LOGI(
            "VirtualPad: fire skipped mode=skill slot=%d skillIndex=%d skillType=%d reason=self-target-unavailable",
            skillSlot,
            Hero->CurrentSkill,
            static_cast<int>(skillType));
        Hero->CurrentSkill = static_cast<BYTE>(previousSkillIndex);
        return;
    }

    if (!supportSkill && selectedBeforeAttack < 0)
    {
        LOGI(
            "VirtualPad: fire skipped mode=skill slot=%d skillIndex=%d skillType=%d reason=no-target",
            skillSlot,
            Hero->CurrentSkill,
            static_cast<int>(skillType));
        Hero->CurrentSkill = static_cast<BYTE>(previousSkillIndex);
        return;
    }

    const bool isBuffType = IsCorrectSkillType_Buff(skillType) == TRUE;
    const bool isFriendlyType = IsCorrectSkillType_FrendlySkill(skillType) == TRUE;
    if ((nowMs - s_lastVirtualCombatLog) > 150)
    {
        s_lastVirtualCombatLog = nowMs;
        LOGI(
            "VirtualPad: fire mode=skill slot=%d skillIndex=%d skillType=%d target=%d",
            skillSlot,
            Hero->CurrentSkill,
            static_cast<int>(skillType),
            selectedBeforeAttack);
        LOGI(
            "VirtualPad: skill meta index=%d type=%d support=%d buff=%d friendly=%d target=%d",
            Hero->CurrentSkill,
            static_cast<int>(skillType),
            supportSkill ? 1 : 0,
            isBuffType ? 1 : 0,
            isFriendlyType ? 1 : 0,
            selectedBeforeAttack);
    }

    MouseRButtonPop = false;
    MouseRButtonPush = true;
    MouseRButton = true;
    Attack(Hero);
    MouseRButtonPush = false;
    MouseRButton = false;

    Hero->CurrentSkill = static_cast<BYTE>(previousSkillIndex);
}

bool AndroidTriggerNormalAttackButtonInternal()
{
    if (!kShowVirtualAttackButton)
    {
        return false;
    }

    LoadVirtualSkillSlots();
    if (IsAssignableVirtualSkillIndex(g_virtualSkillSlots[kVirtualAttackSkillSlot]))
    {
        TriggerVirtualCombat(false, kVirtualAttackSkillSlot);
        return true;
    }

    const int selectedBefore = SelectedCharacter;
    TriggerVirtualCombat(true, -1);
    return SelectedCharacter != -1 || selectedBefore != SelectedCharacter;
}

bool AndroidTriggerHotKeySkillTapInternal(int hotKeySkillIndex)
{
    if (!IsVirtualPadAvailable()
        || Hero == nullptr
        || CharacterAttribute == nullptr
        || Hero->Dead > 0)
    {
        return false;
    }

    if (hotKeySkillIndex >= AT_PET_COMMAND_DEFAULT && hotKeySkillIndex < AT_PET_COMMAND_END)
    {
        if (Hero->m_pPet == nullptr)
        {
            return false;
        }

        Hero->CurrentSkill = static_cast<BYTE>(hotKeySkillIndex);
        return true;
    }

    if (!IsValidSkillIndex(hotKeySkillIndex))
    {
        return false;
    }

    const int rawSkillType = CharacterAttribute->Skill[hotKeySkillIndex];
    if (rawSkillType <= 0 || rawSkillType >= MAX_SKILLS)
    {
        return false;
    }

    const int previousSkillIndex = Hero->CurrentSkill;
    const ActionSkillType skillType = static_cast<ActionSkillType>(rawSkillType);
    const bool supportSkill = IsSupportOrSelfSkill(skillType);

    if (supportSkill)
    {
        SelectedCharacter = GetHeroCharacterIndex();
    }
    else
    {
        EnsureOffensiveSkillTarget();
    }

    if (SelectedCharacter < 0)
    {
        LOGI(
            "VirtualPad: hotkey skill skipped skillIndex=%d skillType=%d reason=no-target support=%d",
            hotKeySkillIndex,
            rawSkillType,
            supportSkill ? 1 : 0);
        return false;
    }

    const float skillDistance = gSkillManager.GetSkillDistance(skillType, Hero);
    const int executeResult = ExecuteSkill(Hero, skillType, skillDistance);
    const bool startedSkillMove = Hero->Movement && Hero->MovementType == MOVEMENT_SKILL;

    LOGI(
        "VirtualPad: hotkey skill skillIndex=%d skillType=%d target=%d result=%d move=%d movementType=%d visible=%d",
        hotKeySkillIndex,
        rawSkillType,
        SelectedCharacter,
        executeResult,
        startedSkillMove ? 1 : 0,
        Hero->MovementType,
        (SelectedCharacter >= 0 && SelectedCharacter < MAX_CHARACTERS_CLIENT && CharactersClient[SelectedCharacter].Object.Visible) ? 1 : 0);

    Hero->CurrentSkill = static_cast<BYTE>(previousSkillIndex);
    return executeResult != 0 || startedSkillMove;

}

int HitTestVirtualAttackButton(float uiX, float uiY)
{
    if (!kShowVirtualAttackButton || !IsVirtualPadAvailable())
    {
        return -1;
    }

    const VirtualButtonLayout& button = kVirtualButtons[kVirtualAttackButton];
    const float dx = uiX - button.cx;
    const float dy = uiY - button.cy;
    const float hitRadius = GetVirtualButtonHitRadius(kVirtualAttackButton);
    return ((dx * dx) + (dy * dy)) <= (hitRadius * hitRadius)
        ? kVirtualAttackButton
        : -1;
}

int HitTestVirtualSkillButton(float uiX, float uiY)
{
    if (!kShowVirtualSkillButtons || !IsVirtualPadAvailable())
    {
        return -1;
    }

    for (int visualSlot = 0; visualSlot < kVirtualVisibleSkillButtonCount; ++visualSlot)
    {
        const int buttonIndex = kVirtualSkillButtonBase + visualSlot;
        const VirtualButtonLayout& button = kVirtualButtons[buttonIndex];
        const float dx = uiX - button.cx;
        const float dy = uiY - button.cy;
        const float hitRadius = GetVirtualButtonHitRadius(buttonIndex);
        if (((dx * dx) + (dy * dy)) <= (hitRadius * hitRadius))
        {
            return buttonIndex;
        }
    }

    return -1;
}

bool HandleVirtualFingerDown(const SDL_TouchFingerEvent& touch)
{
    float uiX = 0.0f;
    float uiY = 0.0f;
    TouchToVirtualUi(touch, uiX, uiY);

    // Chat input must receive tap priority so Android IME pops up reliably.
    if (g_pNewUISystem != nullptr
        && g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHATINPUTBOX)
        && g_pChatInputBox != nullptr)
    {
        const bool focused = FocusVirtualChatInputAt(uiX, uiY);
#if !defined(MU_ANDROID_DISABLE_LOG)
        LOGI(
            "CHATIME tap ui=(%.1f,%.1f) focused=%d hasFocusedText=%d sdlTextInput=%d",
            uiX,
            uiY,
            focused ? 1 : 0,
            AndroidHasFocusedTextInput() ? 1 : 0,
            MU_MobileIsTextInputActive() ? 1 : 0);
#endif
        if (focused)
        {
            return true;
        }
    }

    if (CUIMng::Instance().m_MsgWin.AndroidTryFocusDeleteResidentInput(uiX, uiY))
    {
        return true;
    }

#if defined(__ANDROID__) || defined(MU_IOS)
    if (kUseLegacyMainHud
        && g_pSkillList != nullptr
        && g_pSkillList->TryToggleSkillPickerAtTouch(uiX, uiY))
    {
        return true;
    }
#endif

    // Season3B modal (guild invite, stat points, mix, …): do not steal the touch for
    // virtual attack/skill/joystick — let MouseLButton* reach g_MessageBox.
    if (g_MessageBox != nullptr && !g_MessageBox->IsEmpty())
    {
        return false;
    }

    if (!IsVirtualPadAvailable())
    {
        return false;
    }

    const int zoomButton = HitTestVirtualZoomButton(uiX, uiY);
    if (zoomButton >= 0)
    {
        return HandleVirtualZoomButtonTap(zoomButton);
    }

    if (HandleVirtualTopControlTap(uiX, uiY))
    {
        return true;
    }

    if (kUseLegacyMainHud)
    {
        return HandleVirtualJoystickFingerDown(touch);
    }

    if (HitTestVirtualAttackButton(uiX, uiY) == kVirtualAttackButton)
    {
        const uint32_t nowMs = MU_MobileGetTicks();
        const int pendingSkill = GetPendingVirtualAssignSkillIndex(nowMs);
        if (IsAssignableVirtualSkillIndex(pendingSkill))
        {
            SetVirtualSkillSlot(kVirtualAttackSkillSlot, pendingSkill);
            DeactivateVirtualAssignMode("assigned-attack-slot");
            g_virtualAssignPickerSkillIndex = -1;
            g_virtualAssignConsumedForPickerSkill = true;
            g_virtualAssignConsumedForPickerSession = true;
            return true;
        }

        const int slot = AcquireActiveVirtualTouchSlot(touch.fingerId);
        if (slot >= 0)
        {
            g_activeVirtualTouches[slot].fingerId = touch.fingerId;
            g_activeVirtualTouches[slot].button = kVirtualAttackButton;
            g_activeVirtualTouches[slot].downMs = nowMs;
            g_activeVirtualTouches[slot].lastRepeatMs = g_activeVirtualTouches[slot].downMs;
        }

        AndroidTriggerNormalAttackButtonInternal();
        return true;
    }

    const int skillButton = HitTestVirtualSkillButton(uiX, uiY);
    if (skillButton >= kVirtualSkillButtonBase)
    {
        const int skillSlot = skillButton - kVirtualSkillButtonBase + 1;
        const uint32_t nowMs = MU_MobileGetTicks();
        const int pendingSkill = GetPendingVirtualAssignSkillIndex(nowMs);
        if (IsAssignableVirtualSkillIndex(pendingSkill))
        {
            SetVirtualSkillSlot(skillSlot, pendingSkill);
            DeactivateVirtualAssignMode("assigned-slot");
            g_virtualAssignPickerSkillIndex = -1;
            g_virtualAssignConsumedForPickerSkill = true;
            g_virtualAssignConsumedForPickerSession = true;
        }
        else
        {
            const int activeSlot = AcquireActiveVirtualTouchSlot(touch.fingerId);
            if (activeSlot >= 0)
            {
                g_activeVirtualTouches[activeSlot].fingerId = touch.fingerId;
                g_activeVirtualTouches[activeSlot].button = skillButton;
                g_activeVirtualTouches[activeSlot].downMs = nowMs;
                g_activeVirtualTouches[activeSlot].lastRepeatMs = nowMs;
            }
            TriggerVirtualCombat(false, skillSlot);
        }
        return true;
    }

    return HandleVirtualJoystickFingerDown(touch);
}

bool HandleVirtualFingerMotion(const SDL_TouchFingerEvent& touch)
{
    if (kUseLegacyMainHud)
    {
        return HandleVirtualJoystickFingerMotion(touch);
    }

    if (FindActiveVirtualTouchSlot(touch.fingerId) >= 0)
    {
        return true;
    }

    return HandleVirtualJoystickFingerMotion(touch);
}

bool HandleVirtualFingerUp(const SDL_TouchFingerEvent& touch)
{
    if (kUseLegacyMainHud)
    {
        return HandleVirtualJoystickFingerUp(touch);
    }

    const int slot = FindActiveVirtualTouchSlot(touch.fingerId);
    if (slot >= 0)
    {
        ClearActiveVirtualTouchSlot(slot);
        return true;
    }

    return HandleVirtualJoystickFingerUp(touch);
}

bool IsVirtualButtonPressed(int button)
{
    for (const ActiveVirtualTouch& active : g_activeVirtualTouches)
    {
        if (active.button == button && active.fingerId != static_cast<SDL_FingerID>(-1))
        {
            return true;
        }
    }
    return false;
}

void UpdateVirtualPadHolds()
{
    if (!IsVirtualPadAvailable())
    {
        for (int i = 0; i < static_cast<int>(g_activeVirtualTouches.size()); ++i)
        {
            ClearActiveVirtualTouchSlot(i);
        }
        ClearVirtualJoystick();
        return;
    }

    if (kUseLegacyMainHud)
    {
        for (int i = 0; i < static_cast<int>(g_activeVirtualTouches.size()); ++i)
        {
            if (g_activeVirtualTouches[i].fingerId != static_cast<SDL_FingerID>(-1))
            {
                ClearActiveVirtualTouchSlot(i);
            }
        }

        ApplyVirtualJoystickMovement();
        return;
    }

    const uint32_t nowMs = MU_MobileGetTicks();
    for (ActiveVirtualTouch& active : g_activeVirtualTouches)
    {
        if (active.fingerId == static_cast<SDL_FingerID>(-1)
            || active.button != kVirtualAttackButton)
        {
            continue;
        }

        if ((nowMs - active.lastRepeatMs) >= kVirtualAttackRepeatMs)
        {
            active.lastRepeatMs = nowMs;
            AndroidTriggerNormalAttackButtonInternal();
        }
    }

    ApplyVirtualJoystickMovement();
}

void UpdateVirtualProximityCombat()
{
    if (!IsVirtualPadAvailable() || Hero == nullptr || Hero->Dead > 0)
    {
        return;
    }

    if (IsVirtualButtonPressed(kVirtualAttackButton))
    {
        return;
    }

    // PC-style: do not auto-lock nearest monster while moving — only attack under cursor / attack button.
    if (MU_AndroidShouldSuppressCombatTargeting())
    {
        return;
    }

    static uint32_t s_lastProximityAttackMs = 0;
    const uint32_t nowMs = MU_MobileGetTicks();
    if (s_lastProximityAttackMs != 0 && (nowMs - s_lastProximityAttackMs) < kVirtualProximityAttackMs)
    {
        return;
    }

    RefreshSelectedCharacterAtMouse();
    const int target = SelectedCharacter;
    if (target < 0 || !IsWithinVirtualAutoAcquireRange(target))
    {
        return;
    }

    if (!TriggerVirtualNormalAutoAttack())
    {
        return;
    }

    s_lastProximityAttackMs = nowMs;
}

float GetVirtualUiScaleX()
{
    return static_cast<float>(WindowWidth) / 640.0f;
}

float GetVirtualUiScaleY()
{
    return static_cast<float>(WindowHeight) / 480.0f;
}

float GetVirtualUiUniformScale()
{
    return std::min(GetVirtualUiScaleX(), GetVirtualUiScaleY());
}

float UiToScreenX(float uiX)
{
    return uiX * GetVirtualUiScaleX();
}

float UiToScreenY(float uiY)
{
    return uiY * GetVirtualUiScaleY();
}

float UiToScreenUniform(float uiLen)
{
    return uiLen * GetVirtualUiUniformScale();
}

void DrawVirtualCircle(float uiX, float uiY, float uiRadius, float red, float green, float blue, float alpha, bool filled)
{
    if (alpha <= 0.001f || uiRadius <= 0.5f)
    {
        return;
    }

    const float centerX = UiToScreenX(uiX);
    const float centerY = static_cast<float>(WindowHeight) - UiToScreenY(uiY);
    const float radiusScreen = UiToScreenUniform(uiRadius);
    constexpr int kSegments = 36;
    constexpr float kPi = 3.14159265358979323846f;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_TEXTURE_2D);
    glColor4f(red, green, blue, alpha);
    glBegin(filled ? GL_TRIANGLE_FAN : GL_LINE_LOOP);
    if (filled)
    {
        glVertex2f(centerX, centerY);
    }

    for (int i = 0; i <= kSegments; ++i)
    {
        const float angle = (static_cast<float>(i) / static_cast<float>(kSegments))
            * 2.0f * kPi;
        const float px = centerX + std::cos(angle) * radiusScreen;
        const float py = centerY + std::sin(angle) * radiusScreen;
        glVertex2f(px, py);
    }
    glEnd();
}

// Highlight overlay for a sub-range of a horizontal bar (no background).
void DrawVirtualBarFillRange(float uiLeft, float uiTop, float uiW, float uiH,
                             float startRatio, float endRatio,
                             float fillR, float fillG, float fillB, float fillA)
{
    const float start = std::clamp(startRatio, 0.0f, 1.0f);
    const float end = std::clamp(endRatio, 0.0f, 1.0f);
    if (end <= start + 0.0001f)
    {
        return;
    }

    const float sx0 = UiToScreenX(uiLeft + uiW * start);
    const float sx1 = UiToScreenX(uiLeft + uiW * end);
    const float syT = static_cast<float>(WindowHeight) - UiToScreenY(uiTop);
    const float syB = static_cast<float>(WindowHeight) - UiToScreenY(uiTop + uiH);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(fillR, fillG, fillB, fillA);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(sx0, syB);
    glVertex2f(sx1, syB);
    glVertex2f(sx1, syT);
    glVertex2f(sx0, syT);
    glEnd();
}

// â”€â”€ Horizontal status bar (fill from left to right) â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
// uiLeft/uiTop: UI-space top-left.  uiW/uiH: virtual width/height.
// ratio: 0=empty, 1=full.
void DrawVirtualBarH(float uiLeft, float uiTop, float uiW, float uiH, float ratio,
                     float fillR, float fillG, float fillB,
                     float bgR, float bgG, float bgB)
{
    const float sx  = UiToScreenX(uiLeft);
    const float sw  = UiToScreenX(uiLeft + uiW) - sx;
    const float syT = static_cast<float>(WindowHeight) - UiToScreenY(uiTop);
    const float syB = static_cast<float>(WindowHeight) - UiToScreenY(uiTop + uiH);
    // syB < syT in GL (Y-up)

    // Background â€” solid dark
    glColor4f(bgR, bgG, bgB, 1.0f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(sx,      syB);
    glVertex2f(sx + sw, syB);
    glVertex2f(sx + sw, syT);
    glVertex2f(sx,      syT);
    glEnd();

    // Filled portion (left â†’ right) â€” solid bright color
    const float fillW = sw * std::clamp(ratio, 0.0f, 1.0f);
    if (fillW > 0.5f)
    {
        glColor4f(fillR, fillG, fillB, 1.0f);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(sx,         syB);
        glVertex2f(sx + fillW, syB);
        glVertex2f(sx + fillW, syT);
        glVertex2f(sx,         syT);
        glEnd();
    }

    // White border
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(sx + 0.5f,       syB + 0.5f);
    glVertex2f(sx + sw - 0.5f,  syB + 0.5f);
    glVertex2f(sx + sw - 0.5f,  syT - 0.5f);
    glVertex2f(sx + 0.5f,       syT - 0.5f);
    glEnd();
}

// â”€â”€ UI texture loader (called once after GL context is ready) â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
// MuMainNativeActivity.extractGameAssets() copies assets/ui/*.png to the external
// files dir before native code runs. fopen("ui/map.png") therefore finds the
// file on the real filesystem (cwd = /sdcard/.../files).
// stbi_load_from_memory is used for decoding (STBI_NO_STDIO is set elsewhere).
static void ComputeTextureContentMetrics(
    int w,
    int h,
    const stbi_uc* pixels,
    float& outCenterU,
    float& outCenterV,
    float& outVisibleFrac)
{
    outCenterU = 0.5f;
    outCenterV = 0.5f;
    outVisibleFrac = 1.0f;
    if (w <= 0 || h <= 0 || pixels == nullptr)
    {
        return;
    }

    int minX = w;
    int minY = h;
    int maxX = 0;
    int maxY = 0;
    long long sumX = 0;
    long long sumY = 0;
    int count = 0;
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            const int alpha = pixels[(y * w + x) * 4 + 3];
            if (alpha <= 16)
            {
                continue;
            }

            minX = std::min(minX, x);
            minY = std::min(minY, y);
            maxX = std::max(maxX, x);
            maxY = std::max(maxY, y);
            sumX += x;
            sumY += y;
            count++;
        }
    }

    if (count <= 0)
    {
        return;
    }

    const float centroidX = static_cast<float>(sumX) / static_cast<float>(count);
    const float centroidY = static_cast<float>(sumY) / static_cast<float>(count);
    outCenterU = centroidX / static_cast<float>(w);
    outCenterV = centroidY / static_cast<float>(h);

    const float bboxW = static_cast<float>(maxX - minX + 1);
    const float bboxH = static_cast<float>(maxY - minY + 1);
    const float ar = static_cast<float>(w) / static_cast<float>(h);
    const float spanW = bboxW / static_cast<float>(w);
    const float spanH = (bboxH / static_cast<float>(h)) / std::max(ar, 0.001f);
    outVisibleFrac = std::max(spanW, spanH);
}

static UITexture LoadUITextureAsset(const char* assetPath)
{
    UITexture tex;

    std::ifstream file(assetPath, std::ios::binary | std::ios::ate);
    if (!file)
    {
        LOGE("LoadUITextureAsset: fopen failed for '%s'", assetPath);
        return tex;
    }

    const std::streamsize size = file.tellg();
    if (size <= 0)
    {
        LOGE("LoadUITextureAsset: zero size for '%s'", assetPath);
        return tex;
    }

    std::vector<stbi_uc> buf(static_cast<size_t>(size));
    file.seekg(0, std::ios::beg);
    if (!file.read(reinterpret_cast<char*>(buf.data()), size))
    {
        LOGE("LoadUITextureAsset: read failed for '%s'", assetPath);
        return tex;
    }

    stbi_set_flip_vertically_on_load(1);   // flip so (0,0) = bottom-left for GL
    int w = 0, h = 0, comp = 0;
    stbi_uc* pixels = stbi_load_from_memory(buf.data(), static_cast<int>(size), &w, &h, &comp, 4);
    stbi_set_flip_vertically_on_load(0);   // do not leak flip flag into OZJ / GlobalBitmap
    if (!pixels)
    {
        LOGE("LoadUITextureAsset: stbi decode failed for '%s'", assetPath);
        return tex;
    }

    glGenTextures(1, &tex.id);
    glBindTexture(GL_TEXTURE_2D, tex.id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    ComputeTextureContentMetrics(
        w,
        h,
        pixels,
        tex.contentCenterU,
        tex.contentCenterV,
        tex.contentVisibleFrac);
    stbi_image_free(pixels);

    tex.w = w; tex.h = h;
    LOGI(
        "LoadUITextureAsset: OK '%s' (%dx%d texId=%u anchor=%.3f,%.3f visibleFrac=%.3f)",
        assetPath,
        w,
        h,
        tex.id,
        tex.contentCenterU,
        tex.contentCenterV,
        tex.contentVisibleFrac);
    return tex;
}

static void EnsureUITextures()
{
    if (g_uiTexturesLoaded) return;
    g_uiTexturesLoaded = true;
    g_uiTex_map       = LoadUITextureAsset("ui/map.png");
    g_uiTex_minimap   = LoadUITextureAsset("ui/minimap.png");
    g_uiTex_attack    = LoadUITextureAsset("ui/attack.png");
    g_uiTex_skillbox  = LoadUITextureAsset("ui/skillbox.png");
    g_uiTex_skillline = LoadUITextureAsset("ui/skillline.png");
    g_uiTex_joystick1 = LoadUITextureAsset("ui/joystick1.png");
    g_uiTex_joystick2 = LoadUITextureAsset("ui/joystick2.png");
    g_uiTex_balo      = LoadUITextureAsset("ui/balo.png");
    g_uiTex_character = LoadUITextureAsset("ui/character.png");
    g_uiTex_setting   = LoadUITextureAsset("ui/setting.png");
}

// Draw a PNG icon at the given UI rect â€” NO background, NO border.
// Renders the texture as-is with correct alpha transparency.
// If the texture hasn't loaded yet, draws nothing.
static void DrawIconButton(float uiX, float uiY, float uiW, float uiH,
                           const UITexture& tex, float alpha = 1.0f,
                           float bgR = 0.0f, float bgG = 0.0f, float bgB = 0.0f)
{
    if (tex.id == 0) return;  // texture not loaded â€” skip entirely

    const float sx  = UiToScreenX(uiX);
    const float sw  = UiToScreenX(uiX + uiW) - sx;
    const float syB = static_cast<float>(WindowHeight) - UiToScreenY(uiY + uiH);
    const float syT = static_cast<float>(WindowHeight) - UiToScreenY(uiY);

    // RenderNumber may have left GL_TEXTURE_2D enabled with an atlas bound â€” reset it.
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Draw PNG texture directly â€” no background, no border
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex.id);
    glColor4f(1.0f, 1.0f, 1.0f, alpha);
    glBegin(GL_TRIANGLE_FAN);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(sx,      syB);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(sx + sw, syB);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(sx + sw, syT);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(sx,      syT);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    // Restore additive blend expected by the rest of the virtual pad
    glBlendFunc(GL_ONE, GL_ONE);
}

static void DrawIconButtonUv(float uiX, float uiY, float uiW, float uiH,
                             const UITexture& tex,
                             float u0, float v0, float uW, float vH,
                             float alpha = 1.0f)
{
    if (tex.id == 0) return;

    const float sx  = UiToScreenX(uiX);
    const float sw  = UiToScreenX(uiX + uiW) - sx;
    const float syB = static_cast<float>(WindowHeight) - UiToScreenY(uiY + uiH);
    const float syT = static_cast<float>(WindowHeight) - UiToScreenY(uiY);

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex.id);
    glColor4f(1.0f, 1.0f, 1.0f, alpha);
    glBegin(GL_TRIANGLE_FAN);
    glTexCoord2f(u0,      v0);      glVertex2f(sx,      syB);
    glTexCoord2f(u0 + uW, v0);      glVertex2f(sx + sw, syB);
    glTexCoord2f(u0 + uW, v0 + vH); glVertex2f(sx + sw, syT);
    glTexCoord2f(u0,      v0 + vH); glVertex2f(sx,      syT);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    glBlendFunc(GL_ONE, GL_ONE);
}

static void DrawIconButtonUvSquare(
    float centerUiX,
    float centerUiY,
    float uiDiameter,
    const UITexture& tex,
    float alpha)
{
    if (tex.id == 0 || uiDiameter <= 0.5f)
    {
        return;
    }

    const float ar = (tex.w > 0 && tex.h > 0)
        ? (static_cast<float>(tex.w) / static_cast<float>(tex.h))
        : 1.0f;
    const float visibleFrac = std::max(tex.contentVisibleFrac, 0.01f);
    const float drawDiameterUi = uiDiameter / visibleFrac;
    const float halfW = UiToScreenUniform(drawDiameterUi * 0.5f);
    const float halfH = halfW / std::max(ar, 0.001f);
    const float sw = halfW * 2.0f;
    const float sh = halfH * 2.0f;

    const float du = tex.contentCenterU - 0.5f;
    const float dv = tex.contentCenterV - 0.5f;
    const float centerSx = UiToScreenX(centerUiX) - (du * sw);
    const float centerSy =
        static_cast<float>(WindowHeight) - UiToScreenY(centerUiY) - (dv * sh);

    const float sx = centerSx - halfW;
    const float syB = centerSy - halfH;
    const float syT = centerSy + halfH;

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex.id);
    glColor4f(1.0f, 1.0f, 1.0f, alpha);
    glBegin(GL_TRIANGLE_FAN);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(sx,      syB);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(sx + sw, syB);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(sx + sw, syT);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(sx,      syT);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    glBlendFunc(GL_ONE, GL_ONE);
}

void DrawVirtualJoystickHud()
{
    const bool joystickActive = g_virtualJoystick.fingerId != static_cast<SDL_FingerID>(-1);
    const float visualScale = joystickActive ? 1.0f : kVirtualJoystickIdleVisualScale;
    const float ringAlpha = joystickActive ? kVirtualJoystickActiveRingAlpha : kVirtualJoystickIdleRingAlpha;
    const float knobAlpha = joystickActive ? kVirtualJoystickActiveKnobAlpha : kVirtualJoystickIdleKnobAlpha;

    const float centerX = GetVirtualJoystickRenderCenterX();
    const float centerY = GetVirtualJoystickRenderCenterY();
    const float ringCx = std::round(centerX);
    const float ringCy = std::round(centerY);

    const float outerW = kVirtualJoystickOuterRenderW * visualScale;
    const float outerH = kVirtualJoystickOuterRenderH * visualScale;
    const float knobW = kVirtualJoystickKnobRenderW * visualScale;
    const float knobH = kVirtualJoystickKnobRenderH * visualScale;

    const float layoutDiameterUi = std::max(outerW, outerH) * kJoystickRingDrawDiameterScale;
    const float R_softOuter = (layoutDiameterUi * 0.5f) * kJoystickRingSoftOuterRadiusMul;

    // Soft base — same center; outer radius R_softOuter defines the faint boundary; ring PNG fills it.
    DrawVirtualCircle(ringCx, ringCy, R_softOuter, 0.05f, 0.08f, 0.12f, ringAlpha * 0.55f, true);
    DrawVirtualCircle(ringCx, ringCy, R_softOuter * 0.93f, 0.82f, 0.86f, 0.92f, ringAlpha * 0.18f, true);
    DrawVirtualCircle(ringCx, ringCy, R_softOuter * 0.86f, 0.92f, 0.94f, 0.98f, ringAlpha * 0.42f, false);

    if (joystickActive && g_virtualJoystick.moveStrength > 0.05f)
    {
        const float pulseAlpha = ringAlpha * (0.18f + 0.22f * g_virtualJoystick.moveStrength);
        DrawVirtualCircle(
            ringCx,
            ringCy,
            R_softOuter * (1.0f + 0.08f * g_virtualJoystick.moveStrength),
            0.35f,
            0.72f,
            1.0f,
            pulseAlpha,
            false);
    }

    EnsureUITextures();
    if (g_uiTex_joystick2.id != 0)
    {
        const float ringTexDiameterUi = R_softOuter * 2.0f;
        DrawIconButtonUvSquare(
            ringCx,
            ringCy,
            ringTexDiameterUi,
            g_uiTex_joystick2,
            ringAlpha);
    }

    const float thumbDrawX = joystickActive
        ? std::round(ringCx + g_virtualJoystick.thumbOffsetX)
        : ringCx;
    const float thumbDrawY = joystickActive
        ? std::round(ringCy + g_virtualJoystick.thumbOffsetY)
        : ringCy;
    const float knobDrawCx = thumbDrawX;
    const float knobDrawCy = thumbDrawY;

    const float knobDiameterUi = std::max(knobW, knobH) * kJoystickKnobDrawDiameterScale;
    DrawVirtualCircle(knobDrawCx, knobDrawCy, knobDiameterUi * 0.42f, 1.0f, 1.0f, 1.0f, knobAlpha * 0.35f, true);
    if (g_uiTex_joystick1.id != 0)
    {
        DrawIconButtonUvSquare(
            knobDrawCx,
            knobDrawCy,
            knobDiameterUi,
            g_uiTex_joystick1,
            knobAlpha);
    }
}

// â”€â”€ Map button (top-right companion to minimap) â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
void DrawVirtualMapButton()
{
    EnsureUITextures();
    const AndroidUiRect rect = GetMapButtonRect();
    DrawIconButton(rect.x, rect.y, rect.w, rect.h, g_uiTex_map, 1.0f);
}

void DrawVirtualChatUtilityButton()
{
    if (SceneFlag != MAIN_SCENE || g_pNewUISystem == nullptr)
    {
        return;
    }

    const AndroidUiRect rect = GetVirtualUtilityButtonRect(kVirtualUtilityButtonChat);
    const bool isActive = IsVirtualUtilityButtonActive(kVirtualUtilityButtonChat);
    const float uiCx = rect.x + (rect.w * 0.5f);
    const float uiCy = rect.y + (rect.h * 0.5f);
    const float uiRadius = rect.w * 0.5f;
    const float cx = UiToScreenX(uiCx);
    const float cy = static_cast<float>(WindowHeight) - UiToScreenY(uiCy);
    const float rx = UiToScreenX(uiRadius);
    const float ry = UiToScreenY(uiRadius);

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(isActive ? 0.56f : 0.06f, isActive ? 0.20f : 0.06f, isActive ? 0.10f : 0.06f, isActive ? 0.94f : 0.88f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= 40; ++i)
    {
        const float angle = (static_cast<float>(i) / 40.0f) * 6.28318530718f;
        glVertex2f(cx + (std::cos(angle) * rx), cy + (std::sin(angle) * ry));
    }
    glEnd();

    glLineWidth(2.4f);
    glColor4f(isActive ? 1.0f : 0.94f, isActive ? 0.88f : 0.94f, isActive ? 0.58f : 0.94f, 0.98f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 40; ++i)
    {
        const float angle = (static_cast<float>(i) / 40.0f) * 6.28318530718f;
        glVertex2f(cx + (std::cos(angle) * rx), cy + (std::sin(angle) * ry));
    }
    glEnd();
    glLineWidth(1.0f);

    glColor4f(0.08f, 0.08f, 0.08f, 0.92f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= 40; ++i)
    {
        const float angle = (static_cast<float>(i) / 40.0f) * 6.28318530718f;
        glVertex2f(cx + (std::cos(angle) * rx * 0.72f), cy + (std::sin(angle) * ry * 0.72f));
    }
    glEnd();

    glColor4f(isActive ? 1.0f : 0.92f, isActive ? 0.72f : 0.92f, isActive ? 0.22f : 0.92f, 0.92f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 40; ++i)
    {
        const float angle = (static_cast<float>(i) / 40.0f) * 6.28318530718f;
        glVertex2f(cx + (std::cos(angle) * rx * 0.72f), cy + (std::sin(angle) * ry * 0.72f));
    }
    glEnd();

    g_pRenderText->SetFont(g_hFixFont != nullptr ? g_hFixFont : g_hFont);
    g_pRenderText->SetBgColor(0);
    g_pRenderText->SetTextColor(isActive ? CLRDW_BR_ORANGE : CLRDW_WHITE);
    g_pRenderText->RenderText(
        static_cast<int>(uiCx),
        static_cast<int>(uiCy - 5.0f),
        _T("chat"),
        0,
        0,
        RT3_WRITE_CENTER);
}

void DrawVirtualZoomButtons()
{
    BeginBitmap();
    DisableTexture();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    auto drawZoomBtn = [](float uiLeft, float uiTop, float uiW, float uiH, bool isMinus)
    {
        const float x = UiToScreenX(uiLeft);
        const float yTop = UiToScreenY(uiTop);
        const float w = UiToScreenX(uiW);
        const float h = UiToScreenY(uiH);
        const float y = static_cast<float>(WindowHeight) - yTop - h;

        glColor4f(0.02f, 0.02f, 0.02f, 0.96f);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(x, y);
        glVertex2f(x + w, y);
        glVertex2f(x + w, y + h);
        glVertex2f(x, y + h);
        glEnd();

        glLineWidth(3.0f);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(x + 1.5f, y + 1.5f);
        glVertex2f(x + w - 1.5f, y + 1.5f);
        glVertex2f(x + w - 1.5f, y + h - 1.5f);
        glVertex2f(x + 1.5f, y + h - 1.5f);
        glEnd();

        glLineWidth(1.0f);
        glColor4f(0.15f, 0.15f, 0.15f, 1.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(x + 4.0f, y + 4.0f);
        glVertex2f(x + w - 4.0f, y + 4.0f);
        glVertex2f(x + w - 4.0f, y + h - 4.0f);
        glVertex2f(x + 4.0f, y + h - 4.0f);
        glEnd();

        const float cx = x + w * 0.5f;
        const float cy = y + h * 0.5f;
        const float armLen = std::min(w, h) * 0.28f;
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glLineWidth(4.0f);
        glBegin(GL_LINES);
        glVertex2f(cx - armLen, cy);
        glVertex2f(cx + armLen, cy);
        if (!isMinus)
        {
            glVertex2f(cx, cy - armLen);
            glVertex2f(cx, cy + armLen);
        }
        glEnd();
        glLineWidth(1.0f);
    };

    drawZoomBtn(kZoomMinusX, kZoomButtonY, kZoomButtonW, kZoomButtonH, true);
    drawZoomBtn(kZoomPlusX,  kZoomButtonY, kZoomButtonW, kZoomButtonH, false);
    EndBitmap();
}

// Helper: draw a solid filled axis-aligned rectangle in screen space (GL y-up).
static void DrawFilledRect(float x, float y, float w, float h)
{
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x,     y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x,     y + h);
    glEnd();
}

static void EmitVirtualUiVertex(float uiX, float uiY)
{
    glVertex2f(UiToScreenX(uiX), static_cast<float>(WindowHeight) - UiToScreenY(uiY));
}

static void DrawVirtualUtilityLabel(int button, float uiCx, float uiCy, float uiRadius)
{
    const float w = uiRadius * 0.95f;
    const float h = uiRadius * 1.15f;
    glColor4f(1.0f, 1.0f, 1.0f, 0.96f);

    switch (button)
    {
    case 0: // I = Inventory
        glBegin(GL_LINES);
        EmitVirtualUiVertex(uiCx, uiCy - h * 0.45f);
        EmitVirtualUiVertex(uiCx, uiCy + h * 0.45f);
        glEnd();
        break;
    case 1: // C = Character
        glBegin(GL_LINE_STRIP);
        EmitVirtualUiVertex(uiCx + w * 0.40f, uiCy + h * 0.42f);
        EmitVirtualUiVertex(uiCx - w * 0.35f, uiCy + h * 0.42f);
        EmitVirtualUiVertex(uiCx - w * 0.45f, uiCy);
        EmitVirtualUiVertex(uiCx - w * 0.35f, uiCy - h * 0.42f);
        EmitVirtualUiVertex(uiCx + w * 0.40f, uiCy - h * 0.42f);
        glEnd();
        break;
    case 2: // S = Settings
        glBegin(GL_LINE_STRIP);
        EmitVirtualUiVertex(uiCx + w * 0.40f, uiCy + h * 0.42f);
        EmitVirtualUiVertex(uiCx - w * 0.20f, uiCy + h * 0.42f);
        EmitVirtualUiVertex(uiCx - w * 0.40f, uiCy + h * 0.15f);
        EmitVirtualUiVertex(uiCx + w * 0.20f, uiCy);
        EmitVirtualUiVertex(uiCx + w * 0.40f, uiCy - h * 0.20f);
        EmitVirtualUiVertex(uiCx + w * 0.15f, uiCy - h * 0.42f);
        EmitVirtualUiVertex(uiCx - w * 0.40f, uiCy - h * 0.42f);
        glEnd();
        break;
    default:
        break;
    }
}

// Render state for extra-sharp skill icons:
// keep alpha-test cutout but disable blending to avoid soft/washed edges.
static void ConfigureVirtualSkillIconNoBlendState()
{
    EnableAlphaTest();
    DisableCullFace();
    // DrawIconButton() temporarily restores additive blending for the rest of the
    // HUD but does not update the legacy state tracker. Force the real GL blend
    // mode back to standard alpha before any skill/icon atlas render.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_ALPHA_TEST);
    glEnable(GL_TEXTURE_2D);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

// Draw one 7-segment digit at screen coordinates (sx,sy) with size (sw,sh).
// In OpenGL y-up: sy = bottom of digit box, sy+sh = top.
// Each segment is rendered as a filled rectangle â€” always crisp on any DPI.
void DrawVirtualDigit(float sx, float sy, float sw, float sh, int digit)
{
    if (digit < 0 || digit > 9)
        return;
    // Bitmask: bit0=top, bit1=topRight, bit2=botRight, bit3=bottom, bit4=botLeft, bit5=topLeft, bit6=middle
    static constexpr uint8_t kSegMap[10] = {
        0b0111111u, // 0
        0b0000110u, // 1
        0b1011011u, // 2
        0b1001111u, // 3
        0b1100110u, // 4
        0b1101101u, // 5
        0b1111101u, // 6
        0b0000111u, // 7
        0b1111111u, // 8
        0b1101111u, // 9
    };
    const uint8_t s = kSegMap[digit];

    // Segment thickness and gap (in screen pixels)
    const float t  = sw * 0.20f;          // segment bar thickness
    const float g  = t  * 0.30f;          // gap at corners

    // Key coordinates
    const float x0 = sx;                   // left edge of left vertical column
    const float x1 = sx + sw - t;          // left edge of right vertical column
    const float y0 = sy;                   // bottom horizontal
    const float yM = sy + sh * 0.5f;      // middle
    const float y1 = sy + sh - t;         // bottom edge of top horizontal

    // Width/height of each sub-segment
    const float hW = sw - 2.0f * t - 2.0f * g;  // horizontal bar inner width
    const float vH = sh * 0.5f - t - 2.0f * g;   // vertical bar half-height

    // Horizontal bars
    if (s & 0x01u) DrawFilledRect(x0 + t + g,  y1,            hW, t);  // top
    if (s & 0x40u) DrawFilledRect(x0 + t + g,  yM - t * 0.5f, hW, t);  // middle
    if (s & 0x08u) DrawFilledRect(x0 + t + g,  y0,            hW, t);  // bottom

    // Vertical bars â€” top half
    if (s & 0x20u) DrawFilledRect(x0, yM + g,       t, vH);  // top-left
    if (s & 0x02u) DrawFilledRect(x1, yM + g,       t, vH);  // top-right

    // Vertical bars â€” bottom half
    if (s & 0x10u) DrawFilledRect(x0, y0 + t + g,  t, vH);  // bot-left
    if (s & 0x04u) DrawFilledRect(x1, y0 + t + g,  t, vH);  // bot-right
}

// Render an integer number centered at (uiCenterX, uiTopY) in UI coordinates.
// Uses filled-rect 7-segment digits so they are always crisp regardless of DPI.
// Draws a dark shadow first, then white on top for maximum contrast over any bar color.
// digitW/digitH control the size of each digit in virtual UI units.
void DrawVirtualNumber(float uiCenterX, float uiTopY, int number,
                       float digitW = 4.0f, float digitH = 5.0f)
{
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", number);
    const int numDigits = static_cast<int>(strlen(buf));
    if (numDigits <= 0)
        return;

    const float kGap = digitW * 0.25f;  // gap scales with digit size

    const float totalW_ui = numDigits * (digitW + kGap) - kGap;
    const float startX0_ui = uiCenterX - totalW_ui * 0.5f;

    // Pre-compute screen sizes (same for every digit)
    const float sw = UiToScreenX(digitW);
    const float sh = UiToScreenY(digitH);

    // Pass 1 â€” dark shadow, offset +1px right and -1px up (screen space)
    glColor4f(0.0f, 0.0f, 0.0f, 0.85f);
    {
        float startX_ui = startX0_ui;
        for (int i = 0; i < numDigits; ++i)
        {
            const int d = buf[i] - '0';
            const float sx = UiToScreenX(startX_ui) + 1.0f;
            const float sy = static_cast<float>(WindowHeight) - UiToScreenY(uiTopY + digitH) - 1.0f;
            DrawVirtualDigit(sx, sy, sw, sh, d);
            startX_ui += digitW + kGap;
        }
    }

    // Pass 2 â€” bright white foreground
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    {
        float startX_ui = startX0_ui;
        for (int i = 0; i < numDigits; ++i)
        {
            const int d = buf[i] - '0';
            const float sx = UiToScreenX(startX_ui);
            const float sy = static_cast<float>(WindowHeight) - UiToScreenY(uiTopY + digitH);
            DrawVirtualDigit(sx, sy, sw, sh, d);
            startX_ui += digitW + kGap;
        }
    }
}

void RenderVirtualPad()
{
    // Keep only the virtual joystick overlay on mobile.
    // The rest of the custom Android HUD is intentionally disabled so the
    // original MU UI can render and handle input again.
    if (!IsVirtualPadAvailable())
    {
        static uint32_t s_lastUnavailableLog = 0;
        const uint32_t nowMs = MU_MobileGetTicks();
        if (SceneFlag == MAIN_SCENE && (nowMs - s_lastUnavailableLog) > 3000)
        {
            s_lastUnavailableLog = nowMs;
            LOGI(
                "VirtualPad unavailable scene=%d hero=%d attr=%d mainFrame=%d focusedInput=%d",
                static_cast<int>(SceneFlag),
                Hero != nullptr ? 1 : 0,
                CharacterAttribute != nullptr ? 1 : 0,
                g_pMainFrame != nullptr ? 1 : 0,
                AndroidHasFocusedTextInput() ? 1 : 0);
        }
        return;
    }

    // Scene() leaves a 3D depth buffer; if depth test/mask are out of sync with the
    // wrapper flags, BeginBitmap's DisableDepthTest() may not actually disable GL
    // depth testing and the virtual joystick (same z) can be partially rejected.
    glViewport(0, 0, static_cast<GLsizei>(WindowWidth), static_cast<GLsizei>(WindowHeight));
    glClear(GL_DEPTH_BUFFER_BIT);

    BeginBitmap();
    EnableAlphaBlend();
    DisableTexture();
    DrawVirtualJoystickHud();
    EndBitmap();

    DrawVirtualZoomButtons();

    if (kUseLegacyMainHud)
    {
        return;
    }

    if (kShowVirtualAttackButton)
    {
        BeginBitmap();
        EnsureUITextures();
        LoadVirtualSkillSlots();
        const VirtualButtonLayout& attackButton = kVirtualButtons[kVirtualAttackButton];
        const int attackSkillIndex = g_virtualSkillSlots[kVirtualAttackSkillSlot];
        const bool attackHasSkill =
            (g_pSkillList != nullptr) && IsAssignableVirtualSkillIndex(attackSkillIndex);
        const float attackRingSize = attackButton.radius * 2.0f + 20.0f;
        const float attackIconSize = attackButton.radius * 1.34f + 3.0f;
        DrawIconButton(
            attackButton.cx - attackRingSize * 0.5f,
            attackButton.cy - attackRingSize * 0.5f,
            attackRingSize,
            attackRingSize,
            g_uiTex_skillline,
            IsVirtualButtonPressed(kVirtualAttackButton) ? 1.0f : 0.98f);
        if (!attackHasSkill)
        {
            DrawIconButton(
                attackButton.cx - attackIconSize * 0.5f,
                attackButton.cy - attackIconSize * 0.5f,
                attackIconSize,
                attackIconSize,
                g_uiTex_attack,
                IsVirtualButtonPressed(kVirtualAttackButton) ? 1.0f : 0.96f);
        }
        EndBitmap();

        if (attackHasSkill)
        {
            ConfigureVirtualSkillIconNoBlendState();
            const float skillIconSize = attackButton.radius * 1.12f;
            g_pSkillList->RenderSkillIcon(
                attackSkillIndex,
                attackButton.cx - skillIconSize * 0.5f,
                attackButton.cy - skillIconSize * 0.5f,
                skillIconSize,
                skillIconSize);
        }

        g_pRenderText->SetFont(g_hFontBold != nullptr ? g_hFontBold : g_hFixFont);
        g_pRenderText->SetBgColor(0);
        g_pRenderText->SetTextColor(CLRDW_BR_ORANGE);
        const int attackLabelX = static_cast<int>(attackButton.cx);
        const int attackLabelY = static_cast<int>(attackButton.cy - attackRingSize * 0.5f - 18.0f);
        g_pRenderText->RenderText(attackLabelX, attackLabelY, attackHasSkill ? _T("SKILL") : _T("ATTACK"), 0, 0, RT3_WRITE_CENTER);
        g_pRenderText->SetTextColor(CLRDW_WHITE);
        g_pRenderText->RenderText(attackLabelX, attackLabelY + 12, attackHasSkill ? _T("SLOT") : _T("NORMAL"), 0, 0, RT3_WRITE_CENTER);
    }

    if (kShowVirtualSkillButtons)
    {
        LoadVirtualSkillSlots();
        const bool assignModeActive = IsVirtualAssignModeActive()
            || IsAssignableVirtualSkillIndex(g_virtualAssignPickerSkillIndex);
        const uint32_t nowMs = MU_MobileGetTicks();
        const float assignPulse = 0.5f + 0.5f * std::sin(static_cast<float>(nowMs % 900u) * 0.006981317f);

        BeginBitmap();
        EnsureUITextures();
        for (int visualSlot = 0; visualSlot < kVirtualVisibleSkillButtonCount; ++visualSlot)
        {
            const int buttonIndex = kVirtualSkillButtonBase + visualSlot;
            const int skillSlot = visualSlot + 1;
            const VirtualButtonLayout& button = kVirtualButtons[buttonIndex];
            const float boxSize = button.radius * 2.0f + 5.0f;
            const float alpha = IsVirtualButtonPressed(buttonIndex) ? 1.0f : 0.92f;
            DrawIconButton(
                button.cx - boxSize * 0.5f,
                button.cy - boxSize * 0.5f,
                boxSize,
                boxSize,
                g_uiTex_skillbox,
                assignModeActive ? (0.86f + 0.14f * assignPulse) : alpha);
        }
        EndBitmap();

        for (int visualSlot = 0; visualSlot < kVirtualVisibleSkillButtonCount; ++visualSlot)
        {
            const int buttonIndex = kVirtualSkillButtonBase + visualSlot;
            const int skillSlot = visualSlot + 1;
            const VirtualButtonLayout& button = kVirtualButtons[buttonIndex];
            const int renderSkillIndex = g_virtualSkillSlots[skillSlot];
            if (g_pSkillList != nullptr && IsAssignableVirtualSkillIndex(renderSkillIndex))
            {
                ConfigureVirtualSkillIconNoBlendState();
                const float skillIconSize = button.radius * 1.18f;
                g_pSkillList->RenderSkillIcon(
                    renderSkillIndex,
                    button.cx - skillIconSize * 0.5f,
                    button.cy - skillIconSize * 0.5f,
                    skillIconSize,
                    skillIconSize);
            }
        }
    }

#if 0
    // Disabled: custom Android HUD. We keep this code commented for now so it
    // can be restored later if needed, but the active mobile UI should remain
    // the original MU interface plus joystick movement only.

    for (int i = 0; i < static_cast<int>(kVirtualButtons.size()); ++i)
    {
        const bool isAttack = (i == kVirtualAttackButton);
        const bool isAssignGlow = assignModeActive && !isAttack;

        if (isAssignGlow)
        {
            // Skill button in assign-mode: keep a soft fill under the decorative ring
            // so the user still gets clear feedback.
            const float fillA = 0.68f + 0.08f * assignPulse;
            DrawVirtualCircle(kVirtualButtons[i].cx, kVirtualButtons[i].cy,
                kVirtualButtons[i].radius,
                0.18f, 0.55f, 0.80f, fillA, true);
        }
        else
        {
            // Decorative ring is drawn in the skill loop so there is no extra GL circle here.
        }
    }

    // â”€â”€ Top-right utility buttons (Inventory / Character / Settings) â€” PNG icons â”€â”€
    {
        const VirtualButtonLayout& attackButton = kVirtualButtons[kVirtualAttackButton];
        const float attackRingSize = attackButton.radius * 2.0f + 34.0f;
        const float attackIconSize = attackButton.radius * 1.48f + 5.0f;
        DrawIconButton(
            attackButton.cx - attackRingSize * 0.5f,
            attackButton.cy - attackRingSize * 0.5f,
            attackRingSize,
            attackRingSize,
            g_uiTex_skillline,
            IsVirtualButtonPressed(kVirtualAttackButton) ? 1.0f : 0.98f);
        DrawIconButton(
            attackButton.cx - attackIconSize * 0.5f,
            attackButton.cy - attackIconSize * 0.5f,
            attackIconSize,
            attackIconSize,
            g_uiTex_attack,
            IsVirtualButtonPressed(kVirtualAttackButton) ? 1.0f : 0.96f);

        const UITexture* kUtilIcons[kVirtualUtilityButtonCount] = {
            &g_uiTex_balo,
            &g_uiTex_character,
            &g_uiTex_setting,
            &g_uiTex_map,
        };
        for (int i = 0; i < kVirtualUtilityButtonCount; ++i)
        {
            if (i == kVirtualUtilityButtonChat)
            {
                continue;
            }
            const AndroidUiRect rect = GetVirtualUtilityButtonRect(i);
            DrawIconButton(rect.x, rect.y, rect.w, rect.h, *kUtilIcons[i], 1.0f);
        }
    }

    // â”€â”€ Bottom HUD â€” HP/MP/AG/EXP (1/3 width, solid, numbers on bar) â”€â”€â”€â”€â”€â”€â”€â”€â”€
    // Solid bars, no blending artefacts. Numbers centered on each bar.
    {
        // Switch to normal alpha blend so solid colors render correctly over game world
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        const float barW = kHudBarRight - kHudBarLeft;

        // Row y positions (stacked top-down from kHudStripY)
        const float yHP  = kHudStripY;
        const float yMP  = yHP  + kHudBarH + kHudBarGap;
        const float yAG  = yMP  + kHudBarH + kHudBarGap;
        const float yEXP = yAG  + kHudBarH + kHudBarGap;

        DWORD curHP = 0;
        DWORD maxHP = 0;
        DWORD curMP = 0;
        DWORD maxMP = 0;
        DWORD curAG = 0;
        DWORD maxAG = 0;
        TakumiGetHudVitals(curHP, maxHP, curMP, maxMP, curAG, maxAG);
        maxHP = std::max(1u, maxHP);
        maxMP = std::max(1u, maxMP);
        maxAG = std::max(1u, maxAG);

        // HP â€” bright red
        DrawVirtualBarH(kHudBarLeft, yHP, barW, kHudBarH,
                        static_cast<float>(curHP) / static_cast<float>(maxHP),
                        0.95f, 0.15f, 0.15f,  0.22f, 0.04f, 0.04f);

        // MP â€” bright blue
        DrawVirtualBarH(kHudBarLeft, yMP, barW, kHudBarH,
                        static_cast<float>(curMP) / static_cast<float>(maxMP),
                        0.15f, 0.45f, 1.00f,  0.04f, 0.08f, 0.28f);

        // AG â€” cyan
        DrawVirtualBarH(kHudBarLeft, yAG, barW, kHudBarH,
                        static_cast<float>(curAG) / static_cast<float>(maxAG),
                        0.10f, 0.95f, 0.90f,  0.04f, 0.20f, 0.18f);

        // EXP — gold; gain flash uses g_pMainFrame SetPreExp/SetGetExp (parity RenderExperience)
        float expCurrent = 0.f;
        float expPrior = 0.f;
        bool expHighlight = false;
        bool expHighlightFull = false;
        TakumiGetHudExperienceBarFill(expCurrent, expPrior, expHighlight, expHighlightFull);
        expCurrent = std::clamp(expCurrent, 0.0f, 1.0f);
        expPrior = std::clamp(expPrior, 0.0f, expCurrent);
        DrawVirtualBarH(kHudBarLeft, yEXP, barW, kHudBarH,
                        expCurrent,
                        1.00f, 0.85f, 0.05f,  0.22f, 0.18f, 0.02f);
        if (expHighlight)
        {
            const float highlightStart = expHighlightFull ? 0.0f : expPrior;
            DrawVirtualBarFillRange(kHudBarLeft, yEXP, barW, kHudBarH,
                                    highlightStart, expCurrent,
                                    1.0f, 1.0f, 1.0f, 0.4f);
        }

        // â”€â”€ Numbers centered ON each bar â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
        // Use the legacy number atlas for sharper, anti-aliased digits like old UI.
        constexpr float kHudNumberScale   = 0.90f;
        constexpr float kHudNumberOffsetY = 0.50f;
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        SEASON3B::RenderNumber(kHudNumCenterX, yHP  + kHudNumberOffsetY, static_cast<int>(curHP), kHudNumberScale);
        SEASON3B::RenderNumber(kHudNumCenterX, yMP  + kHudNumberOffsetY, static_cast<int>(curMP), kHudNumberScale);
        SEASON3B::RenderNumber(kHudNumCenterX, yAG  + kHudNumberOffsetY, static_cast<int>(curAG), kHudNumberScale);
        const int expPercent = static_cast<int>(expCurrent * 100.0f + 0.5f);
        SEASON3B::RenderNumber(kHudNumCenterX, yEXP + kHudNumberOffsetY, expPercent, kHudNumberScale);

        // Restore additive blend used by the rest of the virtual pad rendering
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
    }

    // Top utility controls (map/minimap/zoom) must use regular alpha blending.
    // Additive blend can make them appear "missing" on bright backgrounds.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // â”€â”€ Map list button (top-left red square, in game area) â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    DrawVirtualMapButton();

    if (IsMiniMapToggleAvailable())
    {
        const AndroidUiRect rect = GetMiniMapButtonRect();
        // minimap.png icon â€” full alpha always
        DrawIconButton(rect.x, rect.y, rect.w, rect.h, g_uiTex_minimap, 1.0f);
    }

    DrawVirtualChatUtilityButton();

    // â”€â”€ ZOOM -/+ buttons (top center) â”€â”€
    {
        auto drawZoomBtn = [](float uiLeft, float uiTop, float uiW, float uiH,
                              const char* label, bool isMinus)
        {
            const float x = UiToScreenX(uiLeft);
            const float yTop = UiToScreenY(uiTop);
            const float w = UiToScreenX(uiW);
            const float h = UiToScreenY(uiH);
            const float y = static_cast<float>(WindowHeight) - yTop - h;

            // Background
            glColor4f(0.10f, 0.10f, 0.10f, 0.82f);
            glBegin(GL_TRIANGLE_FAN);
            glVertex2f(x, y);
            glVertex2f(x + w, y);
            glVertex2f(x + w, y + h);
            glVertex2f(x, y + h);
            glEnd();

            // Border
            glColor4f(1.0f, 1.0f, 1.0f, 0.96f);
            glBegin(GL_LINE_LOOP);
            glVertex2f(x + 0.5f, y + 0.5f);
            glVertex2f(x + w - 0.5f, y + 0.5f);
            glVertex2f(x + w - 0.5f, y + h - 0.5f);
            glVertex2f(x + 0.5f, y + h - 0.5f);
            glEnd();

            // Symbol: horizontal line (minus) + vertical line (plus)
            const float cx = x + w * 0.5f;
            const float cy = y + h * 0.5f;
            const float armLen = std::min(w, h) * 0.25f;
            glColor4f(1.0f, 1.0f, 1.0f, 0.95f);
            glBegin(GL_LINES);
            glVertex2f(cx - armLen, cy);
            glVertex2f(cx + armLen, cy);
            if (!isMinus)
            {
                glVertex2f(cx, cy - armLen);
                glVertex2f(cx, cy + armLen);
            }
            glEnd();
        };

        drawZoomBtn(kZoomMinusX, kZoomButtonY, kZoomButtonW, kZoomButtonH, "ZOOM-", true);
        drawZoomBtn(kZoomPlusX,  kZoomButtonY, kZoomButtonW, kZoomButtonH, "ZOOM+", false);
    }

    // â”€â”€ Current skill box (legacy-like UI element on custom Android HUD) â”€â”€
    if (kShowVirtualCurrentSkillBox)
    {
        const bool pickerOpen = (g_pSkillList != nullptr) && g_pSkillList->IsSkillPickerOpen();
        const GLuint frameImage = pickerOpen
            ? SEASON3B::CNewUISkillList::IMAGE_SKILLBOX_USE
            : SEASON3B::CNewUISkillList::IMAGE_SKILLBOX;

        ConfigureVirtualSkillIconNoBlendState();
        SEASON3B::RenderImage(
            frameImage,
            kVirtualCurrentSkillBoxX,
            kVirtualCurrentSkillBoxY,
            kVirtualCurrentSkillBoxW,
            kVirtualCurrentSkillBoxH);

        if (g_pSkillList != nullptr && Hero != nullptr)
        {
            g_pSkillList->RenderSkillIcon(
                static_cast<int>(Hero->CurrentSkill),
                kVirtualCurrentSkillBoxX + 6.0f,
                kVirtualCurrentSkillBoxY + 6.0f,
                20.0f,
                28.0f);
        }
    }

    // Check stencil availability once per frame (cached in static to avoid repeated queries).
    static GLint s_cachedStencilBits = -1;
    if (s_cachedStencilBits < 0)
        glGetIntegerv(GL_STENCIL_BITS, &s_cachedStencilBits);
    const bool kUseStencilClip = (s_cachedStencilBits > 0);
    if (kUseStencilClip)
    {
        GL_FlushPending();
        glClearStencil(0);
        glStencilMask(0xFF);
        glClear(GL_STENCIL_BUFFER_BIT);
    }

    if (kShowVirtualSkillButtons)
    {
        for (int slot = 0; slot < kVirtualSkillSlotCount; ++slot)
        {
            const VirtualButtonLayout& button = kVirtualButtons[kVirtualSkillButtonBase + slot];
            const float skillRingSize = button.radius * 2.0f + 24.0f;
            const float skillClipRadius = std::max(skillRingSize * 0.30f, 1.0f);
            const float skillIconSize = std::max(skillClipRadius * 1.28f - 1.0f, 1.0f);
            const int renderSkillIndex = g_virtualSkillSlots[slot];

            DrawIconButton(
                button.cx - skillRingSize * 0.5f,
                button.cy - skillRingSize * 0.5f,
                skillRingSize,
                skillRingSize,
                g_uiTex_skillline,
                IsVirtualButtonPressed(kVirtualSkillButtonBase + slot) ? 1.0f : 0.98f);

            const bool hasSkillIcon =
                (g_pSkillList != nullptr) && IsAssignableVirtualSkillIndex(renderSkillIndex);

            if (hasSkillIcon && kUseStencilClip)
            {
                GL_FlushPending();
                glEnable(GL_STENCIL_TEST);
                glStencilMask(0xFF);
                glStencilFunc(GL_ALWAYS, 1, 0xFF);
                glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
                glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
                DrawVirtualCircle(button.cx, button.cy, skillClipRadius,
                    0.0f, 0.0f, 0.0f, 1.0f, true);
                GL_FlushPending();

                glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                glStencilFunc(GL_EQUAL, 1, 0xFF);
                glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
            }

            if (hasSkillIcon)
            {
                ConfigureVirtualSkillIconNoBlendState();
                g_pSkillList->RenderSkillIcon(
                    renderSkillIndex,
                    button.cx - (skillIconSize * 0.5f),
                    button.cy - (skillIconSize * 0.5f),
                    skillIconSize,
                    skillIconSize);
            }

            if (hasSkillIcon && kUseStencilClip)
            {
                GL_FlushPending();
                glStencilMask(0xFF);
                glStencilFunc(GL_ALWAYS, 0, 0xFF);
                glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
                glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
                DrawVirtualCircle(button.cx, button.cy, skillClipRadius,
                    0.0f, 0.0f, 0.0f, 1.0f, true);
                GL_FlushPending();

                glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                glDisable(GL_STENCIL_TEST);
            }
        }
    }

    // Restore standard blend state for anything that follows (consumable icons, etc.)
    EnableAlphaBlend();

    // â”€â”€ Consumable item icons â”€â”€
    // RenderItem3D is a 3D function â€” needs perspective projection, NOT the
    // 2D ortho mode set up by BeginBitmap.  Follow the same pattern used by
    // CNewUI3DCamera::Render() / CNewUIGoldBowmanLena::Render3D().
    {
        bool anyItem = false;
        for (int i = 0; i < kVirtualConsumableSlotCount; ++i)
            if (g_virtualConsumableSlots[i].itemType >= 0) { anyItem = true; break; }

        if (anyItem)
        {
            EndBitmap(); // leave 2D ortho

            glMatrixMode(GL_PROJECTION);
            glPushMatrix();
            glLoadIdentity();
            glViewport2(0, 0, WindowWidth, WindowHeight);
            gluPerspective2(1.f,
                            static_cast<float>(WindowWidth) / static_cast<float>(WindowHeight),
                            RENDER_ITEMVIEW_NEAR,
                            RENDER_ITEMVIEW_FAR);
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();
            GetOpenGLMatrix(CameraMatrix);
            EnableDepthTest();
            EnableDepthMask();
            glDisable(GL_BLEND);
            glClear(GL_DEPTH_BUFFER_BIT);

            for (int i = 0; i < kVirtualConsumableSlotCount; ++i)
            {
                const VirtualConsumableSlot& cs = g_virtualConsumableSlots[i];
                if (cs.itemType < 0) continue;

                const VirtualButtonLayout& layout = kVirtualConsumableSlots[i];
                const float iconHalf = layout.radius * 0.85f;
                RenderItem3D(
                    layout.cx - iconHalf,
                    layout.cy - iconHalf,
                    iconHalf * 2.0f,
                    iconHalf * 2.0f,
                    cs.itemType, cs.itemLevel,
                    0, 0);
            }

            UpdateMousePositionn();

            glMatrixMode(GL_MODELVIEW);
            glPopMatrix();
            glMatrixMode(GL_PROJECTION);
            glPopMatrix();

            BeginBitmap(); // back to 2D ortho for quantity numbers
            glEnable(GL_BLEND);
        }
    }

    // â”€â”€ Consumable rings (same skillline frame for visual consistency) â”€â”€
    for (int i = 0; i < kVirtualConsumableSlotCount; ++i)
    {
        const VirtualButtonLayout& layout = kVirtualConsumableSlots[i];
        DrawVirtualCircle(
            layout.cx,
            layout.cy,
            layout.radius + 11.0f,
            0.05f,
            0.05f,
            0.05f,
            (g_virtualConsumableSlots[i].itemType >= 0) ? 0.55f : 0.32f,
            true);
        const float ringSize = layout.radius * 2.0f + 20.0f;
        const float ringAlpha = (g_virtualConsumableSlots[i].itemType >= 0) ? 1.0f : 0.84f;
        DrawIconButton(
            layout.cx - ringSize * 0.5f,
            layout.cy - ringSize * 0.5f,
            ringSize,
            ringSize,
            g_uiTex_skillline,
            ringAlpha);
    }

    // â”€â”€ Consumable quantity numbers â”€â”€
    // RenderNumber expects 640x480 virtual coords (same as inventory panel).
    // Durability==0 in ITEM means a stack of 1 (see CNewUIInventoryCtrl::GetItemCount).
    EnableAlphaTest();
    glColor4f(1.0f, 0.9f, 0.7f, 1.0f);
    glEnable(GL_TEXTURE_2D);
    for (int i = 0; i < kVirtualConsumableSlotCount; ++i)
    {
        VirtualConsumableSlot& cs = g_virtualConsumableSlots[i];
        if (cs.itemType < 0)
            continue;

        const int idx = FindConsumableInInventory(cs.itemType, cs.itemLevel);
        if (idx < 0)
            continue;  // not found this frame â€” keep slot, skip quantity

        // Get item data from the UI inventory (authoritative source)
        const ITEM* pFoundItem = (g_pMyInventory != nullptr)
                                  ? g_pMyInventory->FindItem(idx)
                                  : nullptr;
        if (pFoundItem == nullptr)
            continue;  // skip quantity display; do NOT clear slot here

        // Durability==0 means exactly 1 item in the slot (not empty)
        const int qty = (pFoundItem->Durability == 0)
                         ? 1
                         : static_cast<int>(pFoundItem->Durability);

        const VirtualButtonLayout& layout = kVirtualConsumableSlots[i];
        // Position number at bottom-right of the circle
        SEASON3B::RenderNumber(
            layout.cx + layout.radius * 0.5f,
            layout.cy + layout.radius * 0.65f,
            qty);
    }
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    EndBitmap();
#endif
}
} // namespace

bool MU_AndroidIsVirtualJoystickDrivingMouse()
{
    return g_virtualJoystickDrivingMouse;
}

bool MU_AndroidShouldSuppressCombatTargeting()
{
    return g_virtualJoystick.fingerId != static_cast<SDL_FingerID>(-1)
        && g_virtualJoystick.moveStrength > kVirtualJoystickCombatMoveStrength;
}

bool AndroidTriggerNormalAttackButton()
{
    return AndroidTriggerNormalAttackButtonInternal();
}

bool AndroidTriggerHotKeySkillTap(int hotKeySkillIndex)
{
    return AndroidTriggerHotKeySkillTapInternal(hotKeySkillIndex);
}

float AndroidGetCompactMiniMapTopY()
{
    return GetAndroidCompactMiniMapTopYInternal();
}

float AndroidGetCompactMiniMapLeftX()
{
    return GetAndroidCompactMiniMapLeftXInternal();
}

bool AndroidGetMoveMapWindowPosition(int panelWidth, int panelHeight, int* outX, int* outY)
{
    return GetAndroidMoveMapWindowPositionInternal(panelWidth, panelHeight, outX, outY);
}

static void UpdateMouseFromPixel(int pixelX, int pixelY, int screenW, int screenH)
{
    const int safeW = (screenW > 0) ? screenW : 1;
    const int safeH = (screenH > 0) ? screenH : 1;

    const int clampedX = std::clamp(pixelX, 0, safeW - 1);
    const int clampedY = std::clamp(pixelY, 0, safeH - 1);

    MouseX = (int)((float)clampedX * 640.0f / (float)safeW);
    MouseY = (int)((float)clampedY * 480.0f / (float)safeH);
    MouseX = std::clamp(MouseX, 0, 640);
    MouseY = std::clamp(MouseY, 0, 480);
}

static void UpdateMouseFromTouch(const SDL_TouchFingerEvent& touch, int screenW, int screenH)
{
    const int safeW = (screenW > 0) ? screenW : 1;
    const int safeH = (screenH > 0) ? screenH : 1;

    const int pixelX = std::clamp((int)(touch.x * (float)safeW), 0, safeW - 1);
    const int pixelY = std::clamp((int)(touch.y * (float)safeH), 0, safeH - 1);

    UpdateMouseFromPixel(pixelX, pixelY, safeW, safeH);
}

BOOL Util_CheckOption(std::wstring /*lpszCommandLine*/, wchar_t /*cOption*/, std::wstring& /*lpszString*/) {
    return FALSE;
}

// Application-level DestroyWindow() â€” not the Win32 API (which takes HWND)
// Declared in Winmain.h as extern void DestroyWindow();
void DestroyWindow() {}

// =============================================================================
// Audio â€” SDL_mixer replaces wzAudio + DirectSound
// =============================================================================

static Mix_Music* g_pCurrentMusic = nullptr;
static char g_LastFailedMp3Name[256] = {};
static uint32_t g_LastFailedMp3Tick = 0;
static bool g_AndroidAudioAvailable = false;
static std::unordered_map<std::string, std::string> g_MusicPathCache;

static inline bool EqualsIgnoreCaseAscii(std::string_view lhs, std::string_view rhs)
{
    if (lhs.size() != rhs.size())
    {
        return false;
    }

    for (size_t i = 0; i < lhs.size(); ++i)
    {
        if (std::tolower(static_cast<unsigned char>(lhs[i])) !=
            std::tolower(static_cast<unsigned char>(rhs[i])))
        {
            return false;
        }
    }

    return true;
}

static std::string ToAbsoluteGenericPath(const std::filesystem::path& path)
{
    namespace fs = std::filesystem;
    std::error_code ec;
    const fs::path absolute = fs::absolute(path, ec);
    if (!ec)
    {
        return absolute.generic_string();
    }

    return path.generic_string();
}

static std::string NormalizeMusicPath(const char* name)
{
    std::string normalized = name ? name : "";
    for (char& c : normalized)
    {
        if (c == '\\')
        {
            c = '/';
        }
    }

    return normalized;
}

static std::string ResolveMusicPath(const char* name)
{
    namespace fs = std::filesystem;

    const std::string requested = NormalizeMusicPath(name);
    if (requested.empty())
    {
        return requested;
    }

    const auto cacheIt = g_MusicPathCache.find(requested);
    if (cacheIt != g_MusicPathCache.end())
    {
        return cacheIt->second;
    }

    if (fs::exists(requested))
    {
        const std::string resolved = ToAbsoluteGenericPath(fs::path(requested));
        g_MusicPathCache[requested] = resolved;
        return resolved;
    }

    EnsureAndroidHostPackageInitialized();
    char externalBases[2][512] = {};
    std::snprintf(
        externalBases[0],
        sizeof(externalBases[0]),
        "/sdcard/Android/data/%s/files",
        g_androidHostPackage);
    std::snprintf(
        externalBases[1],
        sizeof(externalBases[1]),
        "/storage/emulated/0/Android/data/%s/files",
        g_androidHostPackage);
    const std::array<const char*, 2> kExternalBaseDirs = { externalBases[0], externalBases[1] };

    for (const char* baseRaw : kExternalBaseDirs)
    {
        const fs::path absoluteCandidate = fs::path(baseRaw) / requested;
        if (fs::exists(absoluteCandidate))
        {
            const std::string resolved = ToAbsoluteGenericPath(absoluteCandidate);
            g_MusicPathCache[requested] = resolved;
            return resolved;
        }
    }

    const std::string fileName = fs::path(requested).filename().string();
    static constexpr std::array<const char*, 4> kMusicRoots = {
        "Data/Music",
        "data/music",
        "Data/music",
        "data/Music"
    };

    for (const char* baseRaw : kExternalBaseDirs)
    {
        for (const char* rootRaw : kMusicRoots)
        {
            const fs::path root = fs::path(baseRaw) / rootRaw;
            if (!fs::exists(root) || !fs::is_directory(root))
            {
                continue;
            }

            const fs::path directCandidate = root / fileName;
            if (fs::exists(directCandidate))
            {
                const std::string resolved = ToAbsoluteGenericPath(directCandidate);
                g_MusicPathCache[requested] = resolved;
                return resolved;
            }

            for (const auto& entry : fs::directory_iterator(root))
            {
                if (!entry.is_regular_file())
                {
                    continue;
                }

                const std::string entryName = entry.path().filename().string();
                if (EqualsIgnoreCaseAscii(entryName, fileName))
                {
                    const std::string resolved = ToAbsoluteGenericPath(entry.path());
                    g_MusicPathCache[requested] = resolved;
                    return resolved;
                }
            }
        }
    }

    for (const char* rootRaw : kMusicRoots)
    {
        const fs::path root(rootRaw);
        if (!fs::exists(root) || !fs::is_directory(root))
        {
            continue;
        }

        const fs::path directCandidate = root / fileName;
        if (fs::exists(directCandidate))
        {
            const std::string resolved = ToAbsoluteGenericPath(directCandidate);
            g_MusicPathCache[requested] = resolved;
            return resolved;
        }

        for (const auto& entry : fs::directory_iterator(root))
        {
            if (!entry.is_regular_file())
            {
                continue;
            }

            const std::string entryName = entry.path().filename().string();
            if (EqualsIgnoreCaseAscii(entryName, fileName))
            {
                const std::string resolved = ToAbsoluteGenericPath(entry.path());
                g_MusicPathCache[requested] = resolved;
                return resolved;
            }
        }
    }

    g_MusicPathCache[requested] = requested;
    return requested;
}

void StopMp3(char* Name, BOOL bEnforce)
{
    if (!g_AndroidAudioAvailable) return;
    if (!m_MusicOnOff && !bEnforce) return;
    if (Mp3FileName[0] != '\0' && strcmp(Name, Mp3FileName) == 0)
    {
        // Keep current music handle cached so frequent stop/play calls for the
        // same track can resume without expensive reloading.
        Mix_HaltMusic();
    }
}

void PlayMp3(char* Name, BOOL bEnforce)
{
    if (!g_AndroidAudioAvailable) return;
    if (Destroy) return;
    if (!Name || Name[0] == '\0') return;
    if (!m_MusicOnOff && !bEnforce) return;
    if (strcmp(Name, Mp3FileName) == 0)
    {
        // Same track requested again: if it was halted, just resume.
        if (g_pCurrentMusic && !Mix_PlayingMusic())
        {
            Mix_PlayMusic(g_pCurrentMusic, -1);
        }

        // If previous load failed, g_pCurrentMusic is null; continue below so
        // retry logic can attempt to load it again.
        if (g_pCurrentMusic)
        {
            return;
        }
    }

    const uint32_t now = MU_MobileGetTicks();
    constexpr uint32_t kFailedRetryMs = 3000;
    if (g_LastFailedMp3Name[0] != '\0'
        && strcmp(Name, g_LastFailedMp3Name) == 0
        && (now - g_LastFailedMp3Tick) < kFailedRetryMs)
    {
        return;
    }

    Mix_HaltMusic();
    if (g_pCurrentMusic) { Mix_FreeMusic(g_pCurrentMusic); g_pCurrentMusic = nullptr; }

    const std::string path = ResolveMusicPath(Name);

    g_pCurrentMusic = Mix_LoadMUS(path.c_str());
    if (g_pCurrentMusic) {
        Mix_PlayMusic(g_pCurrentMusic, -1);
        static int s_playOkLogCount = 0;
        if (s_playOkLogCount < 8)
        {
            LOGI("PlayMp3 OK requested=[%s] resolved=[%s]", Name, path.c_str());
            ++s_playOkLogCount;
        }
        strncpy(Mp3FileName, Name, sizeof(Mp3FileName) - 1);
        Mp3FileName[sizeof(Mp3FileName) - 1] = '\0';
        g_LastFailedMp3Name[0] = '\0';
        g_LastFailedMp3Tick = 0;
    } else {
        if (strcmp(Name, g_LastFailedMp3Name) != 0 || (now - g_LastFailedMp3Tick) >= kFailedRetryMs)
        {
            char cwd[512] = {};
            const char* cwdPtr = getcwd(cwd, sizeof(cwd));
            const bool requestedExists = std::filesystem::exists(NormalizeMusicPath(Name));
            const bool resolvedExists = std::filesystem::exists(path);
            LOGE("PlayMp3 failed requested=[%s] resolved=[%s] reqExists=%d resolvedExists=%d cwd=[%s]: %s",
                Name,
                path.c_str(),
                requestedExists ? 1 : 0,
                resolvedExists ? 1 : 0,
                cwdPtr ? cwdPtr : "(getcwd failed)",
                Mix_GetError());
        }
        strncpy(g_LastFailedMp3Name, Name, sizeof(g_LastFailedMp3Name) - 1);
        g_LastFailedMp3Name[sizeof(g_LastFailedMp3Name) - 1] = '\0';
        g_LastFailedMp3Tick = now;

        // Mark as current request to avoid expensive reload attempts every frame.
        strncpy(Mp3FileName, Name, sizeof(Mp3FileName) - 1);
        Mp3FileName[sizeof(Mp3FileName) - 1] = '\0';
    }
}

bool IsEndMp3()           { return !g_AndroidAudioAvailable || !Mix_PlayingMusic(); }
int  GetMp3PlayPosition() { return (g_AndroidAudioAvailable && Mix_PlayingMusic()) ? 50 : 100; }

// =============================================================================
// Stubs for Windows-only features
// =============================================================================

void CheckHack()
{
#ifdef NEW_PROTOCOL_SYSTEM
    gProtocolSend.SendCheckOnline();
#else
    if (!g_bGameServerConnected || CharacterAttribute == nullptr)
    {
        return;
    }

    const SOCKET socket = SocketClient.GetSocket();
    if (socket == INVALID_SOCKET)
    {
        return;
    }

    WORD physSpeed = CharacterAttribute->AttackSpeed;
    WORD magicSpeed = CharacterAttribute->MagicSpeed;
    const DWORD currentTick = static_cast<DWORD>(MU_MobileGetTicks());

    if ((CharacterAttribute->Ability & ABILITY_FAST_ATTACK_SPEED) != 0
        || (CharacterAttribute->Ability & ABILITY_FAST_ATTACK_SPEED2) != 0)
    {
        physSpeed = static_cast<WORD>(std::max<int>(0, static_cast<int>(physSpeed) - 20));
        magicSpeed = static_cast<WORD>(std::max<int>(0, static_cast<int>(magicSpeed) - 20));
    }

    uint8_t packet[12] = {};
    packet[0] = 0xC1;
    packet[1] = sizeof(packet);
    packet[2] = 0x0E;
    std::memcpy(packet + 4, &currentTick, sizeof(DWORD));
    std::memcpy(packet + 8, &physSpeed, sizeof(WORD));
    std::memcpy(packet + 10, &magicSpeed, sizeof(WORD));

    mu::Xor32Encrypt(packet, static_cast<int>(sizeof(packet)), 2);

    BYTE encryptSource[MAX_SPE_BUFFERSIZE_] = {};
    std::memcpy(encryptSource, packet, sizeof(packet));
    encryptSource[1] = g_byPacketSerialSend++;

    PBMSG_ENCRYPTED encrypted {};
    const int encryptedBodySize = g_SimpleModulusCS.Encrypt(nullptr, encryptSource + 1, static_cast<int>(sizeof(packet)) - 1);
    if (encryptedBodySize <= 0 || encryptedBodySize >= 256)
    {
        return;
    }

    encrypted.Code = 0xC3;
    encrypted.Size = static_cast<BYTE>(encryptedBodySize + 2);
    g_SimpleModulusCS.Encrypt(encrypted.byBuffer, encryptSource + 1, static_cast<int>(sizeof(packet)) - 1);
    SocketClient.sSend(socket, reinterpret_cast<char*>(&encrypted), encrypted.Size);

    if (!First)
    {
        First = true;
        FirstTime = static_cast<int>(currentTick);
    }
#endif
}
DWORD GetCheckSum(WORD /*wKey*/) { return 0; }  // GameGuard not on Android

void SetMaxMessagePerCycle(int messages)
{
    constexpr int custom_min = 3;
    g_MaxMessagePerCycle = (messages > 0) ? std::max<int>(messages, custom_min) : messages;
}

static uint32_t g_androidWorldLoadGraceUntil = 0;

void TakumiAndroidOnWorldJoinLoaded()
{
    g_androidWorldLoadGraceUntil = MU_MobileGetTicks() + 8000u;
    const int restoreBudget = (g_MaxMessagePerCycle > 0 && g_MaxMessagePerCycle < 90)
        ? 90
        : 120;
    if (g_MaxMessagePerCycle < restoreBudget)
    {
        SetMaxMessagePerCycle(restoreBudget);
    }
    LOGI("Android post-join grace 8s (maxMsgPerCycle=%d)", g_MaxMessagePerCycle);
}

namespace
{
    float g_currentAdaptiveEffectScale = 1.0f;

    struct AdaptivePerfState
    {
        bool enabled = true;
        bool initialized = false;
        bool isEmulator = false;
        // User controls render-level slider. Adaptive keeps effects ON and
        // only changes effect spawn density + packet budget under stress.
        int lowFpsStreak = 0;
        int highFpsStreak = 0;
        int defaultMessageBudget = 120;
        int minMessageBudget = 90;
        double targetFps = -1.0;        // -1 = uncapped, run at full GPU speed
        double lowFpsThreshold = 56.0;
        double highFpsThreshold = 62.0;
        double minEffectScale = 0.55;
        double effectScaleStep = 0.10;
        double effectScaleHysteresis = 0.08;
        uint32_t fxAdjustCooldownMs = 1200;
        uint32_t lastFxAdjustTick = 0;
        int minAdaptiveRenderLevel = 2;
        int maxAdaptiveRenderLevel = 4;
        int renderDownStreak = 0;
        int renderUpStreak = 0;
        uint32_t renderAdjustCooldownMs = 3000;
        uint32_t lastRenderAdjustTick = 0;
    };

    AdaptivePerfState g_adaptivePerf;

    static int ClampRenderLevel(int level)
    {
        return std::clamp(level, 0, 4);
    }

    static int CountLiveCharacters()
    {
        if (!CharactersClient) return 0;
        int count = 0;
        for (int i = 0; i < MAX_CHARACTERS_CLIENT; ++i)
        {
            if (CharactersClient[i].Object.Live) ++count;
        }
        return count;
    }

    struct CrowdTypeEntry
    {
        int type = -1;
        int count = 0;
    };

    struct SceneCrowdSnapshot
    {
        int liveCharacters = 0;
        int visibleCharacters = 0;
        int visiblePlayers = 0;
        int visibleMonsters = 0;
        int visibleNpcs = 0;
        int visiblePets = 0;
        int visiblePriorityCharacters = 0;
        int liveWorldObjects = 0;
        int visibleWorldObjects = 0;
        int visibleOperateObjects = 0;
        int visibleTrapObjects = 0;
        int dominantMonsterType = -1;
        int dominantMonsterCount = 0;
        int dominantObjectType = -1;
        int dominantObjectCount = 0;
    };

    static void TrackCrowdType(std::array<CrowdTypeEntry, 8>& slots, const int type)
    {
        for (CrowdTypeEntry& slot : slots)
        {
            if (slot.count > 0 && slot.type == type)
            {
                ++slot.count;
                return;
            }
        }

        for (CrowdTypeEntry& slot : slots)
        {
            if (slot.count == 0)
            {
                slot.type = type;
                slot.count = 1;
                return;
            }
        }

        CrowdTypeEntry* weakest = &slots[0];
        for (CrowdTypeEntry& slot : slots)
        {
            if (slot.count < weakest->count)
            {
                weakest = &slot;
            }
        }

        if (weakest->count <= 1)
        {
            weakest->type = type;
            weakest->count = 1;
        }
    }

    static CrowdTypeEntry FindDominantCrowdType(const std::array<CrowdTypeEntry, 8>& slots)
    {
        CrowdTypeEntry dominant;
        for (const CrowdTypeEntry& slot : slots)
        {
            if (slot.count > dominant.count)
            {
                dominant = slot;
            }
        }
        return dominant;
    }

    static SceneCrowdSnapshot CaptureSceneCrowdSnapshot()
    {
        SceneCrowdSnapshot snapshot;
        std::array<CrowdTypeEntry, 8> monsterTypes = {};
        std::array<CrowdTypeEntry, 8> objectTypes = {};

        if (CharactersClient)
        {
            for (int i = 0; i < MAX_CHARACTERS_CLIENT; ++i)
            {
                CHARACTER* c = &CharactersClient[i];
                OBJECT* o = &c->Object;
                if (!o->Live)
                {
                    continue;
                }

                ++snapshot.liveCharacters;
                if (!o->Visible)
                {
                    continue;
                }

                ++snapshot.visibleCharacters;
                const bool isPriority =
                    (c == Hero) ||
                    (i == SelectedCharacter) ||
                    (i == SelectedNpc) ||
                    (Hero != nullptr && Hero->TargetCharacter == c->Key);
                if (isPriority)
                {
                    ++snapshot.visiblePriorityCharacters;
                }

                switch (o->Kind)
                {
                case KIND_PLAYER:
                    ++snapshot.visiblePlayers;
                    break;
                case KIND_MONSTER:
                    ++snapshot.visibleMonsters;
                    TrackCrowdType(monsterTypes, o->Type);
                    break;
                case KIND_NPC:
                    ++snapshot.visibleNpcs;
                    break;
                case KIND_PET:
                    ++snapshot.visiblePets;
                    break;
                default:
                    break;
                }
            }
        }

        for (int block = 0; block < 256; ++block)
        {
            OBJECT* o = ObjectBlock[block].Head;
            while (o != nullptr)
            {
                if (o->Live)
                {
                    ++snapshot.liveWorldObjects;
                    if (o->Visible)
                    {
                        ++snapshot.visibleWorldObjects;
                        if (o->Kind == KIND_OPERATE)
                        {
                            ++snapshot.visibleOperateObjects;
                        }
                        else if (o->Kind == KIND_TRAP)
                        {
                            ++snapshot.visibleTrapObjects;
                        }

                        if (o->Kind != KIND_PLAYER && o->Kind != KIND_MONSTER)
                        {
                            TrackCrowdType(objectTypes, o->Type);
                        }
                    }
                }
                o = o->Next;
            }
        }

        const CrowdTypeEntry dominantMonster = FindDominantCrowdType(monsterTypes);
        snapshot.dominantMonsterType = dominantMonster.type;
        snapshot.dominantMonsterCount = dominantMonster.count;

        const CrowdTypeEntry dominantObject = FindDominantCrowdType(objectTypes);
        snapshot.dominantObjectType = dominantObject.type;
        snapshot.dominantObjectCount = dominantObject.count;

        return snapshot;
    }

    static const char* GetPerfMonsterDebugName(const int type)
    {
        if (type < 0)
        {
            return "n/a";
        }

        const char* const name = getMonsterName(type);
        return (name != nullptr && name[0] != '\0') ? name : "unnamed";
    }

    static const char* GetPerfObjectDebugName(const int type)
    {
        if (type < 0 || type >= MAX_MODELS)
        {
            return "n/a";
        }

        const char* const name = Models[type].Name;
        return (name != nullptr && name[0] != '\0') ? name : "unnamed";
    }

    static const char* GetPerfWorldDebugName(const int world)
    {
        if (world < 0 || world >= NUM_WD)
        {
            return "unknown";
        }

        const char* const name = gMapManager.GetMapName(world);
        return (name != nullptr && name[0] != '\0') ? name : "unknown";
    }

    static void UpdateAdaptivePerformance(double fps, double frameMs)
    {
        if (!g_adaptivePerf.enabled)
            return;

        // Keep all effects enabled; only adjust spawn density to smooth spikes.
        // Render-level slider remains user-controlled.

        const uint32_t nowTick = MU_MobileGetTicks();
        const bool inWorldLoadGrace =
            g_androidWorldLoadGraceUntil != 0 && nowTick < g_androidWorldLoadGraceUntil;
        if (inWorldLoadGrace)
        {
            g_adaptivePerf.lowFpsStreak = 0;
            g_adaptivePerf.renderDownStreak = 0;
        }

        if (fps < g_adaptivePerf.lowFpsThreshold)
        {
            ++g_adaptivePerf.lowFpsStreak;
            g_adaptivePerf.highFpsStreak = 0;
        }
        else if (fps > g_adaptivePerf.highFpsThreshold)
        {
            ++g_adaptivePerf.highFpsStreak;
            g_adaptivePerf.lowFpsStreak = 0;
        }
        else
        {
            g_adaptivePerf.lowFpsStreak  = std::max(0, g_adaptivePerf.lowFpsStreak  - 1);
            g_adaptivePerf.highFpsStreak = std::max(0, g_adaptivePerf.highFpsStreak - 1);
        }

        double targetEffectScale = 1.0;
        if (frameMs >= 52.0 || fps < 18.0)
        {
            targetEffectScale = g_adaptivePerf.minEffectScale;
        }
        else if (frameMs >= 42.0 || fps < 26.0)
        {
            targetEffectScale = std::max(g_adaptivePerf.minEffectScale, 0.65);
        }
        else if (frameMs >= 34.0 || fps < 36.0)
        {
            targetEffectScale = 0.80;
        }
        else if (frameMs >= 29.0 || fps < 48.0)
        {
            targetEffectScale = 0.90;
        }

        const double currentEffectScale = g_currentAdaptiveEffectScale;
        if (std::abs(currentEffectScale - targetEffectScale) >= g_adaptivePerf.effectScaleHysteresis &&
            (g_adaptivePerf.lastFxAdjustTick == 0 ||
                (nowTick - g_adaptivePerf.lastFxAdjustTick) >= g_adaptivePerf.fxAdjustCooldownMs))
        {
            double nextEffectScale = currentEffectScale;
            if (targetEffectScale > currentEffectScale)
            {
                nextEffectScale = std::min(targetEffectScale, currentEffectScale + g_adaptivePerf.effectScaleStep);
            }
            else
            {
                nextEffectScale = std::max(targetEffectScale, currentEffectScale - g_adaptivePerf.effectScaleStep);
            }

            g_currentAdaptiveEffectScale = static_cast<float>(std::clamp(nextEffectScale, g_adaptivePerf.minEffectScale, 1.0));
            g_adaptivePerf.lastFxAdjustTick = nowTick;
            LOGI("ADAPT fx scale: fps=%.1f frameMs=%.2f %.2f->%.2f target=%.2f",
                fps, frameMs, currentEffectScale, g_currentAdaptiveEffectScale, targetEffectScale);
        }

        if (g_pOption && !g_adaptivePerf.isEmulator && !inWorldLoadGrace)
        {
            const int currentRenderLevel = ClampRenderLevel(g_pOption->GetRenderLevel());
            const bool shouldRenderDown = (frameMs >= 42.0 || fps < 24.0);
            const bool shouldRenderUp = (frameMs <= 24.0 && fps > 52.0);
            if (shouldRenderDown)
            {
                ++g_adaptivePerf.renderDownStreak;
                g_adaptivePerf.renderUpStreak = 0;
            }
            else if (shouldRenderUp)
            {
                ++g_adaptivePerf.renderUpStreak;
                g_adaptivePerf.renderDownStreak = 0;
            }
            else
            {
                g_adaptivePerf.renderDownStreak = std::max(0, g_adaptivePerf.renderDownStreak - 1);
                g_adaptivePerf.renderUpStreak = std::max(0, g_adaptivePerf.renderUpStreak - 1);
            }

            if (g_adaptivePerf.lastRenderAdjustTick == 0 ||
                (nowTick - g_adaptivePerf.lastRenderAdjustTick) >= g_adaptivePerf.renderAdjustCooldownMs)
            {
                if (g_adaptivePerf.renderDownStreak >= 3 &&
                    currentRenderLevel > g_adaptivePerf.minAdaptiveRenderLevel)
                {
                    const int newRenderLevel = currentRenderLevel - 1;
                    g_pOption->SetRenderLevel(newRenderLevel);
                    g_adaptivePerf.lastRenderAdjustTick = nowTick;
                    g_adaptivePerf.renderDownStreak = 0;
                    g_adaptivePerf.renderUpStreak = 0;
                    LOGW("ADAPT render level down: fps=%.1f frameMs=%.2f %d->%d",
                        fps, frameMs, currentRenderLevel, newRenderLevel);
                }
                else if (g_adaptivePerf.renderUpStreak >= 4 &&
                    currentRenderLevel < g_adaptivePerf.maxAdaptiveRenderLevel)
                {
                    const int newRenderLevel = currentRenderLevel + 1;
                    g_pOption->SetRenderLevel(newRenderLevel);
                    g_adaptivePerf.lastRenderAdjustTick = nowTick;
                    g_adaptivePerf.renderDownStreak = 0;
                    g_adaptivePerf.renderUpStreak = 0;
                    LOGI("ADAPT render level up: fps=%.1f frameMs=%.2f %d->%d",
                        fps, frameMs, currentRenderLevel, newRenderLevel);
                }
            }
        }

        // Reduce per-frame packet budget if FPS stays low for 3+ windows.
        if (!inWorldLoadGrace &&
            g_adaptivePerf.lowFpsStreak >= 3 &&
            g_MaxMessagePerCycle > g_adaptivePerf.minMessageBudget)
        {
            const int newBudget = std::max(g_adaptivePerf.minMessageBudget,
                g_MaxMessagePerCycle - 10);
            LOGW("ADAPT net budget down: fps=%.1f frameMs=%.2f maxMsg %d->%d",
                fps, frameMs, g_MaxMessagePerCycle, newBudget);
            SetMaxMessagePerCycle(newBudget);
            return;
        }

        // Recover packet budget once FPS is stable.
        if (g_adaptivePerf.highFpsStreak >= 4 &&
            g_MaxMessagePerCycle < g_adaptivePerf.defaultMessageBudget)
        {
            g_adaptivePerf.highFpsStreak = 0;
            g_adaptivePerf.lowFpsStreak  = 0;
            const int newBudget = std::min(g_adaptivePerf.defaultMessageBudget,
                g_MaxMessagePerCycle + 5);
            LOGI("ADAPT net budget up: fps=%.1f frameMs=%.2f maxMsg %d->%d",
                fps, frameMs, g_MaxMessagePerCycle, newBudget);
            SetMaxMessagePerCycle(newBudget);
        }
    }
}

GLvoid KillGLWindow(GLvoid)
{
    g_hDC = nullptr;
    g_hRC = nullptr;
}

void DestroySound()
{
    if (!g_AndroidAudioAvailable)
    {
        return;
    }

    Mix_HaltMusic();
    Mix_HaltChannel(-1);
    if (g_pCurrentMusic) { Mix_FreeMusic(g_pCurrentMusic); g_pCurrentMusic = nullptr; }
    Mix_CloseAudio();
    Mix_Quit();
    g_AndroidAudioAvailable = false;
    LOGI("Audio destroyed");
}

void DestroyWindow_Android()
{
    GameConfig::GetInstance().SetVolumeLevel(g_pOption ? g_pOption->GetVolumeLevel() : 5);
    GameConfig::GetInstance().Save();

    CUIMng::Instance().Release();
    ReleaseCharacters();

    SAFE_DELETE(GateAttribute);
    SAFE_DELETE(SkillAttribute);
    SAFE_DELETE(CharacterMachine);
    

    gMapManager.DeleteObjects();
    for (int i = MODEL_LOGO; i < MAX_MODELS; i++) Models[i].Release();
    Bitmaps.UnloadAllImages();

    SAFE_DELETE_ARRAY(CharacterMemoryDump);
    SAFE_DELETE_ARRAY(ItemAttRibuteMemoryDump);
    SAFE_DELETE_ARRAY(RendomMemoryDump);
    SAFE_DELETE_ARRAY(ModelsDump);

    SAFE_DELETE(g_pMercenaryInputBox);
    SAFE_DELETE(g_pSingleTextInputBox);
    SAFE_DELETE(g_pSinglePasswdInputBox);
    SAFE_DELETE(g_pUIMapName);
    SAFE_DELETE(g_pTimer);
    SAFE_DELETE(g_pUIManager);
    SAFE_DELETE(pMultiLanguage);




    if (g_hFont) {
        DeleteObject((HGDIOBJ)g_hFont);
        g_hFont = nullptr;
    }
    if (g_hFontBold) {
        DeleteObject((HGDIOBJ)g_hFontBold);
        g_hFontBold = nullptr;
    }
    if (g_hFontBig) {
        DeleteObject((HGDIOBJ)g_hFontBig);
        g_hFontBig = nullptr;
    }
    if (g_hFixFont) {
        DeleteObject((HGDIOBJ)g_hFixFont);
        g_hFixFont = nullptr;
    }
    AndroidGDI_Shutdown();

    LOGI("Game systems destroyed");
}

static SDL_Keymod ConvertSappModifiersToSDLKeymod(uint32_t modifiers)
{
    int result = KMOD_NONE;
    if ((modifiers & SAPP_MODIFIER_SHIFT) != 0) result |= KMOD_SHIFT;
    if ((modifiers & SAPP_MODIFIER_CTRL) != 0)  result |= KMOD_CTRL;
    if ((modifiers & SAPP_MODIFIER_ALT) != 0)   result |= KMOD_ALT;
    if ((modifiers & SAPP_MODIFIER_SUPER) != 0) result |= KMOD_GUI;
    return static_cast<SDL_Keymod>(result);
}

static SDL_Keycode ConvertSappKeycodeToSDLKeycode(sapp_keycode keycode)
{
    if ((keycode >= SAPP_KEYCODE_SPACE && keycode <= SAPP_KEYCODE_GRAVE_ACCENT) ||
        (keycode >= SAPP_KEYCODE_0 && keycode <= SAPP_KEYCODE_9) ||
        (keycode >= SAPP_KEYCODE_A && keycode <= SAPP_KEYCODE_Z))
    {
        return static_cast<SDL_Keycode>(keycode);
    }

    switch (keycode)
    {
    case SAPP_KEYCODE_ENTER:        return SDLK_RETURN;
    case SAPP_KEYCODE_TAB:          return SDLK_TAB;
    case SAPP_KEYCODE_BACKSPACE:    return SDLK_BACKSPACE;
    case SAPP_KEYCODE_INSERT:       return SDLK_INSERT;
    case SAPP_KEYCODE_DELETE:       return SDLK_DELETE;
    case SAPP_KEYCODE_RIGHT:        return SDLK_RIGHT;
    case SAPP_KEYCODE_LEFT:         return SDLK_LEFT;
    case SAPP_KEYCODE_DOWN:         return SDLK_DOWN;
    case SAPP_KEYCODE_UP:           return SDLK_UP;
    case SAPP_KEYCODE_PAGE_UP:      return SDLK_PAGEUP;
    case SAPP_KEYCODE_PAGE_DOWN:    return SDLK_PAGEDOWN;
    case SAPP_KEYCODE_HOME:         return SDLK_HOME;
    case SAPP_KEYCODE_END:          return SDLK_END;
    case SAPP_KEYCODE_CAPS_LOCK:    return SDLK_CAPSLOCK;
    case SAPP_KEYCODE_SCROLL_LOCK:  return SDLK_SCROLLLOCK;
    case SAPP_KEYCODE_NUM_LOCK:     return SDLK_NUMLOCKCLEAR;
    case SAPP_KEYCODE_PRINT_SCREEN: return SDLK_PRINTSCREEN;
    case SAPP_KEYCODE_PAUSE:        return SDLK_PAUSE;
    case SAPP_KEYCODE_F1:           return SDLK_F1;
    case SAPP_KEYCODE_F2:           return SDLK_F2;
    case SAPP_KEYCODE_F3:           return SDLK_F3;
    case SAPP_KEYCODE_F4:           return SDLK_F4;
    case SAPP_KEYCODE_F5:           return SDLK_F5;
    case SAPP_KEYCODE_F6:           return SDLK_F6;
    case SAPP_KEYCODE_F7:           return SDLK_F7;
    case SAPP_KEYCODE_F8:           return SDLK_F8;
    case SAPP_KEYCODE_F9:           return SDLK_F9;
    case SAPP_KEYCODE_F10:          return SDLK_F10;
    case SAPP_KEYCODE_F11:          return SDLK_F11;
    case SAPP_KEYCODE_F12:          return SDLK_F12;
    case SAPP_KEYCODE_KP_0:         return SDLK_KP_0;
    case SAPP_KEYCODE_KP_1:         return SDLK_KP_1;
    case SAPP_KEYCODE_KP_2:         return SDLK_KP_2;
    case SAPP_KEYCODE_KP_3:         return SDLK_KP_3;
    case SAPP_KEYCODE_KP_4:         return SDLK_KP_4;
    case SAPP_KEYCODE_KP_5:         return SDLK_KP_5;
    case SAPP_KEYCODE_KP_6:         return SDLK_KP_6;
    case SAPP_KEYCODE_KP_7:         return SDLK_KP_7;
    case SAPP_KEYCODE_KP_8:         return SDLK_KP_8;
    case SAPP_KEYCODE_KP_9:         return SDLK_KP_9;
    case SAPP_KEYCODE_KP_DECIMAL:   return SDLK_KP_PERIOD;
    case SAPP_KEYCODE_KP_DIVIDE:    return SDLK_KP_DIVIDE;
    case SAPP_KEYCODE_KP_MULTIPLY:  return SDLK_KP_MULTIPLY;
    case SAPP_KEYCODE_KP_SUBTRACT:  return SDLK_KP_MINUS;
    case SAPP_KEYCODE_KP_ADD:       return SDLK_KP_PLUS;
    case SAPP_KEYCODE_KP_ENTER:     return SDLK_KP_ENTER;
    case SAPP_KEYCODE_LEFT_SHIFT:   return SDLK_LSHIFT;
    case SAPP_KEYCODE_LEFT_CONTROL: return SDLK_LCTRL;
    case SAPP_KEYCODE_LEFT_ALT:     return SDLK_LALT;
    case SAPP_KEYCODE_LEFT_SUPER:   return SDLK_LGUI;
    case SAPP_KEYCODE_RIGHT_SHIFT:  return SDLK_RSHIFT;
    case SAPP_KEYCODE_RIGHT_CONTROL:return SDLK_RCTRL;
    case SAPP_KEYCODE_RIGHT_ALT:    return SDLK_RALT;
    case SAPP_KEYCODE_RIGHT_SUPER:  return SDLK_RGUI;
    case SAPP_KEYCODE_MENU:         return SDLK_MENU;
    case SAPP_KEYCODE_ESCAPE:       return SDLK_AC_BACK;
    default:                        return SDLK_UNKNOWN;
    }
}

static SDL_Scancode ConvertSappKeycodeToSDLScancode(sapp_keycode keycode)
{
    if (keycode >= SAPP_KEYCODE_A && keycode <= SAPP_KEYCODE_Z)
    {
        return static_cast<SDL_Scancode>(SDL_SCANCODE_A + (keycode - SAPP_KEYCODE_A));
    }
    if (keycode >= SAPP_KEYCODE_0 && keycode <= SAPP_KEYCODE_9)
    {
        return static_cast<SDL_Scancode>(SDL_SCANCODE_0 + (keycode - SAPP_KEYCODE_0));
    }
    if (keycode >= SAPP_KEYCODE_F1 && keycode <= SAPP_KEYCODE_F12)
    {
        return static_cast<SDL_Scancode>(SDL_SCANCODE_F1 + (keycode - SAPP_KEYCODE_F1));
    }
    if (keycode >= SAPP_KEYCODE_KP_0 && keycode <= SAPP_KEYCODE_KP_9)
    {
        return static_cast<SDL_Scancode>(SDL_SCANCODE_KP_0 + (keycode - SAPP_KEYCODE_KP_0));
    }

    switch (keycode)
    {
    case SAPP_KEYCODE_SPACE:         return SDL_SCANCODE_SPACE;
    case SAPP_KEYCODE_APOSTROPHE:    return SDL_SCANCODE_APOSTROPHE;
    case SAPP_KEYCODE_COMMA:         return SDL_SCANCODE_COMMA;
    case SAPP_KEYCODE_MINUS:         return SDL_SCANCODE_MINUS;
    case SAPP_KEYCODE_PERIOD:        return SDL_SCANCODE_PERIOD;
    case SAPP_KEYCODE_SLASH:         return SDL_SCANCODE_SLASH;
    case SAPP_KEYCODE_SEMICOLON:     return SDL_SCANCODE_SEMICOLON;
    case SAPP_KEYCODE_EQUAL:         return SDL_SCANCODE_EQUALS;
    case SAPP_KEYCODE_LEFT_BRACKET:  return SDL_SCANCODE_LEFTBRACKET;
    case SAPP_KEYCODE_BACKSLASH:     return SDL_SCANCODE_BACKSLASH;
    case SAPP_KEYCODE_RIGHT_BRACKET: return SDL_SCANCODE_RIGHTBRACKET;
    case SAPP_KEYCODE_GRAVE_ACCENT:  return SDL_SCANCODE_GRAVE;
    case SAPP_KEYCODE_ENTER:         return SDL_SCANCODE_RETURN;
    case SAPP_KEYCODE_TAB:           return SDL_SCANCODE_TAB;
    case SAPP_KEYCODE_BACKSPACE:     return SDL_SCANCODE_BACKSPACE;
    case SAPP_KEYCODE_INSERT:        return SDL_SCANCODE_INSERT;
    case SAPP_KEYCODE_DELETE:        return SDL_SCANCODE_DELETE;
    case SAPP_KEYCODE_RIGHT:         return SDL_SCANCODE_RIGHT;
    case SAPP_KEYCODE_LEFT:          return SDL_SCANCODE_LEFT;
    case SAPP_KEYCODE_DOWN:          return SDL_SCANCODE_DOWN;
    case SAPP_KEYCODE_UP:            return SDL_SCANCODE_UP;
    case SAPP_KEYCODE_PAGE_UP:       return SDL_SCANCODE_PAGEUP;
    case SAPP_KEYCODE_PAGE_DOWN:     return SDL_SCANCODE_PAGEDOWN;
    case SAPP_KEYCODE_HOME:          return SDL_SCANCODE_HOME;
    case SAPP_KEYCODE_END:           return SDL_SCANCODE_END;
    case SAPP_KEYCODE_CAPS_LOCK:     return SDL_SCANCODE_CAPSLOCK;
    case SAPP_KEYCODE_SCROLL_LOCK:   return SDL_SCANCODE_SCROLLLOCK;
    case SAPP_KEYCODE_NUM_LOCK:      return SDL_SCANCODE_NUMLOCKCLEAR;
    case SAPP_KEYCODE_PRINT_SCREEN:  return SDL_SCANCODE_PRINTSCREEN;
    case SAPP_KEYCODE_PAUSE:         return SDL_SCANCODE_PAUSE;
    case SAPP_KEYCODE_KP_DECIMAL:    return SDL_SCANCODE_KP_PERIOD;
    case SAPP_KEYCODE_KP_DIVIDE:     return SDL_SCANCODE_KP_DIVIDE;
    case SAPP_KEYCODE_KP_MULTIPLY:   return SDL_SCANCODE_KP_MULTIPLY;
    case SAPP_KEYCODE_KP_SUBTRACT:   return SDL_SCANCODE_KP_MINUS;
    case SAPP_KEYCODE_KP_ADD:        return SDL_SCANCODE_KP_PLUS;
    case SAPP_KEYCODE_KP_ENTER:      return SDL_SCANCODE_KP_ENTER;
    case SAPP_KEYCODE_LEFT_SHIFT:    return SDL_SCANCODE_LSHIFT;
    case SAPP_KEYCODE_LEFT_CONTROL:  return SDL_SCANCODE_LCTRL;
    case SAPP_KEYCODE_LEFT_ALT:      return SDL_SCANCODE_LALT;
    case SAPP_KEYCODE_LEFT_SUPER:    return SDL_SCANCODE_LGUI;
    case SAPP_KEYCODE_RIGHT_SHIFT:   return SDL_SCANCODE_RSHIFT;
    case SAPP_KEYCODE_RIGHT_CONTROL: return SDL_SCANCODE_RCTRL;
    case SAPP_KEYCODE_RIGHT_ALT:     return SDL_SCANCODE_RALT;
    case SAPP_KEYCODE_RIGHT_SUPER:   return SDL_SCANCODE_RGUI;
    case SAPP_KEYCODE_MENU:          return SDL_SCANCODE_APPLICATION;
    case SAPP_KEYCODE_ESCAPE:        return SDL_SCANCODE_ESCAPE;
    default:                         return SDL_SCANCODE_UNKNOWN;
    }
}

static void UpdateKeyboardStateFromSappEvent(const sapp_event* event)
{
    if (!event || ((event->type != SAPP_EVENTTYPE_KEY_DOWN) && (event->type != SAPP_EVENTTYPE_KEY_UP)))
    {
        return;
    }

    const SDL_Scancode scancode = ConvertSappKeycodeToSDLScancode(event->key_code);
    if (scancode != SDL_SCANCODE_UNKNOWN)
    {
        MU_MobileSetKeyState(scancode, event->type == SAPP_EVENTTYPE_KEY_DOWN);
    }
}

static void EncodeUtf32ToUtf8(uint32_t charCode, char out[SDL_TEXTINPUTEVENT_TEXT_SIZE])
{
    std::memset(out, 0, SDL_TEXTINPUTEVENT_TEXT_SIZE);
    if (charCode == 0 || charCode > 0x10FFFF)
    {
        return;
    }

    if (charCode <= 0x7F)
    {
        out[0] = static_cast<char>(charCode);
    }
    else if (charCode <= 0x7FF)
    {
        out[0] = static_cast<char>(0xC0 | ((charCode >> 6) & 0x1F));
        out[1] = static_cast<char>(0x80 | (charCode & 0x3F));
    }
    else if (charCode <= 0xFFFF)
    {
        out[0] = static_cast<char>(0xE0 | ((charCode >> 12) & 0x0F));
        out[1] = static_cast<char>(0x80 | ((charCode >> 6) & 0x3F));
        out[2] = static_cast<char>(0x80 | (charCode & 0x3F));
    }
    else
    {
        out[0] = static_cast<char>(0xF0 | ((charCode >> 18) & 0x07));
        out[1] = static_cast<char>(0x80 | ((charCode >> 12) & 0x3F));
        out[2] = static_cast<char>(0x80 | ((charCode >> 6) & 0x3F));
        out[3] = static_cast<char>(0x80 | (charCode & 0x3F));
    }
}

// =============================================================================
// Input mapping: SDL events â†’ game mouse/key globals
// Maps SDL touch/mouse events to the same globals that WndProc sets on Windows.
// =============================================================================

static wchar_t TranslateKeyToAsciiFallback(const SDL_KeyboardEvent& keyEvent)
{
    const SDL_Keycode key = keyEvent.keysym.sym;
    const SDL_Keymod modifiers = static_cast<SDL_Keymod>(keyEvent.keysym.mod);
    const bool shift = (modifiers & KMOD_SHIFT) != 0;
    const bool caps = (modifiers & KMOD_CAPS) != 0;

    if ((modifiers & (KMOD_CTRL | KMOD_ALT | KMOD_GUI)) != 0)
    {
        return 0;
    }

    if (key >= SDLK_a && key <= SDLK_z)
    {
        const wchar_t base = (shift ^ caps) ? L'A' : L'a';
        return static_cast<wchar_t>(base + (key - SDLK_a));
    }

    if (key >= SDLK_0 && key <= SDLK_9)
    {
        if (!shift)
        {
            return static_cast<wchar_t>(L'0' + (key - SDLK_0));
        }

        static const wchar_t shiftedDigits[] = L")!@#$%^&*(";
        return shiftedDigits[key - SDLK_0];
    }

    if (key >= SDLK_KP_0 && key <= SDLK_KP_9)
    {
        return static_cast<wchar_t>(L'0' + (key - SDLK_KP_0));
    }

    switch (key)
    {
    case SDLK_SPACE:      return L' ';
    case SDLK_MINUS:      return shift ? L'_' : L'-';
    case SDLK_EQUALS:     return shift ? L'+' : L'=';
    case SDLK_LEFTBRACKET:return shift ? L'{' : L'[';
    case SDLK_RIGHTBRACKET:return shift ? L'}' : L']';
    case SDLK_BACKSLASH:  return shift ? L'|' : L'\\';
    case SDLK_SEMICOLON:  return shift ? L':' : L';';
    case SDLK_QUOTE:      return shift ? L'"' : L'\'';
    case SDLK_COMMA:      return shift ? L'<' : L',';
    case SDLK_PERIOD:     return shift ? L'>' : L'.';
    case SDLK_SLASH:      return shift ? L'?' : L'/';
    case SDLK_BACKQUOTE:  return shift ? L'~' : L'`';
    case SDLK_KP_PERIOD:  return L'.';
    case SDLK_KP_DIVIDE:  return L'/';
    case SDLK_KP_MULTIPLY:return L'*';
    case SDLK_KP_MINUS:   return L'-';
    case SDLK_KP_PLUS:    return L'+';
    default:
        break;
    }

    return 0;
}

static constexpr bool kLogInputEvents = false;

bool IsAggressiveMobilePerfModeEnabled()
{
#if defined(__ANDROID__) || defined(MU_IOS)
    return g_adaptivePerf.isEmulator;
#else
    return false;
#endif
}

static void ApplyAndroidDrawableSize(int screenW, int screenH, const char* reason)
{
    if ((screenW <= 1) || (screenH <= 1))
    {
        return;
    }

    const bool changed =
        (screenW != g_DrawableWidth) ||
        (screenH != g_DrawableHeight) ||
        (static_cast<int>(WindowWidth) != screenW) ||
        (static_cast<int>(WindowHeight) != screenH);
    if (!changed)
    {
        return;
    }

    g_DrawableWidth = screenW;
    g_DrawableHeight = screenH;
    UpdateAndroidScreenMetrics(screenW, screenH);

    if (g_RenderBackend)
    {
        g_RenderBackend->OnDrawableSizeChanged(screenW, screenH);
    }
    else
    {
        glViewport(0, 0, screenW, screenH);
    }

    if (g_hWnd)
    {
        CInput::Instance().Create(g_hWnd, static_cast<long>(WindowWidth), static_cast<long>(WindowHeight));
    }

    LOGI(
        "Drawable sync (%s) -> %dx%d",
        (reason && reason[0]) ? reason : "unknown",
        screenW,
        screenH);
}

static void SyncAndroidDrawableSizeFromSokol(const char* reason)
{
    const int screenW = sapp_width();
    const int screenH = sapp_height();
    ApplyAndroidDrawableSize(screenW, screenH, reason);
}

static void HandleSDLEvent(const SDL_Event& ev, int& screenW, int& screenH)
{
    switch (ev.type)
    {
    // â”€â”€ App lifecycle â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    case SDL_QUIT:
        Destroy = true;
        break;

    case SDL_USEREVENT:
        if (ev.user.code == kSdlUserEventLoginIntroMovieFinished)
        {
            CUIMng::Instance().SetMoving(false);
            ::PlayMp3(g_lpszMp3[MUSIC_MAIN_THEME]);
        }
        break;

    case SDL_APP_WILLENTERBACKGROUND:
    case SDL_APP_DIDENTERBACKGROUND:
        g_bWndActive = false;
        g_primaryTouchFinger = -1;
        MU_MobileClearKeyboardState();
        ClearVirtualJoystick();
        // Release held buttons when minimized
        MouseLButton = MouseRButton = MouseMButton = false;
        MouseLButtonPop = MouseRButtonPop = MouseMButtonPop = false;
        MouseLButtonPush = MouseRButtonPush = MouseMButtonPush = false;
        MouseWheel = 0;
        break;

    case SDL_APP_WILLENTERFOREGROUND:
    case SDL_APP_DIDENTERFOREGROUND:
        g_bWndActive = true;
        break;

    // â”€â”€ Mouse motion (SDL2 maps single-touch â†’ mouse on Android) â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    case SDL_MOUSEMOTION:
    {
        if (ev.motion.which == SDL_TOUCH_MOUSEID && g_seenFingerInput) break;
        if (g_joystickPcMouseCaptured)
        {
            const float uiX = std::clamp(static_cast<float>(ev.motion.x) * 640.0f / static_cast<float>(screenW > 0 ? screenW : 1), 0.0f, 640.0f);
            const float uiY = std::clamp(static_cast<float>(ev.motion.y) * 480.0f / static_cast<float>(screenH > 0 ? screenH : 1), 0.0f, 480.0f);
            if (ev.motion.state & SDL_BUTTON_LMASK)
                UpdateVirtualJoystickByUi(uiX, uiY);
            else
            { g_joystickPcMouseCaptured = false; ClearVirtualJoystick(); }
            break;
        }
        UpdateMouseFromPixel(ev.motion.x, ev.motion.y, screenW, screenH);
        break;
    }

    // â”€â”€ Mouse buttons â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    case SDL_MOUSEBUTTONDOWN:
        if (ev.button.which == SDL_TOUCH_MOUSEID && g_seenFingerInput) break;
        g_iNoMouseTime = 0;
        UpdateMouseFromPixel(ev.button.x, ev.button.y, screenW, screenH);
        if (ev.button.button == SDL_BUTTON_LEFT) {
            const int zoomButton = HitTestVirtualZoomButton(static_cast<float>(MouseX), static_cast<float>(MouseY));
            if (zoomButton >= 0)
            {
                HandleVirtualZoomButtonTap(zoomButton);
                break;
            }
            if (IsVirtualPadAvailable() && HitTestVirtualJoystick(static_cast<float>(MouseX), static_cast<float>(MouseY)))
            {
                g_joystickPcMouseCaptured = true;
                StartVirtualJoystick(kPcMouseJoystickFingerId, static_cast<float>(MouseX), static_cast<float>(MouseY));
            }
            else
            {
                g_joystickPcMouseCaptured = false;
                MouseLButtonPop  = false;
                MouseLButtonPush = !MouseLButton;
                MouseLButton     = true;
            }
        } else if (ev.button.button == SDL_BUTTON_RIGHT) {
            MouseRButtonPop  = false;
            MouseRButtonPush = !MouseRButton;
            MouseRButton     = true;
        } else if (ev.button.button == SDL_BUTTON_MIDDLE) {
            MouseMButtonPop  = false;
            MouseMButtonPush = !MouseMButton;
            MouseMButton     = true;
        }
        break;

    case SDL_MOUSEBUTTONUP:
        if (ev.button.which == SDL_TOUCH_MOUSEID && g_seenFingerInput) break;
        g_iNoMouseTime = 0;
        UpdateMouseFromPixel(ev.button.x, ev.button.y, screenW, screenH);
        if (ev.button.button == SDL_BUTTON_LEFT) {
            if (g_joystickPcMouseCaptured)
            { g_joystickPcMouseCaptured = false; ClearVirtualJoystick(); }
            else
            {
                MouseLButtonPush = false;
                if (MouseLButton) { MouseLButtonPop = true; g_iMousePopPosition_x = MouseX; g_iMousePopPosition_y = MouseY; }
                MouseLButton = false;
            }
        } else if (ev.button.button == SDL_BUTTON_RIGHT) {
            MouseRButtonPush = false;
            if (MouseRButton) MouseRButtonPop = true;
            MouseRButton = false;
        } else if (ev.button.button == SDL_BUTTON_MIDDLE) {
            MouseMButtonPush = false;
            if (MouseMButton) MouseMButtonPop = true;
            MouseMButton = false;
        }
        break;

    // â”€â”€ Mouse wheel â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    case SDL_MOUSEWHEEL:
        MouseWheel = ev.wheel.y;
        break;

    // â”€â”€ Multi-touch (additional fingers) â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    // SDL2 maps first finger to mouse; additional fingers need manual mapping
    // For now: second finger â†’ right-click
    case SDL_FINGERDOWN:
        g_seenFingerInput = true;
        // Sync touch → MouseX/MouseY before any handler that may return early (chat,
        // delete-resident IME, etc.). Otherwise MsgWin::UpdateWhileShow still sees
        // the previous cursor, treats the tap as "outside" the field, and calls
        // SetFocus(nullptr) — IME opens then immediately closes; users double-tap.
        g_iNoMouseTime = 0;
        UpdateMouseFromTouch(ev.tfinger, screenW, screenH);
        if (HandleVirtualPickerFingerDown(ev.tfinger))
        {
            break;
        }
        if (HandleVirtualFingerDown(ev.tfinger))
        {
            break;
        }
#if defined(__ANDROID__) || defined(MU_IOS)
        if (kLogInputEvents)
        {
            LOGI("INPUT finger down id=%lld x=%.3f y=%.3f mapped=%d,%d primaryTouch=%lld",
                static_cast<long long>(ev.tfinger.fingerId),
                ev.tfinger.x,
                ev.tfinger.y,
                MouseX,
                MouseY,
                static_cast<long long>(g_primaryTouchFinger));
        }
#endif
        if (g_primaryTouchFinger == -1 || ev.tfinger.fingerId == g_primaryTouchFinger) {
            g_primaryTouchFinger = ev.tfinger.fingerId;

            // â”€â”€ Double-tap detection (Android replacement for WM_LBUTTONDBLCLK) â”€â”€
            {
                const uint32_t nowMs = MU_MobileGetTicks();
                const float dx = ev.tfinger.x - s_doubleTapLastUpNX;
                const float dy = ev.tfinger.y - s_doubleTapLastUpNY;
                if (s_doubleTapLastUpMs > 0
                    && (nowMs - s_doubleTapLastUpMs) <= kDoubleTapMaxMs
                    && (dx * dx + dy * dy) <= (kDoubleTapMaxDist * kDoubleTapMaxDist))
                {
                    MouseLButtonDBClick = true;
                    s_doubleTapLastUpMs = 0; // consume: don't triple-fire
                }
            }

            MouseLButtonPop  = false;
            MouseLButtonPush = !MouseLButton;
            MouseLButton     = true;
        } else {
            MouseRButtonPop  = false;
            MouseRButtonPush = !MouseRButton;
            MouseRButton     = true;
        }
        break;

    case SDL_FINGERMOTION:
        g_seenFingerInput = true;
        if (HandleVirtualPickerFingerMotion(ev.tfinger))
        {
            break;
        }
        if (HandleVirtualFingerMotion(ev.tfinger))
        {
            break;
        }
        UpdateMouseFromTouch(ev.tfinger, screenW, screenH);
        break;

    case SDL_FINGERUP:
        g_seenFingerInput = true;
        if (HandleVirtualPickerFingerUp(ev.tfinger))
        {
            break;
        }
        if (HandleVirtualFingerUp(ev.tfinger))
        {
            break;
        }
        g_iNoMouseTime = 0;
        UpdateMouseFromTouch(ev.tfinger, screenW, screenH);
#if defined(__ANDROID__) || defined(MU_IOS)
        if (kLogInputEvents)
        {
            LOGI("INPUT finger up id=%lld x=%.3f y=%.3f mapped=%d,%d primaryTouch=%lld",
                static_cast<long long>(ev.tfinger.fingerId),
                ev.tfinger.x,
                ev.tfinger.y,
                MouseX,
                MouseY,
                static_cast<long long>(g_primaryTouchFinger));
        }
#endif
        if (ev.tfinger.fingerId == g_primaryTouchFinger) {
            MouseLButtonPush = false;
            if (MouseLButton) {
                MouseLButtonPop = true;
                g_iMousePopPosition_x = MouseX;
                g_iMousePopPosition_y = MouseY;
            }
            MouseLButton = false;
            g_primaryTouchFinger = -1;

            // Record this finger-up so the next finger-down can detect double-tap
            s_doubleTapLastUpMs = MU_MobileGetTicks();
            s_doubleTapLastUpNX = ev.tfinger.x;
            s_doubleTapLastUpNY = ev.tfinger.y;
        } else {
            MouseRButtonPush = false;
            if (MouseRButton) MouseRButtonPop = true;
            MouseRButton = false;
        }
        break;

    // â”€â”€ Keyboard â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    case SDL_TEXTINPUT:
#if !defined(MU_ANDROID_DISABLE_LOG)
        LOGI(
            "CHATIME SDL_TEXTINPUT text='%s' hasFocusedText=%d",
            ev.text.text,
            AndroidHasFocusedTextInput() ? 1 : 0);
#endif
        if (g_charNameInputActive) {
            // Route directly into the custom char-name buffer
            const unsigned char* p = reinterpret_cast<const unsigned char*>(ev.text.text);
            while (*p && g_charNameLen < 10) {
                wchar_t ch = 0;
                if ((*p & 0x80u) == 0) {
                    ch = static_cast<wchar_t>(*p++);
                } else if ((*p & 0xE0u) == 0xC0u && p[1]) {
                    ch = static_cast<wchar_t>(((*p & 0x1Fu) << 6) | (p[1] & 0x3Fu));
                    p += 2;
                } else if ((*p & 0xF0u) == 0xE0u && p[1] && p[2]) {
                    ch = static_cast<wchar_t>(((*p & 0x0Fu) << 12) | ((p[1] & 0x3Fu) << 6) | (p[2] & 0x3Fu));
                    p += 3;
                } else { p++; continue; }
                if (ch >= 0x20) {
                    g_charNameBuf[g_charNameLen++] = ch;
                    g_charNameBuf[g_charNameLen]   = L'\0';
                }
            }
        } else if (AndroidHasFocusedTextInput()) {
            if (std::strchr(ev.text.text, '\n') != nullptr
                || std::strchr(ev.text.text, '\r') != nullptr)
            {
                if (g_pendingImeEnterTextInput)
                {
                    g_pendingImeEnterTextInput = false;
                    break;
                }

                const HWND focusedBeforeTextInput = GetFocus();
                AndroidInjectUtf8ToFocusedTextInput(ev.text.text);
                if (GetFocus() == focusedBeforeTextInput) {
                    SetEnterPressed(true);
                }
            } else {
                g_pendingImeEnterTextInput = false;
                AndroidInjectUtf8ToFocusedTextInput(ev.text.text);
            }
        }
        break;

    case SDL_KEYDOWN:
        if (ev.key.keysym.scancode != SDL_SCANCODE_UNKNOWN)
        {
            MU_MobileSetKeyState(ev.key.keysym.scancode, true);
        }
#if !defined(MU_ANDROID_DISABLE_LOG)
        LOGI(
            "CHATIME SDL_KEYDOWN sym=%d repeat=%d hasFocusedText=%d sdlTextInput=%d",
            static_cast<int>(ev.key.keysym.sym),
            ev.key.repeat,
            AndroidHasFocusedTextInput() ? 1 : 0,
            MU_MobileIsTextInputActive() ? 1 : 0);
#endif
        switch (ev.key.keysym.sym) {
        case SDLK_AC_BACK:   // Android back button
            Destroy = true;
            break;
        case SDLK_BACKSPACE:
            if (g_charNameInputActive) {
                if (g_charNameLen > 0) g_charNameBuf[--g_charNameLen] = L'\0';
            } else if (AndroidHasFocusedTextInput()) {
                AndroidInjectCharToFocusedTextInput(VK_BACK);
            }
            break;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            if (g_charNameInputActive) {
                // Let the game loop detect Enter via CInput::IsKeyDown(VK_RETURN)
            } else if (AndroidHasFocusedTextInput()) {
                const HWND focusedBeforeReturn = GetFocus();
                if (AndroidInjectCharToFocusedTextInput(VK_RETURN)) {
                    g_pendingImeEnterTextInput = true;
                }
                if (GetFocus() == focusedBeforeReturn) {
                    SetEnterPressed(true);
                }
            } else {
                g_pendingImeEnterTextInput = false;
                SetEnterPressed(true);
            }
            break;
        default:
            // LDPlayer sends SDLK_UNKNOWN (keycode=0) with no SDL_TEXTINPUT.
            // Real devices send valid keycodes AND also fire SDL_TEXTINPUT.
            // Only use TranslateKeyToAsciiFallback for LDPlayer (SDLK_UNKNOWN),
            // otherwise SDL_TEXTINPUT will handle it and we'd double-inject.
            if (ev.key.keysym.sym == SDLK_UNKNOWN && ev.key.repeat == 0) {
                if (g_charNameInputActive) {
                    const wchar_t fallbackCharacter = TranslateKeyToAsciiFallback(ev.key);
                    if (fallbackCharacter != 0 && g_charNameLen < 10) {
                        g_charNameBuf[g_charNameLen++] = fallbackCharacter;
                        g_charNameBuf[g_charNameLen]   = L'\0';
                    }
                } else if (AndroidHasFocusedTextInput()) {
                    const wchar_t fallbackCharacter = TranslateKeyToAsciiFallback(ev.key);
                    if (fallbackCharacter != 0)
                        AndroidInjectCharToFocusedTextInput(fallbackCharacter);
                }
            }
            break;
        }
        break;

    case SDL_KEYUP:
        if (ev.key.keysym.scancode != SDL_SCANCODE_UNKNOWN)
        {
            MU_MobileSetKeyState(ev.key.keysym.scancode, false);
        }
        break;

    // â”€â”€ Window resize â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    case SDL_WINDOWEVENT:
        if (ev.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
            screenW = (ev.window.data1 > 0) ? ev.window.data1 : g_DrawableWidth;
            screenH = (ev.window.data2 > 0) ? ev.window.data2 : g_DrawableHeight;
            ApplyAndroidDrawableSize(screenW, screenH, "event");
        }
        break;

    default:
        break;
    }
}

namespace
{
struct AndroidFrameState
{
    Uint64 perfFrequency = 0;
    Uint64 perfWindowStart = 0;
    int perfFrames = 0;
    int drawCallsTotal = 0;
    int vertsTotal = 0;
    int detailLogWindowCounter = 0;
    int objRenderCandidatesTotal = 0;
    int objRenderRenderedTotal = 0;
    int objRenderCulledTotal = 0;
    int objRenderBaseCallsTotal = 0;
    int objVisualCallsTotal = 0;
    int objRenderAfterCallsTotal = 0;
    int charRenderCandidatesTotal = 0;
    int charRenderRenderedTotal = 0;
    int charRenderDeferredTotal = 0;
    int charRenderPlayersTotal = 0;
    int charRenderMonstersTotal = 0;
    int terrainNormalBlocksTotal = 0;
    int terrainNormalTilesTotal = 0;
    int terrainGrassBlocksTotal = 0;
    int terrainGrassTilesTotal = 0;
    int terrainAfterBlocksTotal = 0;
    int terrainAfterTilesTotal = 0;
    Uint64 sceneTicksTotal = 0;
    Uint64 padTicksTotal = 0;
    Uint64 presentTicksTotal = 0;
    Uint64 objMoveTicksTotal = 0;
    Uint64 objRenderTicksTotal = 0;
    Uint64 terrainRenderTicksTotal = 0;
    Uint64 terrainAfterTicksTotal = 0;
    Uint64 charMoveTicksTotal = 0;
    Uint64 charRenderTicksTotal = 0;
    Uint64 mainShadowTicksTotal = 0;
    Uint64 mainBoidsTicksTotal = 0;
    Uint64 mainMiscWorldTicksTotal = 0;
    Uint64 mainJointsTicksTotal = 0;
    Uint64 mainEffectsTicksTotal = 0;
    Uint64 mainBlursTicksTotal = 0;
    Uint64 mainSpritesTicksTotal = 0;
    Uint64 mainParticlesTicksTotal = 0;
    Uint64 mainPointsTicksTotal = 0;
    Uint64 mainAfterEffectsTicksTotal = 0;
    Uint64 mainUiTicksTotal = 0;
    int dbgFrameCount = 0;
    uint32_t muHelperLastTickMs = 0;
    uint32_t hackLastTickMs = 0;
};

std::mutex g_PendingSdlEventsMutex;
std::vector<SDL_Event> g_PendingSdlEvents;
AndroidFrameState g_AndroidFrameState = {};

constexpr jint kAndroidKeyActionDown = 0;
constexpr jint kAndroidKeyActionUp = 1;
constexpr jint kAndroidKeyActionMultiple = 2;
} // namespace

static void QueueSyntheticSDLEvent(const SDL_Event& event)
{
    std::lock_guard<std::mutex> lock(g_PendingSdlEventsMutex);
    g_PendingSdlEvents.push_back(event);
}

static std::vector<SDL_Event> DrainSyntheticSDLEvents()
{
    std::vector<SDL_Event> events;
    std::lock_guard<std::mutex> lock(g_PendingSdlEventsMutex);
    events.swap(g_PendingSdlEvents);
    return events;
}

static SDL_Keymod ConvertAndroidMetaStateToSDLKeymod(jint metaState)
{
    SDL_Keymod modifiers = KMOD_NONE;
    if ((metaState & AMETA_SHIFT_ON) != 0) modifiers = static_cast<SDL_Keymod>(modifiers | KMOD_SHIFT);
    if ((metaState & AMETA_CTRL_ON) != 0) modifiers = static_cast<SDL_Keymod>(modifiers | KMOD_CTRL);
    if ((metaState & AMETA_ALT_ON) != 0) modifiers = static_cast<SDL_Keymod>(modifiers | KMOD_ALT);
    if ((metaState & AMETA_META_ON) != 0) modifiers = static_cast<SDL_Keymod>(modifiers | KMOD_GUI);
    if ((metaState & AMETA_CAPS_LOCK_ON) != 0) modifiers = static_cast<SDL_Keymod>(modifiers | KMOD_CAPS);
    return modifiers;
}

static SDL_Keycode ConvertAndroidKeycodeToSDLKeycode(jint keyCode)
{
    switch (keyCode)
    {
    case AKEYCODE_A: return SDLK_a;
    case AKEYCODE_B: return SDLK_b;
    case AKEYCODE_C: return SDLK_c;
    case AKEYCODE_D: return SDLK_d;
    case AKEYCODE_E: return SDLK_e;
    case AKEYCODE_F: return SDLK_f;
    case AKEYCODE_G: return SDLK_g;
    case AKEYCODE_H: return SDLK_h;
    case AKEYCODE_I: return SDLK_i;
    case AKEYCODE_J: return SDLK_j;
    case AKEYCODE_K: return SDLK_k;
    case AKEYCODE_L: return SDLK_l;
    case AKEYCODE_M: return SDLK_m;
    case AKEYCODE_N: return SDLK_n;
    case AKEYCODE_O: return SDLK_o;
    case AKEYCODE_P: return SDLK_p;
    case AKEYCODE_Q: return SDLK_q;
    case AKEYCODE_R: return SDLK_r;
    case AKEYCODE_S: return SDLK_s;
    case AKEYCODE_T: return SDLK_t;
    case AKEYCODE_U: return SDLK_u;
    case AKEYCODE_V: return SDLK_v;
    case AKEYCODE_W: return SDLK_w;
    case AKEYCODE_X: return SDLK_x;
    case AKEYCODE_Y: return SDLK_y;
    case AKEYCODE_Z: return SDLK_z;
    case AKEYCODE_0: return SDLK_0;
    case AKEYCODE_1: return SDLK_1;
    case AKEYCODE_2: return SDLK_2;
    case AKEYCODE_3: return SDLK_3;
    case AKEYCODE_4: return SDLK_4;
    case AKEYCODE_5: return SDLK_5;
    case AKEYCODE_6: return SDLK_6;
    case AKEYCODE_7: return SDLK_7;
    case AKEYCODE_8: return SDLK_8;
    case AKEYCODE_9: return SDLK_9;
    case AKEYCODE_SPACE: return SDLK_SPACE;
    case AKEYCODE_TAB: return SDLK_TAB;
    case AKEYCODE_ENTER: return SDLK_RETURN;
    case AKEYCODE_NUMPAD_ENTER: return SDLK_KP_ENTER;
    case AKEYCODE_DEL: return SDLK_BACKSPACE;
    case AKEYCODE_ESCAPE: return SDLK_ESCAPE;
    case AKEYCODE_BACK: return SDLK_AC_BACK;
    case AKEYCODE_COMMA: return SDLK_COMMA;
    case AKEYCODE_PERIOD: return SDLK_PERIOD;
    case AKEYCODE_MINUS: return SDLK_MINUS;
    case AKEYCODE_EQUALS: return SDLK_EQUALS;
    case AKEYCODE_SEMICOLON: return SDLK_SEMICOLON;
    case AKEYCODE_APOSTROPHE: return SDLK_QUOTE;
    case AKEYCODE_SLASH: return SDLK_SLASH;
    case AKEYCODE_BACKSLASH: return SDLK_BACKSLASH;
    case AKEYCODE_LEFT_BRACKET: return SDLK_LEFTBRACKET;
    case AKEYCODE_RIGHT_BRACKET: return SDLK_RIGHTBRACKET;
    case AKEYCODE_GRAVE: return SDLK_BACKQUOTE;
    case AKEYCODE_DPAD_LEFT: return SDLK_LEFT;
    case AKEYCODE_DPAD_RIGHT: return SDLK_RIGHT;
    case AKEYCODE_DPAD_UP: return SDLK_UP;
    case AKEYCODE_DPAD_DOWN: return SDLK_DOWN;
    case AKEYCODE_PAGE_UP: return SDLK_PAGEUP;
    case AKEYCODE_PAGE_DOWN: return SDLK_PAGEDOWN;
    case AKEYCODE_MOVE_HOME: return SDLK_HOME;
    case AKEYCODE_MOVE_END: return SDLK_END;
    case AKEYCODE_INSERT: return SDLK_INSERT;
    case AKEYCODE_FORWARD_DEL: return SDLK_DELETE;
    case AKEYCODE_SHIFT_LEFT: return SDLK_LSHIFT;
    case AKEYCODE_SHIFT_RIGHT: return SDLK_RSHIFT;
    case AKEYCODE_CTRL_LEFT: return SDLK_LCTRL;
    case AKEYCODE_CTRL_RIGHT: return SDLK_RCTRL;
    case AKEYCODE_ALT_LEFT: return SDLK_LALT;
    case AKEYCODE_ALT_RIGHT: return SDLK_RALT;
    default: return SDLK_UNKNOWN;
    }
}

static SDL_Scancode ConvertAndroidKeycodeToSDLScancode(jint keyCode)
{
    switch (keyCode)
    {
    case AKEYCODE_A: return SDL_SCANCODE_A;
    case AKEYCODE_B: return SDL_SCANCODE_B;
    case AKEYCODE_C: return SDL_SCANCODE_C;
    case AKEYCODE_D: return SDL_SCANCODE_D;
    case AKEYCODE_E: return SDL_SCANCODE_E;
    case AKEYCODE_F: return SDL_SCANCODE_F;
    case AKEYCODE_G: return SDL_SCANCODE_G;
    case AKEYCODE_H: return SDL_SCANCODE_H;
    case AKEYCODE_I: return SDL_SCANCODE_I;
    case AKEYCODE_J: return SDL_SCANCODE_J;
    case AKEYCODE_K: return SDL_SCANCODE_K;
    case AKEYCODE_L: return SDL_SCANCODE_L;
    case AKEYCODE_M: return SDL_SCANCODE_M;
    case AKEYCODE_N: return SDL_SCANCODE_N;
    case AKEYCODE_O: return SDL_SCANCODE_O;
    case AKEYCODE_P: return SDL_SCANCODE_P;
    case AKEYCODE_Q: return SDL_SCANCODE_Q;
    case AKEYCODE_R: return SDL_SCANCODE_R;
    case AKEYCODE_S: return SDL_SCANCODE_S;
    case AKEYCODE_T: return SDL_SCANCODE_T;
    case AKEYCODE_U: return SDL_SCANCODE_U;
    case AKEYCODE_V: return SDL_SCANCODE_V;
    case AKEYCODE_W: return SDL_SCANCODE_W;
    case AKEYCODE_X: return SDL_SCANCODE_X;
    case AKEYCODE_Y: return SDL_SCANCODE_Y;
    case AKEYCODE_Z: return SDL_SCANCODE_Z;
    case AKEYCODE_0: return SDL_SCANCODE_0;
    case AKEYCODE_1: return SDL_SCANCODE_1;
    case AKEYCODE_2: return SDL_SCANCODE_2;
    case AKEYCODE_3: return SDL_SCANCODE_3;
    case AKEYCODE_4: return SDL_SCANCODE_4;
    case AKEYCODE_5: return SDL_SCANCODE_5;
    case AKEYCODE_6: return SDL_SCANCODE_6;
    case AKEYCODE_7: return SDL_SCANCODE_7;
    case AKEYCODE_8: return SDL_SCANCODE_8;
    case AKEYCODE_9: return SDL_SCANCODE_9;
    case AKEYCODE_SPACE: return SDL_SCANCODE_SPACE;
    case AKEYCODE_TAB: return SDL_SCANCODE_TAB;
    case AKEYCODE_ENTER: return SDL_SCANCODE_RETURN;
    case AKEYCODE_NUMPAD_ENTER: return SDL_SCANCODE_KP_ENTER;
    case AKEYCODE_DEL: return SDL_SCANCODE_BACKSPACE;
    case AKEYCODE_ESCAPE: return SDL_SCANCODE_ESCAPE;
    case AKEYCODE_BACK: return SDL_SCANCODE_AC_BACK;
    case AKEYCODE_COMMA: return SDL_SCANCODE_COMMA;
    case AKEYCODE_PERIOD: return SDL_SCANCODE_PERIOD;
    case AKEYCODE_MINUS: return SDL_SCANCODE_MINUS;
    case AKEYCODE_EQUALS: return SDL_SCANCODE_EQUALS;
    case AKEYCODE_SEMICOLON: return SDL_SCANCODE_SEMICOLON;
    case AKEYCODE_APOSTROPHE: return SDL_SCANCODE_APOSTROPHE;
    case AKEYCODE_SLASH: return SDL_SCANCODE_SLASH;
    case AKEYCODE_BACKSLASH: return SDL_SCANCODE_BACKSLASH;
    case AKEYCODE_LEFT_BRACKET: return SDL_SCANCODE_LEFTBRACKET;
    case AKEYCODE_RIGHT_BRACKET: return SDL_SCANCODE_RIGHTBRACKET;
    case AKEYCODE_GRAVE: return SDL_SCANCODE_GRAVE;
    case AKEYCODE_DPAD_LEFT: return SDL_SCANCODE_LEFT;
    case AKEYCODE_DPAD_RIGHT: return SDL_SCANCODE_RIGHT;
    case AKEYCODE_DPAD_UP: return SDL_SCANCODE_UP;
    case AKEYCODE_DPAD_DOWN: return SDL_SCANCODE_DOWN;
    case AKEYCODE_PAGE_UP: return SDL_SCANCODE_PAGEUP;
    case AKEYCODE_PAGE_DOWN: return SDL_SCANCODE_PAGEDOWN;
    case AKEYCODE_MOVE_HOME: return SDL_SCANCODE_HOME;
    case AKEYCODE_MOVE_END: return SDL_SCANCODE_END;
    case AKEYCODE_INSERT: return SDL_SCANCODE_INSERT;
    case AKEYCODE_FORWARD_DEL: return SDL_SCANCODE_DELETE;
    case AKEYCODE_SHIFT_LEFT: return SDL_SCANCODE_LSHIFT;
    case AKEYCODE_SHIFT_RIGHT: return SDL_SCANCODE_RSHIFT;
    case AKEYCODE_CTRL_LEFT: return SDL_SCANCODE_LCTRL;
    case AKEYCODE_CTRL_RIGHT: return SDL_SCANCODE_RCTRL;
    case AKEYCODE_ALT_LEFT: return SDL_SCANCODE_LALT;
    case AKEYCODE_ALT_RIGHT: return SDL_SCANCODE_RALT;
    default: return SDL_SCANCODE_UNKNOWN;
    }
}

static void QueueTextInputCodepoint(uint32_t charCode)
{
    if ((charCode == 0) || (charCode > 0x10FFFF))
    {
        return;
    }

    SDL_Event event {};
    event.type = SDL_TEXTINPUT;
    EncodeUtf32ToUtf8(charCode, event.text.text);
    if (event.text.text[0] != '\0')
    {
        QueueSyntheticSDLEvent(event);
    }
}

static void QueueTextInputUtf8(const char* textUtf8)
{
    if (!textUtf8 || !textUtf8[0])
    {
        return;
    }

    SDL_Event event {};
    event.type = SDL_TEXTINPUT;
    std::strncpy(event.text.text, textUtf8, SDL_TEXTINPUTEVENT_TEXT_SIZE - 1);
    event.text.text[SDL_TEXTINPUTEVENT_TEXT_SIZE - 1] = '\0';
    QueueSyntheticSDLEvent(event);
}

static bool ShouldEmitTextInputForAndroidKey(jint unicodeChar, SDL_Keycode keycode)
{
    if (unicodeChar == 0)
    {
        return false;
    }

    if (keycode == SDLK_BACKSPACE || keycode == SDLK_RETURN || keycode == SDLK_KP_ENTER
        || keycode == SDLK_AC_BACK || keycode == SDLK_TAB)
    {
        return false;
    }

    return unicodeChar >= 0x20;
}

static void QueueAndroidKeyEvent(
    jint action,
    jint keyCode,
    jint unicodeChar,
    jint metaState,
    jint repeatCount)
{
    if ((action != kAndroidKeyActionDown) && (action != kAndroidKeyActionUp))
    {
        return;
    }

    SDL_Event event {};
    event.type = (action == kAndroidKeyActionDown) ? SDL_KEYDOWN : SDL_KEYUP;
    event.key.state = (action == kAndroidKeyActionDown) ? SDL_PRESSED : SDL_RELEASED;
    event.key.repeat = (repeatCount > 0) ? 1 : 0;
    event.key.keysym.sym = ConvertAndroidKeycodeToSDLKeycode(keyCode);
    event.key.keysym.scancode = ConvertAndroidKeycodeToSDLScancode(keyCode);
    event.key.keysym.mod = ConvertAndroidMetaStateToSDLKeymod(metaState);
    QueueSyntheticSDLEvent(event);

    if ((action == kAndroidKeyActionDown) && ShouldEmitTextInputForAndroidKey(unicodeChar, event.key.keysym.sym))
    {
        QueueTextInputCodepoint(static_cast<uint32_t>(unicodeChar));
    }
}

static SDL_FingerID ToSdlFingerId(uintptr_t identifier)
{
    return static_cast<SDL_FingerID>(identifier);
}

static void QueueSappEventAsSDL(const sapp_event* event)
{
    if (!event)
    {
        return;
    }

    switch (event->type)
    {
    case SAPP_EVENTTYPE_QUIT_REQUESTED:
        {
            SDL_Event sdlEvent {};
            sdlEvent.type = SDL_QUIT;
            QueueSyntheticSDLEvent(sdlEvent);
        }
        break;

    case SAPP_EVENTTYPE_SUSPENDED:
    case SAPP_EVENTTYPE_UNFOCUSED:
    case SAPP_EVENTTYPE_ICONIFIED:
        {
            SDL_Event sdlEvent {};
            sdlEvent.type = SDL_APP_DIDENTERBACKGROUND;
            QueueSyntheticSDLEvent(sdlEvent);
        }
        break;

    case SAPP_EVENTTYPE_RESUMED:
    case SAPP_EVENTTYPE_FOCUSED:
    case SAPP_EVENTTYPE_RESTORED:
        {
            SDL_Event sdlEvent {};
            sdlEvent.type = SDL_APP_DIDENTERFOREGROUND;
            QueueSyntheticSDLEvent(sdlEvent);
        }
        break;

    case SAPP_EVENTTYPE_RESIZED:
        {
            g_DrawableWidth = (event->framebuffer_width > 0) ? event->framebuffer_width : g_DrawableWidth;
            g_DrawableHeight = (event->framebuffer_height > 0) ? event->framebuffer_height : g_DrawableHeight;
            SDL_Event sdlEvent {};
            sdlEvent.type = SDL_WINDOWEVENT;
            sdlEvent.window.event = SDL_WINDOWEVENT_SIZE_CHANGED;
            sdlEvent.window.data1 = g_DrawableWidth;
            sdlEvent.window.data2 = g_DrawableHeight;
            QueueSyntheticSDLEvent(sdlEvent);
        }
        break;

    case SAPP_EVENTTYPE_MOUSE_MOVE:
        {
            SDL_Event sdlEvent {};
            sdlEvent.type = SDL_MOUSEMOTION;
            sdlEvent.motion.x = static_cast<Sint32>(event->mouse_x);
            sdlEvent.motion.y = static_cast<Sint32>(event->mouse_y);
            if ((event->modifiers & SAPP_MODIFIER_LMB) != 0) sdlEvent.motion.state |= SDL_BUTTON_LMASK;
            if ((event->modifiers & SAPP_MODIFIER_RMB) != 0) sdlEvent.motion.state |= SDL_BUTTON_RMASK;
            if ((event->modifiers & SAPP_MODIFIER_MMB) != 0) sdlEvent.motion.state |= SDL_BUTTON_MMASK;
            QueueSyntheticSDLEvent(sdlEvent);
        }
        break;

    case SAPP_EVENTTYPE_MOUSE_DOWN:
    case SAPP_EVENTTYPE_MOUSE_UP:
        {
            SDL_Event sdlEvent {};
            sdlEvent.type = (event->type == SAPP_EVENTTYPE_MOUSE_DOWN) ? SDL_MOUSEBUTTONDOWN : SDL_MOUSEBUTTONUP;
            sdlEvent.button.state = (event->type == SAPP_EVENTTYPE_MOUSE_DOWN) ? SDL_PRESSED : SDL_RELEASED;
            sdlEvent.button.x = static_cast<Sint32>(event->mouse_x);
            sdlEvent.button.y = static_cast<Sint32>(event->mouse_y);
            switch (event->mouse_button)
            {
            case SAPP_MOUSEBUTTON_LEFT: sdlEvent.button.button = SDL_BUTTON_LEFT; break;
            case SAPP_MOUSEBUTTON_RIGHT: sdlEvent.button.button = SDL_BUTTON_RIGHT; break;
            case SAPP_MOUSEBUTTON_MIDDLE: sdlEvent.button.button = SDL_BUTTON_MIDDLE; break;
            default: sdlEvent.button.button = 0; break;
            }
            QueueSyntheticSDLEvent(sdlEvent);
        }
        break;

    case SAPP_EVENTTYPE_MOUSE_SCROLL:
        {
            SDL_Event sdlEvent {};
            sdlEvent.type = SDL_MOUSEWHEEL;
            sdlEvent.wheel.x = static_cast<Sint32>(std::lround(event->scroll_x));
            sdlEvent.wheel.y = static_cast<Sint32>(std::lround(event->scroll_y));
            QueueSyntheticSDLEvent(sdlEvent);
        }
        break;

    case SAPP_EVENTTYPE_TOUCHES_BEGAN:
    case SAPP_EVENTTYPE_TOUCHES_MOVED:
    case SAPP_EVENTTYPE_TOUCHES_ENDED:
    case SAPP_EVENTTYPE_TOUCHES_CANCELLED:
        {
            const float safeW = static_cast<float>((g_DrawableWidth > 0) ? g_DrawableWidth : 1);
            const float safeH = static_cast<float>((g_DrawableHeight > 0) ? g_DrawableHeight : 1);
            Uint32 sdlType = SDL_FINGERMOTION;
            if (event->type == SAPP_EVENTTYPE_TOUCHES_BEGAN) sdlType = SDL_FINGERDOWN;
            else if ((event->type == SAPP_EVENTTYPE_TOUCHES_ENDED) || (event->type == SAPP_EVENTTYPE_TOUCHES_CANCELLED)) sdlType = SDL_FINGERUP;

            for (int i = 0; i < event->num_touches; ++i)
            {
                const sapp_touchpoint& touch = event->touches[i];
                if (!touch.changed)
                {
                    continue;
                }

                SDL_Event sdlEvent {};
                sdlEvent.type = sdlType;
                sdlEvent.tfinger.touchId = 0;
                sdlEvent.tfinger.fingerId = ToSdlFingerId(touch.identifier);
                sdlEvent.tfinger.x = std::clamp(touch.pos_x / safeW, 0.0f, 1.0f);
                sdlEvent.tfinger.y = std::clamp(touch.pos_y / safeH, 0.0f, 1.0f);
                sdlEvent.tfinger.dx = 0.0f;
                sdlEvent.tfinger.dy = 0.0f;
                sdlEvent.tfinger.pressure = 1.0f;
                QueueSyntheticSDLEvent(sdlEvent);
            }
        }
        break;

    case SAPP_EVENTTYPE_KEY_DOWN:
    case SAPP_EVENTTYPE_KEY_UP:
        {
            SDL_Event sdlEvent {};
            sdlEvent.type = (event->type == SAPP_EVENTTYPE_KEY_DOWN) ? SDL_KEYDOWN : SDL_KEYUP;
            sdlEvent.key.state = (event->type == SAPP_EVENTTYPE_KEY_DOWN) ? SDL_PRESSED : SDL_RELEASED;
            sdlEvent.key.repeat = event->key_repeat ? 1 : 0;
            sdlEvent.key.keysym.sym = ConvertSappKeycodeToSDLKeycode(event->key_code);
            sdlEvent.key.keysym.scancode = ConvertSappKeycodeToSDLScancode(event->key_code);
            sdlEvent.key.keysym.mod = ConvertSappModifiersToSDLKeymod(event->modifiers);
            QueueSyntheticSDLEvent(sdlEvent);
        }
        break;

    case SAPP_EVENTTYPE_CHAR:
        QueueTextInputCodepoint(event->char_code);
        break;

    default:
        break;
    }
}

namespace
{
std::wstring g_androidBootstrapServerIp;
int g_androidBootstrapServerPort = 0;

void TrimAsciiUtf8InPlace(std::string& s)
{
    while (!s.empty() && (s.back() == ' ' || s.back() == '\t' || s.back() == '\r' || s.back() == '\n'))
    {
        s.pop_back();
    }
    size_t i = 0;
    while (i < s.size() && (s[i] == ' ' || s[i] == '\t' || s[i] == '\r' || s[i] == '\n'))
    {
        ++i;
    }
    if (i > 0)
    {
        s.erase(0, i);
    }
}

static bool IsUnsetOrLegacyDevServerIp(const std::wstring& ip)
{
    return ip.empty() || ip == L"127.127.127.127";
}

void ApplyAndroidNetworkBootstrapOverrides(std::wstring& serverIP, int& configuredPort)
{
    if (!g_androidBootstrapServerIp.empty())
    {
        serverIP = g_androidBootstrapServerIp;
    }
    if (g_androidBootstrapServerPort > 0)
    {
        configuredPort = g_androidBootstrapServerPort;
    }
}
} // namespace

extern "C" JNIEXPORT void JNICALL
Java_com_muonline_client_MuMainNativeActivity_nativeApplyNetworkBootstrap(
    JNIEnv* env,
    jclass,
    jstring host,
    jint port)
{
    g_androidBootstrapServerIp.clear();
    g_androidBootstrapServerPort = 0;
    if (env == nullptr)
    {
        return;
    }

    if (host != nullptr)
    {
        const char* utf8 = env->GetStringUTFChars(host, nullptr);
        if (utf8 != nullptr)
        {
            std::string narrow(utf8);
            env->ReleaseStringUTFChars(host, utf8);
            TrimAsciiUtf8InPlace(narrow);
            if (!narrow.empty())
            {
                g_androidBootstrapServerIp.assign(narrow.begin(), narrow.end());
            }
        }
    }

    if (port > 0 && port <= 65535)
    {
        g_androidBootstrapServerPort = static_cast<int>(port);
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_muonline_client_MuMainNativeActivity_nativeOnTextInput(
    JNIEnv* env,
    jclass,
    jstring text)
{
    if ((env == nullptr) || (text == nullptr))
    {
        return;
    }

    const char* textUtf8 = env->GetStringUTFChars(text, nullptr);
    if (textUtf8 != nullptr)
    {
        QueueTextInputUtf8(textUtf8);
        env->ReleaseStringUTFChars(text, textUtf8);
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_muonline_client_MuMainNativeActivity_nativeOnKeyEvent(
    JNIEnv*,
    jclass,
    jint action,
    jint keyCode,
    jint unicodeChar,
    jint metaState,
    jint repeatCount)
{
    if (action == kAndroidKeyActionMultiple)
    {
        return;
    }
    QueueAndroidKeyEvent(action, keyCode, unicodeChar, metaState, repeatCount);
}

extern "C" JNIEXPORT void JNICALL
Java_com_muonline_client_MuMainNativeActivity_nativeOnImeEditorAction(JNIEnv*, jclass, jint actionCode)
{
    // EditorInfo.IME_ACTION_UNSPECIFIED = 0 — no-op.
    if (actionCode == 0)
    {
        return;
    }

    // Same path as hardware Enter: SDL_KEYDOWN/UP SDLK_RETURN → HandleSDLEvent → WM_CHAR / SetEnterPressed.
    SDL_Event down{};
    down.type = SDL_KEYDOWN;
    down.key.state = SDL_PRESSED;
    down.key.repeat = 0;
    down.key.keysym.sym = SDLK_RETURN;
    down.key.keysym.scancode = SDL_SCANCODE_RETURN;
    down.key.keysym.mod = KMOD_NONE;
    QueueSyntheticSDLEvent(down);

    SDL_Event up{};
    up.type = SDL_KEYUP;
    up.key.state = SDL_RELEASED;
    up.key.repeat = 0;
    up.key.keysym.sym = SDLK_RETURN;
    up.key.keysym.scancode = SDL_SCANCODE_RETURN;
    up.key.keysym.mod = KMOD_NONE;
    QueueSyntheticSDLEvent(up);
}

extern "C" JNIEXPORT void JNICALL
Java_com_muonline_client_MuMainNativeActivity_nativeLoginMovieComplete(JNIEnv*, jobject)
{
    SDL_Event e {};
    e.type = SDL_USEREVENT;
    e.user.code = kSdlUserEventLoginIntroMovieFinished;
    e.user.data1 = nullptr;
    e.user.data2 = nullptr;
    QueueSyntheticSDLEvent(e);
}

extern "C" JNIEXPORT void JNICALL
Java_com_muonline_client_MuMainNativeActivity_nativeLoginBackgroundMovieStopped(JNIEnv*, jclass)
{
    MU_AndroidMarkLoginBackgroundMovieStopped();
}

#if defined(__ANDROID__) || defined(MU_IOS)
static void AndroidSyncImeWithFocusedTextInput()
{
    const bool hasFocusedTextInput = AndroidHasFocusedTextInput() || g_charNameInputActive;
#if !defined(MU_ANDROID_DISABLE_LOG)
    static int s_lastImeState = -1;
    const int imeState = hasFocusedTextInput ? 1 : 0;
    if (s_lastImeState != imeState)
    {
        LOGI(
            "CHATIME IME state changed focused=%d sdlTextInput=%d",
            imeState,
            MU_MobileIsTextInputActive() ? 1 : 0);
        s_lastImeState = imeState;
    }
#endif
    if (hasFocusedTextInput)
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
}
#endif

static void ProcessAndroidEventQueue()
{
    int screenW = g_DrawableWidth;
    int screenH = g_DrawableHeight;
    std::vector<SDL_Event> events = DrainSyntheticSDLEvents();
    for (const SDL_Event& event : events)
    {
        HandleSDLEvent(event, screenW, screenH);
        if (Destroy)
        {
            break;
        }
    }
    g_DrawableWidth = screenW;
    g_DrawableHeight = screenH;
}

static void ShutdownAndroidGame();

static bool InitializeAndroidGame()
{
    if (g_AndroidGameInitialized)
    {
        return true;
    }

    LOGI("=== MuMain Android sokol bootstrap ===");
    Destroy = false;
    g_bWndActive = true;
    g_AndroidQuitRequested = false;
    g_pendingImeEnterTextInput = false;
    g_DrawableWidth = (g_DrawableWidth > 0) ? g_DrawableWidth : 1280;
    g_DrawableHeight = (g_DrawableHeight > 0) ? g_DrawableHeight : 720;

    MU_MobilePlatformInit();
    SetWorkingDirectoryToMobileDataRoot();
    InitializeTakumiProtectState();
    InitializeTakumiPacketKeys();
    g_AndroidAudioAvailable = false;

    const char* backendEnv = std::getenv("MU_RENDER_BACKEND");
    const RenderBackendType requestedBackend = ParseRenderBackendType(backendEnv);
    LOGI(
        "Render backend request env=%s -> %s",
        backendEnv ? backendEnv : "(null)",
        RenderBackendTypeToString(requestedBackend));

    int screenW = (g_DrawableWidth > 0) ? g_DrawableWidth : 1280;
    int screenH = (g_DrawableHeight > 0) ? g_DrawableHeight : 720;
    UpdateAndroidScreenMetrics(screenW, screenH);
    LOGI(
        "Screen size (drawable): %dx%d (scale: %.2f x %.2f)",
        screenW,
        screenH,
        g_fScreenRate_x,
        g_fScreenRate_y);

    auto initializeRenderBackend = [&](RenderBackendType backendType) -> bool
    {
        std::unique_ptr<IRenderBackend> backend = CreateRenderBackend(backendType);
        if (!backend)
        {
            LOGE("CreateRenderBackend failed for type=%s", RenderBackendTypeToString(backendType));
            return false;
        }

        if (!backend->Initialize(screenW, screenH))
        {
            LOGW("Render backend init failed: %s", backend->GetName());
            return false;
        }

        LOGI("Render backend active: %s", backend->GetName());
        g_RenderBackend = std::move(backend);
        return true;
    };

    if (!initializeRenderBackend(requestedBackend))
    {
        if ((requestedBackend != RenderBackendType::OpenGLCompat)
            && initializeRenderBackend(RenderBackendType::OpenGLCompat))
        {
            (void)0;
        }
        else
        {
            LOGE("No render backend could be initialized");
            ShutdownAndroidGame();
            return false;
        }
    }

    const bool preferDirectVertexArrays = IsLikelyAndroidEmulator();
    g_adaptivePerf.isEmulator = preferDirectVertexArrays;
    GL_SetPreferDirectVertexArrays(preferDirectVertexArrays);
    GL_SetSkipVBOOrphan(preferDirectVertexArrays);
    LOGI(
        "GL compat VA policy: preferDirect=%d (emulator=%d)",
        preferDirectVertexArrays ? 1 : 0,
        preferDirectVertexArrays ? 1 : 0);

    {
        int fontSize = static_cast<int>(std::ceil(12.0f + (static_cast<float>(WindowHeight) - 480.0f) / 200.0f));
        if (fontSize < 10) fontSize = 10;
        AndroidGDI_Init(fontSize);
        g_hFont = AndroidCreateFont(fontSize, 400);
        g_hFontBold = AndroidCreateFont(fontSize, 600);
        g_hFontBig = AndroidCreateFont(fontSize * 2, 600);
        g_hFixFont = AndroidCreateFont((static_cast<int>(WindowHeight) <= 600) ? 13 : 14, 400);
        LOGI("GDI fonts created: size=%d big=%d", fontSize, fontSize * 2);
    }

    if (!g_hWnd) g_hWnd = reinterpret_cast<HWND>(0x1);
    CInput::Instance().Create(g_hWnd, static_cast<long>(WindowWidth), static_cast<long>(WindowHeight));

    GameConfig::GetInstance().Load();

    const bool requestedSoundEnabled = GameConfig::GetInstance().GetSoundEnabled();
    const bool requestedMusicEnabled = GameConfig::GetInstance().GetMusicEnabled();
    m_SoundOnOff = requestedSoundEnabled && g_AndroidAudioAvailable;
    m_MusicOnOff = requestedMusicEnabled && g_AndroidAudioAvailable;
    m_RememberMe = GameConfig::GetInstance().GetRememberMe() ? 1 : 0;
    LOGI(
        "Audio config requested(sound=%d music=%d) active(sound=%d music=%d) rememberMe=%d",
        requestedSoundEnabled ? 1 : 0,
        requestedMusicEnabled ? 1 : 0,
        static_cast<int>(m_SoundOnOff),
        static_cast<int>(m_MusicOnOff),
        static_cast<int>(m_RememberMe));
    LOGW("SDL audio runtime is disabled under NativeActivity; running muted");

    static std::wstring serverIP = GameConfig::GetInstance().GetServerIP();
    int configuredPort = GameConfig::GetInstance().GetServerPort();
    ApplyAndroidNetworkBootstrapOverrides(serverIP, configuredPort);
    if (IsUnsetOrLegacyDevServerIp(serverIP))
    {
        serverIP = CfgDefaults::CfgDefaultServerIP;
    }
    // Only remap invalid ports or a known-wrong first-hop (55901 = game shard in MuServer docs).
    // Keep OpenMU connect (44405/44406) and server-next (e.g. 44605/44606) exactly as in GameConfig.
    if (configuredPort <= 0 || configuredPort == static_cast<int>(MuLanDefaults::kDefaultGameShardPortMin))
    {
        configuredPort = CfgDefaults::CfgDefaultServerPort;
    }
    GameConfig::GetInstance().SetServerIP(serverIP);
    GameConfig::GetInstance().SetServerPort(configuredPort);

    static char androidServerIpA[64] = {};
    std::wcstombs(androidServerIpA, serverIP.c_str(), sizeof(androidServerIpA) - 1);
    szServerIpAddress = androidServerIpA;
    g_ServerPort = static_cast<WORD>(configuredPort);
    LOGI("Network target = %s:%u", szServerIpAddress, g_ServerPort);

    if (m_RememberMe)
    {
        wchar_t usernameW[_countof(m_Username)] = {};
        wchar_t passwordW[_countof(m_Password)] = {};
        GameConfig::GetInstance().DecryptCredentials(usernameW, passwordW, _countof(usernameW), _countof(passwordW));
        std::wcstombs(m_Username, usernameW, _countof(m_Username) - 1);
        std::wcstombs(m_Password, passwordW, _countof(m_Password) - 1);
    }

    std::wstring langSel = GameConfig::GetInstance().GetLanguageSelection();
    std::wcstombs(g_aszMLSelection, langSel.c_str(), MAX_LANGUAGE_NAME_LENGTH - 1);
    g_aszMLSelection[MAX_LANGUAGE_NAME_LENGTH - 1] = '\0';
    if (g_aszMLSelection[0] == '\0')
    {
        std::strcpy(g_aszMLSelection, "Eng");
    }
    g_strSelectedML = g_aszMLSelection;
    pMultiLanguage = new CMultiLanguage(g_strSelectedML);

    LOGI("INIT: srand");
    srand(static_cast<unsigned int>(time(nullptr)));
    for (int& value : RandomTable)
    {
        value = rand() % 360;
    }

    LOGI("INIT: alloc GateAttribute");
    RendomMemoryDump = new BYTE[rand() % 100 + 1];
    GateAttribute = new GATE_ATTRIBUTE[MAX_GATES] {};
    LOGI("INIT: alloc SkillAttribute");
    SkillAttribute = new SKILL_ATTRIBUTE[MAX_SKILLS] {};
    LOGI("INIT: alloc ItemAttRibute");
    ItemAttRibuteMemoryDump = new ITEM_ATTRIBUTE[MAX_ITEM + 1024] {};
    ItemAttribute = ItemAttRibuteMemoryDump + rand() % 1024;
    LOGI("INIT: alloc CharacterMemoryDump");
    CharacterMemoryDump = new CHARACTER[MAX_CHARACTERS_CLIENT + 1 + 128] {};
    CharactersClient = CharacterMemoryDump + rand() % 128;
    LOGI("INIT: alloc CharacterMachine");
    CharacterMachine = new CHARACTER_MACHINE;

    std::memset(GateAttribute, 0, sizeof(GATE_ATTRIBUTE) * MAX_GATES);
    std::memset(ItemAttribute, 0, sizeof(ITEM_ATTRIBUTE) * MAX_ITEM);
    std::memset(SkillAttribute, 0, sizeof(SKILL_ATTRIBUTE) * MAX_SKILLS);
    std::memset(CharacterMachine, 0, sizeof(CHARACTER_MACHINE));

    LOGI("INIT: CharacterMachine->Init()");
    CharacterAttribute = &CharacterMachine->Character;
    CharacterMachine->Init();
    Hero = &CharactersClient[0];

    LOGI("INIT: new UI objects");
    g_pMercenaryInputBox = new CUIMercenaryInputBox;
    g_pSingleTextInputBox = new CUITextInputBox;
    g_pSinglePasswdInputBox = new CUITextInputBox;
    g_pUIManager = new CUIManager;
    g_pUIMapName = new CUIMapName;

    // Winmain.cpp parity: without Init(), m_hEditWnd stays NULL and GiveFocus() is a no-op
    // (delete-character resident code / other modal passwd flows never open SDL IME on Android).
    if (g_iChatInputType == 1)
    {
        g_pMercenaryInputBox->Init(g_hWnd);
        g_pSingleTextInputBox->Init(g_hWnd, 200, 20);
        g_pSinglePasswdInputBox->Init(g_hWnd, 200, 20, 9, TRUE);
        g_pSingleTextInputBox->SetState(UISTATE_HIDE);
        g_pSinglePasswdInputBox->SetState(UISTATE_HIDE);
        g_pMercenaryInputBox->SetFont(g_hFont);
        g_pSingleTextInputBox->SetFont(g_hFont);
        g_pSinglePasswdInputBox->SetFont(g_hFont);
    }

    LOGI("INIT: BuffStateSystem::Make()");
    g_BuffSystem = BuffStateSystem::Make();
    LOGI("INIT: MapProcess::Make()");
    g_MapProcess = MapProcess::Make();
    LOGI("INIT: PetProcess::Make()");
    g_petProcess = PetProcess::Make();

    LOGI("INIT: CUIMng::Create()");
    CUIMng::Instance().Create();
    LOGI("INIT: g_pNewUISystem->Create()");
    g_pNewUISystem->Create();
    LOGI("INIT: UI creation done");
    LoadVirtualSkillSlots();
    LOGI("VirtualPad slots: %s", BuildVirtualSkillArrayString(g_virtualSkillSlots).c_str());

    if (g_adaptivePerf.isEmulator)
    {
        g_adaptivePerf.defaultMessageBudget = 70;
        g_adaptivePerf.minMessageBudget = 30;
    }

    SetMaxMessagePerCycle(g_adaptivePerf.defaultMessageBudget);
    SetTargetFps(g_adaptivePerf.targetFps);
    LOGI(
        "Android perf defaults: maxMsgPerCycle=%d targetFps=%.1f fxScale=%.2f adaptive=on isEmulator=%d",
        g_MaxMessagePerCycle,
        g_adaptivePerf.targetFps,
        1.0f,
        g_adaptivePerf.isEmulator ? 1 : 0);

    if (g_AndroidAudioAvailable && (m_SoundOnOff || m_MusicOnOff) && g_pOption)
    {
        int vol = GameConfig::GetInstance().GetVolumeLevel();
        if ((vol < 0) || (vol > 10)) vol = 10;
        g_pOption->SetVolumeLevel(vol);
        SetEffectVolumeLevel(vol);
        LOGI("SDL mixer volume set: level=%d", vol);
    }
    else if (g_AndroidAudioAvailable && (m_MusicOnOff || m_SoundOnOff))
    {
        SetEffectVolumeLevel(10);
        LOGI("SDL mixer fallback volume applied: level=10");
    }

    g_AndroidFrameState.perfFrequency = static_cast<Uint64>(MU_MobilePerfFrequency());
    g_AndroidFrameState.perfWindowStart = static_cast<Uint64>(MU_MobilePerfNow());
    g_AndroidFrameState.perfFrames = 0;
    g_AndroidFrameState.drawCallsTotal = 0;
    g_AndroidFrameState.vertsTotal = 0;
    g_AndroidFrameState.dbgFrameCount = 0;
    g_AndroidFrameState.muHelperLastTickMs = MU_MobileGetTicks();
    g_AndroidFrameState.hackLastTickMs = MU_MobileGetTicks();

    g_AndroidGameInitialized = true;
    LOGI("All systems initialized - sokol frame loop active");
    return true;
}

static void RunAndroidGameFrame()
{
    if (!g_AndroidGameInitialized)
    {
        if (!InitializeAndroidGame())
        {
            if (!g_AndroidQuitRequested)
            {
                g_AndroidQuitRequested = true;
                sapp_request_quit();
            }
            return;
        }
    }

    if (Destroy)
    {
        if (!g_AndroidQuitRequested)
        {
            g_AndroidQuitRequested = true;
            sapp_request_quit();
        }
        return;
    }

    SyncAndroidDrawableSizeFromSokol("frame");

    MouseLButtonDBClick = false;
    if (MouseLButtonPop && ((g_iMousePopPosition_x != MouseX) || (g_iMousePopPosition_y != MouseY)))
    {
        MouseLButtonPop = false;
    }

    ProcessAndroidEventQueue();
    if (Destroy)
    {
        if (!g_AndroidQuitRequested)
        {
            g_AndroidQuitRequested = true;
            sapp_request_quit();
        }
        return;
    }

    AndroidDrainPackets();
    UpdateVirtualPadHolds();
    UpdateVirtualProximityCombat();

    const uint32_t nowTicks = MU_MobileGetTicks();
    int hackTickBudget = 4;
    while ((nowTicks - g_AndroidFrameState.hackLastTickMs) >= 20000u && (hackTickBudget-- > 0))
    {
        CheckHack();
        g_AndroidFrameState.hackLastTickMs += 20000u;
    }

    int muHelperTickBudget = 4;
    while ((nowTicks - g_AndroidFrameState.muHelperLastTickMs) >= 250u && (muHelperTickBudget-- > 0))
    {
        g_AndroidFrameState.muHelperLastTickMs += 250u;
    }

    if (g_bWndActive)
    {
        // Drain TCP recv queue before Scene so login/server replies apply before UI logic this frame.
        ProtocolCompiler();
        const Uint64 renderSceneStart = static_cast<Uint64>(MU_MobilePerfNow());
        Scene(nullptr);
#if defined(__ANDROID__) || defined(MU_IOS)
        // After UI updates focus from this frame's taps, sync soft keyboard (was before Scene = one frame stale).
        AndroidSyncImeWithFocusedTextInput();
#endif
        const Uint64 virtualPadStart = static_cast<Uint64>(MU_MobilePerfNow());
        RenderVirtualPad();
        const Uint64 presentStart = static_cast<Uint64>(MU_MobilePerfNow());
        if (g_RenderBackend)
        {
            g_RenderBackend->Present();
        }
        else
        {
            GL_FlushPending();
        }
        const Uint64 presentEnd = static_cast<Uint64>(MU_MobilePerfNow());

        int frameDrawCalls = 0;
        int frameVerts = 0;
        if (g_RenderBackend)
        {
            const RenderBackendStats stats = g_RenderBackend->GetAndResetStats();
            frameDrawCalls = stats.drawCalls;
            frameVerts = stats.vertices;
        }
        else
        {
            GL_GetDrawStats(&frameDrawCalls, &frameVerts);
            GL_ResetDrawStats();
        }

        const ObjectPerfSnapshot objectPerf = ConsumeObjectPerfSnapshot();
        const CharacterPerfSnapshot characterPerf = ConsumeCharacterPerfSnapshot();
        const TerrainPerfSnapshot terrainPerf = ConsumeTerrainPerfSnapshot();
        const MainScenePerfSnapshot mainScenePerf = ConsumeMainScenePerfSnapshot();

        ++g_AndroidFrameState.perfFrames;
        g_AndroidFrameState.drawCallsTotal += frameDrawCalls;
        g_AndroidFrameState.vertsTotal += frameVerts;
        g_AndroidFrameState.objRenderCandidatesTotal += objectPerf.renderCandidates;
        g_AndroidFrameState.objRenderRenderedTotal += objectPerf.renderRendered;
        g_AndroidFrameState.objRenderCulledTotal += objectPerf.renderDistanceCulled;
        g_AndroidFrameState.objRenderBaseCallsTotal += objectPerf.renderBaseCalls;
        g_AndroidFrameState.objVisualCallsTotal += objectPerf.visualCalls;
        g_AndroidFrameState.objRenderAfterCallsTotal += objectPerf.renderAfterCalls;
        g_AndroidFrameState.charRenderCandidatesTotal += characterPerf.renderCandidates;
        g_AndroidFrameState.charRenderRenderedTotal += characterPerf.renderRendered;
        g_AndroidFrameState.charRenderDeferredTotal += characterPerf.renderDeferred;
        g_AndroidFrameState.charRenderPlayersTotal += characterPerf.renderPlayers;
        g_AndroidFrameState.charRenderMonstersTotal += characterPerf.renderMonsters;
        g_AndroidFrameState.terrainNormalBlocksTotal += terrainPerf.normalBlocks;
        g_AndroidFrameState.terrainNormalTilesTotal += terrainPerf.normalTiles;
        g_AndroidFrameState.terrainGrassBlocksTotal += terrainPerf.grassBlocks;
        g_AndroidFrameState.terrainGrassTilesTotal += terrainPerf.grassTiles;
        g_AndroidFrameState.terrainAfterBlocksTotal += terrainPerf.afterBlocks;
        g_AndroidFrameState.terrainAfterTilesTotal += terrainPerf.afterTiles;
        g_AndroidFrameState.sceneTicksTotal += virtualPadStart - renderSceneStart;
        g_AndroidFrameState.padTicksTotal += presentStart - virtualPadStart;
        g_AndroidFrameState.presentTicksTotal += presentEnd - presentStart;
        g_AndroidFrameState.objMoveTicksTotal += static_cast<Uint64>(objectPerf.moveTicks);
        g_AndroidFrameState.objRenderTicksTotal += static_cast<Uint64>(objectPerf.renderTicks);
        g_AndroidFrameState.terrainRenderTicksTotal += static_cast<Uint64>(terrainPerf.renderTicks);
        g_AndroidFrameState.terrainAfterTicksTotal += static_cast<Uint64>(terrainPerf.afterTicks);
        g_AndroidFrameState.charMoveTicksTotal += static_cast<Uint64>(characterPerf.moveTicks);
        g_AndroidFrameState.charRenderTicksTotal += static_cast<Uint64>(characterPerf.renderTicks);
        g_AndroidFrameState.mainShadowTicksTotal += static_cast<Uint64>(mainScenePerf.shadowTicks);
        g_AndroidFrameState.mainBoidsTicksTotal += static_cast<Uint64>(mainScenePerf.boidsTicks);
        g_AndroidFrameState.mainMiscWorldTicksTotal += static_cast<Uint64>(mainScenePerf.miscWorldTicks);
        g_AndroidFrameState.mainJointsTicksTotal += static_cast<Uint64>(mainScenePerf.jointsTicks);
        g_AndroidFrameState.mainEffectsTicksTotal += static_cast<Uint64>(mainScenePerf.effectsTicks);
        g_AndroidFrameState.mainBlursTicksTotal += static_cast<Uint64>(mainScenePerf.blursTicks);
        g_AndroidFrameState.mainSpritesTicksTotal += static_cast<Uint64>(mainScenePerf.spritesTicks);
        g_AndroidFrameState.mainParticlesTicksTotal += static_cast<Uint64>(mainScenePerf.particlesTicks);
        g_AndroidFrameState.mainPointsTicksTotal += static_cast<Uint64>(mainScenePerf.pointsTicks);
        g_AndroidFrameState.mainAfterEffectsTicksTotal += static_cast<Uint64>(mainScenePerf.afterEffectsTicks);
        g_AndroidFrameState.mainUiTicksTotal += static_cast<Uint64>(mainScenePerf.uiTicks);

        const Uint64 nowCounter = static_cast<Uint64>(MU_MobilePerfNow());
        const double elapsedSec =
            static_cast<double>(nowCounter - g_AndroidFrameState.perfWindowStart)
            / static_cast<double>((g_AndroidFrameState.perfFrequency > 0) ? g_AndroidFrameState.perfFrequency : 1);
        if ((elapsedSec >= 0.25) && (g_AndroidFrameState.perfFrames > 0))
        {
            const double perfFreq =
                (g_AndroidFrameState.perfFrequency > 0)
                    ? static_cast<double>(g_AndroidFrameState.perfFrequency)
                    : 1.0;
            const double tickToMs = 1000.0 / perfFreq;
            const double fps = static_cast<double>(g_AndroidFrameState.perfFrames) / elapsedSec;
            const double avgFrameMs = (elapsedSec * 1000.0) / static_cast<double>(g_AndroidFrameState.perfFrames);
            const int avgDrawCalls = g_AndroidFrameState.drawCallsTotal / g_AndroidFrameState.perfFrames;
            const int avgVerts = g_AndroidFrameState.vertsTotal / g_AndroidFrameState.perfFrames;
            const double avgSceneMs =
                (static_cast<double>(g_AndroidFrameState.sceneTicksTotal) * tickToMs)
                / static_cast<double>(g_AndroidFrameState.perfFrames);
            const double avgPadMs =
                (static_cast<double>(g_AndroidFrameState.padTicksTotal) * tickToMs)
                / static_cast<double>(g_AndroidFrameState.perfFrames);
            const double avgPresentMs =
                (static_cast<double>(g_AndroidFrameState.presentTicksTotal) * tickToMs)
                / static_cast<double>(g_AndroidFrameState.perfFrames);
            const int avgObjRenderCandidates =
                g_AndroidFrameState.objRenderCandidatesTotal / g_AndroidFrameState.perfFrames;
            const int avgObjRenderRendered =
                g_AndroidFrameState.objRenderRenderedTotal / g_AndroidFrameState.perfFrames;
            const int avgObjRenderCulled =
                g_AndroidFrameState.objRenderCulledTotal / g_AndroidFrameState.perfFrames;
            const int avgObjRenderBaseCalls =
                g_AndroidFrameState.objRenderBaseCallsTotal / g_AndroidFrameState.perfFrames;
            const int avgObjVisualCalls =
                g_AndroidFrameState.objVisualCallsTotal / g_AndroidFrameState.perfFrames;
            const int avgObjRenderAfterCalls =
                g_AndroidFrameState.objRenderAfterCallsTotal / g_AndroidFrameState.perfFrames;
            const int avgCharRenderCandidates =
                g_AndroidFrameState.charRenderCandidatesTotal / g_AndroidFrameState.perfFrames;
            const int avgCharRenderRendered =
                g_AndroidFrameState.charRenderRenderedTotal / g_AndroidFrameState.perfFrames;
            const int avgCharRenderDeferred =
                g_AndroidFrameState.charRenderDeferredTotal / g_AndroidFrameState.perfFrames;
            const int avgCharRenderPlayers =
                g_AndroidFrameState.charRenderPlayersTotal / g_AndroidFrameState.perfFrames;
            const int avgCharRenderMonsters =
                g_AndroidFrameState.charRenderMonstersTotal / g_AndroidFrameState.perfFrames;
            const int avgTerrainNormalBlocks =
                g_AndroidFrameState.terrainNormalBlocksTotal / g_AndroidFrameState.perfFrames;
            const int avgTerrainNormalTiles =
                g_AndroidFrameState.terrainNormalTilesTotal / g_AndroidFrameState.perfFrames;
            const int avgTerrainGrassBlocks =
                g_AndroidFrameState.terrainGrassBlocksTotal / g_AndroidFrameState.perfFrames;
            const int avgTerrainGrassTiles =
                g_AndroidFrameState.terrainGrassTilesTotal / g_AndroidFrameState.perfFrames;
            const int avgTerrainAfterBlocks =
                g_AndroidFrameState.terrainAfterBlocksTotal / g_AndroidFrameState.perfFrames;
            const int avgTerrainAfterTiles =
                g_AndroidFrameState.terrainAfterTilesTotal / g_AndroidFrameState.perfFrames;
            const double avgObjMoveMs =
                (static_cast<double>(g_AndroidFrameState.objMoveTicksTotal) * tickToMs)
                / static_cast<double>(g_AndroidFrameState.perfFrames);
            const double avgObjRenderMs =
                (static_cast<double>(g_AndroidFrameState.objRenderTicksTotal) * tickToMs)
                / static_cast<double>(g_AndroidFrameState.perfFrames);
            const double avgTerrainRenderMs =
                (static_cast<double>(g_AndroidFrameState.terrainRenderTicksTotal) * tickToMs)
                / static_cast<double>(g_AndroidFrameState.perfFrames);
            const double avgTerrainAfterMs =
                (static_cast<double>(g_AndroidFrameState.terrainAfterTicksTotal) * tickToMs)
                / static_cast<double>(g_AndroidFrameState.perfFrames);
            const double avgCharMoveMs =
                (static_cast<double>(g_AndroidFrameState.charMoveTicksTotal) * tickToMs)
                / static_cast<double>(g_AndroidFrameState.perfFrames);
            const double avgCharRenderMs =
                (static_cast<double>(g_AndroidFrameState.charRenderTicksTotal) * tickToMs)
                / static_cast<double>(g_AndroidFrameState.perfFrames);
            const double avgMainShadowMs =
                (static_cast<double>(g_AndroidFrameState.mainShadowTicksTotal) * tickToMs)
                / static_cast<double>(g_AndroidFrameState.perfFrames);
            const double avgMainBoidsMs =
                (static_cast<double>(g_AndroidFrameState.mainBoidsTicksTotal) * tickToMs)
                / static_cast<double>(g_AndroidFrameState.perfFrames);
            const double avgMainMiscWorldMs =
                (static_cast<double>(g_AndroidFrameState.mainMiscWorldTicksTotal) * tickToMs)
                / static_cast<double>(g_AndroidFrameState.perfFrames);
            const double avgMainJointsMs =
                (static_cast<double>(g_AndroidFrameState.mainJointsTicksTotal) * tickToMs)
                / static_cast<double>(g_AndroidFrameState.perfFrames);
            const double avgMainEffectsMs =
                (static_cast<double>(g_AndroidFrameState.mainEffectsTicksTotal) * tickToMs)
                / static_cast<double>(g_AndroidFrameState.perfFrames);
            const double avgMainBlursMs =
                (static_cast<double>(g_AndroidFrameState.mainBlursTicksTotal) * tickToMs)
                / static_cast<double>(g_AndroidFrameState.perfFrames);
            const double avgMainSpritesMs =
                (static_cast<double>(g_AndroidFrameState.mainSpritesTicksTotal) * tickToMs)
                / static_cast<double>(g_AndroidFrameState.perfFrames);
            const double avgMainParticlesMs =
                (static_cast<double>(g_AndroidFrameState.mainParticlesTicksTotal) * tickToMs)
                / static_cast<double>(g_AndroidFrameState.perfFrames);
            const double avgMainPointsMs =
                (static_cast<double>(g_AndroidFrameState.mainPointsTicksTotal) * tickToMs)
                / static_cast<double>(g_AndroidFrameState.perfFrames);
            const double avgMainAfterEffectsMs =
                (static_cast<double>(g_AndroidFrameState.mainAfterEffectsTicksTotal) * tickToMs)
                / static_cast<double>(g_AndroidFrameState.perfFrames);
            const double avgMainUiMs =
                (static_cast<double>(g_AndroidFrameState.mainUiTicksTotal) * tickToMs)
                / static_cast<double>(g_AndroidFrameState.perfFrames);
            const double avgTrackedSceneMs =
                avgObjMoveMs + avgObjRenderMs +
                avgCharMoveMs + avgCharRenderMs +
                avgTerrainRenderMs + avgTerrainAfterMs;
            const double avgOtherSceneMs =
                (avgSceneMs > avgTrackedSceneMs) ? (avgSceneMs - avgTrackedSceneMs) : 0.0;
            const double avgPhaseTrackedMs =
                avgMainShadowMs + avgMainBoidsMs + avgMainMiscWorldMs +
                avgMainJointsMs + avgMainEffectsMs + avgMainBlursMs +
                avgMainSpritesMs + avgMainParticlesMs + avgMainPointsMs +
                avgMainAfterEffectsMs + avgMainUiMs;
            const double avgPhaseRemainderMs =
                (avgOtherSceneMs > avgPhaseTrackedMs) ? (avgOtherSceneMs - avgPhaseTrackedMs) : 0.0;
            UpdateAdaptivePerformance(fps, avgFrameMs);
            PERF_LOGI(
                "PERF fps=%.1f frameMs=%.2f sceneMs=%.2f padMs=%.2f presentMs=%.2f avg(draw=%d verts=%d)",
                fps,
                avgFrameMs,
                avgSceneMs,
                avgPadMs,
                avgPresentMs,
                avgDrawCalls,
                avgVerts);
            if (((g_AndroidFrameState.detailLogWindowCounter++ % 4) == 0) || fps < 15.0)
            {
                const SceneCrowdSnapshot crowdSnapshot = CaptureSceneCrowdSnapshot();
                PERF_LOGI(
                    "PERF_SCENE world=%d:%s fxScale=%.2f char(vis=%d ply=%d mon=%d npc=%d pet=%d topMon=%d:%s x%d) obj(vis=%d op=%d trap=%d topObj=%d:%s x%d)",
                    gMapManager.WorldActive,
                    GetPerfWorldDebugName(gMapManager.WorldActive),
                    GetAdaptiveEffectSpawnScale(),
                    crowdSnapshot.visibleCharacters,
                    crowdSnapshot.visiblePlayers,
                    crowdSnapshot.visibleMonsters,
                    crowdSnapshot.visibleNpcs,
                    crowdSnapshot.visiblePets,
                    crowdSnapshot.dominantMonsterType,
                    GetPerfMonsterDebugName(crowdSnapshot.dominantMonsterType),
                    crowdSnapshot.dominantMonsterCount,
                    crowdSnapshot.visibleWorldObjects,
                    crowdSnapshot.visibleOperateObjects,
                    crowdSnapshot.visibleTrapObjects,
                    crowdSnapshot.dominantObjectType,
                    GetPerfObjectDebugName(crowdSnapshot.dominantObjectType),
                    crowdSnapshot.dominantObjectCount);
                PERF_LOGI(
                    "PERF_CPU obj(move=%.2f render=%.2f cand=%d draw=%d cull=%d calls=%d/%d/%d) char(move=%.2f render=%.2f cand=%d draw=%d def=%d ply=%d mon=%d) terrain(render=%.2f after=%.2f blk=%d/%d grass=%d/%d after=%d/%d)",
                    avgObjMoveMs,
                    avgObjRenderMs,
                    avgObjRenderCandidates,
                    avgObjRenderRendered,
                    avgObjRenderCulled,
                    avgObjRenderBaseCalls,
                    avgObjVisualCalls,
                    avgObjRenderAfterCalls,
                    avgCharMoveMs,
                    avgCharRenderMs,
                    avgCharRenderCandidates,
                    avgCharRenderRendered,
                    avgCharRenderDeferred,
                    avgCharRenderPlayers,
                    avgCharRenderMonsters,
                    avgTerrainRenderMs,
                    avgTerrainAfterMs,
                    avgTerrainNormalBlocks,
                    avgTerrainNormalTiles,
                    avgTerrainGrassBlocks,
                    avgTerrainGrassTiles,
                    avgTerrainAfterBlocks,
                    avgTerrainAfterTiles);
                PERF_LOGI(
                    "PERF_PHASE shadow=%.2f boids=%.2f misc=%.2f joints=%.2f effects=%.2f blurs=%.2f sprites=%.2f particles=%.2f points=%.2f afterfx=%.2f ui=%.2f rem=%.2f",
                    avgMainShadowMs,
                    avgMainBoidsMs,
                    avgMainMiscWorldMs,
                    avgMainJointsMs,
                    avgMainEffectsMs,
                    avgMainBlursMs,
                    avgMainSpritesMs,
                    avgMainParticlesMs,
                    avgMainPointsMs,
                    avgMainAfterEffectsMs,
                    avgMainUiMs,
                    avgPhaseRemainderMs);
            }
            g_AndroidFrameState.perfWindowStart = nowCounter;
            g_AndroidFrameState.perfFrames = 0;
            g_AndroidFrameState.drawCallsTotal = 0;
            g_AndroidFrameState.vertsTotal = 0;
            g_AndroidFrameState.objRenderCandidatesTotal = 0;
            g_AndroidFrameState.objRenderRenderedTotal = 0;
            g_AndroidFrameState.objRenderCulledTotal = 0;
            g_AndroidFrameState.objRenderBaseCallsTotal = 0;
            g_AndroidFrameState.objVisualCallsTotal = 0;
            g_AndroidFrameState.objRenderAfterCallsTotal = 0;
            g_AndroidFrameState.charRenderCandidatesTotal = 0;
            g_AndroidFrameState.charRenderRenderedTotal = 0;
            g_AndroidFrameState.charRenderDeferredTotal = 0;
            g_AndroidFrameState.charRenderPlayersTotal = 0;
            g_AndroidFrameState.charRenderMonstersTotal = 0;
            g_AndroidFrameState.terrainNormalBlocksTotal = 0;
            g_AndroidFrameState.terrainNormalTilesTotal = 0;
            g_AndroidFrameState.terrainGrassBlocksTotal = 0;
            g_AndroidFrameState.terrainGrassTilesTotal = 0;
            g_AndroidFrameState.terrainAfterBlocksTotal = 0;
            g_AndroidFrameState.terrainAfterTilesTotal = 0;
            g_AndroidFrameState.sceneTicksTotal = 0;
            g_AndroidFrameState.padTicksTotal = 0;
            g_AndroidFrameState.presentTicksTotal = 0;
            g_AndroidFrameState.objMoveTicksTotal = 0;
            g_AndroidFrameState.objRenderTicksTotal = 0;
            g_AndroidFrameState.terrainRenderTicksTotal = 0;
            g_AndroidFrameState.terrainAfterTicksTotal = 0;
            g_AndroidFrameState.charMoveTicksTotal = 0;
            g_AndroidFrameState.charRenderTicksTotal = 0;
            g_AndroidFrameState.mainShadowTicksTotal = 0;
            g_AndroidFrameState.mainBoidsTicksTotal = 0;
            g_AndroidFrameState.mainMiscWorldTicksTotal = 0;
            g_AndroidFrameState.mainJointsTicksTotal = 0;
            g_AndroidFrameState.mainEffectsTicksTotal = 0;
            g_AndroidFrameState.mainBlursTicksTotal = 0;
            g_AndroidFrameState.mainSpritesTicksTotal = 0;
            g_AndroidFrameState.mainParticlesTicksTotal = 0;
            g_AndroidFrameState.mainPointsTicksTotal = 0;
            g_AndroidFrameState.mainAfterEffectsTicksTotal = 0;
            g_AndroidFrameState.mainUiTicksTotal = 0;
        }

        if (g_AndroidFrameState.dbgFrameCount < 10)
        {
            LOGI(
                "Frame %d - drawCalls=%d verts=%d",
                g_AndroidFrameState.dbgFrameCount,
                frameDrawCalls,
                frameVerts);
            ++g_AndroidFrameState.dbgFrameCount;
        }
    }
    else
    {
        MU_MobileSleep(16);
    }

    ProtocolCompiler();

    MouseLButtonPush = false;
    MouseRButtonPush = false;
    MouseMButtonPush = false;
    MouseWheel = 0;
}

static void ShutdownAndroidGame()
{
    if (!g_AndroidGameInitialized)
    {
        MU_MobilePlatformShutdown();
        return;
    }

    LOGI("Main loop exited - cleaning up");
    SaveVirtualSkillSlots();

    if (g_RenderBackend)
    {
        g_RenderBackend->Shutdown();
        g_RenderBackend.reset();
    }

    DestroyWindow_Android();
    if (g_AndroidAudioAvailable)
    {
        DestroySound();
    }
    KillGLWindow();
    MU_MobilePlatformShutdown();
    MU_MobileClearKeyboardState();

    {
        std::lock_guard<std::mutex> lock(g_PendingSdlEventsMutex);
        g_PendingSdlEvents.clear();
    }

    g_AndroidGameInitialized = false;
    LOGI("=== MuMain shutdown complete ===");
}

static void OnAndroidSappInit()
{
    if (!InitializeAndroidGame() && !g_AndroidQuitRequested)
    {
        g_AndroidQuitRequested = true;
        sapp_request_quit();
    }
}

static void OnAndroidSappFrame()
{
    RunAndroidGameFrame();
}

static void OnAndroidSappCleanup()
{
    ShutdownAndroidGame();
}

static void OnAndroidSappEvent(const sapp_event* event)
{
    QueueSappEventAsSDL(event);
}

// =============================================================================
// SDL2 main â€” replaces WinMain
// SDL2 renames this to android_main internally via SDL_main.h macro
// =============================================================================
#if 0
int SDL_main(int argc, char* argv[])
{
    LOGI("=== MuMain Android v1.0 starting ===");
    (void)argc;
    (void)argv;

    SetWorkingDirectoryToMobileDataRoot();
    InitializeTakumiProtectState();
    InitializeTakumiPacketKeys();

    // â”€â”€ Initialize SDL2 â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_TIMER) < 0) {
        LOGE("SDL_Init failed: %s", SDL_GetError());
        return -1;
    }
    LOGI("SDL2 initialized");

    const char* backendEnv = SDL_getenv("MU_RENDER_BACKEND");
    const RenderBackendType requestedBackend = ParseRenderBackendType(backendEnv);
    LOGI(
        "Render backend request env=%s -> %s",
        backendEnv ? backendEnv : "(null)",
        RenderBackendTypeToString(requestedBackend));

    // â”€â”€ OpenGL ES 3.1 context attributes â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,  SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,    1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,      16);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE,    8);   // 8-bit stencil for circular icon clipping
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE,        5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,      6);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,       5);

    // â”€â”€ Create fullscreen window â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    g_SDLWindow = SDL_CreateWindow(
        "MU Online",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        0, 0,
        SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_SHOWN
    );
    if (!g_SDLWindow) {
        LOGE("SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    // â”€â”€ Create OpenGL ES context â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    g_SDLGLContext = SDL_GL_CreateContext(g_SDLWindow);
    if (!g_SDLGLContext) {
        LOGE("SDL_GL_CreateContext failed: %s", SDL_GetError());
        SDL_DestroyWindow(g_SDLWindow);
        SDL_Quit();
        return -1;
    }

    SDL_GL_MakeCurrent(g_SDLWindow, g_SDLGLContext);

    // VSync OFF â€” allow software frame cap to drive render pacing.
    SDL_GL_SetSwapInterval(0);

    // Initialize OpenGL ES 2.0 compatibility layer (immediate mode emulation)

    // â”€â”€ Get actual GL drawable resolution (may differ from window size on Android
    //    due to system bars â€” always use drawable size for viewport/rendering)
    int screenW = 1280, screenH = 720;
    SDL_GL_GetDrawableSize(g_SDLWindow, &screenW, &screenH);
    UpdateAndroidScreenMetrics(screenW, screenH);
    LOGI("Screen size (drawable): %dx%d (scale: %.2f x %.2f)",
        screenW, screenH, g_fScreenRate_x, g_fScreenRate_y);

    auto initializeRenderBackend = [&](RenderBackendType backendType) -> bool
    {
        std::unique_ptr<IRenderBackend> backend = CreateRenderBackend(backendType);
        if (!backend)
        {
            LOGE("CreateRenderBackend failed for type=%s", RenderBackendTypeToString(backendType));
            return false;
        }

        if (!backend->Initialize(g_SDLWindow, screenW, screenH))
        {
            LOGW("Render backend init failed: %s", backend->GetName());
            return false;
        }

        LOGI("Render backend active: %s", backend->GetName());
        g_RenderBackend = std::move(backend);
        return true;
    };

    if (!initializeRenderBackend(requestedBackend))
    {
        if (requestedBackend != RenderBackendType::OpenGLCompat)
        {
            LOGW("Falling back to OpenGLCompat backend");
            if (!initializeRenderBackend(RenderBackendType::OpenGLCompat))
            {
                LOGE("No render backend could be initialized");
                KillGLWindow();
                SDL_Quit();
                return -1;
            }
        }
        else
        {
            LOGE("OpenGLCompat backend failed to initialize");
            KillGLWindow();
            SDL_Quit();
            return -1;
        }
    }

    const bool preferDirectVertexArrays = IsLikelyAndroidEmulator();
    g_adaptivePerf.isEmulator = preferDirectVertexArrays;
    GL_SetPreferDirectVertexArrays(preferDirectVertexArrays);
    // SwiftShader (emulator) has no real GPU pipeline â†’ skip VBO orphaning.
    // Eliminates ~105 unnecessary driver-level malloc() calls per frame.
    GL_SetSkipVBOOrphan(preferDirectVertexArrays);
    LOGI(
        "GL compat VA policy: preferDirect=%d (emulator=%d)",
        preferDirectVertexArrays ? 1 : 0,
        preferDirectVertexArrays ? 1 : 0);

    // â”€â”€ Init Android GDI (SDL2_ttf text rendering)
    {
        int fontSize = (int)std::ceil(12.0f + ((float)WindowHeight - 480.0f) / 200.0f);
        if (fontSize < 10) fontSize = 10;
        AndroidGDI_Init(fontSize);

        // Create Windows-like font handles for the game
        g_hFont     = AndroidCreateFont(fontSize,     400); // FW_NORMAL
        g_hFontBold = AndroidCreateFont(fontSize,     600); // FW_SEMIBOLD
        g_hFontBig  = AndroidCreateFont(fontSize * 2, 600);
        g_hFixFont  = AndroidCreateFont(
            (int)WindowHeight <= 600 ? 13 : 14, 400);
        LOGI("GDI fonts created: size=%d big=%d", fontSize, fontSize*2);
    }

    // â”€â”€ Init input system with actual screen size so dialogs position correctly
    // g_hWnd is null on Android; use a dummy non-null value so Create() doesn't
    // bail out before setting m_lScreenWidth/m_lScreenHeight.
    if (!g_hWnd) g_hWnd = (HWND)0x1;
    CInput::Instance().Create(g_hWnd, (long)WindowWidth, (long)WindowHeight);

    // â”€â”€ Load game config â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    GameConfig::GetInstance().Load();

    m_SoundOnOff = GameConfig::GetInstance().GetSoundEnabled();
    m_MusicOnOff = GameConfig::GetInstance().GetMusicEnabled();
    m_RememberMe = GameConfig::GetInstance().GetRememberMe() ? 1 : 0;
    LOGI("Audio config: sound=%d music=%d rememberMe=%d", (int)m_SoundOnOff, (int)m_MusicOnOff, (int)m_RememberMe);

    static std::wstring serverIP = GameConfig::GetInstance().GetServerIP();
    int configuredPort = GameConfig::GetInstance().GetServerPort();
    ApplyAndroidNetworkBootstrapOverrides(serverIP, configuredPort);
    if (IsUnsetOrLegacyDevServerIp(serverIP))
    {
        serverIP = CfgDefaults::CfgDefaultServerIP;
    }
    // Only remap invalid ports or a known-wrong first-hop (55901 = game shard in MuServer docs).
    // Keep OpenMU connect (44405/44406) and server-next (e.g. 44605/44606) exactly as in GameConfig.
    if (configuredPort <= 0 || configuredPort == static_cast<int>(MuLanDefaults::kDefaultGameShardPortMin))
    {
        configuredPort = CfgDefaults::CfgDefaultServerPort;
    }
    GameConfig::GetInstance().SetServerIP(serverIP);
    GameConfig::GetInstance().SetServerPort(configuredPort);

    static char androidServerIpA[64] = {}; std::wcstombs(androidServerIpA, serverIP.c_str(), sizeof(androidServerIpA) - 1); szServerIpAddress = androidServerIpA;
    g_ServerPort = static_cast<WORD>(configuredPort);
    LOGI("Network target = %s:%u", szServerIpAddress, g_ServerPort);

    if (m_RememberMe)
    {
        wchar_t usernameW[_countof(m_Username)] = {};
        wchar_t passwordW[_countof(m_Password)] = {};
        GameConfig::GetInstance().DecryptCredentials(usernameW, passwordW,
            _countof(usernameW), _countof(passwordW));
        std::wcstombs(m_Username, usernameW, _countof(m_Username) - 1);
        std::wcstombs(m_Password, passwordW, _countof(m_Password) - 1);
    }

    // â”€â”€ Multi-language setup â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    std::wstring langSel = GameConfig::GetInstance().GetLanguageSelection();
    std::wcstombs(g_aszMLSelection, langSel.c_str(), MAX_LANGUAGE_NAME_LENGTH - 1);
    g_aszMLSelection[MAX_LANGUAGE_NAME_LENGTH - 1] = '\0';
    if (g_aszMLSelection[0] == '\0')
        strcpy(g_aszMLSelection, "Eng");
    g_strSelectedML = g_aszMLSelection;
    pMultiLanguage  = new CMultiLanguage(g_strSelectedML);

    // â”€â”€ Audio: SDL_mixer replaces wzAudio + DirectSound â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    if (m_MusicOnOff || m_SoundOnOff)
    {
        const int mixerFlags = MIX_INIT_MP3;
        const int loadedFlags = Mix_Init(mixerFlags);
        LOGI("Mix_Init requested=0x%x loaded=0x%x", mixerFlags, loadedFlags);
        if ((loadedFlags & MIX_INIT_MP3) == 0)
        {
            LOGW("MP3 decoder unavailable in SDL_mixer runtime");
        }

        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
            LOGW("Mix_OpenAudio failed: %s", Mix_GetError());
        } else {
            Mix_AllocateChannels(32);
            LOGI("SDL_mixer initialized (32 channels)");
        }
    }

    // Text loaded by OpenTextData() -> GlobalText.Load(text_eng.bmd) in OpenBasicData()

    // â”€â”€ Init game data â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    LOGI("INIT: srand");
    srand((unsigned)time(nullptr));
    for (int& v : RandomTable) v = rand() % 360;

    LOGI("INIT: alloc GateAttribute");
    RendomMemoryDump        = new BYTE[rand() % 100 + 1];
    GateAttribute           = new GATE_ATTRIBUTE[MAX_GATES]{};
    LOGI("INIT: alloc SkillAttribute");
    SkillAttribute          = new SKILL_ATTRIBUTE[MAX_SKILLS]{};
    LOGI("INIT: alloc ItemAttRibute");
    ItemAttRibuteMemoryDump = new ITEM_ATTRIBUTE[MAX_ITEM + 1024]{};
    ItemAttribute           = ItemAttRibuteMemoryDump + rand() % 1024;
    LOGI("INIT: alloc CharacterMemoryDump");
    CharacterMemoryDump     = new CHARACTER[MAX_CHARACTERS_CLIENT + 1 + 128]{};
    CharactersClient        = CharacterMemoryDump + rand() % 128;
    LOGI("INIT: alloc CharacterMachine");
    CharacterMachine        = new CHARACTER_MACHINE;

    memset(GateAttribute,    0, sizeof(GATE_ATTRIBUTE)    * MAX_GATES);
    memset(ItemAttribute,    0, sizeof(ITEM_ATTRIBUTE)    * MAX_ITEM);
    memset(SkillAttribute,   0, sizeof(SKILL_ATTRIBUTE)   * MAX_SKILLS);
    memset(CharacterMachine, 0, sizeof(CHARACTER_MACHINE));

    LOGI("INIT: CharacterMachine->Init()");
    CharacterAttribute = &CharacterMachine->Character;
    CharacterMachine->Init();
    Hero = &CharactersClient[0];

    // â”€â”€ Init UI â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    LOGI("INIT: new UI objects");
    g_pMercenaryInputBox    = new CUIMercenaryInputBox;
    g_pSingleTextInputBox   = new CUITextInputBox;
    g_pSinglePasswdInputBox = new CUITextInputBox;
    g_pUIManager            = new CUIManager;
    g_pUIMapName            = new CUIMapName;

    if (g_iChatInputType == 1)
    {
        g_pMercenaryInputBox->Init(g_hWnd);
        g_pSingleTextInputBox->Init(g_hWnd, 200, 20);
        g_pSinglePasswdInputBox->Init(g_hWnd, 200, 20, 9, TRUE);
        g_pSingleTextInputBox->SetState(UISTATE_HIDE);
        g_pSinglePasswdInputBox->SetState(UISTATE_HIDE);
        g_pMercenaryInputBox->SetFont(g_hFont);
        g_pSingleTextInputBox->SetFont(g_hFont);
        g_pSinglePasswdInputBox->SetFont(g_hFont);
    }

    LOGI("INIT: BuffStateSystem::Make()");
    g_BuffSystem = BuffStateSystem::Make();
    LOGI("INIT: MapProcess::Make()");
    g_MapProcess = MapProcess::Make();
    LOGI("INIT: PetProcess::Make()");
    g_petProcess = PetProcess::Make();

    LOGI("INIT: CUIMng::Create()");
    CUIMng::Instance().Create();
    LOGI("INIT: g_pNewUISystem->Create()");
    g_pNewUISystem->Create();
    LOGI("INIT: UI creation done");
    LoadVirtualSkillSlots();
    {
        const std::string slotText = BuildVirtualSkillArrayString(g_virtualSkillSlots);
        LOGI("VirtualPad slots: %s", slotText.c_str());
    }

    // On emulator: tighten packet budget to free CPU for rendering.
    if (g_adaptivePerf.isEmulator)
    {
        g_adaptivePerf.defaultMessageBudget = 70;
        g_adaptivePerf.minMessageBudget     = 30;
    }

    // Keep effects ON globally. Adaptive may only trim spawn density under load.
    if (g_pOption)
    {
        (void)0;
    }

    (void)0;
    SetMaxMessagePerCycle(g_adaptivePerf.defaultMessageBudget);
    SetTargetFps(g_adaptivePerf.targetFps);  // -1 = uncapped
    LOGI(
        "Android perf defaults: maxMsgPerCycle=%d targetFps=%.1f fxScale=%.2f adaptive=on isEmulator=%d",
        g_MaxMessagePerCycle,
        g_adaptivePerf.targetFps,
        1.0f,
        g_adaptivePerf.isEmulator ? 1 : 0);

    // â”€â”€ Volume â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    if ((m_SoundOnOff || m_MusicOnOff) && g_pOption)
    {
        int vol = GameConfig::GetInstance().GetVolumeLevel();
        if (vol < 0 || vol > 10) vol = 10;
        g_pOption->SetVolumeLevel(vol);
        SetEffectVolumeLevel(vol);
        LOGI("SDL mixer volume set: level=%d", vol);
    }
    else if (m_MusicOnOff || m_SoundOnOff)
    {
        SetEffectVolumeLevel(10);
        LOGI("SDL mixer fallback volume applied: level=10");
    }

    // Keep FPS overlay visible for runtime performance measurement on device.
    (void)0;
    LOGI("FPS overlay enabled");

    LOGI("All systems initialized â€” entering main loop");

    // ==========================================================================
    // MAIN LOOP
    // Replaces WinMain's: while(PeekMessage) + WndProc + RenderScene(g_hDC)
    // ==========================================================================
    SDL_Event event;
    const Uint64 s_perfFreq = static_cast<Uint64>(MU_MobilePerfFrequency());
    Uint64 s_perfWindowStart = static_cast<Uint64>(MU_MobilePerfNow());
    int s_perfFrames = 0;
    int s_perfDrawCallsTotal = 0;
    int s_perfVertsTotal = 0;
    int s_perfDrawCallsMax = 0;
    int s_perfVertsMax = 0;
    int s_perfImDrawCallsTotal = 0;
    int s_perfVaDirectTotal = 0;
    int s_perfVaConvertedTotal = 0;
    int s_perfQuadIndexedTotal = 0;
    int s_perfQuadExpandedTotal = 0;
    int s_objMoveCandidatesTotal = 0;
    int s_objMoveUpdatedTotal = 0;
    int s_objMoveDeferredTotal = 0;
    int s_objMoveNearTotal = 0;
    int s_objMoveMidTotal = 0;
    int s_objMoveFarTotal = 0;
                int s_objRenderCandidatesTotal = 0;
                int s_objRenderRenderedTotal = 0;
                int s_objRenderCulledTotal = 0;
                int s_objRenderNearTotal = 0;
    int s_objRenderMidTotal = 0;
    int s_objRenderFarTotal = 0;
    int s_objVisualRenderedTotal = 0;
    int s_objVisualDeferredTotal = 0;
    int s_objRenderBaseCallsTotal = 0;
    int s_objVisualCallsTotal = 0;
    int s_objRenderAfterCallsTotal = 0;
    int s_charMoveCandidatesTotal = 0;
    int s_charMoveUpdatedTotal = 0;
    int s_charMoveDeferredTotal = 0;
    int s_charMoveNearTotal = 0;
    int s_charMoveMidTotal = 0;
    int s_charMoveFarTotal = 0;
    int s_charRenderCandidatesTotal = 0;
    int s_charRenderRenderedTotal = 0;
    int s_charRenderDeferredTotal = 0;
                int s_charRenderCulledTotal = 0;
                int s_charRenderNearTotal = 0;
                int s_charRenderMidTotal = 0;
                int s_charRenderFarTotal = 0;
    int s_charRenderPlayersTotal = 0;
    int s_charRenderMonstersTotal = 0;
    int s_charMonsterObjectCallsTotal = 0;
                int s_terrainNormalBlocksTotal = 0;
                int s_terrainNormalTilesTotal = 0;
    int s_terrainGrassBlocksTotal = 0;
    int s_terrainGrassTilesTotal = 0;
    int s_terrainAfterBlocksTotal = 0;
    int s_terrainAfterTilesTotal = 0;
    Uint64 s_perfRenderSceneTicksTotal = 0;
    Uint64 s_perfVirtualPadTicksTotal = 0;
    Uint64 s_perfPresentTicksTotal = 0;
    Uint64 s_objMoveTicksTotal = 0;
    Uint64 s_objRenderTicksTotal = 0;
    Uint64 s_objRenderBaseTicksTotal = 0;
    Uint64 s_objRenderVisualTicksTotal = 0;
    Uint64 s_objRenderAfterTicksTotal = 0;
    Uint64 s_charMoveTicksTotal = 0;
    Uint64 s_charRenderTicksTotal = 0;
    Uint64 s_charMonsterObjectTicksTotal = 0;
    Uint64 s_charRenderShadowTicksTotal = 0;
    Uint64 s_charRenderPostTicksTotal = 0;
    Uint64 s_charRenderAttachmentTicksTotal = 0;
    Uint64 s_terrainRenderTicksTotal = 0;
    Uint64 s_terrainAfterTicksTotal = 0;
    Uint64 s_mainShadowTicksTotal = 0;
    Uint64 s_mainBoidsTicksTotal = 0;
    Uint64 s_mainMiscWorldTicksTotal = 0;
    Uint64 s_mainJointsTicksTotal = 0;
    Uint64 s_mainEffectsTicksTotal = 0;
    Uint64 s_mainBlursTicksTotal = 0;
    Uint64 s_mainSpritesTicksTotal = 0;
    Uint64 s_mainParticlesTicksTotal = 0;
    Uint64 s_mainPointsTicksTotal = 0;
    Uint64 s_mainAfterEffectsTicksTotal = 0;
    Uint64 s_mainUiTicksTotal = 0;
    double s_cameraDistanceTotal = 0.0;
    double s_cameraDistanceTargetTotal = 0.0;
    double s_cameraOverrideTotal = 0.0;
    double s_cameraViewNearTotal = 0.0;
    double s_cameraViewFarTotal = 0.0;
    double s_cameraPitchTotal = 0.0;
    double s_cameraYawTotal = 0.0;
    float s_cameraDistanceMin = (std::numeric_limits<float>::max)();
    float s_cameraDistanceMax = (std::numeric_limits<float>::lowest)();
    float s_cameraTargetMin = (std::numeric_limits<float>::max)();
    float s_cameraTargetMax = (std::numeric_limits<float>::lowest)();
    float s_cameraFarMin = (std::numeric_limits<float>::max)();
    float s_cameraFarMax = (std::numeric_limits<float>::lowest)();
    float s_cameraPitchMin = (std::numeric_limits<float>::max)();
    float s_cameraPitchMax = (std::numeric_limits<float>::lowest)();
    float s_cameraYawMin = (std::numeric_limits<float>::max)();
    float s_cameraYawMax = (std::numeric_limits<float>::lowest)();
    int s_cameraTopViewFrames = 0;
    int s_cameraSettlingFrames = 0;
    uint32_t s_muHelperLastTickMs = MU_MobileGetTicks();
    uint32_t s_hackLastTickMs = MU_MobileGetTicks();

    while (!Destroy)
    {
        // Reset per-frame transient mouse state
        MouseLButtonDBClick = false;
        if (MouseLButtonPop &&
            (g_iMousePopPosition_x != MouseX || g_iMousePopPosition_y != MouseY))
            MouseLButtonPop = false;

        // Process all pending events
        while (SDL_PollEvent(&event))
        {
            HandleSDLEvent(event, screenW, screenH);
            if (Destroy) break;
        }
        if (Destroy) break;

        const bool hasFocusedTextInput = AndroidHasFocusedTextInput() || g_charNameInputActive;
#if !defined(MU_ANDROID_DISABLE_LOG)
        static int s_lastImeState = -1;
        const int imeState = hasFocusedTextInput ? 1 : 0;
        if (s_lastImeState != imeState)
        {
            LOGI(
                "CHATIME IME state changed focused=%d sdlTextInput=%d",
                imeState,
                SDL_IsTextInputActive() ? 1 : 0);
            s_lastImeState = imeState;
        }
#endif
        if (hasFocusedTextInput)
        {
            if (!SDL_IsTextInputActive())
            {
                SDL_StartTextInput();
            }
        }
        else if (SDL_IsTextInputActive())
        {
            SDL_StopTextInput();
        }

        AndroidDrainPackets();
        UpdateVirtualPadHolds();
        UpdateVirtualProximityCombat();

        // Windows builds drive MU Helper via Win32 SetTimer(MUHELPER_TIMER, 250ms).
        // On Android, SetTimer is a stub, so tick MU Helper from the main loop.
        const uint32_t nowTicks = MU_MobileGetTicks();
        int hackTickBudget = 4;
        while ((nowTicks - s_hackLastTickMs) >= 20000u && hackTickBudget-- > 0)
        {
            CheckHack();
            s_hackLastTickMs += 20000u;
        }

        int muHelperTickBudget = 4;
        while ((nowTicks - s_muHelperLastTickMs) >= 250u && muHelperTickBudget-- > 0)
        {
            (void)nowTicks;
            s_muHelperLastTickMs += 250u;
        }
        if (g_bWndActive)
        {
            static bool s_crfResult = false;
            s_crfResult = true;

            if (s_crfResult)
            {
                ProtocolCompiler();
                const Uint64 renderSceneStart = static_cast<Uint64>(MU_MobilePerfNow());
                Scene(nullptr);
                const Uint64 virtualPadStart = static_cast<Uint64>(MU_MobilePerfNow());
                RenderVirtualPad();
                const Uint64 presentStart = static_cast<Uint64>(MU_MobilePerfNow());
                if (g_RenderBackend)
                {
                    g_RenderBackend->Present(g_SDLWindow);
                }
                else
                {
                    SDL_GL_SwapWindow(g_SDLWindow);
                }
                const Uint64 presentEnd = static_cast<Uint64>(MU_MobilePerfNow());

                s_perfRenderSceneTicksTotal += virtualPadStart - renderSceneStart;
                s_perfVirtualPadTicksTotal += presentStart - virtualPadStart;
                s_perfPresentTicksTotal += presentEnd - presentStart;

                int frameDrawCalls = 0;
                int frameVerts = 0;
                int frameImDrawCalls = 0;
                int frameVaDirect = 0;
                int frameVaConverted = 0;
                int frameQuadIndexed = 0;
                int frameQuadExpanded = 0;
                static int s_perfHeartbeatFrameCounter = 0;
                if (g_RenderBackend)
                {
                    const RenderBackendStats stats = g_RenderBackend->GetAndResetStats();
                    frameDrawCalls = stats.drawCalls;
                    frameVerts = stats.vertices;
                    frameImDrawCalls = stats.imDrawCalls;
                    frameVaDirect = stats.vaDirectDrawCalls;
                    frameVaConverted = stats.vaConvertedDrawCalls;
                    frameQuadIndexed = stats.quadIndexedDrawCalls;
                    frameQuadExpanded = stats.quadExpandedDrawCalls;
                }
                else
                {
                    GL_GetDrawStats(&frameDrawCalls, &frameVerts);
                    GL_ResetDrawStats();
                }

                const ObjectPerfSnapshot objectPerf = ConsumeObjectPerfSnapshot();
                const CharacterPerfSnapshot characterPerf = ConsumeCharacterPerfSnapshot();
                const TerrainPerfSnapshot terrainPerf = ConsumeTerrainPerfSnapshot();
                const MainScenePerfSnapshot mainScenePerf = ConsumeMainScenePerfSnapshot();

                ++s_perfHeartbeatFrameCounter;
                if ((s_perfHeartbeatFrameCounter % 120) == 0)
                {
                    PERF_LOGI("HEARTBEAT frame=%d draw=%d verts=%d im=%d vaD=%d vaC=%d qI=%d qE=%d wndActive=%d scene=%d world=%d fpsAvg=%.1f",
                        s_perfHeartbeatFrameCounter,
                        frameDrawCalls,
                        frameVerts,
                        frameImDrawCalls,
                        frameVaDirect,
                        frameVaConverted,
                        frameQuadIndexed,
                        frameQuadExpanded,
                        g_bWndActive ? 1 : 0,
                        SceneFlag,
                        gMapManager.WorldActive,
                        FPS_AVG);
                }

                s_perfFrames++;
                s_perfDrawCallsTotal += frameDrawCalls;
                s_perfVertsTotal += frameVerts;
                s_perfImDrawCallsTotal += frameImDrawCalls;
                s_perfVaDirectTotal += frameVaDirect;
                s_perfVaConvertedTotal += frameVaConverted;
                s_perfQuadIndexedTotal += frameQuadIndexed;
                s_perfQuadExpandedTotal += frameQuadExpanded;
                s_objMoveCandidatesTotal += objectPerf.moveCandidates;
                s_objMoveUpdatedTotal += objectPerf.moveUpdated;
                s_objMoveDeferredTotal += objectPerf.moveDeferred;
                s_objMoveNearTotal += objectPerf.moveNearCandidates;
                s_objMoveMidTotal += objectPerf.moveMidCandidates;
                s_objMoveFarTotal += objectPerf.moveFarCandidates;
                s_objRenderCandidatesTotal += objectPerf.renderCandidates;
                s_objRenderRenderedTotal += objectPerf.renderRendered;
                s_objRenderCulledTotal += objectPerf.renderDistanceCulled;
                s_objRenderNearTotal += objectPerf.renderNearCandidates;
                s_objRenderMidTotal += objectPerf.renderMidCandidates;
                s_objRenderFarTotal += objectPerf.renderFarCandidates;
                s_objVisualRenderedTotal += objectPerf.visualRendered;
                s_objVisualDeferredTotal += objectPerf.visualDeferred;
                s_objRenderBaseCallsTotal += objectPerf.renderBaseCalls;
                s_objVisualCallsTotal += objectPerf.visualCalls;
                s_objRenderAfterCallsTotal += objectPerf.renderAfterCalls;
                s_objMoveTicksTotal += static_cast<Uint64>(objectPerf.moveTicks);
                s_objRenderTicksTotal += static_cast<Uint64>(objectPerf.renderTicks);
                s_objRenderBaseTicksTotal += static_cast<Uint64>(objectPerf.renderBaseTicks);
                s_objRenderVisualTicksTotal += static_cast<Uint64>(objectPerf.renderVisualTicks);
                s_objRenderAfterTicksTotal += static_cast<Uint64>(objectPerf.renderAfterTicks);
                s_charMoveCandidatesTotal += characterPerf.moveCandidates;
                s_charMoveUpdatedTotal += characterPerf.moveUpdated;
                s_charMoveDeferredTotal += characterPerf.moveDeferred;
                s_charMoveNearTotal += characterPerf.moveNearCandidates;
                s_charMoveMidTotal += characterPerf.moveMidCandidates;
                s_charMoveFarTotal += characterPerf.moveFarCandidates;
                s_charRenderCandidatesTotal += characterPerf.renderCandidates;
                s_charRenderRenderedTotal += characterPerf.renderRendered;
                s_charRenderDeferredTotal += characterPerf.renderDeferred;
                s_charRenderCulledTotal += characterPerf.renderDistanceCulled;
                s_charRenderNearTotal += characterPerf.renderNearCandidates;
                s_charRenderMidTotal += characterPerf.renderMidCandidates;
                s_charRenderFarTotal += characterPerf.renderFarCandidates;
                s_charRenderPlayersTotal += characterPerf.renderPlayers;
                s_charRenderMonstersTotal += characterPerf.renderMonsters;
                s_charMonsterObjectCallsTotal += characterPerf.renderMonsterObjectCalls;
                s_charMoveTicksTotal += static_cast<Uint64>(characterPerf.moveTicks);
                s_charRenderTicksTotal += static_cast<Uint64>(characterPerf.renderTicks);
                s_charMonsterObjectTicksTotal += static_cast<Uint64>(characterPerf.renderMonsterObjectTicks);
                s_charRenderShadowTicksTotal += static_cast<Uint64>(characterPerf.renderShadowTicks);
                s_charRenderPostTicksTotal += static_cast<Uint64>(characterPerf.renderPostTicks);
                s_charRenderAttachmentTicksTotal += static_cast<Uint64>(characterPerf.renderAttachmentTicks);
                s_terrainNormalBlocksTotal += terrainPerf.normalBlocks;
                s_terrainNormalTilesTotal += terrainPerf.normalTiles;
                s_terrainGrassBlocksTotal += terrainPerf.grassBlocks;
                s_terrainGrassTilesTotal += terrainPerf.grassTiles;
                s_terrainAfterBlocksTotal += terrainPerf.afterBlocks;
                s_terrainAfterTilesTotal += terrainPerf.afterTiles;
                s_terrainRenderTicksTotal += static_cast<Uint64>(terrainPerf.renderTicks);
                s_terrainAfterTicksTotal += static_cast<Uint64>(terrainPerf.afterTicks);
                s_mainShadowTicksTotal += static_cast<Uint64>(mainScenePerf.shadowTicks);
                s_mainBoidsTicksTotal += static_cast<Uint64>(mainScenePerf.boidsTicks);
                s_mainMiscWorldTicksTotal += static_cast<Uint64>(mainScenePerf.miscWorldTicks);
                s_mainJointsTicksTotal += static_cast<Uint64>(mainScenePerf.jointsTicks);
                s_mainEffectsTicksTotal += static_cast<Uint64>(mainScenePerf.effectsTicks);
                s_mainBlursTicksTotal += static_cast<Uint64>(mainScenePerf.blursTicks);
                s_mainSpritesTicksTotal += static_cast<Uint64>(mainScenePerf.spritesTicks);
                s_mainParticlesTicksTotal += static_cast<Uint64>(mainScenePerf.particlesTicks);
                s_mainPointsTicksTotal += static_cast<Uint64>(mainScenePerf.pointsTicks);
                s_mainAfterEffectsTicksTotal += static_cast<Uint64>(mainScenePerf.afterEffectsTicks);
                s_mainUiTicksTotal += static_cast<Uint64>(mainScenePerf.uiTicks);
                const float perfCameraDistance = CameraDistance;
                const float perfCameraTarget = CameraDistanceTarget;
                const float perfCameraOverride = g_androidZoomOverride;
                const float perfCameraNear = CameraViewNear;
                const float perfCameraFar = CameraViewFar;
                const float perfCameraPitch = CameraAngle[0];
                const float perfCameraYaw = CameraAngle[2];
                s_cameraDistanceTotal += perfCameraDistance;
                s_cameraDistanceTargetTotal += perfCameraTarget;
                s_cameraOverrideTotal += perfCameraOverride;
                s_cameraViewNearTotal += perfCameraNear;
                s_cameraViewFarTotal += perfCameraFar;
                s_cameraPitchTotal += perfCameraPitch;
                s_cameraYawTotal += perfCameraYaw;
                s_cameraDistanceMin = (std::min)(s_cameraDistanceMin, perfCameraDistance);
                s_cameraDistanceMax = (std::max)(s_cameraDistanceMax, perfCameraDistance);
                s_cameraTargetMin = (std::min)(s_cameraTargetMin, perfCameraTarget);
                s_cameraTargetMax = (std::max)(s_cameraTargetMax, perfCameraTarget);
                s_cameraFarMin = (std::min)(s_cameraFarMin, perfCameraFar);
                s_cameraFarMax = (std::max)(s_cameraFarMax, perfCameraFar);
                s_cameraPitchMin = (std::min)(s_cameraPitchMin, perfCameraPitch);
                s_cameraPitchMax = (std::max)(s_cameraPitchMax, perfCameraPitch);
                s_cameraYawMin = (std::min)(s_cameraYawMin, perfCameraYaw);
                s_cameraYawMax = (std::max)(s_cameraYawMax, perfCameraYaw);
                if (CameraTopViewEnable)
                {
                    ++s_cameraTopViewFrames;
                }
                if (std::fabs(perfCameraDistance - perfCameraTarget) > 8.0f)
                {
                    ++s_cameraSettlingFrames;
                }
                if (frameDrawCalls > s_perfDrawCallsMax) s_perfDrawCallsMax = frameDrawCalls;
                if (frameVerts > s_perfVertsMax) s_perfVertsMax = frameVerts;

                const Uint64 nowCounter = static_cast<Uint64>(MU_MobilePerfNow());
                const double elapsedSec = (double)(nowCounter - s_perfWindowStart) / (double)s_perfFreq;
                if (elapsedSec >= 0.25 && s_perfFrames > 0)
                {
                    const double fps = (double)s_perfFrames / elapsedSec;
                    const double avgFrameMs = (elapsedSec * 1000.0) / (double)s_perfFrames;
                    const int avgDrawCalls = s_perfDrawCallsTotal / s_perfFrames;
                    const int avgVerts = s_perfVertsTotal / s_perfFrames;
                    const int avgImDrawCalls = s_perfImDrawCallsTotal / s_perfFrames;
                    const int avgVaDirect = s_perfVaDirectTotal / s_perfFrames;
                    const int avgVaConverted = s_perfVaConvertedTotal / s_perfFrames;
                    const int avgQuadIndexed = s_perfQuadIndexedTotal / s_perfFrames;
                    const int avgQuadExpanded = s_perfQuadExpandedTotal / s_perfFrames;
                    const int avgObjMoveCandidates = s_objMoveCandidatesTotal / s_perfFrames;
                    const int avgObjMoveUpdated = s_objMoveUpdatedTotal / s_perfFrames;
                    const int avgObjMoveDeferred = s_objMoveDeferredTotal / s_perfFrames;
                    const int avgObjMoveNear = s_objMoveNearTotal / s_perfFrames;
                    const int avgObjMoveMid = s_objMoveMidTotal / s_perfFrames;
                    const int avgObjMoveFar = s_objMoveFarTotal / s_perfFrames;
                    const int avgObjRenderCandidates = s_objRenderCandidatesTotal / s_perfFrames;
                    const int avgObjRenderRendered = s_objRenderRenderedTotal / s_perfFrames;
                    const int avgObjRenderCulled = s_objRenderCulledTotal / s_perfFrames;
                    const int avgObjRenderNear = s_objRenderNearTotal / s_perfFrames;
                    const int avgObjRenderMid = s_objRenderMidTotal / s_perfFrames;
                    const int avgObjRenderFar = s_objRenderFarTotal / s_perfFrames;
                    const int avgObjVisualRendered = s_objVisualRenderedTotal / s_perfFrames;
                    const int avgObjVisualDeferred = s_objVisualDeferredTotal / s_perfFrames;
                    const int avgObjRenderBaseCalls = s_objRenderBaseCallsTotal / s_perfFrames;
                    const int avgObjVisualCalls = s_objVisualCallsTotal / s_perfFrames;
                    const int avgObjRenderAfterCalls = s_objRenderAfterCallsTotal / s_perfFrames;
                    const int avgCharMoveCandidates = s_charMoveCandidatesTotal / s_perfFrames;
                    const int avgCharMoveUpdated = s_charMoveUpdatedTotal / s_perfFrames;
                    const int avgCharMoveDeferred = s_charMoveDeferredTotal / s_perfFrames;
                    const int avgCharMoveNear = s_charMoveNearTotal / s_perfFrames;
                    const int avgCharMoveMid = s_charMoveMidTotal / s_perfFrames;
                    const int avgCharMoveFar = s_charMoveFarTotal / s_perfFrames;
                    const int avgCharRenderCandidates = s_charRenderCandidatesTotal / s_perfFrames;
                    const int avgCharRenderRendered = s_charRenderRenderedTotal / s_perfFrames;
                    const int avgCharRenderDeferred = s_charRenderDeferredTotal / s_perfFrames;
                    const int avgCharRenderCulled = s_charRenderCulledTotal / s_perfFrames;
                    const int avgCharRenderNear = s_charRenderNearTotal / s_perfFrames;
                    const int avgCharRenderMid = s_charRenderMidTotal / s_perfFrames;
                    const int avgCharRenderFar = s_charRenderFarTotal / s_perfFrames;
                    const int avgCharRenderPlayers = s_charRenderPlayersTotal / s_perfFrames;
                    const int avgCharRenderMonsters = s_charRenderMonstersTotal / s_perfFrames;
                    const int avgCharMonsterObjectCalls = s_charMonsterObjectCallsTotal / s_perfFrames;
                    const int avgTerrainNormalBlocks = s_terrainNormalBlocksTotal / s_perfFrames;
                    const int avgTerrainNormalTiles = s_terrainNormalTilesTotal / s_perfFrames;
                    const int avgTerrainGrassBlocks = s_terrainGrassBlocksTotal / s_perfFrames;
                    const int avgTerrainGrassTiles = s_terrainGrassTilesTotal / s_perfFrames;
                    const int avgTerrainAfterBlocks = s_terrainAfterBlocksTotal / s_perfFrames;
                    const int avgTerrainAfterTiles = s_terrainAfterTilesTotal / s_perfFrames;
                    const double tickToMs = 1000.0 / static_cast<double>(s_perfFreq);
                    const double avgRenderSceneMs =
                        (static_cast<double>(s_perfRenderSceneTicksTotal) * tickToMs) / static_cast<double>(s_perfFrames);
                    const double avgVirtualPadMs =
                        (static_cast<double>(s_perfVirtualPadTicksTotal) * tickToMs) / static_cast<double>(s_perfFrames);
                    const double avgPresentMs =
                        (static_cast<double>(s_perfPresentTicksTotal) * tickToMs) / static_cast<double>(s_perfFrames);
                    const double avgObjMoveMs =
                        (static_cast<double>(s_objMoveTicksTotal) * tickToMs) / static_cast<double>(s_perfFrames);
                    const double avgObjRenderMs =
                        (static_cast<double>(s_objRenderTicksTotal) * tickToMs) / static_cast<double>(s_perfFrames);
                    const double avgObjRenderBaseMs =
                        (static_cast<double>(s_objRenderBaseTicksTotal) * tickToMs) / static_cast<double>(s_perfFrames);
                    const double avgObjRenderVisualMs =
                        (static_cast<double>(s_objRenderVisualTicksTotal) * tickToMs) / static_cast<double>(s_perfFrames);
                    const double avgObjRenderAfterMs =
                        (static_cast<double>(s_objRenderAfterTicksTotal) * tickToMs) / static_cast<double>(s_perfFrames);
                    const double avgCharMoveMs =
                        (static_cast<double>(s_charMoveTicksTotal) * tickToMs) / static_cast<double>(s_perfFrames);
                    const double avgCharRenderMs =
                        (static_cast<double>(s_charRenderTicksTotal) * tickToMs) / static_cast<double>(s_perfFrames);
                    const double avgCharMonsterObjectMs =
                        (static_cast<double>(s_charMonsterObjectTicksTotal) * tickToMs) / static_cast<double>(s_perfFrames);
                    const double avgCharRenderShadowMs =
                        (static_cast<double>(s_charRenderShadowTicksTotal) * tickToMs) / static_cast<double>(s_perfFrames);
                    const double avgCharRenderPostMs =
                        (static_cast<double>(s_charRenderPostTicksTotal) * tickToMs) / static_cast<double>(s_perfFrames);
                    const double avgCharRenderAttachmentMs =
                        (static_cast<double>(s_charRenderAttachmentTicksTotal) * tickToMs) / static_cast<double>(s_perfFrames);
                    const double avgTerrainRenderMs =
                        (static_cast<double>(s_terrainRenderTicksTotal) * tickToMs) / static_cast<double>(s_perfFrames);
                    const double avgTerrainAfterMs =
                        (static_cast<double>(s_terrainAfterTicksTotal) * tickToMs) / static_cast<double>(s_perfFrames);
                    const double avgMainShadowMs =
                        (static_cast<double>(s_mainShadowTicksTotal) * tickToMs) / static_cast<double>(s_perfFrames);
                    const double avgMainBoidsMs =
                        (static_cast<double>(s_mainBoidsTicksTotal) * tickToMs) / static_cast<double>(s_perfFrames);
                    const double avgMainMiscWorldMs =
                        (static_cast<double>(s_mainMiscWorldTicksTotal) * tickToMs) / static_cast<double>(s_perfFrames);
                    const double avgMainJointsMs =
                        (static_cast<double>(s_mainJointsTicksTotal) * tickToMs) / static_cast<double>(s_perfFrames);
                    const double avgMainEffectsMs =
                        (static_cast<double>(s_mainEffectsTicksTotal) * tickToMs) / static_cast<double>(s_perfFrames);
                    const double avgMainBlursMs =
                        (static_cast<double>(s_mainBlursTicksTotal) * tickToMs) / static_cast<double>(s_perfFrames);
                    const double avgMainSpritesMs =
                        (static_cast<double>(s_mainSpritesTicksTotal) * tickToMs) / static_cast<double>(s_perfFrames);
                    const double avgMainParticlesMs =
                        (static_cast<double>(s_mainParticlesTicksTotal) * tickToMs) / static_cast<double>(s_perfFrames);
                    const double avgMainPointsMs =
                        (static_cast<double>(s_mainPointsTicksTotal) * tickToMs) / static_cast<double>(s_perfFrames);
                    const double avgMainAfterEffectsMs =
                        (static_cast<double>(s_mainAfterEffectsTicksTotal) * tickToMs) / static_cast<double>(s_perfFrames);
                    const double avgMainUiMs =
                        (static_cast<double>(s_mainUiTicksTotal) * tickToMs) / static_cast<double>(s_perfFrames);
                    const double avgCameraDistance =
                        s_cameraDistanceTotal / static_cast<double>(s_perfFrames);
                    const double avgCameraTarget =
                        s_cameraDistanceTargetTotal / static_cast<double>(s_perfFrames);
                    const double avgCameraOverride =
                        s_cameraOverrideTotal / static_cast<double>(s_perfFrames);
                    const double avgCameraNear =
                        s_cameraViewNearTotal / static_cast<double>(s_perfFrames);
                    const double avgCameraFar =
                        s_cameraViewFarTotal / static_cast<double>(s_perfFrames);
                    const double avgCameraPitch =
                        s_cameraPitchTotal / static_cast<double>(s_perfFrames);
                    const double avgCameraYaw =
                        s_cameraYawTotal / static_cast<double>(s_perfFrames);
                    const double avgTrackedSceneMs =
                        avgObjMoveMs + avgObjRenderMs +
                        avgCharMoveMs + avgCharRenderMs +
                        avgTerrainRenderMs + avgTerrainAfterMs;
                    const double avgOtherSceneMs =
                        (avgRenderSceneMs > avgTrackedSceneMs) ? (avgRenderSceneMs - avgTrackedSceneMs) : 0.0;
                    const double avgObjRenderLoopMs =
                        (avgObjRenderMs > (avgObjRenderBaseMs + avgObjRenderVisualMs + avgObjRenderAfterMs))
                            ? (avgObjRenderMs - (avgObjRenderBaseMs + avgObjRenderVisualMs + avgObjRenderAfterMs))
                            : 0.0;
                    const double avgCharRenderOtherMs =
                        (avgCharRenderMs > (avgCharMonsterObjectMs + avgCharRenderShadowMs + avgCharRenderPostMs + avgCharRenderAttachmentMs))
                            ? (avgCharRenderMs - (avgCharMonsterObjectMs + avgCharRenderShadowMs + avgCharRenderPostMs + avgCharRenderAttachmentMs))
                            : 0.0;
                    const double avgPhaseTrackedMs =
                        avgMainShadowMs + avgMainBoidsMs + avgMainMiscWorldMs +
                        avgMainJointsMs + avgMainEffectsMs + avgMainBlursMs +
                        avgMainSpritesMs + avgMainParticlesMs + avgMainPointsMs +
                        avgMainAfterEffectsMs + avgMainUiMs;
                    const double avgPhaseRemainderMs =
                        (avgOtherSceneMs > avgPhaseTrackedMs) ? (avgOtherSceneMs - avgPhaseTrackedMs) : 0.0;
                    UpdateAdaptivePerformance(fps, avgFrameMs);
                    static int s_perfLogWindowCounter = 0;
                    if ((s_perfLogWindowCounter++ % 4) == 0)
                    {
                        const SceneCrowdSnapshot crowdSnapshot = CaptureSceneCrowdSnapshot();
                        PERF_LOGI(
                            "PERF fps=%.1f frameMs=%.2f phase(render=%.2f pad=%.2f present=%.2f) renderLevel=%d fxScale=%.2f maxMsg=%d avg(draw=%d verts=%d im=%d vaD=%d vaC=%d qI=%d qE=%d) max(draw=%d verts=%d)",
                            fps,
                            avgFrameMs,
                            avgRenderSceneMs,
                            avgVirtualPadMs,
                            avgPresentMs,
                            g_pOption ? ClampRenderLevel(g_pOption->GetRenderLevel()) : -1,
                            GetAdaptiveEffectSpawnScale(),
                            g_MaxMessagePerCycle,
                            avgDrawCalls,
                            avgVerts,
                            avgImDrawCalls,
                            avgVaDirect,
                            avgVaConverted,
                            avgQuadIndexed,
                            avgQuadExpanded,
                            s_perfDrawCallsMax,
                            s_perfVertsMax);

                        PERF_LOGI(
                            "PERF_SCENE world=%d char(live=%d vis=%d ply=%d mon=%d npc=%d pet=%d prio=%d topMon=%d x%d) obj(live=%d vis=%d op=%d trap=%d topObj=%d x%d)",
                            gMapManager.WorldActive,
                            crowdSnapshot.liveCharacters,
                            crowdSnapshot.visibleCharacters,
                            crowdSnapshot.visiblePlayers,
                            crowdSnapshot.visibleMonsters,
                            crowdSnapshot.visibleNpcs,
                            crowdSnapshot.visiblePets,
                            crowdSnapshot.visiblePriorityCharacters,
                            crowdSnapshot.dominantMonsterType,
                            crowdSnapshot.dominantMonsterCount,
                            crowdSnapshot.liveWorldObjects,
                            crowdSnapshot.visibleWorldObjects,
                            crowdSnapshot.visibleOperateObjects,
                            crowdSnapshot.visibleTrapObjects,
                            crowdSnapshot.dominantObjectType,
                            crowdSnapshot.dominantObjectCount);

                        PERF_LOGI(
                            "PERF_BUCKET objMove(cand=%d upd=%d def=%d n=%d m=%d f=%d) objRender(cand=%d draw=%d cull=%d n=%d m=%d f=%d visDraw=%d visDef=%d) charMove(cand=%d upd=%d def=%d n=%d m=%d f=%d) charRender(cand=%d draw=%d def=%d cull=%d n=%d m=%d f=%d)",
                            avgObjMoveCandidates,
                            avgObjMoveUpdated,
                            avgObjMoveDeferred,
                            avgObjMoveNear,
                            avgObjMoveMid,
                            avgObjMoveFar,
                            avgObjRenderCandidates,
                            avgObjRenderRendered,
                            avgObjRenderCulled,
                            avgObjRenderNear,
                            avgObjRenderMid,
                            avgObjRenderFar,
                            avgObjVisualRendered,
                            avgObjVisualDeferred,
                            avgCharMoveCandidates,
                            avgCharMoveUpdated,
                            avgCharMoveDeferred,
                            avgCharMoveNear,
                            avgCharMoveMid,
                            avgCharMoveFar,
                            avgCharRenderCandidates,
                            avgCharRenderRendered,
                            avgCharRenderDeferred,
                            avgCharRenderCulled,
                            avgCharRenderNear,
                            avgCharRenderMid,
                            avgCharRenderFar);

                        PERF_LOGI(
                            "PERF_CPU terrain=%.2f after=%.2f obj(move=%.2f render=%.2f) char(move=%.2f render=%.2f) other=%.2f",
                            avgTerrainRenderMs,
                            avgTerrainAfterMs,
                            avgObjMoveMs,
                            avgObjRenderMs,
                            avgCharMoveMs,
                            avgCharRenderMs,
                            avgOtherSceneMs);

                        PERF_LOGI(
                            "PERF_DETAIL obj(base=%.2f visual=%.2f after=%.2f loop=%.2f calls=%d/%d/%d) char(monObj=%.2f shadow=%.2f post=%.2f attach=%.2f other=%.2f draw=ply:%d mon:%d monObj:%d)",
                            avgObjRenderBaseMs,
                            avgObjRenderVisualMs,
                            avgObjRenderAfterMs,
                            avgObjRenderLoopMs,
                            avgObjRenderBaseCalls,
                            avgObjVisualCalls,
                            avgObjRenderAfterCalls,
                            avgCharMonsterObjectMs,
                            avgCharRenderShadowMs,
                            avgCharRenderPostMs,
                            avgCharRenderAttachmentMs,
                            avgCharRenderOtherMs,
                            avgCharRenderPlayers,
                            avgCharRenderMonsters,
                            avgCharMonsterObjectCalls);

                        PERF_LOGI(
                            "PERF_PHASE shadow=%.2f boids=%.2f misc=%.2f joints=%.2f effects=%.2f blurs=%.2f sprites=%.2f particles=%.2f points=%.2f afterfx=%.2f ui=%.2f rem=%.2f",
                            avgMainShadowMs,
                            avgMainBoidsMs,
                            avgMainMiscWorldMs,
                            avgMainJointsMs,
                            avgMainEffectsMs,
                            avgMainBlursMs,
                            avgMainSpritesMs,
                            avgMainParticlesMs,
                            avgMainPointsMs,
                            avgMainAfterEffectsMs,
                            avgMainUiMs,
                            avgPhaseRemainderMs);

                        PERF_LOGI(
                            "PERF_CAM zoom(cur=%.0f min=%.0f max=%.0f tgt=%.0f tmin=%.0f tmax=%.0f ov=%.0f) clip(near=%.0f far=%.0f fmin=%.0f fmax=%.0f) ang(p=%.1f pmin=%.1f pmax=%.1f y=%.1f ymin=%.1f ymax=%.1f) top=%d/%d settling=%d/%d",
                            avgCameraDistance,
                            s_cameraDistanceMin,
                            s_cameraDistanceMax,
                            avgCameraTarget,
                            s_cameraTargetMin,
                            s_cameraTargetMax,
                            avgCameraOverride,
                            avgCameraNear,
                            avgCameraFar,
                            s_cameraFarMin,
                            s_cameraFarMax,
                            avgCameraPitch,
                            s_cameraPitchMin,
                            s_cameraPitchMax,
                            avgCameraYaw,
                            s_cameraYawMin,
                            s_cameraYawMax,
                            s_cameraTopViewFrames,
                            s_perfFrames,
                            s_cameraSettlingFrames,
                            s_perfFrames);

                        PERF_LOGI(
                            "PERF_TERRAIN world=%d normal(block=%d tile=%d) grass(block=%d tile=%d) after(block=%d tile=%d)",
                            gMapManager.WorldActive,
                            avgTerrainNormalBlocks,
                            avgTerrainNormalTiles,
                            avgTerrainGrassBlocks,
                            avgTerrainGrassTiles,
                            avgTerrainAfterBlocks,
                            avgTerrainAfterTiles);
                    }
                    s_perfWindowStart = nowCounter;
                    s_perfFrames = 0;
                    s_perfDrawCallsTotal = 0;
                    s_perfVertsTotal = 0;
                    s_perfDrawCallsMax = 0;
                    s_perfVertsMax = 0;
                    s_perfImDrawCallsTotal = 0;
                    s_perfVaDirectTotal = 0;
                    s_perfVaConvertedTotal = 0;
                    s_perfQuadIndexedTotal = 0;
                    s_perfQuadExpandedTotal = 0;
                    s_objMoveCandidatesTotal = 0;
                    s_objMoveUpdatedTotal = 0;
                    s_objMoveDeferredTotal = 0;
                    s_objMoveNearTotal = 0;
                    s_objMoveMidTotal = 0;
                    s_objMoveFarTotal = 0;
                    s_objRenderCandidatesTotal = 0;
                    s_objRenderRenderedTotal = 0;
                    s_objRenderCulledTotal = 0;
                    s_objRenderNearTotal = 0;
                    s_objRenderMidTotal = 0;
                    s_objRenderFarTotal = 0;
                    s_objVisualRenderedTotal = 0;
                    s_objVisualDeferredTotal = 0;
                    s_objRenderBaseCallsTotal = 0;
                    s_objVisualCallsTotal = 0;
                    s_objRenderAfterCallsTotal = 0;
                    s_charMoveCandidatesTotal = 0;
                    s_charMoveUpdatedTotal = 0;
                    s_charMoveDeferredTotal = 0;
                    s_charMoveNearTotal = 0;
                    s_charMoveMidTotal = 0;
                    s_charMoveFarTotal = 0;
                    s_charRenderCandidatesTotal = 0;
                    s_charRenderRenderedTotal = 0;
                    s_charRenderDeferredTotal = 0;
                    s_charRenderCulledTotal = 0;
                    s_charRenderNearTotal = 0;
                    s_charRenderMidTotal = 0;
                    s_charRenderFarTotal = 0;
                    s_charRenderPlayersTotal = 0;
                    s_charRenderMonstersTotal = 0;
                    s_charMonsterObjectCallsTotal = 0;
                    s_terrainNormalBlocksTotal = 0;
                    s_terrainNormalTilesTotal = 0;
                    s_terrainGrassBlocksTotal = 0;
                    s_terrainGrassTilesTotal = 0;
                    s_terrainAfterBlocksTotal = 0;
                    s_terrainAfterTilesTotal = 0;
                    s_objMoveTicksTotal = 0;
                    s_objRenderTicksTotal = 0;
                    s_objRenderBaseTicksTotal = 0;
                    s_objRenderVisualTicksTotal = 0;
                    s_objRenderAfterTicksTotal = 0;
                    s_charMoveTicksTotal = 0;
                    s_charRenderTicksTotal = 0;
                    s_charMonsterObjectTicksTotal = 0;
                    s_charRenderShadowTicksTotal = 0;
                    s_charRenderPostTicksTotal = 0;
                    s_charRenderAttachmentTicksTotal = 0;
                    s_terrainRenderTicksTotal = 0;
                    s_terrainAfterTicksTotal = 0;
                    s_mainShadowTicksTotal = 0;
                    s_mainBoidsTicksTotal = 0;
                    s_mainMiscWorldTicksTotal = 0;
                    s_mainJointsTicksTotal = 0;
                    s_mainEffectsTicksTotal = 0;
                    s_mainBlursTicksTotal = 0;
                    s_mainSpritesTicksTotal = 0;
                    s_mainParticlesTicksTotal = 0;
                    s_mainPointsTicksTotal = 0;
                    s_mainAfterEffectsTicksTotal = 0;
                    s_mainUiTicksTotal = 0;
                    s_cameraDistanceTotal = 0.0;
                    s_cameraDistanceTargetTotal = 0.0;
                    s_cameraOverrideTotal = 0.0;
                    s_cameraViewNearTotal = 0.0;
                    s_cameraViewFarTotal = 0.0;
                    s_cameraPitchTotal = 0.0;
                    s_cameraYawTotal = 0.0;
                    s_cameraDistanceMin = (std::numeric_limits<float>::max)();
                    s_cameraDistanceMax = (std::numeric_limits<float>::lowest)();
                    s_cameraTargetMin = (std::numeric_limits<float>::max)();
                    s_cameraTargetMax = (std::numeric_limits<float>::lowest)();
                    s_cameraFarMin = (std::numeric_limits<float>::max)();
                    s_cameraFarMax = (std::numeric_limits<float>::lowest)();
                    s_cameraPitchMin = (std::numeric_limits<float>::max)();
                    s_cameraPitchMax = (std::numeric_limits<float>::lowest)();
                    s_cameraYawMin = (std::numeric_limits<float>::max)();
                    s_cameraYawMax = (std::numeric_limits<float>::lowest)();
                    s_cameraTopViewFrames = 0;
                    s_cameraSettlingFrames = 0;
                    s_perfRenderSceneTicksTotal = 0;
                    s_perfVirtualPadTicksTotal = 0;
                    s_perfPresentTicksTotal = 0;
                }

                // Debug: log first few frames to verify render loop runs
                static int s_dbgFrameCount = 0;
                if (s_dbgFrameCount < 10) {
                    const char* err = SDL_GetError();
                    LOGI("Frame %d â€” drawCalls=%d verts=%d paths(im=%d vaD=%d vaC=%d qI=%d qE=%d) SDL_err='%s'",
                         s_dbgFrameCount,
                         frameDrawCalls,
                         frameVerts,
                         frameImDrawCalls,
                         frameVaDirect,
                         frameVaConverted,
                         frameQuadIndexed,
                         frameQuadExpanded,
                         err ? err : "");
                    SDL_ClearError();
                    ++s_dbgFrameCount;
                }
            }
            else
            {
                MU_MobileSleep(1);   // yield â€” avoid busy loop
            }
        }
        else
        {
            MU_MobileSleep(16);  // minimize CPU when inactive
        }

        ProtocolCompiler();

        // Reset push states AFTER render so they're valid for one full frame
        MouseLButtonPush = false;
        MouseRButtonPush = false;
        MouseMButtonPush = false;
        MouseWheel       = 0;
    }

    LOGI("Main loop exited â€” cleaning up");
    SaveVirtualSkillSlots();

    // â”€â”€ Cleanup â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    if (g_RenderBackend)
    {
        g_RenderBackend->Shutdown(g_SDLWindow);
        g_RenderBackend.reset();
    }

    DestroyWindow_Android();
    DestroySound();
    KillGLWindow();
    SDL_Quit();

    LOGI("=== MuMain shutdown complete ===");
    return 0;
}
#endif

sapp_desc sokol_main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    sapp_desc desc {};
    desc.init_cb = OnAndroidSappInit;
    desc.frame_cb = OnAndroidSappFrame;
    desc.cleanup_cb = OnAndroidSappCleanup;
    desc.event_cb = OnAndroidSappEvent;
    desc.width = 1280;
    desc.height = 720;
    desc.fullscreen = true;
    desc.high_dpi = true;
    desc.window_title = "MU Online";
    desc.swap_interval = 0;
    desc.gl.major_version = 3;
    desc.gl.minor_version = 1;
    return desc;
}

static bool IsHealingHotKeyItemType(int) { return false; }
static bool IsManaHotKeyItemType(int) { return false; }
static bool IsShieldHotKeyItemType(int) { return false; }
static bool IsComplexHotKeyItemType(int) { return false; }
static bool IsSameHotKeyItemFamily(int lhsType, int rhsType)
{
    if (lhsType < 0 || rhsType < 0)
    {
        return false;
    }

    if (lhsType == rhsType)
    {
        return true;
    }

    return (IsHealingHotKeyItemType(lhsType) && IsHealingHotKeyItemType(rhsType))
        || (IsManaHotKeyItemType(lhsType) && IsManaHotKeyItemType(rhsType))
        || (IsShieldHotKeyItemType(lhsType) && IsShieldHotKeyItemType(rhsType))
        || (IsComplexHotKeyItemType(lhsType) && IsComplexHotKeyItemType(rhsType));
}

static std::array<int, SEASON3B::HOTKEY_COUNT> GetAutoBindHotKeyOrder(int itemType)
{
    if (IsManaHotKeyItemType(itemType))
    {
        return { SEASON3B::HOTKEY_W, SEASON3B::HOTKEY_Q, SEASON3B::HOTKEY_E, SEASON3B::HOTKEY_R };
    }

    if (IsShieldHotKeyItemType(itemType))
    {
        return { SEASON3B::HOTKEY_R, SEASON3B::HOTKEY_E, SEASON3B::HOTKEY_W, SEASON3B::HOTKEY_Q };
    }

    if (IsHealingHotKeyItemType(itemType))
    {
        return { SEASON3B::HOTKEY_Q, SEASON3B::HOTKEY_W, SEASON3B::HOTKEY_E, SEASON3B::HOTKEY_R };
    }

    return { SEASON3B::HOTKEY_E, SEASON3B::HOTKEY_R, SEASON3B::HOTKEY_Q, SEASON3B::HOTKEY_W };
}

static int FindBestAutoBindHotKeySlot(int itemType, int itemLevel)
{
    if (g_pMainFrame == nullptr)
    {
        return -1;
    }

    const auto order = GetAutoBindHotKeyOrder(itemType);

    for (const int hotKey : order)
    {
        if (g_pMainFrame->GetItemHotKey(hotKey) == itemType
            && g_pMainFrame->GetItemHotKeyLevel(hotKey) == itemLevel)
        {
            return hotKey;
        }
    }

    for (const int hotKey : order)
    {
        if (g_pMainFrame->GetItemHotKey(hotKey) < 0)
        {
            return hotKey;
        }
    }

    for (const int hotKey : order)
    {
        if (IsSameHotKeyItemFamily(g_pMainFrame->GetItemHotKey(hotKey), itemType))
        {
            return hotKey;
        }
    }

    return order.front();
}

// â”€â”€ Consumable potion slot binding â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
// Called by NewUIMyInventory.cpp when the player taps a consumable item in
// the inventory on mobile.  Bind directly to the legacy MU Q/W/E/R bar so
// the old mainframe UI stays authoritative for both rendering and use.
bool AndroidBindVirtualPotionSlotFromInventory(int itemType, int itemLevel)
{
    if (g_pMainFrame == nullptr
        || !SEASON3B::CNewUIMyInventory::CanRegisterItemHotKey(itemType))
    {
        return false;
    }

    const int hotKey = FindBestAutoBindHotKeySlot(itemType, itemLevel);
    if (hotKey < 0)
    {
        return false;
    }

    g_pMainFrame->SetItemHotKey(hotKey, itemType, itemLevel);
    LOGI("Android auto-bind consumable type=%d level=%d -> hotkey=%d", itemType, itemLevel, hotKey);
    return true;
}

float GetAdaptiveEffectSpawnScale()
{
    return std::clamp(g_currentAdaptiveEffectScale, 0.35f, 1.0f);
}

bool ShouldThrottleAdaptiveEffectSpawn(int kind, int type, vec3_t Position, int SubType, float Scale, OBJECT* Owner)
{
    (void)type;

    if (!g_adaptivePerf.enabled)
    {
        return false;
    }

    const float currentScale = GetAdaptiveEffectSpawnScale();
    if (currentScale >= 0.995f)
    {
        return false;
    }

    if (Hero == nullptr || Hero->Object.Live == false)
    {
        return false;
    }

    if (Owner == &Hero->Object)
    {
        return false;
    }

    if (Owner != nullptr && IsPersistentAmbientFireObject(Owner))
    {
        return false;
    }

    const float dx = Position[0] - Hero->Object.Position[0];
    const float dy = Position[1] - Hero->Object.Position[1];
    const float distanceSq = (dx * dx) + (dy * dy);
    if (distanceSq <= (420.0f * 420.0f))
    {
        return false;
    }

    float keepScale = currentScale;
    if (kind == ADAPTIVE_EFFECT_SPRITE)
    {
        keepScale = std::max(keepScale, 0.80f);
    }
    else if (kind == ADAPTIVE_EFFECT_PARTICLE)
    {
        keepScale = std::max(keepScale, 0.68f);
    }

    if (Scale >= 1.35f)
    {
        keepScale = std::min(1.0f, keepScale + 0.10f);
    }

    if (distanceSq > (900.0f * 900.0f))
    {
        keepScale -= 0.15f;
    }
    else if (distanceSq > (700.0f * 700.0f))
    {
        keepScale -= 0.08f;
    }

    if (SubType == 0 && kind == ADAPTIVE_EFFECT_SPRITE)
    {
        keepScale = std::min(1.0f, keepScale + 0.05f);
    }

    keepScale = std::clamp(keepScale, 0.55f, 1.0f);
    if (keepScale >= 0.995f)
    {
        return false;
    }

    static uint32_t s_adaptiveSpawnCounter = 0;
    const uint32_t counter = ++s_adaptiveSpawnCounter;

    uint32_t hash = 2166136261u;
    const auto mixHash = [&hash](uint32_t value)
    {
        hash ^= value;
        hash *= 16777619u;
    };

    mixHash(static_cast<uint32_t>(kind));
    mixHash(static_cast<uint32_t>(type & 0xFFFF));
    mixHash(static_cast<uint32_t>(SubType & 0xFFFF));
    mixHash(static_cast<uint32_t>(counter * 2246822519u));
    mixHash(static_cast<uint32_t>(static_cast<int>(Position[0] * 0.1f)) & 0xFFFFu);
    mixHash(static_cast<uint32_t>(static_cast<int>(Position[1] * 0.1f)) & 0xFFFFu);
    if (Owner != nullptr)
    {
        mixHash(static_cast<uint32_t>(Owner->Type & 0xFFFF));
    }

    const uint32_t keepThreshold = static_cast<uint32_t>(keepScale * 1000.0f);
    return (hash % 1000u) >= keepThreshold;
}

#endif // __ANDROID__


