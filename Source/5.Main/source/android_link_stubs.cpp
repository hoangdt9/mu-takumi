#ifdef __ANDROID__

#include "stdafx.h"
#include "APICB.h"
#include "DSPlaySound.h"
#include "WSctlc.h"
#include "wsctlc_addon.h"
#include "ZzzBMD.h"
#include "ZzzLodTerrain.h"
#include "SimpleModulus.h"
#include "Jpeglib.h"
#include "UIManager.h"
#include "ExternalObject/Leaf/xstreambuf.h"
#include "ExternalObject/curl/curl.h"
#include "TrayMode.h"
#include "android/SimpleModulusCrypt.h"

#include <SDL_mixer.h>
#include <android/log.h>

#include <algorithm>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <mutex>
#include <string>
#include <unordered_map>
#include <string.h>

extern "C" int32_t ConnectionManager_Connect(
    const wchar_t* host,
    int32_t port,
    BYTE isEncrypted,
    void(*onPacket)(int32_t, int32_t, uint8_t*),
    void(*onDisconnect)(int32_t));
extern "C" int32_t ConnectionManager_GetLastConnectErrno(void);
extern "C" void ConnectionManager_Disconnect(int32_t handle);
extern "C" void ConnectionManager_Send(int32_t handle, const uint8_t* data, int32_t size);
extern "C" void SendServerListRequest(int32_t handle);

extern BOOL g_bGameServerConnected;

char* szServerIpAddress = (char*)"127.0.0.1";
std::string g_strSelectedML = "Eng";
int m_SavePassOnOff = 0;
unsigned short g_ServerPort = 63000;
BYTE Version[5] = { '1' + 1, '0' + 2, '4' + 3, '0' + 4, '5' + 5 };
BYTE Serial[17] = "TbYehR2hFUPBKgZj";
char m_ID[11] = {};
int mShowName = 0;
int mShowHPBar = 1;
int mShowMiniMap = 1;
int mShowDanhHieu = 1;
float g_androidZoomOverride = 0.0f;
HFONT g_hFontMini = NULL;

class CChatRoomSocketList;
CChatRoomSocketList* g_pChatRoomSocketList = nullptr;

extern int TextNum;
extern int g_iItemInfo[16][17];
extern int ItemHelp;

namespace SEASON3B
{
    float g_fScreenRate_x = 1.0f;
    float g_fScreenRate_y = 1.0f;
    int MouseY = 0;
    int SelectedCharacter = -1;
    int g_iCancelSkillTarget = 0;
    int g_iLengthAuthorityCode = 20;
    char TextList[60][512] = {};
    int TextListColor[60] = {};
    int TextBold[60] = {};
    int& TextNum = ::TextNum;
    int (&g_iItemInfo)[16][17] = ::g_iItemInfo;
    int& ItemHelp = ::ItemHelp;
    JewelHarmonyInfo*& g_pUIJewelHarmonyinfo = ::g_pUIJewelHarmonyinfo;
}

CTrayMode gTrayMode;

CTrayMode::CTrayMode()
{
    m_TrayIcon = NULL;
    m_MainWndProc = NULL;
}

void CTrayMode::Init(HINSTANCE) {}
void CTrayMode::Toggle() {}
LONG CTrayMode::GetMainWndProc() { return 0; }
void CTrayMode::ShowNotify(bool) {}
LRESULT CALLBACK CTrayMode::TrayModeWndProc(HWND, UINT, WPARAM, LPARAM) { return 0; }

extern "C" CURL* curl_easy_init(void) { return nullptr; }
extern "C" CURLcode curl_easy_setopt(CURL*, CURLoption, ...) { return CURLE_FAILED_INIT; }
extern "C" CURLcode curl_easy_perform(CURL*) { return CURLE_FAILED_INIT; }
extern "C" void curl_easy_cleanup(CURL*) {}

void SetAPIDATA_SEND(DATA_SEND) {}
void CBAnihack_Work() {}
void CBAnihack_Init() {}
void CBAnihack_Attack() {}
void CBAnihack_Recv(BYTE*) {}

namespace
{
constexpr long kAndroidDsVolumeMin = -10000;
constexpr long kAndroidDsVolumeMax = 0;

bool g_androidSoundEnabled = true;
long g_androidMasterVolume = 0;

int ConvertDirectSoundVolumeToSdl(long volume)
{
    if (!g_androidSoundEnabled)
    {
        return 0;
    }

    volume = std::clamp<long>(volume, kAndroidDsVolumeMin, kAndroidDsVolumeMax);

    if (volume <= kAndroidDsVolumeMin)
    {
        return 0;
    }

    if (volume >= kAndroidDsVolumeMax)
    {
        return MIX_MAX_VOLUME;
    }

    const double gain = std::pow(10.0, static_cast<double>(volume) / 2000.0);
    const int sdlVolume = static_cast<int>(std::lround(gain * MIX_MAX_VOLUME));
    return std::clamp(sdlVolume, 0, MIX_MAX_VOLUME);
}

void ApplyAndroidMasterVolume()
{
    const int sdlVolume = ConvertDirectSoundVolumeToSdl(g_androidMasterVolume);
    Mix_Volume(-1, sdlVolume);
    Mix_VolumeMusic(sdlVolume);
}

bool ShouldSuppressAndroidRuntimeLog(const char* format)
{
    if (format == nullptr)
    {
        return true;
    }

    // Do not suppress "[Android Socket] send packet" / "parsed packet": those are required for
    // diagnosing login and MU TCP flows (they were hidden here and looked like "no send/recv").
    static const char* kOpenMonsterPrefix = "OpenMonsterModel(";

    return std::strncmp(format, kOpenMonsterPrefix, std::strlen(kOpenMonsterPrefix)) == 0;
}
}

DWORD API_TypeMain = 0;
DWORD API_CharLevel = 0;
DWORD API_CharWeaponFirstSlot = 0;
DWORD API_CharWeaponSecondSlot = 0;
DWORD API_CharHelmSlot = 0;
DWORD API_CharArmorSlot = 0;
DWORD API_CharPantsSlot = 0;
DWORD API_CharGlovesSlot = 0;
DWORD API_CharBootsSlot = 0;
DWORD API_CharWingsSlot = 0;
DWORD API_CharPetSlot = 0;
DWORD API_CharAnimationID = 0;
DWORD API_CharActiveMagic = 0;
DWORD* API_GameState = nullptr;
DWORD* API_FrameValue = nullptr;
DWORD* API_SpeedValue = nullptr;
DWORD* API_MainTickCount = nullptr;
DWORD* API_SyncTickCount = nullptr;
DWORD* API_CountModifier = nullptr;
DWORD* API_DelayModifier = nullptr;
DWORD* API_HasteModifier = nullptr;
DWORD* API_SleepModifier = nullptr;
DWORD* API_SpeedModifier1 = nullptr;
DWORD* API_SpeedModifier2 = nullptr;
DWORD* API_ModelModifier1 = nullptr;
DWORD* API_ModelModifier2 = nullptr;
DWORD* API_ModelModifier3 = nullptr;
DWORD* API_ViewPoint = nullptr;
DWORD* API_ViewStrength = nullptr;
DWORD* API_ViewDexterity = nullptr;
DWORD* API_ViewVitality = nullptr;
DWORD* API_ViewEnergy = nullptr;
DWORD* API_ViewLeadership = nullptr;
DWORD* API_ViewAddStrength = nullptr;
DWORD* API_ViewAddDexterity = nullptr;
DWORD* API_ViewAddVitality = nullptr;
DWORD* API_ViewAddEnergy = nullptr;
DWORD* API_ViewAddLeadership = nullptr;
DWORD* API_ViewPhysiSpeed = nullptr;
DWORD* API_ViewMagicSpeed = nullptr;

HRESULT InitDirectSound(HWND) { return S_OK; }
void SetEnableSound(bool enabled)
{
    g_androidSoundEnabled = enabled;
    if (!enabled)
    {
        Mix_HaltChannel(-1);
        Mix_Volume(-1, 0);
        Mix_VolumeMusic(0);
        return;
    }

    ApplyAndroidMasterVolume();
}
void FreeDirectSound()
{
    Mix_HaltChannel(-1);
    g_androidSoundEnabled = false;
}
void LoadWaveFile(int, TCHAR*, int, bool) {}
HRESULT PlayBuffer(int, OBJECT*, BOOL) { return S_OK; }
void StopBuffer(int, BOOL) { Mix_HaltChannel(-1); }
void AllStopSound(void) { Mix_HaltChannel(-1); }
void Set3DSoundPosition() {}
HRESULT ReleaseBuffer(int) { return S_OK; }
HRESULT RestoreBuffers(int, int) { return S_OK; }
void SetVolume(int, long vol)
{
    g_androidMasterVolume = std::clamp<long>(vol, kAndroidDsVolumeMin, kAndroidDsVolumeMax);
    ApplyAndroidMasterVolume();
}
void SetMasterVolume(long vol)
{
    g_androidMasterVolume = std::clamp<long>(vol, kAndroidDsVolumeMin, kAndroidDsVolumeMax);
    ApplyAndroidMasterVolume();
}

static BYTE g_wsctlcReadMessage[MAX_RECVBUF] = {};
namespace
{
    constexpr DWORD kSimpleModulusSaveLoadXor[SIZE_ENCRYPTION_KEY] = {
        0x3F08A79B,
        0xE25CC287,
        0x93D27AB9,
        0x20DEA7BF,
    };

    constexpr DWORD kTakumiModulus[SIZE_ENCRYPTION_KEY] = { 95770, 84999, 76807, 71847 };
    constexpr DWORD kTakumiEncryptKey[SIZE_ENCRYPTION_KEY] = { 29511, 17162, 10281, 17092 };
    constexpr DWORD kTakumiDecryptKey[SIZE_ENCRYPTION_KEY] = { 17521, 13625, 21598, 26083 };
    constexpr DWORD kTakumiXorKey[SIZE_ENCRYPTION_KEY] = { 24429, 15164, 17929, 12915 };

    std::mutex g_androidSocketMutex;
    std::unordered_map<int32_t, CWsctlc*> g_androidSocketOwners;

    std::wstring ToWideString(const char* text)
    {
        std::wstring wide;
        if (!text)
        {
            return wide;
        }

        while (*text)
        {
            wide.push_back(static_cast<unsigned char>(*text));
            ++text;
        }

        return wide;
    }

    bool IsConnectServerPort(unsigned short port)
    {
        return (port == g_ServerPort || port == 63000);
    }

    void CopyPacketKey(DWORD* target, const DWORD* source)
    {
        if (target && source)
        {
            std::memcpy(target, source, sizeof(DWORD) * SIZE_ENCRYPTION_KEY);
        }
    }

    void SyncClientKey(const DWORD* modulus, const DWORD* key, const DWORD* xorKey)
    {
        mu::PacketKeySet keySet {};
        for (int i = 0; i < SIZE_ENCRYPTION_KEY; ++i)
        {
            keySet.modulus[static_cast<size_t>(i)] = modulus[i];
            keySet.key[static_cast<size_t>(i)] = key[i];
            keySet.xorKey[static_cast<size_t>(i)] = xorKey[i];
        }
        mu::SetClientToServerKey(keySet);
    }

    void SyncServerKey(const DWORD* modulus, const DWORD* key, const DWORD* xorKey)
    {
        mu::PacketKeySet keySet {};
        for (int i = 0; i < SIZE_ENCRYPTION_KEY; ++i)
        {
            keySet.modulus[static_cast<size_t>(i)] = modulus[i];
            keySet.key[static_cast<size_t>(i)] = key[i];
            keySet.xorKey[static_cast<size_t>(i)] = xorKey[i];
        }
        mu::SetServerToClientKey(keySet);
    }

    bool LoadSimpleModulusKeyFile(const char* path, DWORD* outModulus, DWORD* outKey, DWORD* outXorKey)
    {
        if (!path || !outModulus || !outKey || !outXorKey)
        {
            return false;
        }

        std::ifstream file(path, std::ios::binary);
        if (!file)
        {
            return false;
        }

        ChunkHeader header {};
        file.read(reinterpret_cast<char*>(&header), sizeof(header));

        if (!file || header.m_sID != CHUNKID_ONEKEY || header.m_iSize != (sizeof(ChunkHeader) + sizeof(DWORD) * SIZE_ENCRYPTION_KEY * 3))
        {
            return false;
        }

        DWORD table[SIZE_ENCRYPTION_KEY] = {};

        for (int section = 0; section < 3; ++section)
        {
            file.read(reinterpret_cast<char*>(table), sizeof(table));
            if (!file)
            {
                return false;
            }

            for (int i = 0; i < SIZE_ENCRYPTION_KEY; ++i)
            {
                table[i] ^= kSimpleModulusSaveLoadXor[i];
            }

            if (section == 0)
            {
                std::memcpy(outModulus, table, sizeof(table));
            }
            else if (section == 1)
            {
                std::memcpy(outKey, table, sizeof(table));
            }
            else
            {
                std::memcpy(outXorKey, table, sizeof(table));
            }
        }

        return true;
    }

}

CWsctlc::CWsctlc()
{
    m_hWnd = NULL;
    m_bGame = FALSE;
    m_iMaxSockets = 0;
    m_socket = INVALID_SOCKET;
    std::memset(m_SendBuf, 0, sizeof(m_SendBuf));
    m_nSendBufLen = 0;
    std::memset(m_RecvBuf, 0, sizeof(m_RecvBuf));
    m_nRecvBufLen = 0;
    m_LogPrint = 0;
    m_logfp = NULL;
    m_pPacketQueue = new CPacketQueue;
}

CWsctlc::~CWsctlc()
{
    Close();
    delete m_pPacketQueue;
    m_pPacketQueue = nullptr;
}

void CWsctlc::AndroidClearPacketQueue()
{
    if (m_pPacketQueue == nullptr)
    {
        return;
    }

    while (!m_pPacketQueue->IsEmpty())
    {
        m_pPacketQueue->PopPacket();
    }

    m_pPacketQueue->ClearGarbage();
}

void CWsctlc::AndroidOnPacket(int32_t handle, int32_t size, uint8_t* data)
{
    if (handle <= 0 || size <= 0 || data == nullptr)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(g_androidSocketMutex);
    const auto it = g_androidSocketOwners.find(handle);
    if (it == g_androidSocketOwners.end() || it->second == nullptr || it->second->m_pPacketQueue == nullptr)
    {
        return;
    }

    CWsctlc* client = it->second;

    if (size > MAX_RECVBUF || client->m_nRecvBufLen < 0 || client->m_nRecvBufLen >= MAX_RECVBUF)
    {
        g_ErrorReport.Write("[Android Socket] packet too large size=%d handle=%d\r\n", size, handle);
        return;
    }

    if ((client->m_nRecvBufLen + size) > MAX_RECVBUF)
    {
        g_ErrorReport.Write(
            "[Android Socket] recv buffer overflow size=%d current=%d handle=%d\r\n",
            size,
            client->m_nRecvBufLen,
            handle);
        client->m_nRecvBufLen = 0;
        return;
    }

    std::memcpy(client->m_RecvBuf + client->m_nRecvBufLen, data, static_cast<size_t>(size));

#if(ENCRYPT_STATE==1)
    if (gProtect.CheckSocketPort(client->m_socket) != 0)
    {
        gProtect.DecryptData(client->m_RecvBuf + client->m_nRecvBufLen, size);
    }
#endif

    client->m_nRecvBufLen += size;

    if (client->m_nRecvBufLen < 3)
    {
        return;
    }

    int offset = 0;
    int packetSize = 0;

    while (1)
    {
        if (client->m_RecvBuf[offset] == 0xC1 || client->m_RecvBuf[offset] == 0xC3)
        {
            packetSize = static_cast<int>(client->m_RecvBuf[offset + 1]);
        }
        else if (client->m_RecvBuf[offset] == 0xC2 || client->m_RecvBuf[offset] == 0xC4)
        {
            packetSize = (static_cast<int>(client->m_RecvBuf[offset + 1]) << 8) + client->m_RecvBuf[offset + 2];
        }
        else
        {
            char hexPrefix[4 * 16 + 8] = {};
            const int avail = client->m_nRecvBufLen - offset;
            const int n = (avail > 16) ? 16 : avail;
            int p = 0;
            for (int i = 0; i < n && p < (int)sizeof(hexPrefix) - 4; ++i)
            {
                p += std::snprintf(
                    hexPrefix + p,
                    sizeof(hexPrefix) - static_cast<size_t>(p),
                    "%02X ",
                    static_cast<unsigned int>(client->m_RecvBuf[offset + i]));
            }
            g_ErrorReport.Write(
                "[Android Socket] packet sync lost handle=%d first=0x%02X bufLen=%d prefix=%s\r\n",
                handle,
                client->m_RecvBuf[offset],
                client->m_nRecvBufLen,
                hexPrefix);
            g_ErrorReport.Write(
                "[Android Socket] sync hint: expect C1/C2/C3/C4 — if garbage, wrong TCP service on port or extra Protect/XOR layer\r\n");
            client->m_nRecvBufLen = 0;
            return;
        }

        if (packetSize <= 0)
        {
            g_ErrorReport.Write("[Android Socket] invalid packet size=%d handle=%d\r\n", packetSize, handle);
            client->m_nRecvBufLen = 0;
            return;
        }
        else if (packetSize <= client->m_nRecvBufLen)
        {
            g_ErrorReport.Write(
                "[Android Socket] parsed packet handle=%d type=0x%02X size=%d head=0x%02X sub=0x%02X\r\n",
                handle,
                client->m_RecvBuf[offset],
                packetSize,
                (packetSize > 2) ? client->m_RecvBuf[offset + 2] : 0,
                (packetSize > 3) ? client->m_RecvBuf[offset + 3] : 0);
            client->m_pPacketQueue->PushPacket(client->m_RecvBuf + offset, packetSize);
            offset += packetSize;
            client->m_nRecvBufLen -= packetSize;

            if (client->m_nRecvBufLen <= 0)
            {
                break;
            }
        }
        else
        {
            if (offset > 0 && client->m_nRecvBufLen > 0)
            {
                std::memmove(client->m_RecvBuf, client->m_RecvBuf + offset, static_cast<size_t>(client->m_nRecvBufLen));
            }
            break;
        }
    }

    client->m_pPacketQueue->ClearGarbage();
}

void CWsctlc::AndroidOnDisconnect(int32_t handle)
{
    std::lock_guard<std::mutex> lock(g_androidSocketMutex);
    const auto it = g_androidSocketOwners.find(handle);
    if (it == g_androidSocketOwners.end())
    {
        return;
    }

    CWsctlc* client = it->second;
    g_androidSocketOwners.erase(it);

    if (client != nullptr)
    {
        client->m_socket = INVALID_SOCKET;
        client->m_nRecvBufLen = 0;
        client->m_nSendBufLen = 0;

        if (client->m_bGame)
        {
            g_bGameServerConnected = FALSE;
        }
    }
}

SOCKET CWsctlc::GetSocket()
{
    return m_socket;
}

BOOL CWsctlc::Startup()
{
    m_socket = INVALID_SOCKET;
    m_iMaxSockets = 1;
    return TRUE;
}

void CWsctlc::Cleanup()
{
}

int CWsctlc::Create(HWND hWnd, BOOL bGame)
{
    m_hWnd = hWnd;
    m_bGame = bGame;
    m_socket = INVALID_SOCKET;
    m_nSendBufLen = 0;
    m_nRecvBufLen = 0;

    if (m_bGame)
    {
        g_bGameServerConnected = FALSE;
    }

    return TRUE;
}

BOOL CWsctlc::Close()
{
    const int32_t handle = (m_socket == INVALID_SOCKET) ? 0 : static_cast<int32_t>(m_socket);

    {
        std::lock_guard<std::mutex> lock(g_androidSocketMutex);
        if (handle > 0)
        {
            g_androidSocketOwners.erase(handle);
        }

        AndroidClearPacketQueue();
    }

    std::memset(m_SendBuf, 0, sizeof(m_SendBuf));
    std::memset(m_RecvBuf, 0, sizeof(m_RecvBuf));
    m_nSendBufLen = 0;
    m_nRecvBufLen = 0;
    m_socket = INVALID_SOCKET;

    if (m_bGame)
    {
        g_bGameServerConnected = FALSE;
    }

    if (handle > 0)
    {
        ConnectionManager_Disconnect(handle);
    }

    return TRUE;
}

BOOL CWsctlc::Close(SOCKET& socket)
{
    if (socket == m_socket)
    {
        Close();
    }
    else if (socket != INVALID_SOCKET)
    {
        const int32_t handle = static_cast<int32_t>(socket);
        {
            std::lock_guard<std::mutex> lock(g_androidSocketMutex);
            g_androidSocketOwners.erase(handle);
        }
        ConnectionManager_Disconnect(handle);
        socket = INVALID_SOCKET;
    }

    socket = INVALID_SOCKET;
    return TRUE;
}

BOOL CWsctlc::Connect(char* ipAddr, unsigned short port, DWORD)
{
    // Android path does not use a real HWND; blocking connect runs on the caller thread.
    if (ipAddr == nullptr || ipAddr[0] == '\0')
    {
        return FALSE;
    }

    if (m_socket != INVALID_SOCKET)
    {
        Close();
    }

    const std::wstring hostWide = ToWideString(ipAddr);
    const int32_t handle = ConnectionManager_Connect(
        hostWide.c_str(),
        static_cast<int32_t>(port),
        0,
        CWsctlc::AndroidOnPacket,
        CWsctlc::AndroidOnDisconnect);

    if (handle <= 0)
    {
        const int32_t lastErr = ConnectionManager_GetLastConnectErrno();
        char errBuf[128] = {};
        if (lastErr != 0)
        {
            strerror_r(lastErr, errBuf, sizeof(errBuf));
        }
        g_ErrorReport.Write(
            "[Android Socket] connect failed ip=%s port=%d errno=%d (%s). "
            "Kiem tra may chu dang mo TCP tai dia chi nay va dien thoai cung mang Wi-Fi (build: -PmuBootstrapServerHost / -PmuBootstrapServerPort).\r\n",
            ipAddr,
            port,
            static_cast<int>(lastErr),
            errBuf[0] != 0 ? errBuf : "unknown");
        return FALSE;
    }

    {
        std::lock_guard<std::mutex> lock(g_androidSocketMutex);
        g_androidSocketOwners[handle] = this;
        AndroidClearPacketQueue();
    }

    m_socket = static_cast<SOCKET>(handle);
    g_ErrorReport.Write(
        "[Android Socket] connected ip=%s port=%d handle=%d [AndroidLogin] native CM ok\r\n",
        ipAddr,
        port,
        handle);

    if (IsConnectServerPort(port))
    {
        SendServerListRequest(handle);
        g_ErrorReport.Write("[Android Socket] server list requested handle=%d\r\n", handle);
    }

    return TRUE;
}

int CWsctlc::sSend(SOCKET socket, char* buf, int len)
{
    if (socket == INVALID_SOCKET || buf == nullptr || len <= 0)
    {
        return FALSE;
    }

    g_ErrorReport.Write(
        "[Android Socket] send packet handle=%d type=0x%02X size=%d head=0x%02X sub=0x%02X\r\n",
        static_cast<int>(socket),
        static_cast<unsigned char>(buf[0]),
        len,
        (len > 2) ? static_cast<unsigned char>(buf[2]) : 0,
        (len > 3) ? static_cast<unsigned char>(buf[3]) : 0);

    std::vector<uint8_t> sendBuffer(reinterpret_cast<uint8_t*>(buf), reinterpret_cast<uint8_t*>(buf) + len);

#if(ENCRYPT_STATE==1)
    if (gProtect.CheckSocketPort(socket) != 0)
    {
        gProtect.EncryptData(sendBuffer.data(), len);
    }
#endif

    ConnectionManager_Send(static_cast<int32_t>(socket), sendBuffer.data(), len);
    return TRUE;
}

int CWsctlc::FDWriteSend()
{
    return TRUE;
}

int CWsctlc::nRecv()
{
    return 0;
}

BYTE* CWsctlc::GetReadMsg()
{
    std::lock_guard<std::mutex> lock(g_androidSocketMutex);

    if (m_pPacketQueue != nullptr && !m_pPacketQueue->IsEmpty())
    {
        CPacket* packet = m_pPacketQueue->FrontPacket();
        m_pPacketQueue->PopPacket();
        return packet->GetBuffer();
    }

    return g_wsctlcReadMessage[0] ? g_wsctlcReadMessage : NULL;
}

void CWsctlc::LogPrint(char*, ...) {}
void CWsctlc::LogHexPrint(BYTE*, int) {}
void CWsctlc::LogHexPrintS(BYTE*, int) {}
void CWsctlc::LogPrintOn() {}
void CWsctlc::LogPrintOff() {}

CErrorReport::CErrorReport() { Clear(); }
CErrorReport::~CErrorReport() {}
void CErrorReport::Clear(void) { m_hFile = INVALID_HANDLE_VALUE; m_lpszFileName[0] = 0; m_iKey = 0; }
void CErrorReport::Create(char*) {}
void CErrorReport::Destroy(void) {}
void CErrorReport::CutHead(void) {}
char* CErrorReport::CheckHeadToCut(char*, DWORD) { return nullptr; }
BOOL CErrorReport::WriteFile(HANDLE, void*, DWORD, LPDWORD, LPOVERLAPPED) { return TRUE; }
void CErrorReport::WriteDebugInfoStr(char*) {}
void CErrorReport::Write(const char* lpszFormat, ...)
{
    if (ShouldSuppressAndroidRuntimeLog(lpszFormat))
    {
        return;
    }

    va_list args;
    va_start(args, lpszFormat);
    __android_log_vprint(ANDROID_LOG_INFO, "TakumiErrorReport", lpszFormat ? lpszFormat : "", args);
    va_end(args);
}
void CErrorReport::HexWrite(void*, int) {}
void CErrorReport::AddSeparator(void) {}
void CErrorReport::WriteLogBegin(void) {}
void CErrorReport::WriteCurrentTime(BOOL) {}
void CErrorReport::WriteSystemInfo(ER_SystemInfo*) {}
void CErrorReport::WriteOpenGLInfo(void) {}
void CErrorReport::WriteImeInfo(HWND) {}
void CErrorReport::WriteSoundCardInfo(void) {}
void GetSystemInfo(ER_SystemInfo*) {}

int TERRAIN_INDEX_REPEAT(int x, int y)
{
    return (y & TERRAIN_SIZE_MASK) * TERRAIN_SIZE + (x & TERRAIN_SIZE_MASK);
}

int TERRAIN_INDEX(int x, int y)
{
    return y * TERRAIN_SIZE + x;
}

WORD TERRAIN_ATTRIBUTE(float x, float y)
{
    int xf = (int)(x / TERRAIN_SCALE);
    int yf = (int)(y / TERRAIN_SCALE);
    return TerrainWall[yf * TERRAIN_SIZE + xf];
}

__attribute__((used, noinline)) void TEXCOORD(float* c, float u, float v)
{
    if (!c) return;
    c[0] = u;
    c[1] = v;
}

namespace SEASON4A
{
    float IntensityTransform[MAX_MESH][MAX_VERTICES];
    int g_iLimitAttackTime = 15;
}

CSimpleModulus::CSimpleModulus()
{
    Init();
}
CSimpleModulus::~CSimpleModulus() {}
void CSimpleModulus::Init(void)
{
    std::memset(m_dwModulus, 0, sizeof(m_dwModulus));
    std::memset(m_dwEncryptionKey, 0, sizeof(m_dwEncryptionKey));
    std::memset(m_dwDecryptionKey, 0, sizeof(m_dwDecryptionKey));
    std::memset(m_dwXORKey, 0, sizeof(m_dwXORKey));
}
int CSimpleModulus::Encrypt(void* lpTarget, void* lpSource, int iSize)
{
    if (lpSource == nullptr || iSize <= 0)
    {
        return 0;
    }

    if (lpTarget == nullptr)
    {
        return ((iSize + 7) / 8) * 11;
    }

    const auto* source = reinterpret_cast<const uint8_t*>(lpSource);
    const uint8_t counter = source[0];

    std::vector<uint8_t> plainPacket(static_cast<size_t>(iSize + 1), 0);
    plainPacket[0] = 0xC3;
    plainPacket[1] = static_cast<uint8_t>(plainPacket.size() & 0xFF);

    if (iSize > 1)
    {
        std::memcpy(plainPacket.data() + 2, source + 1, static_cast<size_t>(iSize - 1));
    }

    mu::PacketKeySet keySet {};
    for (int i = 0; i < SIZE_ENCRYPTION_KEY; ++i)
    {
        keySet.modulus[static_cast<size_t>(i)] = m_dwModulus[i];
        keySet.key[static_cast<size_t>(i)] = m_dwEncryptionKey[i];
        keySet.xorKey[static_cast<size_t>(i)] = m_dwXORKey[i];
    }

    const std::vector<uint8_t> encryptedPacket = mu::SM_EncryptPacketWithKey(plainPacket, counter, keySet);
    const int encryptedSize = static_cast<int>(encryptedPacket.size()) - 2;

    if (encryptedSize > 0)
    {
        std::memcpy(lpTarget, encryptedPacket.data() + 2, static_cast<size_t>(encryptedSize));
    }

    return encryptedSize;
}
int CSimpleModulus::Decrypt(void* lpTarget, void* lpSource, int iSize)
{
    if (lpSource == nullptr || iSize <= 0)
    {
        return 0;
    }

    std::vector<uint8_t> encryptedPacket;

    if ((iSize + 2) <= 0xFF)
    {
        encryptedPacket.resize(static_cast<size_t>(iSize + 2), 0);
        encryptedPacket[0] = 0xC3;
        encryptedPacket[1] = static_cast<uint8_t>(encryptedPacket.size() & 0xFF);
        std::memcpy(encryptedPacket.data() + 2, lpSource, static_cast<size_t>(iSize));
    }
    else
    {
        encryptedPacket.resize(static_cast<size_t>(iSize + 3), 0);
        encryptedPacket[0] = 0xC4;
        encryptedPacket[1] = static_cast<uint8_t>((encryptedPacket.size() >> 8) & 0xFF);
        encryptedPacket[2] = static_cast<uint8_t>(encryptedPacket.size() & 0xFF);
        std::memcpy(encryptedPacket.data() + 3, lpSource, static_cast<size_t>(iSize));
    }

    bool success = false;
    uint8_t packetCounter = 0;
    mu::PacketKeySet keySet {};
    for (int i = 0; i < SIZE_ENCRYPTION_KEY; ++i)
    {
        keySet.modulus[static_cast<size_t>(i)] = m_dwModulus[i];
        keySet.key[static_cast<size_t>(i)] = m_dwDecryptionKey[i];
        keySet.xorKey[static_cast<size_t>(i)] = m_dwXORKey[i];
    }

    const std::vector<uint8_t> plainPacket = mu::SM_DecryptPacketWithKey(encryptedPacket, keySet, &success, &packetCounter);

    if (!success || plainPacket.size() < 3)
    {
        return -1;
    }

    const int headerSize = (plainPacket[0] == 0xC4) ? 3 : 2;
    const int payloadSize = static_cast<int>(plainPacket.size()) - headerSize;
    const int resultSize = payloadSize + 1;

    if (lpTarget != nullptr)
    {
        auto* target = reinterpret_cast<uint8_t*>(lpTarget);
        target[0] = packetCounter;
        if (payloadSize > 0)
        {
            std::memcpy(target + 1, plainPacket.data() + headerSize, static_cast<size_t>(payloadSize));
        }
    }

    return resultSize;
}
void CSimpleModulus::EncryptBlock(void*, void*, int) {}
int CSimpleModulus::DecryptBlock(void*, void*) { return 0; }
int CSimpleModulus::AddBits(void*, int, void*, int, int) { return 0; }
void CSimpleModulus::Shift(void*, int, int) {}
int CSimpleModulus::GetByteOfBit(int nBit) { return nBit / 8; }
BOOL CSimpleModulus::SaveAllKey(char*) { return TRUE; }
BOOL CSimpleModulus::LoadAllKey(char* lpszFileName) { return (LoadEncryptionKey(lpszFileName) && LoadDecryptionKey(lpszFileName)); }
BOOL CSimpleModulus::SaveEncryptionKey(char*) { return TRUE; }
BOOL CSimpleModulus::LoadEncryptionKey(char* lpszFileName) { return LoadKey(lpszFileName, CHUNKID_ONEKEY, TRUE, TRUE, FALSE, TRUE); }
BOOL CSimpleModulus::SaveDecryptionKey(char*) { return TRUE; }
BOOL CSimpleModulus::LoadDecryptionKey(char* lpszFileName) { return LoadKey(lpszFileName, CHUNKID_ONEKEY, TRUE, FALSE, TRUE, TRUE); }
BOOL CSimpleModulus::SaveKey(char*, unsigned short, BOOL, BOOL, BOOL, BOOL) { return TRUE; }
BOOL CSimpleModulus::LoadKey(char* lpszFileName, unsigned short sID, BOOL bMod, BOOL bEnc, BOOL bDec, BOOL bXOR)
{
    if (sID != CHUNKID_ONEKEY)
    {
        return FALSE;
    }

    DWORD modulus[SIZE_ENCRYPTION_KEY] = {};
    DWORD key[SIZE_ENCRYPTION_KEY] = {};
    DWORD xorKey[SIZE_ENCRYPTION_KEY] = {};

    bool loaded = LoadSimpleModulusKeyFile(lpszFileName, modulus, key, xorKey);

    if (!loaded)
    {
        CopyPacketKey(modulus, kTakumiModulus);
        CopyPacketKey(xorKey, kTakumiXorKey);

        if (bEnc)
        {
            CopyPacketKey(key, kTakumiEncryptKey);
            g_ErrorReport.Write("[AndroidLogin] SimpleModulus encrypt key fallback active path=%s\r\n", lpszFileName ? lpszFileName : "(null)");
        }
        else if (bDec)
        {
            CopyPacketKey(key, kTakumiDecryptKey);
            g_ErrorReport.Write("[AndroidLogin] SimpleModulus decrypt key fallback active path=%s\r\n", lpszFileName ? lpszFileName : "(null)");
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        g_ErrorReport.Write("[AndroidLogin] SimpleModulus key loaded path=%s\r\n", lpszFileName ? lpszFileName : "(null)");
    }

    if (bMod)
    {
        std::memcpy(m_dwModulus, modulus, sizeof(m_dwModulus));
    }

    if (bEnc)
    {
        std::memcpy(m_dwEncryptionKey, key, sizeof(m_dwEncryptionKey));
    }

    if (bDec)
    {
        std::memcpy(m_dwDecryptionKey, key, sizeof(m_dwDecryptionKey));
    }

    if (bXOR)
    {
        std::memcpy(m_dwXORKey, xorKey, sizeof(m_dwXORKey));
    }

    if (bEnc)
    {
        SyncClientKey(m_dwModulus, m_dwEncryptionKey, m_dwXORKey);
    }

    if (bDec)
    {
        SyncServerKey(m_dwModulus, m_dwDecryptionKey, m_dwXORKey);
    }

    return TRUE;
}
BOOL CSimpleModulus::LoadKeyFromBuffer(BYTE* pbyBuffer, BOOL bMod, BOOL bEnc, BOOL bDec, BOOL bXOR)
{
    if (pbyBuffer == nullptr)
    {
        return FALSE;
    }

    DWORD temp[SIZE_ENCRYPTION_KEY] = {};
    std::memcpy(temp, pbyBuffer, sizeof(temp));

    if (bMod)
    {
        std::memcpy(m_dwModulus, temp, sizeof(m_dwModulus));
    }

    if (bEnc)
    {
        std::memcpy(m_dwEncryptionKey, temp, sizeof(m_dwEncryptionKey));
    }

    if (bDec)
    {
        std::memcpy(m_dwDecryptionKey, temp, sizeof(m_dwDecryptionKey));
    }

    if (bXOR)
    {
        std::memcpy(m_dwXORKey, temp, sizeof(m_dwXORKey));
    }

    if (bEnc)
    {
        SyncClientKey(m_dwModulus, m_dwEncryptionKey, m_dwXORKey);
    }

    if (bDec)
    {
        SyncServerKey(m_dwModulus, m_dwDecryptionKey, m_dwXORKey);
    }

    return TRUE;
}

DWORD CSimpleModulus::s_dwSaveLoadXOR[SIZE_ENCRYPTION_KEY] = {
    0x3F08A79B,
    0xE25CC287,
    0x93D27AB9,
    0x20DEA7BF,
};

namespace leaf
{
    void xstreambuf::init()
    {
        m_pBuffer = nullptr;
        m_offset = 0;
        m_size = 0;
        m_capacity = 0;
        m_chunksize = 4096;
    }

    xstreambuf::xstreambuf() { init(); }
    xstreambuf::xstreambuf(unsigned int size) { init(); resize(size); }
    xstreambuf::xstreambuf(const xstreambuf& xbuf) { init(); *this = xbuf; }
    xstreambuf::~xstreambuf() { std::free(m_pBuffer); }

    void xstreambuf::seek(unsigned int n, XBUF_POS pos)
    {
        m_offset = (pos == XBUF_END) ? (unsigned int)std::min<size_t>(m_size, n) : n;
        if (m_offset > m_size) m_offset = (unsigned int)m_size;
    }

    xstreambuf& xstreambuf::write(const void* src, unsigned int n)
    {
        if (m_offset + n > m_capacity) resize(m_offset + n);
        if (src && n > 0) std::memcpy((char*)m_pBuffer + m_offset, src, n);
        m_offset += n;
        if (m_offset > m_size) m_size = m_offset;
        return *this;
    }

    xstreambuf& xstreambuf::read(void* dest, unsigned int n)
    {
        const size_t available = (m_offset < m_size) ? (m_size - m_offset) : 0;
        const size_t readable = (available < n) ? available : n;
        if (dest && readable > 0) std::memcpy(dest, (char*)m_pBuffer + m_offset, readable);
        m_offset += (unsigned int)readable;
        return *this;
    }

    void xstreambuf::clear() { m_offset = 0; m_size = 0; }
    void xstreambuf::flush() {}
    const void* xstreambuf::data() const { return m_pBuffer; }
    void* xstreambuf::get_writebuf() const { return m_pBuffer; }
    unsigned int xstreambuf::get_curpos() const { return m_offset; }
    size_t xstreambuf::size() const { return m_size; }
    void xstreambuf::resize(size_t s)
    {
        if (s <= m_capacity) { if (s > m_size) m_size = s; return; }
        const size_t newCapacity = ((s + m_chunksize - 1) / m_chunksize) * m_chunksize;
        void* newBuffer = std::realloc(m_pBuffer, newCapacity);
        if (!newBuffer) return;
        m_pBuffer = newBuffer;
        if (newCapacity > m_capacity) std::memset((char*)m_pBuffer + m_capacity, 0, newCapacity - m_capacity);
        m_capacity = newCapacity;
        m_size = s;
    }
    size_t xstreambuf::capacity() const { return m_capacity; }
    bool xstreambuf::empty() const { return m_size == 0; }
    void xstreambuf::set_chunksize(size_t size) { m_chunksize = size ? size : 4096; }
    size_t xstreambuf::get_chunksize() { return m_chunksize; }

    xfstreambuf::xfstreambuf() {}
    xfstreambuf::xfstreambuf(const xstreambuf& xbuf) : xstreambuf(xbuf) {}
    xfstreambuf::xfstreambuf(const std::string& filename) { load(filename); }
    xfstreambuf::~xfstreambuf() {}
    bool xfstreambuf::load(const std::string&) { clear(); return false; }
    bool xfstreambuf::save(const std::string&) { return false; }
}

static void AndroidJpegNoopMessage(j_common_ptr) {}
static void AndroidJpegNoopEmit(j_common_ptr, int) {}
static void AndroidJpegNoopOutput(j_common_ptr) {}
static void AndroidJpegNoopFormat(j_common_ptr, char*) {}
static void AndroidJpegNoopReset(j_common_ptr) {}
static int AndroidJpegNoopTrace(j_common_ptr, int) { return 0; }
static void AndroidJpegNoopMemVoid(j_common_ptr, int) {}
static void* AndroidJpegAllocSmall(j_common_ptr, int, size_t size) { return std::calloc(1, size); }
static void* AndroidJpegAllocLarge(j_common_ptr, int, size_t size) { return std::calloc(1, size); }
static JSAMPARRAY AndroidJpegAllocSArray(j_common_ptr, int, JDIMENSION samplesperrow, JDIMENSION numrows)
{
    const JDIMENSION rowCount = (numrows > 0) ? numrows : 1;
    const JDIMENSION sampleCount = (samplesperrow > 0) ? samplesperrow : 1;
    JSAMPARRAY rows = (JSAMPARRAY)std::calloc(rowCount, sizeof(JSAMPROW));
    for (JDIMENSION i = 0; rows && i < rowCount; ++i)
        rows[i] = (JSAMPROW)std::calloc(sampleCount, sizeof(JSAMPLE));
    return rows;
}

static jpeg_memory_mgr g_androidJpegMemory = {};

extern "C" jpeg_error_mgr* jpeg_std_error(jpeg_error_mgr* err)
{
    if (!err) return nullptr;
    std::memset(err, 0, sizeof(*err));
    err->error_exit = AndroidJpegNoopMessage;
    err->emit_message = AndroidJpegNoopEmit;
    err->output_message = AndroidJpegNoopOutput;
    err->format_message = AndroidJpegNoopFormat;
    err->reset_error_mgr = AndroidJpegNoopReset;
    err->msg_code = 0;
    err->trace_level = 0;
    err->num_warnings = 0;
    return err;
}

extern "C" void jpeg_CreateDecompress(j_decompress_ptr cinfo, int, size_t)
{
    if (!cinfo) return;
    g_androidJpegMemory.alloc_small = AndroidJpegAllocSmall;
    g_androidJpegMemory.alloc_large = AndroidJpegAllocLarge;
    g_androidJpegMemory.alloc_sarray = AndroidJpegAllocSArray;
    g_androidJpegMemory.free_pool = AndroidJpegNoopMemVoid;
    cinfo->mem = &g_androidJpegMemory;
    cinfo->image_width = 1;
    cinfo->image_height = 1;
    cinfo->output_width = 1;
    cinfo->output_height = 1;
    cinfo->output_components = 3;
}

extern "C" void jpeg_destroy_decompress(j_decompress_ptr) {}
extern "C" void jpeg_stdio_src(j_decompress_ptr, FILE*) {}
extern "C" int jpeg_read_header(j_decompress_ptr, j_boolean) { return JPEG_HEADER_OK; }
extern "C" j_boolean jpeg_start_decompress(j_decompress_ptr cinfo)
{
    if (cinfo)
    {
        cinfo->output_width = cinfo->output_width ? cinfo->output_width : 1;
        cinfo->output_height = cinfo->output_height ? cinfo->output_height : 1;
        cinfo->output_components = cinfo->output_components ? cinfo->output_components : 3;
        cinfo->output_scanline = 0;
    }
    return TRUE;
}
extern "C" JDIMENSION jpeg_read_scanlines(j_decompress_ptr cinfo, JSAMPARRAY scanlines, JDIMENSION max_lines)
{
    if (!cinfo || max_lines == 0) return 0;
    if (scanlines && scanlines[0])
    {
        const JDIMENSION rowSize = cinfo->output_width * cinfo->output_components;
        std::memset(scanlines[0], 0xff, rowSize > 0 ? rowSize : 1);
    }
    cinfo->output_scanline++;
    return 1;
}
extern "C" j_boolean jpeg_finish_decompress(j_decompress_ptr) { return TRUE; }
extern "C" void jpeg_CreateCompress(j_compress_ptr cinfo, int, size_t)
{
    if (!cinfo) return;
    g_androidJpegMemory.alloc_small = AndroidJpegAllocSmall;
    g_androidJpegMemory.alloc_large = AndroidJpegAllocLarge;
    g_androidJpegMemory.alloc_sarray = AndroidJpegAllocSArray;
    g_androidJpegMemory.free_pool = AndroidJpegNoopMemVoid;
    cinfo->mem = &g_androidJpegMemory;
    cinfo->next_scanline = 0;
}
extern "C" void jpeg_destroy_compress(j_compress_ptr) {}
extern "C" void jpeg_stdio_dest(j_compress_ptr, FILE*) {}
extern "C" void jpeg_set_defaults(j_compress_ptr) {}
extern "C" void jpeg_set_quality(j_compress_ptr, int, j_boolean) {}
extern "C" void jpeg_start_compress(j_compress_ptr cinfo, j_boolean)
{
    if (cinfo) cinfo->next_scanline = 0;
}
extern "C" JDIMENSION jpeg_write_scanlines(j_compress_ptr cinfo, JSAMPARRAY, JDIMENSION num_lines)
{
    if (cinfo) cinfo->next_scanline += num_lines;
    return num_lines;
}
extern "C" void jpeg_finish_compress(j_compress_ptr) {}

BOOL Util_CheckOption(char*, unsigned char, char*)
{
    return FALSE;
}

#endif
