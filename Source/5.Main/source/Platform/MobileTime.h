#pragma once

#if defined(__ANDROID__) || defined(MU_IOS)

#include <cstdint>

void MU_MobileTimeInit();
uint32_t MU_MobileGetTicks();
uint64_t MU_MobilePerfNow();
uint64_t MU_MobilePerfFrequency();
double MU_MobilePerfToSeconds(uint64_t ticks);
double MU_MobilePerfToMilliseconds(uint64_t ticks);
void MU_MobileSleep(uint32_t ms);

#endif // defined(__ANDROID__) || defined(MU_IOS)
