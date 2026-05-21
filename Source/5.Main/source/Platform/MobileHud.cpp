#if defined(__ANDROID__) || defined(MU_IOS)

#include "MobileHud.h"

#include "MobileChatHud.h"
#include "MobilePlatform.h"

#include <cstdio>
#include <cstring>

#include "NewUISystem.h"
#include "NewUIMessageBox.h"
#include "NewUIMyInventory.h"
#include "NewUICharacterInfoWindow.h"
#include "NewUIMoveCommandWindow.h"

extern int DisplayHeight;

#if defined(__ANDROID__)
extern bool MU_Android_UtilityToolbarExpandedQuery();
#endif

namespace
{
constexpr const char* kHudModeConfigRelativePath = "Data/Local/android_hud_mode.cfg";
constexpr int kHudModeLegacy = 0;
constexpr int kHudModeModern = 1;

// First launch on phone defaults to the redesigned mobile HUD (not classic bar).
bool g_mobileLegacyMainHud = false;
bool g_mobileHudModeLoaded = false;

std::string BuildHudModeConfigPath()
{
    const std::string dataRoot = MU_MobileGetExternalDataPath();
    if (dataRoot.empty())
    {
        return kHudModeConfigRelativePath;
    }

    if (dataRoot.back() == '/')
    {
        return dataRoot + kHudModeConfigRelativePath;
    }

    return dataRoot + "/" + kHudModeConfigRelativePath;
}
} // namespace

void MU_MobileLoadMainHudMode()
{
    if (g_mobileHudModeLoaded)
    {
        return;
    }

    g_mobileHudModeLoaded = true;
    // g_mobileLegacyMainHud stays false (Mobile HUD) until config overrides.

    const std::string configPath = BuildHudModeConfigPath();
    FILE* file = fopen(configPath.c_str(), "rb");
    if (file == nullptr)
    {
        return;
    }

    char buffer[32] = {};
    const size_t readBytes = fread(buffer, 1, sizeof(buffer) - 1, file);
    fclose(file);

    if (readBytes == 0)
    {
        return;
    }

    buffer[readBytes] = '\0';
    const int mode = atoi(buffer);
    g_mobileLegacyMainHud = (mode != kHudModeModern);
}

void MU_MobileSaveMainHudMode()
{
    MU_MobileLoadMainHudMode();

    const std::string configPath = BuildHudModeConfigPath();
    FILE* file = fopen(configPath.c_str(), "wb");
    if (file == nullptr)
    {
        return;
    }

    const int mode = g_mobileLegacyMainHud ? kHudModeLegacy : kHudModeModern;
    fprintf(file, "%d\n", mode);
    fclose(file);
}

bool MU_MobileIsLegacyMainHudEnabled()
{
    MU_MobileLoadMainHudMode();
    return g_mobileLegacyMainHud;
}

bool MU_MobileIsModernMobileHudEnabled()
{
    return !MU_MobileIsLegacyMainHudEnabled();
}

bool MU_MobileIsUtilityToolbarExpanded()
{
#if defined(__ANDROID__)
    return MU_Android_UtilityToolbarExpandedQuery();
#else
    return true;
#endif
}

void MU_MobileToggleMainHudMode()
{
    MU_MobileLoadMainHudMode();
    g_mobileLegacyMainHud = !g_mobileLegacyMainHud;
    MU_MobileSaveMainHudMode();
}

void MU_MobileSwitchMainHudModeWithUiSync()
{
    MU_MobileToggleMainHudMode();
    if (MU_MobileIsLegacyMainHudEnabled())
    {
        MU_MobileChatHudRestoreLegacyLayout();
    }
    else
    {
        MU_MobileChatHudSyncLayout();
    }
}

void MU_MobileEnterClassicMainHudWithUiSync()
{
    MU_MobileLoadMainHudMode();
    if (g_mobileLegacyMainHud)
    {
        return;
    }

    g_mobileLegacyMainHud = true;
    MU_MobileSaveMainHudMode();
    MU_MobileChatHudRestoreLegacyLayout();
}

MU_MobileMinimapClusterLayout MU_MobileGetMinimapClusterLayout()
{
    MU_MobileMinimapClusterLayout layout{};
    constexpr float kScreenW = 640.0f;
    constexpr float kMinimapPanelW = 108.0f;
    constexpr float kMinimapFrameW = 92.0f;
    constexpr float kMinimapFrameInsetX = 5.0f;
    constexpr float kCoordBarH = 15.0f;
    constexpr float kCloseSize = 15.0f;
    constexpr float kRowInnerGap = 1.0f;
    constexpr float kBarGapBelow = 2.0f;
    constexpr float kPetFrameW = 57.0f;
    constexpr float kPetGap = 6.0f;

    constexpr float kMinimapContentTopInset = 18.0f;

    layout.minimapX = kScreenW - kMinimapPanelW;
    layout.minimapY = 10.0f;
    layout.coordBarH = kCoordBarH;
    layout.closeSize = kCloseSize;
    layout.rowWidth = kMinimapFrameW;
    layout.coordBarW = kMinimapFrameW - kCloseSize - kRowInnerGap;
    layout.coordBarX = layout.minimapX + kMinimapFrameInsetX;
    // Sát mép trên khung minimap (y + 18 trong DrawMiniMap).
    layout.coordBarY =
        layout.minimapY + kMinimapContentTopInset - layout.coordBarH - kBarGapBelow;
    layout.closeX = layout.coordBarX + layout.coordBarW + kRowInnerGap;
    layout.closeY = layout.coordBarY;
    layout.petBarX = layout.minimapX - kPetFrameW - kPetGap;
    layout.petBarY = layout.coordBarY;
    return layout;
}

int MU_MobileGetSidePanelAnchorY(int panelHeight)
{
    if (!MU_MobileIsModernMobileHudEnabled())
    {
        return 0;
    }

    constexpr int kDefaultPanelH = 429;
    constexpr int kDefaultScreenH = 480;
    const int h = (panelHeight > 0) ? panelHeight : kDefaultPanelH;
    const int screenH = (DisplayHeight > 0) ? DisplayHeight : kDefaultScreenH;
    const int centered = (screenH - h) / 2;
    return (centered > 0) ? centered : 0;
}

bool MU_MobileIsSidePanelOpen()
{
    if (!MU_MobileIsModernMobileHudEnabled() || g_pNewUISystem == nullptr)
    {
        return false;
    }

    return g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INVENTORY)
        || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHARACTER);
}

bool MU_MobileShouldShowCombatCluster()
{
    if (!MU_MobileIsModernMobileHudEnabled())
    {
        return true;
    }

    return !MU_MobileIsSidePanelOpen();
}

int MU_MobileGetSidePanelBottomY()
{
    constexpr int kPanelH = 429;
    return MU_MobileGetSidePanelAnchorY(kPanelH) + kPanelH;
}

bool MU_MobileHitTestMinimapCluster(float uiX, float uiY)
{
    if (!MU_MobileIsModernMobileHudEnabled())
    {
        return false;
    }

    const MU_MobileMinimapClusterLayout layout = MU_MobileGetMinimapClusterLayout();
    constexpr float kMinimapPanelW = 108.0f;
    constexpr float kMinimapPanelH = 100.0f;

    const bool inCoordRow =
        uiX >= layout.coordBarX
        && uiX <= (layout.coordBarX + layout.rowWidth + 4.0f)
        && uiY >= layout.coordBarY
        && uiY <= (layout.coordBarY + layout.coordBarH + 2.0f);
    const bool inMinimapPanel =
        uiX >= layout.minimapX
        && uiX <= (layout.minimapX + kMinimapPanelW)
        && uiY >= layout.minimapY
        && uiY <= (layout.minimapY + kMinimapPanelH);

    return inCoordRow || inMinimapPanel;
}

bool MU_MobileHitTestMoveMapPanel(float uiX, float uiY)
{
    if (!MU_MobileIsModernMobileHudEnabled() || g_pNewUISystem == nullptr)
    {
        return false;
    }

    if (!g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MOVEMAP)
        || g_pMoveCommandWindow == nullptr)
    {
        return false;
    }

    const POINT pos = g_pMoveCommandWindow->GetPos();
    const float left = static_cast<float>(pos.x);
    const float top = static_cast<float>(pos.y);
    const float width = static_cast<float>(g_pMoveCommandWindow->GetWindowWidth());
    const float height = static_cast<float>(g_pMoveCommandWindow->GetWindowHeight());
    return uiX >= left && uiX <= (left + width) && uiY >= top && uiY <= (top + height);
}

bool MU_MobileHitTestBlockingOverlay(float uiX, float uiY)
{
    if (!MU_MobileIsModernMobileHudEnabled())
    {
        return false;
    }

    if (MU_MobileHitTestMoveMapPanel(uiX, uiY))
    {
        return true;
    }

    if (MU_MobileHitTestSidePanel(uiX, uiY))
    {
        return true;
    }

    if (g_MessageBox != nullptr && !g_MessageBox->IsEmpty() && g_MessageBox->HitTestPointer())
    {
        return true;
    }

    return false;
}

bool MU_MobileHitTestSidePanel(float uiX, float uiY)
{
    if (!MU_MobileIsSidePanelOpen())
    {
        return false;
    }

    constexpr float kPanelW = 190.0f;
    constexpr float kPanelH = 429.0f;
    constexpr float kCharVisualTopInset = 20.0f;

    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INVENTORY) && g_pMyInventory != nullptr)
    {
        const POINT pos = g_pMyInventory->GetPos();
        const float left = static_cast<float>(pos.x);
        const float top = static_cast<float>(pos.y);
        if (uiX >= left && uiX <= (left + kPanelW) && uiY >= top && uiY <= (top + kPanelH))
        {
            return true;
        }
    }

    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHARACTER)
        && g_pCharacterInfoWindow != nullptr)
    {
        const POINT pos = g_pCharacterInfoWindow->GetPos();
        const float left = static_cast<float>(pos.x);
        const float top = static_cast<float>(pos.y) - kCharVisualTopInset;
        if (uiX >= left && uiX <= (left + kPanelW) && uiY >= top && uiY <= (top + kPanelH))
        {
            return true;
        }
    }

    return false;
}

#endif
