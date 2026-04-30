#include "stdafx.h"
#include "ScenePerfTelemetry.h"

MainScenePerfSnapshot g_mainScenePerfSnapshot = {};

MainScenePerfSnapshot ConsumeMainScenePerfSnapshot()
{
    const MainScenePerfSnapshot snapshot = g_mainScenePerfSnapshot;
    g_mainScenePerfSnapshot = {};
    return snapshot;
}
