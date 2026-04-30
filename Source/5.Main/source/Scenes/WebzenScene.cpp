///////////////////////////////////////////////////////////////////////////////
// WebzenScene.cpp - Webzen title/intro scene implementation
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WebzenScene.h"
#include "SceneCore.h"
#include "../ZzzOpenglUtil.h"
#include "../ZzzTexture.h"
#include "../ZzzInterface.h"
#include "../ZzzOpenData.h"
#include "../UIMng.h"
#include "../NewUISystem.h"
#if defined(__ANDROID__) || defined(MU_IOS)
#include <android/log.h>
#endif

// External declarations
extern EGameScene SceneFlag;
extern CErrorReport g_ErrorReport;

#if defined(__ANDROID__) || defined(MU_IOS)
#define WEBZEN_LOGI(...) __android_log_print(ANDROID_LOG_INFO, "WebzenScene", __VA_ARGS__)
#define WEBZEN_LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "WebzenScene", __VA_ARGS__)
#else
#define WEBZEN_LOGI(...) ((void)0)
#define WEBZEN_LOGE(...) ((void)0)
#endif

// Constants for background image selection
constexpr int BACKGROUND_SELECTION_PERCENTAGE = 100;
constexpr int CLASSIC_BACKGROUND_PROBABILITY = 70;

// Bitmap indices for title scene resources
constexpr int TITLE_BITMAP_BASE = BITMAP_TITLE;
constexpr int TITLE_BITMAP_BACK_02 = BITMAP_TITLE + 1;
constexpr int TITLE_BITMAP_LOGO = BITMAP_TITLE + 3;
constexpr int TITLE_BITMAP_PATTERN = BITMAP_TITLE + 5;
constexpr int TITLE_BITMAP_DYNAMIC_START = BITMAP_TITLE + 6;
constexpr int TITLE_BITMAP_DYNAMIC_END = BITMAP_TITLE + 14;

enum class BackgroundTheme
{
    Classic,
    Season5
};

/**
 * @brief Determines which background theme to use based on configured probability.
 * @return BackgroundTheme::Classic or BackgroundTheme::Season5
 */
static BackgroundTheme SelectBackgroundTheme()
{
    int roll = rand() % BACKGROUND_SELECTION_PERCENTAGE;
    return (roll <= CLASSIC_BACKGROUND_PROBABILITY) ? BackgroundTheme::Classic : BackgroundTheme::Season5;
}

/**
 * @brief Loads the appropriate background image set based on theme.
 * @param theme The background theme to load (Classic or Season5)
 */
static void LoadBackgroundTheme(BackgroundTheme theme)
{
    if (theme == BackgroundTheme::Classic)
    {
        LoadBitmap(L"Interface\\lo_back_im01.jpg", BITMAP_TITLE + 8, GL_LINEAR);
        LoadBitmap(L"Interface\\lo_back_im02.jpg", BITMAP_TITLE + 9, GL_LINEAR);
        LoadBitmap(L"Interface\\lo_back_im03.jpg", BITMAP_TITLE + 10, GL_LINEAR);
        LoadBitmap(L"Interface\\lo_back_im04.jpg", BITMAP_TITLE + 11, GL_LINEAR);
        LoadBitmap(L"Interface\\lo_back_im05.jpg", BITMAP_TITLE + 12, GL_LINEAR);
        LoadBitmap(L"Interface\\lo_back_im06.jpg", BITMAP_TITLE + 13, GL_LINEAR);
    }
    else
    {
        LoadBitmap(L"Interface\\lo_back_s5_im01.jpg", BITMAP_TITLE + 8, GL_LINEAR);
        LoadBitmap(L"Interface\\lo_back_s5_im02.jpg", BITMAP_TITLE + 9, GL_LINEAR);
        LoadBitmap(L"Interface\\lo_back_s5_im03.jpg", BITMAP_TITLE + 10, GL_LINEAR);
        LoadBitmap(L"Interface\\lo_back_s5_im04.jpg", BITMAP_TITLE + 11, GL_LINEAR);
        LoadBitmap(L"Interface\\lo_back_s5_im05.jpg", BITMAP_TITLE + 12, GL_LINEAR);
        LoadBitmap(L"Interface\\lo_back_s5_im06.jpg", BITMAP_TITLE + 13, GL_LINEAR);
    }
}

/**
 * @brief Loads all common title scene bitmaps.
 */
static void LoadCommonTitleBitmaps()
{
    LoadBitmap(L"Interface\\New_lo_back_01.jpg", BITMAP_TITLE, GL_LINEAR);
    LoadBitmap(L"Interface\\New_lo_back_02.jpg", BITMAP_TITLE + 1, GL_LINEAR);
    LoadBitmap(L"Interface\\lo_121518.tga", BITMAP_TITLE + 3, GL_LINEAR);
    LoadBitmap(L"Interface\\lo_lo.jpg", BITMAP_TITLE + 5, GL_LINEAR, GL_REPEAT);
    LoadBitmap(L"Interface\\lo_back_s5_03.jpg", BITMAP_TITLE + 6, GL_LINEAR);
    LoadBitmap(L"Interface\\lo_back_s5_04.jpg", BITMAP_TITLE + 7, GL_LINEAR);
}

/**
 * @brief Unloads all title scene bitmaps.
 */
static void UnloadTitleBitmaps()
{
    DeleteBitmap(TITLE_BITMAP_BASE);
    DeleteBitmap(TITLE_BITMAP_BACK_02);
    DeleteBitmap(TITLE_BITMAP_LOGO);
    DeleteBitmap(TITLE_BITMAP_PATTERN);

    for (int i = TITLE_BITMAP_DYNAMIC_START; i < TITLE_BITMAP_DYNAMIC_END; ++i)
    {
        DeleteBitmap(i);
    }
}

/**
 * @brief Webzen title/intro scene - displays loading screen and transitions to login.
 * @param hDC Device context for rendering
 */
void WebzenScene(HDC hDC)
{
    CUIMng& rUIMng = CUIMng::Instance();
    WEBZEN_LOGI("Enter WebzenScene");

    if (!OpenFont())
    {
        WEBZEN_LOGE("OpenFont failed");
        return;
    }
    WEBZEN_LOGI("OpenFont ok");
    ClearInput();
    WEBZEN_LOGI("ClearInput ok");

    LoadCommonTitleBitmaps();
    WEBZEN_LOGI("LoadCommonTitleBitmaps ok");
    BackgroundTheme theme = SelectBackgroundTheme();
    LoadBackgroundTheme(theme);
    WEBZEN_LOGI("LoadBackgroundTheme ok (theme=%d)", static_cast<int>(theme));

    rUIMng.CreateTitleSceneUI();
    WEBZEN_LOGI("CreateTitleSceneUI ok");

    FogEnable = false;

    ::EnableAlphaTest();
    WEBZEN_LOGI("EnableAlphaTest ok");
    OpenBasicData(hDC);
    WEBZEN_LOGI("OpenBasicData ok");

    g_pNewUISystem->LoadMainSceneInterface();
    WEBZEN_LOGI("LoadMainSceneInterface ok");

    CUIMng::Instance().RenderTitleSceneUI(hDC, 11, 11);
    WEBZEN_LOGI("RenderTitleSceneUI ok");

    rUIMng.ReleaseTitleSceneUI();
    UnloadTitleBitmaps();
    WEBZEN_LOGI("ReleaseTitleSceneUI + UnloadTitleBitmaps ok");

    g_ErrorReport.Write(L"> Loading ok.\r\n");

    SceneFlag = LOG_IN_SCENE;
    WEBZEN_LOGI("Scene switch -> LOG_IN_SCENE");
}
