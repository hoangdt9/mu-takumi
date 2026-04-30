#pragma once

#include <cstdint>

struct AndroidNetworkOverlayStats
{
    bool connected = false;
    int latencyMs = -1;
    float uploadKBps = 0.0f;
    float downloadKBps = 0.0f;
};

#if defined(__ANDROID__) || defined(MU_IOS)
void AndroidPostPacket(void* packetInfoPtr);
void AndroidDrainPackets();
AndroidNetworkOverlayStats AndroidQueryNetworkOverlayStats(int32_t handle);
#else
inline AndroidNetworkOverlayStats AndroidQueryNetworkOverlayStats(int32_t)
{
    return {};
}
#endif
