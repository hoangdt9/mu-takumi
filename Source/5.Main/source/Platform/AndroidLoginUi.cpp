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
extern BOOL g_bGameServerConnected;
extern int HeroKey;
extern int CurrentProtocolState;
extern CHARACTER* Hero;

uint32_t s_joinServerWaitStartMs = 0;
char s_joinGameHost[16] = { 0 };
int s_joinGamePort = 0;
bool s_joinServerRetriedConnect = false;
bool s_joinServerTimedOut = false;

constexpr uint32_t kJoinServerRetryConnectMs = 8000u;
constexpr uint32_t kJoinServerGiveUpMs = 90000u;

namespace
{
constexpr uint32_t kRevealServerListDelayMs = 2000u;
constexpr uint32_t kF403RetryIntervalMs = 500u;
constexpr uint32_t kF403GiveUpMs = 15000u;
constexpr uint32_t kF403DirectGameMs = 3000u;

uint32_t s_connectFallbackStartMs = 0;
bool s_connectFallbackDone = false;
bool s_triedLoopbackConnect = false;
bool s_gotWireServerList = false;
bool s_preferLoopbackTcp = false;

uint32_t s_serverPickStartMs = 0;
uint32_t s_lastF403RetryMs = 0;
bool s_triedDirectGameLogin = false;

bool IsPrivateIPv4Host(const char* address)
{
	if (address == nullptr || address[0] == '\0')
	{
		return false;
	}

	unsigned int a = 0;
	unsigned int b = 0;
	unsigned int c = 0;
	unsigned int d = 0;
	if (sscanf(address, "%u.%u.%u.%u", &a, &b, &c, &d) != 4)
	{
		return false;
	}

	if (a == 10)
	{
		return true;
	}

	if (a == 192 && b == 168)
	{
		return true;
	}

	if (a == 172 && b >= 16 && b <= 31)
	{
		return true;
	}

	return false;
}

bool AndroidTryReconnectLoopbackConnect()
{
	if (s_triedLoopbackConnect)
	{
		return false;
	}

	if (szServerIpAddress == nullptr || !IsPrivateIPv4Host(szServerIpAddress))
	{
		return false;
	}

	s_triedLoopbackConnect = true;
	s_preferLoopbackTcp = true;

	SocketClient.Close();
	if (!CreateSocket(const_cast<char*>("127.0.0.1"), static_cast<unsigned short>(g_ServerPort)))
	{
		g_ErrorReport.Write(
			"[TakumiLoginBg] loopback reconnect failed 127.0.0.1:%u (run adb-reverse-takumi-dev.sh)\r\n",
			static_cast<unsigned>(g_ServerPort));
		return false;
	}

	const SOCKET fd = SocketClient.GetSocket();
	if (fd != INVALID_SOCKET && static_cast<int>(fd) > 0)
	{
		SendServerListRequest(static_cast<int32_t>(fd));
	}

	g_ErrorReport.Write(
		"[TakumiLoginBg] LAN had no C2 F4 06 — retry connect via 127.0.0.1:%u (Docker Desktop / adb reverse)\r\n",
		static_cast<unsigned>(g_ServerPort));
	return true;
}

void AndroidOpenGameLoginDirect()
{
	s_preferLoopbackTcp = true;
	SocketClient.Close();

	const unsigned short gamePort = MuLanDefaults::kTakumiLegacyLoginGamePort;
	if (!CreateSocket(const_cast<char*>("127.0.0.1"), gamePort))
	{
		g_ErrorReport.Write(
			"[TakumiLoginBg] direct game connect failed 127.0.0.1:%u\r\n",
			static_cast<unsigned>(gamePort));
		return;
	}

	g_bGameServerConnected = TRUE;
	CUIMng& rUIMng = CUIMng::Instance();
	rUIMng.HideWin(&rUIMng.m_ServerSelWin);
	rUIMng.ShowWin(&rUIMng.m_LoginWin);
	HeroKey = 0;
	CurrentProtocolState = REQUEST_JOIN_SERVER;
	MU_AndroidBeginJoinServerWait("127.0.0.1", static_cast<int>(gamePort));

	g_ErrorReport.Write(
		"[TakumiLoginBg] opened LoginWin via direct 127.0.0.1:%u (no F4 03 on LAN)\r\n",
		static_cast<unsigned>(gamePort));
}
} // namespace

void MU_AndroidDismissLoginWaitMsgIfShown()
{
	CUIMng& rUIMng = CUIMng::Instance();
	if (rUIMng.m_MsgWin.IsShow())
	{
		rUIMng.HideWin(&rUIMng.m_MsgWin);
	}
}

void MU_AndroidBeginJoinServerWait(const char* gameHost, int gamePort)
{
	s_joinServerWaitStartMs = MU_MobileGetTicks();
	s_joinServerRetriedConnect = false;
	s_joinServerTimedOut = false;
	s_joinGamePort = gamePort;
	if (gameHost != nullptr)
	{
		strncpy(s_joinGameHost, gameHost, sizeof(s_joinGameHost) - 1);
		s_joinGameHost[sizeof(s_joinGameHost) - 1] = '\0';
	}
	else
	{
		s_joinGameHost[0] = '\0';
	}

	g_ErrorReport.Write(
		"[AndroidLogin] join-server wait begin host=%s port=%d (expect C1 F1 00 from game-host)\r\n",
		s_joinGameHost,
		s_joinGamePort);
}

void MU_AndroidResetJoinServerWait()
{
	s_joinServerWaitStartMs = 0;
	s_joinGameHost[0] = '\0';
	s_joinGamePort = 0;
	s_joinServerRetriedConnect = false;
	s_joinServerTimedOut = false;
}

void MU_AndroidTickJoinServerWait()
{
	if (s_joinServerWaitStartMs == 0 || s_joinServerTimedOut)
	{
		return;
	}

	CUIMng& rUIMng = CUIMng::Instance();
	if (!rUIMng.m_LoginWin.IsShow())
	{
		return;
	}

	if (CurrentProtocolState == RECEIVE_JOIN_SERVER_SUCCESS)
	{
		MU_AndroidDismissLoginWaitMsgIfShown();
		MU_AndroidResetJoinServerWait();
		return;
	}

	const uint32_t elapsedMs = MU_MobileGetTicks() - s_joinServerWaitStartMs;
	for (int pass = 0; pass < 8; ++pass)
	{
		SocketClient.AndroidSyncPollRecvPending();
		ProtocolCompiler();
	}

	if (CurrentProtocolState == RECEIVE_JOIN_SERVER_SUCCESS)
	{
		g_ErrorReport.Write(
			"[AndroidLogin] join-server ready after %u ms (F1 00)\r\n",
			elapsedMs);
		MU_AndroidDismissLoginWaitMsgIfShown();
		MU_AndroidResetJoinServerWait();
		return;
	}

	if (!s_joinServerRetriedConnect
		&& elapsedMs >= kJoinServerRetryConnectMs
		&& s_joinGameHost[0] != '\0'
		&& s_joinGamePort > 0)
	{
		s_joinServerRetriedConnect = true;
		g_ErrorReport.Write(
			"[AndroidLogin] no F1 00 after %u ms — retry game TCP %s:%d (Docker game-host may have just finished building)\r\n",
			elapsedMs,
			s_joinGameHost,
			s_joinGamePort);

		SocketClient.Close();
		g_bGameServerConnected = FALSE;
		if (CreateSocket(s_joinGameHost, static_cast<unsigned short>(s_joinGamePort)))
		{
			g_bGameServerConnected = TRUE;
			for (int pass = 0; pass < 16; ++pass)
			{
				SocketClient.AndroidSyncPollRecvPending();
				ProtocolCompiler();
				if (CurrentProtocolState == RECEIVE_JOIN_SERVER_SUCCESS)
				{
					g_ErrorReport.Write(
						"[AndroidLogin] join-server ready after game TCP retry (%u ms)\r\n",
						MU_MobileGetTicks() - s_joinServerWaitStartMs);
					MU_AndroidDismissLoginWaitMsgIfShown();
					MU_AndroidResetJoinServerWait();
					return;
				}
			}
		}
		else
		{
			g_ErrorReport.Write(
				"[AndroidLogin] game TCP retry failed %s:%d\r\n",
				s_joinGameHost,
				s_joinGamePort);
		}
	}

	if (elapsedMs < kJoinServerGiveUpMs)
	{
		return;
	}

	s_joinServerTimedOut = true;
	g_ErrorReport.Write(
		"[AndroidLogin] join-server timeout after %u ms — is game-host listening? "
		"docker compose logs -f game-host until \"listening on *:55901\"\r\n",
		elapsedMs);
	MU_AndroidDismissLoginWaitMsgIfShown();
	rUIMng.PopUpMsgWin(MESSAGE_SERVER_LOST);
	SocketClient.Close();
	g_bGameServerConnected = FALSE;
	CurrentProtocolState = REQUEST_JOIN_SERVER;
}

bool MU_AndroidShouldPreferLoopbackTcp()
{
	return s_preferLoopbackTcp;
}

void MU_AndroidSetPreferLoopbackTcp(const bool prefer)
{
	s_preferLoopbackTcp = prefer;
}

void MU_AndroidNotifyWireServerListReceived()
{
	s_gotWireServerList = true;
	g_ErrorReport.Write("[TakumiLoginBg] received wire C2 F4 06 server list\r\n");
}

void MU_AndroidResetLoginSceneConnectFallback()
{
	s_connectFallbackStartMs = 0;
	s_connectFallbackDone = false;
	s_triedLoopbackConnect = false;
	s_gotWireServerList = false;
	s_preferLoopbackTcp = false;
	s_serverPickStartMs = 0;
	s_lastF403RetryMs = 0;
	s_triedDirectGameLogin = false;
	MU_AndroidResetJoinServerWait();
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

	const SOCKET fd = SocketClient.GetSocket();
	if (fd != INVALID_SOCKET && static_cast<int>(fd) > 0)
	{
		SocketClient.AndroidSyncPollRecvPending();
		ProtocolCompiler();
	}

	if (s_gotWireServerList)
	{
		MU_AndroidRevealLoginServerUi();
		s_connectFallbackDone = true;
		return;
	}

	const uint32_t elapsedMs = MU_MobileGetTicks() - s_connectFallbackStartMs;
	if (elapsedMs < kRevealServerListDelayMs)
	{
		return;
	}

	if (!s_triedLoopbackConnect && AndroidTryReconnectLoopbackConnect())
	{
		s_connectFallbackStartMs = MU_MobileGetTicks();
		return;
	}

	if (fd != INVALID_SOCKET && static_cast<int>(fd) > 0)
	{
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
	s_triedDirectGameLogin = false;
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

	if (!s_triedDirectGameLogin && elapsedMs >= kF403DirectGameMs)
	{
		s_triedDirectGameLogin = true;
		if (!s_triedLoopbackConnect && IsPrivateIPv4Host(szServerIpAddress))
		{
			AndroidTryReconnectLoopbackConnect();
		}

		AndroidOpenGameLoginDirect();
		if (rUIMng.m_LoginWin.IsShow())
		{
			s_serverPickStartMs = 0;
			return;
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
		"Run: server-next/scripts/adb-reverse-takumi-dev.sh then rebuild APK with "
		"-PmuBootstrapAdbReverse=true (127.0.0.1).\r\n",
		elapsedMs);
}

#endif // __ANDROID__
