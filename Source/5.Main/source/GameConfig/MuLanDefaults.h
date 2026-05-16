#pragma once

#include <cstdint>

// Android NDK: Gradle passes -DMU_LAN_DEFAULT_HOST_A=… into CMake (see app/build.gradle). Fallbacks for other builds.
#ifndef MU_LAN_DEFAULT_SERVER_HOST_A
#define MU_LAN_DEFAULT_SERVER_HOST_A "10.0.2.2"
#endif
#ifndef MU_LAN_DEFAULT_SERVER_HOST_W
#define MU_LAN_DEFAULT_SERVER_HOST_W L"10.0.2.2"
#endif

namespace MuLanDefaults
{
// First-hop TCP (Connect / CS) when config is empty or invalid — matches server-next LegacyLoginHost.
inline constexpr std::uint16_t kDefaultFirstHopConnectPort = 44605u;

// server-next LegacyLoginHost game/login listener (paired with kDefaultFirstHopConnectPort).
inline constexpr std::uint16_t kTakumiLegacyLoginGamePort = 44606u;

// Android NDK: Gradle passes -DMU_GAME_TCP_PORT from server-next/.env TAKUMI_GAME_PORT (else login port).
#ifndef MU_GAME_TCP_PORT
#define MU_GAME_TCP_PORT 44606
#endif
inline constexpr std::uint16_t kTakumiGameTcpPort = static_cast<std::uint16_t>(MU_GAME_TCP_PORT);

// M6 split stack: Connect 44605 → F4 03 → game-host (e.g. 55901). Do not F4-03-bypass to 44606.
inline constexpr bool kTakumiSplitGameHostStack =
    kTakumiGameTcpPort != kTakumiLegacyLoginGamePort;

// Lower bound of game-shard TCP range in Main.info protect fallback (not the CS port).
inline constexpr std::uint16_t kDefaultGameShardPortMin = 55901u;

// Legacy MuServer / VM first-hop (Connect) — some configs still use this; keep in sync with IsConnectServerPort checks.
inline constexpr std::uint16_t kLegacyVmClassicConnectPort = 63000u;
}
