#include "stdafx.h"

#if defined(__ANDROID__)

#include "MobilePlatform.h"
#include "MobileTime.h"
#include "GameConfig/MuLanDefaults.h"

#include <android/log.h>

#include "CameraMove.h"
#include "ServerListManager.h"
#include "UIMng.h"
#include "WSclient.h"
#include "wsclientinline.h"
#include "Utilities/Log/ErrorReport.h"

extern "C" void SendServerListRequest(int32_t handle);

extern char* szServerIpAddress;

namespace
{
constexpr uint32_t kRevealServerListDelayMs = 2000u;
constexpr uint32_t kF403RetryIntervalMs = 500u;
constexpr uint32_t kF403GiveUpMs = 15000u;

uint32_t s_connectFallbackStartMs = 0;
bool s_connectFallbackDone = false;

uint32_t s_serverPickStartMs = 0;
uint32_t s_lastF403RetryMs = 0;
}

void MU_AndroidResetLoginSceneConnectFallback()
{
    s_connectFallbackStartMs = 0;
    s_connectFallbackDone = false;
    s_serverPickStartMs = 0;
    s_lastF403RetryMs = 0;
}

void MU_AndroidRevealLoginServerUi()
{
    CUIMng& rUIMng = CUIMng::Instance();
    rUIMng.SetMoving(false);

    if (!rUIMng.m_CreditWin.IsShow())
    {
        rUIMng.ShowWin(&rUIMng.m_LoginMainWin);
        rUIMng.ShowWin(&rUIMng.m_ServerSelWin);
        rUIMng.m_ServerSelWin.UpdateDisplay();
    }

    __android_log_print(
        ANDROID_LOG_INFO,
        "TakumiErrorReport",
        "[TakumiLoginBg] reveal server/login UI (cinematic stays as background)\r\n");
}

static void AndroidApplyDefaultConnectServerListIfEmpty()
{
    if (g_ServerListManager->GetTotalServer() > 0)
    {
        return;
    }

    static const int kDefaultConnectIds[] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
        20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34,
        40, 41,
    };

    g_ServerListManager->Release();
    g_ServerListManager->SetTotalServer(static_cast<int>(sizeof(kDefaultConnectIds) / sizeof(kDefaultConnectIds[0])));
    for (int connectId : kDefaultConnectIds)
    {
        g_ServerListManager->InsertServerGroup(connectId, 0);
    }

    __android_log_print(
        ANDROID_LOG_INFO,
        "TakumiErrorReport",
        "[TakumiLoginBg] applied default F4 06 server groups (fallback)\r\n");
}

void MU_AndroidTickLoginSceneConnectFallback()
{
    if (s_connectFallbackDone)
    {
        return;
    }

    if (s_connectFallbackStartMs == 0)
    {
        s_connectFallbackStartMs = MU_MobileGetTicks();
    }

    CUIMng& rUIMng = CUIMng::Instance();
    if (rUIMng.m_ServerSelWin.IsShow() && rUIMng.m_LoginMainWin.IsShow())
    {
        s_connectFallbackDone = true;
        return;
    }

    const uint32_t elapsedMs = MU_MobileGetTicks() - s_connectFallbackStartMs;
    if (elapsedMs < kRevealServerListDelayMs)
    {
        return;
    }

    const SOCKET fd = SocketClient.GetSocket();
    if (fd != INVALID_SOCKET && static_cast<int>(fd) > 0)
    {
        SocketClient.AndroidSyncPollRecvPending();
        ProtocolCompiler();
        SendServerListRequest(static_cast<int32_t>(fd));
    }

    AndroidApplyDefaultConnectServerListIfEmpty();
    MU_AndroidRevealLoginServerUi();
    s_connectFallbackDone = true;

    g_ErrorReport.Write(
        "[TakumiLoginBg] connect fallback: revealed login/server UI after %u ms (no C2 F4 06 yet)\r\n",
        elapsedMs);
}

void MU_AndroidNotifyServerSubPickStarted()
{
    s_serverPickStartMs = MU_MobileGetTicks();
    s_lastF403RetryMs = 0;
    g_ErrorReport.Write("[TakumiLoginBg] sub-server picked — waiting for F4 03 on connect socket\r\n");
}

void MU_AndroidTickLoginAfterServerPickFallback()
{
    if (s_serverPickStartMs == 0)
    {
        return;
    }

    CUIMng& rUIMng = CUIMng::Instance();
    if (rUIMng.m_LoginWin.IsShow())
    {
        s_serverPickStartMs = 0;
        return;
    }

    const uint32_t nowMs = MU_MobileGetTicks();
    const uint32_t elapsedMs = nowMs - s_serverPickStartMs;

    const SOCKET fd = SocketClient.GetSocket();
    if (fd != INVALID_SOCKET && static_cast<int>(fd) > 0)
    {
        SocketClient.AndroidSyncPollRecvPending();
        ProtocolCompiler();

        if (rUIMng.m_LoginWin.IsShow())
        {
            g_ErrorReport.Write(
                "[TakumiLoginBg] LoginWin via ReceiveServerConnect after %u ms\r\n",
                elapsedMs);
            s_serverPickStartMs = 0;
            return;
        }
    }

    if (nowMs - s_lastF403RetryMs >= kF403RetryIntervalMs)
    {
        s_lastF403RetryMs = nowMs;
        const int connectIndex = g_ServerListManager->GetSelectServerConnectIndex();
        if (connectIndex >= 0 && fd != INVALID_SOCKET && static_cast<int>(fd) > 0)
        {
            SendRequestServerAddress(connectIndex);
            g_ErrorReport.Write(
                "[TakumiLoginBg] retry C1 F4 03 connectIndex=%d fd=%d elapsed=%u ms\r\n",
                connectIndex,
                static_cast<int>(fd),
                elapsedMs);
        }
    }

    if (elapsedMs < kF403GiveUpMs)
    {
        return;
    }

    s_serverPickStartMs = 0;
    rUIMng.ShowWin(&rUIMng.m_ServerSelWin);
    rUIMng.PopUpMsgWin(MESSAGE_SERVER_LOST);
    g_ErrorReport.Write(
        "[TakumiLoginBg] F4 03 timeout after %u ms — no game socket / LoginWin. "
        "If logcat shows 192.168.x and no recv tcp: run server-next/scripts/adb-reverse-takumi-dev.sh "
        "then rebuild APK with -PmuBootstrapAdbReverse=true (127.0.0.1).\r\n",
        elapsedMs);
}

#endif // __ANDROID__
