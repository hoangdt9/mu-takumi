#if defined(__ANDROID__) || defined(MU_IOS)

#include "MobileHud.h"

#include "MobileChatHud.h"
#include "MobilePlatform.h"

#if defined(__ANDROID__)
extern bool MU_Android_UtilityToolbarExpandedQuery();
#endif

#include <cstdio>
#include <cstring>

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
    g_mobileLegacyMainHud = true;

    const std::string configPath = BuildHudModeConfigPath();
    FILE* file = std::fopen(configPath.c_str(), "rb");
    if (file == nullptr)
    {
        return;
    }

    char buffer[32] = {};
    const size_t readBytes = std::fread(buffer, 1, sizeof(buffer) - 1, file);
    std::fclose(file);

    if (readBytes == 0)
    {
        return;
    }

    buffer[readBytes] = '\0';
    const int mode = std::atoi(buffer);
    g_mobileLegacyMainHud = (mode != kHudModeModern);
}

void MU_MobileSaveMainHudMode()
{
    MU_MobileLoadMainHudMode();

    const std::string configPath = BuildHudModeConfigPath();
    FILE* file = std::fopen(configPath.c_str(), "wb");
    if (file == nullptr)
    {
        return;
    }

    const int mode = g_mobileLegacyMainHud ? kHudModeLegacy : kHudModeModern;
    std::fprintf(file, "%d\n", mode);
    std::fclose(file);
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

#endif
