#if defined(__ANDROID__) || defined(MU_IOS)

#include "MobileTime.h"

#define SOKOL_TIME_IMPL
#include <sokol_time.h>

#include <chrono>
#include <cmath>
#include <mutex>
#include <thread>

namespace
{
std::once_flag g_mobileTimeInitOnce;
uint64_t g_mobilePerfFrequency = 1000000000ULL;

void MU_InitMobileTimeOnce()
{
    stm_setup();

    const double secondsPerTick = stm_sec(1);
    if (secondsPerTick > 0.0)
    {
        const double frequency = 1.0 / secondsPerTick;
        if (std::isfinite(frequency) && frequency > 0.0)
        {
            g_mobilePerfFrequency = static_cast<uint64_t>(frequency + 0.5);
        }
    }
}

inline void MU_EnsureMobileTimeInitialized()
{
    std::call_once(g_mobileTimeInitOnce, MU_InitMobileTimeOnce);
}
} // namespace

void MU_MobileTimeInit()
{
    MU_EnsureMobileTimeInitialized();
}

uint32_t MU_MobileGetTicks()
{
    MU_EnsureMobileTimeInitialized();
    const uint64_t ticksMs = static_cast<uint64_t>(stm_ms(stm_now()));
    return static_cast<uint32_t>(ticksMs & 0xFFFFFFFFULL);
}

uint64_t MU_MobilePerfNow()
{
    MU_EnsureMobileTimeInitialized();
    return stm_now();
}

uint64_t MU_MobilePerfFrequency()
{
    MU_EnsureMobileTimeInitialized();
    return g_mobilePerfFrequency;
}

double MU_MobilePerfToSeconds(uint64_t ticks)
{
    MU_EnsureMobileTimeInitialized();
    return stm_sec(ticks);
}

double MU_MobilePerfToMilliseconds(uint64_t ticks)
{
    MU_EnsureMobileTimeInitialized();
    return stm_ms(ticks);
}

void MU_MobileSleep(uint32_t ms)
{
    if (ms == 0)
    {
        std::this_thread::yield();
        return;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

#endif // defined(__ANDROID__) || defined(MU_IOS)
