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

#endif
