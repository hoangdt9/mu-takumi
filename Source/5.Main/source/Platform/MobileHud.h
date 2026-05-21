#pragma once

#if defined(__ANDROID__) || defined(MU_IOS)

/** Classic MU bottom bar (Q/W/E/R, C/V, menu strip). Default on first launch. */
bool MU_MobileIsLegacyMainHudEnabled();

/** Redesigned mobile HUD: floating skills, PNG utility buttons, no MENU_3 strip. */
bool MU_MobileIsModernMobileHudEnabled();

/** Top-left utility toolbar (inventory row) expanded vs collapsed — affects hero-bar Menu anchor on Android. */
bool MU_MobileIsUtilityToolbarExpanded();

void MU_MobileToggleMainHudMode();
void MU_MobileLoadMainHudMode();
void MU_MobileSaveMainHudMode();

/** Toggle HUD mode and refresh mobile chat layout for the active mode. */
void MU_MobileSwitchMainHudModeWithUiSync();

/** If currently on modern mobile HUD, switch to classic (legacy) and persist; no-op if already classic. */
void MU_MobileEnterClassicMainHudWithUiSync();

/** Modern HUD: minimap top-right, slim coord strip above it, pet HP slot to its left. */
struct MU_MobileMinimapClusterLayout {
    float minimapX;
    float minimapY;
    float coordBarX;
    float coordBarY;
    float coordBarW;
    float coordBarH;
    float closeX;
    float closeY;
    float closeSize;
    float rowWidth;
    float petBarX;
    float petBarY;
};

MU_MobileMinimapClusterLayout MU_MobileGetMinimapClusterLayout();

/** Modern HUD (no bottom bar): vertical center for side panels (C/V), keep size. */
int MU_MobileGetSidePanelAnchorY(int panelHeight);

/** Re-apply centered Y (and C+V horizontal layout) after Show/Hide/Toggle. */
void MU_MobileRefreshSidePanelPositions();

bool MU_MobileIsSidePanelOpen();
/** Modern HUD: show attack / skill / potion cluster only when neither C nor V is open. */
bool MU_MobileShouldShowCombatCluster();
int MU_MobileGetSidePanelBottomY();
bool MU_MobileHitTestSidePanel(float uiX, float uiY);

/** Modern HUD: coord strip + minimap panel (640×480 UI space). */
bool MU_MobileHitTestMinimapCluster(float uiX, float uiY);

/** True when (uiX, uiY) is under C/V panels or a message box (blocks minimap click-through). */
bool MU_MobileHitTestBlockingOverlay(float uiX, float uiY);

#endif
