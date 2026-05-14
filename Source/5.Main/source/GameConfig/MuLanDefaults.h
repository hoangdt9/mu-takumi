#pragma once

#include <cstdint>

// Android NDK: Gradle passes -DMU_LAN_DEFAULT_HOST_A=… into CMake (see app/build.gradle). Fallbacks for other builds.
#ifndef MU_LAN_DEFAULT_SERVER_HOST_A
#define MU_LAN_DEFAULT_SERVER_HOST_A "192.168.1.50"
#endif
#ifndef MU_LAN_DEFAULT_SERVER_HOST_W
#define MU_LAN_DEFAULT_SERVER_HOST_W L"192.168.1.50"
#endif

namespace MuLanDefaults
{
// First-hop TCP (Connect / CS) when config is empty or invalid — matches server-next LegacyLoginHost.
inline constexpr std::uint16_t kDefaultFirstHopConnectPort = 44605u;

// Lower bound of game-shard TCP range in Main.info protect fallback (not the CS port).
inline constexpr std::uint16_t kDefaultGameShardPortMin = 55901u;

// Legacy MuServer / VM first-hop (Connect) — some configs still use this; keep in sync with IsConnectServerPort checks.
inline constexpr std::uint16_t kLegacyVmClassicConnectPort = 63000u;
}
