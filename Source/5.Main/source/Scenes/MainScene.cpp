///////////////////////////////////////////////////////////////////////////////
// MainScene.cpp - Main game scene implementation
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainScene.h"
#include "SceneCommon.h"
#include "../Camera/CameraUtility.h"
#include "../ZzzOpenglUtil.h"
#include "../ZzzObject.h"
#include "../ZzzCharacter.h"
#include "../ZzzLodTerrain.h"
#include "../ZzzInterface.h"
#include "../ZzzEffect.h"
#include "../MapManager.h"
#include "../UIMng.h"
#include "../NewUISystem.h"
#include "../PartyManager.h"
#include "../CDirection.h"
#include "../w_PetProcess.h"
#include "../Utilities/Log/muConsoleDebug.h"
#include "../ZzzInterface.h"
#include "../WSclient.h"
#include "../GOBoid.h"
#include "../PersonalShopTitleImp.h"
#include "../UIManager.h"
#include "../ZzzInventory.h"
#include "../PortalMgr.h"
#include "../Guild/GuildCache.h"
#include "../UIMapName.h"

#if defined(__ANDROID__) || defined(MU_IOS)
#include <android/log.h>
#if defined(MU_ANDROID_DISABLE_LOG)
#define MAINSCENE_LOGI(...) ((void)0)
#else
#define MAINSCENE_LOGI(...) __android_log_print(ANDROID_LOG_INFO, "MuMain", __VA_ARGS__)
#endif
#else
#define MAINSCENE_LOGI(...)
#endif

// External declarations
extern float CameraAngle[3];
extern HWND g_hWnd;
extern CErrorReport g_ErrorReport;
extern float EarthQuake;
extern int CheckSkill;
extern int MouseY;
extern int LoadingWorld;
extern DWORD g_dwKeyFocusUIID;
extern int ErrorMessage;
extern int HeroTile;
extern CHARACTER* Hero;
extern CUIManager* g_pUIManager;
extern CUIMapName* g_pUIMapName;
extern int MouseX;
extern vec3_t MouseTarget;
extern int EditFlag;

namespace
{
    inline Uint64 MainScenePerfNow()
    {
        return static_cast<Uint64>(MU_MobilePerfNow());
    }

    inline unsigned long long MainScenePerfElapsed(Uint64 start)
    {
        return static_cast<unsigned long long>(MU_MobilePerfNow() - start);
    }
}

static bool RequireLeavesEffect()
{
    return (gMapManager.WorldActive == WD_0LORENCIA && HeroTile != 4) ||
           (gMapManager.WorldActive == WD_2DEVIAS && HeroTile != 3 && HeroTile < 10) ||
           gMapManager.WorldActive == WD_3NORIA ||
           gMapManager.WorldActive == WD_7ATLANSE ||
           gMapManager.InDevilSquare() ||
           gMapManager.WorldActive == WD_10HEAVEN ||
           gMapManager.InChaosCastle() ||
           gMapManager.InBattleCastle() ||
           M31HuntingGround::IsInHuntingGround() ||
           M33Aida::IsInAida() ||
           M34CryWolf1st::IsCyrWolf1st() ||
           gMapManager.WorldActive == WD_42CHANGEUP3RD_2ND ||
           IsIceCity() ||
           IsSantaTown() ||
           gMapManager.IsPKField() ||
           IsDoppelGanger2() ||
           gMapManager.IsEmpireGuardian1() ||
           gMapManager.IsEmpireGuardian2() ||
           gMapManager.IsEmpireGuardian3() ||
           gMapManager.IsEmpireGuardian4() ||
           IsUnitedMarketPlace();
}

static bool ShouldRenderLeaves()
{
    return (gMapManager.WorldActive == WD_2DEVIAS && HeroTile != 3 && HeroTile < 10) ||
           IsIceCity() ||
           IsSantaTown() ||
           gMapManager.IsPKField() ||
           IsDoppelGanger2() ||
           gMapManager.IsEmpireGuardian1() ||
           gMapManager.IsEmpireGuardian2() ||
           gMapManager.IsEmpireGuardian3() ||
           gMapManager.IsEmpireGuardian4() ||
           IsUnitedMarketPlace();
}

/**
 * @brief Performs one-time initialization when entering the main game scene.
 *
 * This function is called once when transitioning from character selection to the main game.
 * It performs the following tasks:
 * - Sends character selection to the game server
 * - Initializes UI systems (chat, party, guild, etc.)
 * - Sets up camera and input configuration
 * - Clears previous scene state and prepares for gameplay
 *
 * @note This function should only be called once per main scene entry.
 */
static void InitializeMainScene()
{
    g_ConsoleDebug->Write(MCD_NORMAL, L"Join the game with the following character: %ls", CharactersClient[SelectedHero].ID);
    g_ErrorReport.Write(L"> Character selected <%d> \"%ls\"\r\n", SelectedHero + 1, CharactersClient[SelectedHero].ID);
    g_csMapServer.SetHeroID(CharactersClient[SelectedHero].ID);

    InitMainScene = true;

    g_ConsoleDebug->Write(MCD_SEND, L"SendRequestJoinMapServer");

    CurrentProtocolState = REQUEST_JOIN_MAP_SERVER;
    MAINSCENE_LOGI("SendSelectCharacter index=%d name=%ls",
        SelectedHero,
        CharactersClient[SelectedHero].ID);
    SocketClient->ToGameServer()->SendSelectCharacter(CharactersClient[SelectedHero].ID);

    CUIMng::Instance().CreateMainScene();

    CameraAngle[2] = -45.f;

    ClearInput();
    InputEnable = false;
    TabInputEnable = false;
    InputTextWidth = 256;
    InputTextMax[0] = 42;
    InputTextMax[1] = 10;
    InputNumber = 2;
    for (int i = 0; i < MAX_WHISPER; i++)
    {
        g_pChatListBox->AddText(L"", L"", SEASON3B::TYPE_WHISPER_MESSAGE);
    }

    g_GuildNotice[0][0] = '\0';
    g_GuildNotice[1][0] = '\0';

    g_pPartyManager->Create();

    g_pChatListBox->ClearAll();
    g_pSystemLogBox->ClearAll();

    g_pSlideHelpMgr->Init();
    g_pUIMapName->Init();
    g_pNewUIMuHelper->Reset();

    g_GuildCache.Reset();
    g_PortalMgr.Reset();

    ClearAllObjectBlurs();

    SetFocus(g_hWnd);

    g_ErrorReport.Write(L"> Main Scene init success. ");
    g_ErrorReport.WriteCurrentTime();

    g_ConsoleDebug->Write(MCD_NORMAL, L"MainScene Init Success");
}

/**
 * @brief Resets per-frame state variables at the start of each frame.
 *
 * Initializes frame-dependent state including:
 * - Earthquake effect damping
 * - Terrain lighting
 * - UI interaction flags (inventory, skill checks, mouse window state)
 *
 * @note Called every frame during the main scene update loop.
 */
static void InitializeSceneFrame()
{
    EarthQuake *= 0.2f;
    InitTerrainLight();

    CheckInventory = NULL;
    CheckSkill = -1;
    MouseOnWindow = false;
}

/**
 * @brief Updates user interface and processes player input.
 *
 * Handles all UI-related updates and input processing including:
 * - Party system updates
 * - New UI system updates
 * - Mouse and keyboard input handling
 * - Window focus management
 * - Interface movement and tournament interface updates
 *
 * @note Only processes input when not in top-view camera mode and loading is complete.
 * @note Skips processing if CameraTopViewEnable is true or LoadingWorld >= 30.
 */
static void UpdateUIAndInput()
{
    if (CameraTopViewEnable || LoadingWorld >= 30)
        return;

    if (MouseY >= (int)(480 - 48))
        MouseOnWindow = true;

    g_pPartyManager->Update();
    g_pNewUISystem->Update();

    if (MouseLButton == true &&
        false == g_pNewUISystem->CheckMouseUse() &&
        g_dwMouseUseUIID == 0 &&
        g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHATINPUTBOX) == false)
    {
        g_pWindowMgr->SetWindowsEnable(FALSE);
        g_pFriendMenu->HideMenu();
        g_dwKeyFocusUIID = 0;
        if (GetFocus() != g_hWnd)
        {
            SaveIMEStatus();
            SetFocus(g_hWnd);
        }
    }

    MoveInterface();
    MoveTournamentInterface();

    if (ErrorMessage != MESSAGE_LOG_OUT)
        g_pUIManager->UpdateInput();
}

/**
 * @brief Updates all game entities and visual effects.
 *
 * Performs per-frame updates for all game world entities:
 * - World objects and items
 * - Environmental effects (leaves, boids, fish)
 * - Chat messages and player shops
 * - Player hero and other characters
 * - Mounts and pets
 * - Visual effects (particles, joints, pointers)
 * - Direction indicators
 *
 * @note Some updates are conditional based on camera mode (e.g., items only update when not in top-view).
 * @note Includes editor object updates when ENABLE_EDIT is defined.
 */
static void UpdateGameEntities()
{
    MoveObjects();

    if (!CameraTopViewEnable)
        MoveItems();

    if (RequireLeavesEffect())
    {
        MoveLeaves();
    }

    MoveBoids();
    MoveFishs();
    MoveChat();
    UpdatePersonalShopTitleImp();
    MoveHero();
    MoveCharactersClient();
    MoveMounts();
    ThePetProcess().UpdatePets();
    MovePoints();
    MoveEffects();
    MoveJoints();
    MoveParticles();
    MovePointers();

    g_Direction.CheckDirection();

#ifdef ENABLE_EDIT
    EditObjects();
#endif //ENABLE_EDIT
}

/**
 * @brief Main update function for the game scene.
 *
 * This is the primary per-frame update loop for the main gameplay scene.
 * It orchestrates initialization, server connection waiting, and frame updates by calling:
 * 1. InitializeMainScene() - One-time setup (first call only)
 * 2. Server join synchronization - Waits for server response before enabling rendering
 * 3. InitializeSceneFrame() - Per-frame state reset
 * 4. UpdateUIAndInput() - UI and input processing
 * 5. UpdateGameEntities() - Game world and entity updates
 *
 * @note Returns early if EnableMainRender is false (waiting for server join).
 */
void MoveMainScene()
{
    if (!InitMainScene)
    {
        InitializeMainScene();
    }

    if (CurrentProtocolState == RECEIVE_JOIN_MAP_SERVER)
    {
        EnableMainRender = true;
    }

    if (EnableMainRender == false)
    {
        static DWORD s_joinWaitLogTick = 0;
        const DWORD now = GetTickCount();
        if (now - s_joinWaitLogTick >= 2000)
        {
            s_joinWaitLogTick = now;
            MAINSCENE_LOGI("Waiting join-map response state=%d loadingWorld=%d scene=%d",
                CurrentProtocolState,
                LoadingWorld,
                SceneFlag);
        }
        return;
    }

    InitializeSceneFrame();
    UpdateUIAndInput();

    if (ErrorMessage != NULL)
        MouseOnWindow = true;

    UpdateGameEntities();

    g_ConsoleDebug->UpdateMainScene();
}

/**
 * @brief Sets up OpenGL viewport and clear color for main scene.
 *
 * @param outWidth Output screen width
 * @param outHeight Output screen height
 * @param outByWaterMap Output water map flag (0=normal, 1=hellas water, 2=water terrain)
 * @param cameraPos Camera position for frustum
 */
static void SetupMainSceneViewport(int& outWidth, int& outHeight, BYTE& outByWaterMap, vec3_t cameraPos)
{
    outByWaterMap = 0;

    if (CameraTopViewEnable == false)
    {
#if defined(__ANDROID__) || defined(MU_IOS)
        outHeight = 480;  // On Android the bottom bar is hidden — render game world to full height
#else
        outHeight = 480 - 48;
#endif
    }
    else
    {
        outHeight = 480;
    }

    outWidth = GetScreenWidth();

#if defined(__ANDROID__) || defined(MU_IOS)
    if (outWidth <= 260)
    {
        outWidth = GetWindowsX();
    }
#endif

#if defined(__ANDROID__) || defined(MU_IOS)
    {
        static DWORD s_lastViewportLogTick = 0;
        const DWORD now = GetTickCount();
        if (now - s_lastViewportLogTick >= 2000)
        {
            s_lastViewportLogTick = now;
            MAINSCENE_LOGI(
                "Viewport setup topView=%d width=%d height=%d screenW=%d",
                CameraTopViewEnable ? 1 : 0,
                outWidth,
                outHeight,
                GetScreenWidth());
        }
    }
#endif

    // Set clear color based on world
    if (gMapManager.WorldActive == WD_0LORENCIA)
    {
        glClearColor(10 / 256.f, 20 / 256.f, 14 / 256.f, 1.f);
    }
    else if (gMapManager.WorldActive == WD_2DEVIAS)
    {
        glClearColor(0.f / 256.f, 0.f / 256.f, 10.f / 256.f, 1.f);
    }
    else if (gMapManager.WorldActive == WD_10HEAVEN)
    {
        glClearColor(3.f / 256.f, 25.f / 256.f, 44.f / 256.f, 1.f);
    }
    else if (gMapManager.InChaosCastle() == true)
    {
        glClearColor(0 / 256.f, 0 / 256.f, 0 / 256.f, 1.f);
    }
    else if (gMapManager.WorldActive >= WD_45CURSEDTEMPLE_LV1 && gMapManager.WorldActive <= WD_45CURSEDTEMPLE_LV6)
    {
        glClearColor(9.f / 256.f, 8.f / 256.f, 33.f / 256.f, 1.f);
    }
    else if (gMapManager.InHellas() == true)
    {
        outByWaterMap = 1;
        glClearColor(0.f / 256.f, 0.f / 256.f, 0.f / 256.f, 1.f);
    }
    else
    {
        glClearColor(0 / 256.f, 0 / 256.f, 0 / 256.f, 1.f);
    }

    BeginOpengl(0, 0, outWidth, outHeight);
#if defined(__ANDROID__) || defined(MU_IOS)
    CreateFrustrum((float)GetWindowsX() / 640.0f, (float)outHeight / 480.0f, cameraPos);
#else
    CreateFrustrum((float)outWidth / 640.0f, (float)outHeight / 480.0f, cameraPos);
#endif

    // Setup fog for battle castle
    if (gMapManager.InBattleCastle())
    {
        if (battleCastle::InBattleCastle2(Hero->Object.Position))
        {
            vec3_t Color = { 0.f, 0.f, 0.f };
            battleCastle::StartFog(Color);
        }
        else
        {
            glDisable(GL_FOG);
        }
    }

    CreateScreenVector(MouseX, MouseY, MouseTarget);
}

/**
 * @brief Renders all 3D game entities (terrain, objects, characters, effects).
 *
 * @param byWaterMap Water map mode flag (passed by reference, may be modified)
 * @param width Screen width for water terrain rendering
 * @param height Screen height for water terrain rendering
 */
static void RenderGameWorld(BYTE& byWaterMap, int width, int height)
{
    if (IsWaterTerrain() == false)
    {
        if (gMapManager.WorldActive == WD_39KANTURU_3RD)
        {
            if (!g_Direction.m_CKanturu.IsMayaScene())
                RenderTerrain(false);
        }
        else
            if (gMapManager.WorldActive != WD_10HEAVEN && gMapManager.WorldActive != -1)
            {
                if (gMapManager.IsPKField() || IsDoppelGanger2())
                {
                    RenderObjects();
                }
                RenderTerrain(false);
            }
    }

    if (!gMapManager.IsPKField() && !IsDoppelGanger2())
        RenderObjects();

    Uint64 phaseStart = MainScenePerfNow();
    RenderEffectShadows();
    g_mainScenePerfSnapshot.shadowTicks += MainScenePerfElapsed(phaseStart);

    phaseStart = MainScenePerfNow();
    RenderBoids();
    g_mainScenePerfSnapshot.boidsTicks += MainScenePerfElapsed(phaseStart);

    RenderCharactersClient();

    if (EditFlag != EDIT_NONE)
    {
        RenderTerrain(true);
    }
    if (!CameraTopViewEnable)
    {
        phaseStart = MainScenePerfNow();
        RenderItems();
        g_mainScenePerfSnapshot.miscWorldTicks += MainScenePerfElapsed(phaseStart);
    }

    phaseStart = MainScenePerfNow();
    RenderFishs();
    RenderMount();
    RenderLeaves();

    if (!gMapManager.InChaosCastle())
        ThePetProcess().RenderPets();
    g_mainScenePerfSnapshot.miscWorldTicks += MainScenePerfElapsed(phaseStart);

    phaseStart = MainScenePerfNow();
    RenderBoids(true);
    g_mainScenePerfSnapshot.boidsTicks += MainScenePerfElapsed(phaseStart);
    RenderObjects_AfterCharacter();

    phaseStart = MainScenePerfNow();
    RenderJoints(byWaterMap);
    g_mainScenePerfSnapshot.jointsTicks += MainScenePerfElapsed(phaseStart);

    phaseStart = MainScenePerfNow();
    RenderEffects();
    g_mainScenePerfSnapshot.effectsTicks += MainScenePerfElapsed(phaseStart);

    phaseStart = MainScenePerfNow();
    RenderBlurs();
    g_mainScenePerfSnapshot.blursTicks += MainScenePerfElapsed(phaseStart);
    CheckSprites();
    BeginSprite();

    if (ShouldRenderLeaves())
    {
        phaseStart = MainScenePerfNow();
        RenderLeaves();
        g_mainScenePerfSnapshot.miscWorldTicks += MainScenePerfElapsed(phaseStart);
    }

    phaseStart = MainScenePerfNow();
    RenderSprites();
    g_mainScenePerfSnapshot.spritesTicks += MainScenePerfElapsed(phaseStart);

    phaseStart = MainScenePerfNow();
    RenderParticles();
    g_mainScenePerfSnapshot.particlesTicks += MainScenePerfElapsed(phaseStart);

    if (IsWaterTerrain() == false)
    {
        phaseStart = MainScenePerfNow();
        RenderPoints(byWaterMap);
        g_mainScenePerfSnapshot.pointsTicks += MainScenePerfElapsed(phaseStart);
    }

    EndSprite();

    phaseStart = MainScenePerfNow();
    RenderAfterEffects();
    g_mainScenePerfSnapshot.afterEffectsTicks += MainScenePerfElapsed(phaseStart);

    if (IsWaterTerrain() == true)
    {
        byWaterMap = 2;

        EndOpengl();
        BeginOpengl(0, 0, width, height);
        RenderWaterTerrain();

        phaseStart = MainScenePerfNow();
        RenderJoints(byWaterMap);
        g_mainScenePerfSnapshot.jointsTicks += MainScenePerfElapsed(phaseStart);

        phaseStart = MainScenePerfNow();
        RenderEffects(true);
        g_mainScenePerfSnapshot.effectsTicks += MainScenePerfElapsed(phaseStart);

        phaseStart = MainScenePerfNow();
        RenderBlurs();
        g_mainScenePerfSnapshot.blursTicks += MainScenePerfElapsed(phaseStart);
        CheckSprites();
        BeginSprite();

        if (gMapManager.WorldActive == WD_2DEVIAS && HeroTile != 3 && HeroTile < 10)
        {
            phaseStart = MainScenePerfNow();
            RenderLeaves();
            g_mainScenePerfSnapshot.miscWorldTicks += MainScenePerfElapsed(phaseStart);
        }

        phaseStart = MainScenePerfNow();
        RenderSprites(byWaterMap);
        g_mainScenePerfSnapshot.spritesTicks += MainScenePerfElapsed(phaseStart);

        phaseStart = MainScenePerfNow();
        RenderParticles(byWaterMap);
        g_mainScenePerfSnapshot.particlesTicks += MainScenePerfElapsed(phaseStart);

        phaseStart = MainScenePerfNow();
        RenderPoints(byWaterMap);
        g_mainScenePerfSnapshot.pointsTicks += MainScenePerfElapsed(phaseStart);

        EndSprite();
        EndOpengl();

        BeginOpengl(0, 0, width, height);
    }

    if (gMapManager.InBattleCastle())
    {
        if (battleCastle::InBattleCastle2(Hero->Object.Position))
        {
            battleCastle::EndFog();
        }
    }
}

/**
 * @brief Renders UI elements and overlays for the main scene.
 */
static void RenderMainSceneUI()
{
    const Uint64 uiStart = MainScenePerfNow();

    SelectObjects();
    BeginBitmap();
    RenderObjectDescription();

    if (CameraTopViewEnable == false)
    {
        RenderInterface(true);
    }
    RenderTournamentInterface();
    EndBitmap();

    g_pPartyManager->Render();
    g_pNewUISystem->Render();

    BeginBitmap();
    RenderInfomation();

#ifdef ENABLE_EDIT
    RenderDebugWindow();
#endif //ENABLE_EDIT

    EndBitmap();
    BeginBitmap();

    RenderCursor();

    EndBitmap();

    g_mainScenePerfSnapshot.uiTicks += MainScenePerfElapsed(uiStart);
}

/**
 * @brief Main rendering function for the game scene.
 *
 * Orchestrates the complete rendering pipeline:
 * 1. Determines camera position based on camera mode
 * 2. Sets up viewport and clear color
 * 3. Renders 3D world (terrain, objects, characters, effects)
 * 4. Renders UI and overlays
 *
 * @return true if rendering succeeded, false if rendering was skipped
 */
bool RenderMainScene()
{
    if (EnableMainRender == false)
    {
        return false;
    }

    if ((LoadingWorld) > 30)
    {
        return false;
    }

    FogEnable = false;

    vec3_t cameraPos;
    int width, height;
    BYTE byWaterMap;

    // Determine camera position
    if (MoveMainCamera() == true)
    {
        VectorCopy(Hero->Object.StartPosition, cameraPos);
    }
    else
    {
        g_pCatapultWindow->GetCameraPos(cameraPos);

        if (g_Direction.IsDirection() && g_Direction.m_bDownHero == false)
        {
            g_Direction.GetCameraPosition(cameraPos);
        }
    }

    SetupMainSceneViewport(width, height, byWaterMap, cameraPos);
    RenderGameWorld(byWaterMap, width, height);
    RenderMainSceneUI();

    EndOpengl();

    return true;
}
