#pragma once

struct MainScenePerfSnapshot
{
    unsigned long long shadowTicks = 0;
    unsigned long long boidsTicks = 0;
    unsigned long long miscWorldTicks = 0;
    unsigned long long jointsTicks = 0;
    unsigned long long effectsTicks = 0;
    unsigned long long blursTicks = 0;
    unsigned long long spritesTicks = 0;
    unsigned long long particlesTicks = 0;
    unsigned long long pointsTicks = 0;
    unsigned long long afterEffectsTicks = 0;
    unsigned long long uiTicks = 0;
};

extern MainScenePerfSnapshot g_mainScenePerfSnapshot;

MainScenePerfSnapshot ConsumeMainScenePerfSnapshot();
