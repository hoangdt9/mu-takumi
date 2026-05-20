#pragma once

#if defined(__ANDROID__) || defined(MU_IOS)

/** Classic MU bottom bar (Q/W/E/R, C/V, menu strip). Default on first launch. */
bool MU_MobileIsLegacyMainHudEnabled();

/** Redesigned mobile HUD: floating skills, PNG utility buttons, no MENU_3 strip. */
bool MU_MobileIsModernMobileHudEnabled();

void MU_MobileToggleMainHudMode();
void MU_MobileLoadMainHudMode();
void MU_MobileSaveMainHudMode();

#endif
