#ifdef __ANDROID__

#include "stdafx.h"

#include "AndroidNetwork.h"
#include "SimpleModulusCrypt.h"
#include "WSclient.h"

#include <android/log.h>
#include <arpa/inet.h>
#include <errno.h>
#include <linux/tcp.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include <atomic>
#include <cstdint>
#include <cstring>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#define TAKUMI_PACKETINFO_COMPAT 1
struct PacketInfo { std::unique_ptr<BYTE[]> ReceiveBuffer; int32_t Size; };
static int g_MaxMessagePerCycle = 0;
static void ProcessPacketCallback(const PacketInfo*) {}

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

#define NET_TAG "MuNet"
#if defined(MU_ANDROID_DISABLE_LOG)
#define NET_LOGE(...) ((void)0)
#define NET_LOGI(...) ((void)0)
#define NET_LOGD(...) ((void)0)
#elif defined(MU_ANDROID_VERBOSE_NET_LOG)
#define NET_LOGE(...) __android_log_print(ANDROID_LOG_ERROR, NET_TAG, __VA_ARGS__)
#define NET_LOGI(...) __android_log_print(ANDROID_LOG_INFO, NET_TAG, __VA_ARGS__)
#define NET_LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, NET_TAG, __VA_ARGS__)
#else
#define NET_LOGE(...) __android_log_print(ANDROID_LOG_ERROR, NET_TAG, __VA_ARGS__)
#define NET_LOGI(...) ((void)0)
#define NET_LOGD(...) ((void)0)
#endif

#define MU_EXPORT extern "C" __attribute__((visibility("default")))

namespace
{
struct AndroidConnState
{
    std::shared_ptr<std::atomic<bool>> running;
    bool encrypted;
    std::thread recvThread;
    std::mutex statsMutex;
    uint64_t totalSentBytes;
    uint64_t totalRecvBytes;
    uint64_t lastSampleTimeMs;
    uint64_t lastSentBytes;
    uint64_t lastRecvBytes;
    float uploadKBps;
    float downloadKBps;

    explicit AndroidConnState(bool isEncrypted)
        : running(std::make_shared<std::atomic<bool>>(true))
        , encrypted(isEncrypted)
        , totalSentBytes(0)
        , totalRecvBytes(0)
        , lastSampleTimeMs(0)
        , lastSentBytes(0)
        , lastRecvBytes(0)
        , uploadKBps(0.0f)
        , downloadKBps(0.0f)
    {
    }
};

std::queue<PacketInfo*> g_packetQueue;
std::mutex g_packetMutex;

std::map<int32_t, AndroidConnState*> g_connections;
std::mutex g_connectionsMutex;

std::atomic<uint8_t> g_gameEncryptCounter { 0 };
std::atomic<uint8_t> g_gameDecryptCounter { 0 };
std::atomic<int> g_recvLogBudget { 1200 };

uint64_t GetMonotonicTimeMs()
{
    timespec ts {};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return static_cast<uint64_t>(ts.tv_sec) * 1000ull +
        static_cast<uint64_t>(ts.tv_nsec / 1000000ull);
}

AndroidConnState* FindConnectionState(int32_t handle)
{
    std::lock_guard<std::mutex> lock(g_connectionsMutex);
    const auto it = g_connections.find(handle);
    return (it != g_connections.end()) ? it->second : nullptr;
}

void UpdateTrafficSampleLocked(AndroidConnState& state, uint64_t nowMs)
{
    if (state.lastSampleTimeMs == 0)
    {
        state.lastSampleTimeMs = nowMs;
        state.lastSentBytes = state.totalSentBytes;
        state.lastRecvBytes = state.totalRecvBytes;
        return;
    }

    const uint64_t elapsedMs = nowMs - state.lastSampleTimeMs;
    if (elapsedMs < 500)
    {
        return;
    }

    const uint64_t sentDelta = state.totalSentBytes - state.lastSentBytes;
    const uint64_t recvDelta = state.totalRecvBytes - state.lastRecvBytes;

    state.uploadKBps = static_cast<float>(sentDelta) * 1000.0f /
        static_cast<float>(elapsedMs) / 1024.0f;
    state.downloadKBps = static_cast<float>(recvDelta) * 1000.0f /
        static_cast<float>(elapsedMs) / 1024.0f;

    state.lastSampleTimeMs = nowMs;
    state.lastSentBytes = state.totalSentBytes;
    state.lastRecvBytes = state.totalRecvBytes;
}

void RecordSentBytes(int32_t handle, size_t bytes)
{
    if (bytes == 0)
    {
        return;
    }

    AndroidConnState* state = FindConnectionState(handle);
    if (!state)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(state->statsMutex);
    state->totalSentBytes += static_cast<uint64_t>(bytes);
    UpdateTrafficSampleLocked(*state, GetMonotonicTimeMs());
}

void RecordRecvBytes(int32_t handle, size_t bytes)
{
    if (bytes == 0)
    {
        return;
    }

    AndroidConnState* state = FindConnectionState(handle);
    if (!state)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(state->statsMutex);
    state->totalRecvBytes += static_cast<uint64_t>(bytes);
    UpdateTrafficSampleLocked(*state, GetMonotonicTimeMs());
}

int QuerySocketLatencyMs(int32_t handle)
{
#ifdef TCP_INFO
    tcp_info info {};
    socklen_t infoSize = sizeof(info);
    if (handle > 0 &&
        getsockopt(handle, IPPROTO_TCP, TCP_INFO, &info, &infoSize) == 0 &&
        info.tcpi_rtt > 0)
    {
        return static_cast<int>((info.tcpi_rtt + 500u) / 1000u);
    }
#else
    (void)handle;
#endif

    return -1;
}

bool IsConnectionEncrypted(int32_t handle)
{
    std::lock_guard<std::mutex> lock(g_connectionsMutex);
    const auto it = g_connections.find(handle);
    return it != g_connections.end() && it->second && it->second->encrypted;
}

void WriteUInt16BigEndian(uint8_t* target, uint16_t value)
{
    if (!target)
    {
        return;
    }

    target[0] = static_cast<uint8_t>((value >> 8) & 0xFF);
    target[1] = static_cast<uint8_t>(value & 0xFF);
}

void WriteUInt16LittleEndian(uint8_t* target, uint16_t value)
{
    if (!target)
    {
        return;
    }

    target[0] = static_cast<uint8_t>(value & 0xFF);
    target[1] = static_cast<uint8_t>((value >> 8) & 0xFF);
}

void WriteUInt32LittleEndian(uint8_t* target, uint32_t value)
{
    if (!target)
    {
        return;
    }

    target[0] = static_cast<uint8_t>(value & 0xFF);
    target[1] = static_cast<uint8_t>((value >> 8) & 0xFF);
    target[2] = static_cast<uint8_t>((value >> 16) & 0xFF);
    target[3] = static_cast<uint8_t>((value >> 24) & 0xFF);
}

std::string EncodeUtf8Bounded(const wchar_t* text, size_t maxBytes)
{
    std::string out;
    if (!text || maxBytes == 0)
    {
        return out;
    }

    out.reserve(maxBytes);
    for (size_t i = 0; text[i] != L'\0'; ++i)
    {
        uint32_t codePoint = static_cast<uint32_t>(text[i]);
        char encoded[4] = {};
        size_t encodedLen = 0;

        if (codePoint <= 0x7F)
        {
            encoded[0] = static_cast<char>(codePoint);
            encodedLen = 1;
        }
        else if (codePoint <= 0x7FF)
        {
            encoded[0] = static_cast<char>(0xC0 | ((codePoint >> 6) & 0x1F));
            encoded[1] = static_cast<char>(0x80 | (codePoint & 0x3F));
            encodedLen = 2;
        }
        else if (codePoint <= 0xFFFF)
        {
            encoded[0] = static_cast<char>(0xE0 | ((codePoint >> 12) & 0x0F));
            encoded[1] = static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F));
            encoded[2] = static_cast<char>(0x80 | (codePoint & 0x3F));
            encodedLen = 3;
        }
        else if (codePoint <= 0x10FFFF)
        {
            encoded[0] = static_cast<char>(0xF0 | ((codePoint >> 18) & 0x07));
            encoded[1] = static_cast<char>(0x80 | ((codePoint >> 12) & 0x3F));
            encoded[2] = static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F));
            encoded[3] = static_cast<char>(0x80 | (codePoint & 0x3F));
            encodedLen = 4;
        }

        if (encodedLen == 0 || out.size() + encodedLen > maxBytes)
        {
            break;
        }

        out.append(encoded, encodedLen);
    }

    return out;
}

void SendGameEncrypted(int32_t handle, const uint8_t* plainPacket, size_t plainSize);

void SendCharacterSelectionPacket(int32_t handle, const wchar_t* name, uint8_t subCode, const char* logLabel)
{
    uint8_t packet[14] = { 0xC1, 0x0E, 0xF3, subCode };
    for (int i = 0; name && name[i] && i < 10; ++i)
    {
        packet[4 + i] = static_cast<uint8_t>(name[i] & 0xFF);
    }

    NET_LOGI(
        "[GS fd=%d] %s name=%ls sub=0x%02X",
        handle,
        logLabel ? logLabel : "SendCharacterSelectionPacket",
        name ? name : L"(null)",
        static_cast<unsigned int>(subCode));
    packet[1] = 14;
    SendGameEncrypted(handle, packet, 14);
}

void BuxConvertLocal(uint8_t* buffer, int length)
{
    static constexpr uint8_t key[3] = { 0xFC, 0xCF, 0xAB };
    for (int i = 0; i < length; ++i)
    {
        buffer[i] ^= key[i % 3];
    }
}

void RawSend(int32_t handle, const uint8_t* data, size_t size)
{
    if (handle <= 0 || !data || size == 0)
    {
        return;
    }

    size_t sent = 0;
    while (sent < size)
    {
        const ssize_t current = send(handle, data + sent, size - sent, MSG_NOSIGNAL);
        if (current <= 0)
        {
            NET_LOGE("[fd=%d] send failed: current=%zd errno=%d", handle, current, errno);
            return;
        }

        sent += static_cast<size_t>(current);
        RecordSentBytes(handle, static_cast<size_t>(current));
    }
}

void RecvLoop(
    int32_t handle,
    bool isEncrypted,
    const std::shared_ptr<std::atomic<bool>>& running,
    void(*onPacket)(int32_t, int32_t, uint8_t*),
    void(*onDisconnect)(int32_t))
{
    uint8_t readBuffer[4096];
    NET_LOGI("[fd=%d] recv thread started", handle);
    (void)isEncrypted;

    while (running->load())
    {
        const int count = recv(handle, readBuffer, sizeof(readBuffer), 0);
        if (count <= 0)
        {
            if (count < 0 && errno == EINTR)
            {
                continue;
            }

            NET_LOGE("[fd=%d] recv failed: count=%d errno=%d", handle, count, errno);
            break;
        }

        RecordRecvBytes(handle, static_cast<size_t>(count));
        const int budgetBefore = g_recvLogBudget.fetch_sub(1);
        if (budgetBefore > 0)
        {
            NET_LOGD(
                "[fd=%d] recv chunk size=%d b0=0x%02X b1=0x%02X b2=0x%02X b3=0x%02X",
                handle,
                count,
                count > 0 ? readBuffer[0] : 0,
                count > 1 ? readBuffer[1] : 0,
                count > 2 ? readBuffer[2] : 0,
                count > 3 ? readBuffer[3] : 0);
        }
        else if (budgetBefore == 0)
        {
            NET_LOGD("[fd=%d] recv log budget exhausted", handle);
        }

        if (onPacket)
        {
            onPacket(handle, count, readBuffer);
        }
    }

    NET_LOGI("[fd=%d] recv thread stopped", handle);
    if (onDisconnect)
    {
        onDisconnect(handle);
    }
}

void SendGameEncrypted(int32_t handle, const uint8_t* plainPacket, size_t plainSize)
{
    if (!plainPacket || plainSize < 3)
    {
        return;
    }

    std::vector<uint8_t> plain(plainPacket, plainPacket + plainSize);
    const uint8_t packetType = plain[0];
    int headerSize = 0;
    if (packetType == 0xC1 || packetType == 0xC3)
    {
        headerSize = 2;
    }
    else if (packetType == 0xC2 || packetType == 0xC4)
    {
        headerSize = 3;
    }
    else
    {
        NET_LOGE("[GS fd=%d] send rejected invalid packet type=0x%02X", handle, static_cast<unsigned int>(packetType));
        return;
    }

    mu::Xor32Encrypt(plain.data(), static_cast<int>(plain.size()), headerSize);
    if (packetType >= 0xC3)
    {
        const uint8_t counter = g_gameEncryptCounter.fetch_add(1);
        const std::vector<uint8_t> encrypted = mu::SM_EncryptPacket(plain, counter);
        NET_LOGD(
            "[GS fd=%d] send enc type=0x%02X code=0x%02X plain=%zu enc=%zu ctr=%u",
            handle,
            static_cast<unsigned int>(packetType),
            plainSize > 2 ? plainPacket[2] : 0,
            plainSize,
            encrypted.size(),
            static_cast<unsigned int>(counter));
        RawSend(handle, encrypted.data(), encrypted.size());
    }
    else
    {
        NET_LOGD(
            "[GS fd=%d] send xor type=0x%02X code=0x%02X size=%zu",
            handle,
            static_cast<unsigned int>(packetType),
            plainSize > 2 ? plainPacket[2] : 0,
            plainSize);
        RawSend(handle, plain.data(), plain.size());
    }
}
} // namespace

void AndroidPostPacket(void* packetInfoPtr)
{
    if (!packetInfoPtr)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(g_packetMutex);
    g_packetQueue.push(static_cast<PacketInfo*>(packetInfoPtr));
}

void AndroidDrainPackets()
{
    int processed = 0;
    const int maxPerCycle = (g_MaxMessagePerCycle > 0) ? g_MaxMessagePerCycle : -1;
    for (;;)
    {
        if (maxPerCycle > 0 && processed >= maxPerCycle)
        {
            break;
        }

        PacketInfo* rawPacket = nullptr;
        {
            std::lock_guard<std::mutex> lock(g_packetMutex);
            if (g_packetQueue.empty())
            {
                break;
            }

            rawPacket = g_packetQueue.front();
            g_packetQueue.pop();
        }

        std::unique_ptr<PacketInfo> packet(rawPacket);
        ProcessPacketCallback(packet.get());
        ++processed;
    }
}

MU_EXPORT int32_t ConnectionManager_Connect(
    const wchar_t* host,
    int32_t port,
    BYTE isEncrypted,
    void(*onPacket)(int32_t, int32_t, uint8_t*),
    void(*onDisconnect)(int32_t))
{
    if (!host || port <= 0)
    {
        return 0;
    }

    char hostAscii[256] = {};
    for (int i = 0; host[i] && i < 255; ++i)
    {
        hostAscii[i] = static_cast<char>(host[i] & 0xFF);
    }

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd <= 0)
    {
        NET_LOGE("socket failed errno=%d", errno);
        return 0;
    }

    sockaddr_in address {};
    address.sin_family = AF_INET;
    address.sin_port = htons(static_cast<uint16_t>(port));
    if (inet_pton(AF_INET, hostAscii, &address.sin_addr) != 1)
    {
        NET_LOGE("inet_pton failed for host=%s", hostAscii);
        close(fd);
        return 0;
    }

    NET_LOGI("connect start %s:%d", hostAscii, port);
    if (connect(fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0)
    {
        NET_LOGE("connect failed %s:%d errno=%d", hostAscii, port, errno);
        close(fd);
        return 0;
    }

    auto* state = new AndroidConnState(isEncrypted != 0);
    {
        std::lock_guard<std::mutex> statsLock(state->statsMutex);
        UpdateTrafficSampleLocked(*state, GetMonotonicTimeMs());
    }

    {
        std::lock_guard<std::mutex> lock(g_connectionsMutex);
        g_connections[fd] = state;
    }

    const auto runningFlag = state->running;
    const bool encrypted = state->encrypted;
    state->recvThread = std::thread([fd, encrypted, runningFlag, onPacket, onDisconnect]()
    {
        RecvLoop(fd, encrypted, runningFlag, onPacket, onDisconnect);
    });

    if (encrypted)
    {
        g_gameEncryptCounter.store(0);
        g_gameDecryptCounter.store(0);
    }

    NET_LOGI("connect success %s:%d fd=%d", hostAscii, port, fd);
    return fd;
}

MU_EXPORT void ConnectionManager_Disconnect(int32_t handle)
{
    if (handle <= 0)
    {
        return;
    }

    AndroidConnState* state = nullptr;
    {
        std::lock_guard<std::mutex> lock(g_connectionsMutex);
        const auto it = g_connections.find(handle);
        if (it != g_connections.end())
        {
            state = it->second;
            g_connections.erase(it);
        }
    }

    if (state)
    {
        state->running->store(false);
        shutdown(handle, SHUT_RDWR);
        close(handle);

        if (state->recvThread.joinable())
        {
            state->recvThread.join();
        }

        delete state;
        return;
    }

    close(handle);
}

MU_EXPORT void ConnectionManager_BeginReceive(int32_t /*handle*/)
{
}

MU_EXPORT void ConnectionManager_Send(int32_t handle, const uint8_t* data, int32_t size)
{
    if (!data || size <= 0)
    {
        return;
    }

    if (IsConnectionEncrypted(handle))
    {
        SendGameEncrypted(handle, data, static_cast<size_t>(size));
    }
    else
    {
        RawSend(handle, data, static_cast<size_t>(size));
    }
}

AndroidNetworkOverlayStats AndroidQueryNetworkOverlayStats(int32_t handle)
{
    AndroidNetworkOverlayStats stats {};
    if (handle <= 0)
    {
        return stats;
    }

    AndroidConnState* state = FindConnectionState(handle);
    if (!state)
    {
        return stats;
    }

    stats.connected = true;
    stats.latencyMs = QuerySocketLatencyMs(handle);

    {
        std::lock_guard<std::mutex> lock(state->statsMutex);
        UpdateTrafficSampleLocked(*state, GetMonotonicTimeMs());
        stats.uploadKBps = state->uploadKBps;
        stats.downloadKBps = state->downloadKBps;
    }

    return stats;
}

MU_EXPORT void ConnectionManager_SendLogin(
    int32_t handle,
    const wchar_t* username,
    const wchar_t* password,
    uint32_t tickCount,
    const BYTE* clientVersion,
    const BYTE* clientSerial)
{
    if (handle <= 0 || !username || !password || !clientVersion || !clientSerial)
    {
        return;
    }

    constexpr size_t plainSize = 60;
    uint8_t plain[plainSize] = {};
    plain[0] = 0xC3;
    plain[1] = static_cast<uint8_t>(plainSize);
    plain[2] = 0xF1;
    plain[3] = 0x01;

    for (int i = 0; username[i] && i < 10; ++i)
    {
        plain[4 + i] = static_cast<uint8_t>(username[i] & 0xFF);
    }
    BuxConvertLocal(&plain[4], 10);

    for (int i = 0; password[i] && i < 20; ++i)
    {
        plain[14 + i] = static_cast<uint8_t>(password[i] & 0xFF);
    }
    BuxConvertLocal(&plain[14], 20);

    uint32_t tick = tickCount;
    if (tick == 0)
    {
        timespec ts {};
        clock_gettime(CLOCK_MONOTONIC, &ts);
        tick = static_cast<uint32_t>((ts.tv_sec * 1000ull + ts.tv_nsec / 1000000ull) & 0xFFFFFFFFu);
    }

    plain[34] = static_cast<uint8_t>((tick >> 24) & 0xFF);
    plain[35] = static_cast<uint8_t>((tick >> 16) & 0xFF);
    plain[36] = static_cast<uint8_t>((tick >> 8) & 0xFF);
    plain[37] = static_cast<uint8_t>(tick & 0xFF);

    std::memcpy(&plain[38], clientVersion, 5);
    std::memcpy(&plain[43], clientSerial, 16);

    NET_LOGI("[GS fd=%d] SendLogin user=%ls plain=%zu", handle, username, plainSize);
    SendGameEncrypted(handle, plain, plainSize);
}

MU_EXPORT void SendServerListRequest(int32_t handle)
{
    const uint8_t packet[] = { 0xC1, 0x04, 0xF4, 0x06 };
    RawSend(handle, packet, sizeof(packet));
}

MU_EXPORT void SendConnectionInfoRequest075(int32_t handle, BYTE serverId)
{
    const uint8_t packet[] = { 0xC1, 0x05, 0xF4, 0x03, serverId };
    RawSend(handle, packet, sizeof(packet));
}

MU_EXPORT void SendConnectionInfoRequest(int32_t handle, uint16_t serverCode)
{
    const uint8_t packet[] = {
        0xC1,
        0x06,
        0xF4,
        0x03,
        static_cast<uint8_t>(serverCode & 0xFF),
        static_cast<uint8_t>((serverCode >> 8) & 0xFF)
    };
    RawSend(handle, packet, sizeof(packet));
}

MU_EXPORT void SendConnectionInfo(int32_t /*handle*/, const wchar_t* /*ipAddress*/, uint16_t /*port*/)
{
}

MU_EXPORT void SendServerListRequestOld(int32_t handle)
{
    SendServerListRequest(handle);
}

MU_EXPORT void SendHello(int32_t /*handle*/)
{
}

MU_EXPORT void SendPatchCheckRequest(int32_t /*handle*/, BYTE /*major*/, BYTE /*minor*/, BYTE /*patch*/)
{
}

MU_EXPORT void SendPatchVersionOkay(int32_t /*handle*/)
{
}

MU_EXPORT void SendClientNeedsPatch(int32_t /*handle*/, BYTE /*patchVersion*/, const wchar_t* /*patchAddress*/)
{
}

MU_EXPORT void SendRequestCharacterList(int32_t handle, BYTE language)
{
    const uint8_t packet[] = { 0xC1, 0x05, 0xF3, 0x00, language };
    SendGameEncrypted(handle, packet, sizeof(packet));
}

MU_EXPORT void SendCreateCharacter(int32_t handle, const wchar_t* name, uint32_t characterClass)
{
    uint8_t packet[15] = { 0xC1, 0x0F, 0xF3, 0x01 };
    for (int i = 0; name && name[i] && i < 10; ++i)
    {
        packet[4 + i] = static_cast<uint8_t>(name[i] & 0xFF);
    }

    // OpenMU encodes this field as 6 bits at positions 2..7 (Class << 2).
    // `characterClass` here is the CharacterClassNumber enum value (e.g. 0, 4, 8, 12, ...).
    // Sending it raw would make the server decode a wrong class for all non-zero values.
    packet[14] = static_cast<uint8_t>((characterClass & 0x3F) << 2);
    SendGameEncrypted(handle, packet, sizeof(packet));
}

MU_EXPORT void SendDeleteCharacter(int32_t /*handle*/, const wchar_t* /*name*/, const wchar_t* /*securityCode*/)
{
}

MU_EXPORT void SendPickupItemRequest(int32_t handle, uint16_t itemId)
{
    uint8_t packet[] = { 0xC3, 0x05, 0x22, 0x00, 0x00 };
    WriteUInt16BigEndian(packet + 3, itemId);
    NET_LOGI("[GS fd=%d] SendPickupItemRequest itemId=%u", handle, static_cast<unsigned int>(itemId));
    SendGameEncrypted(handle, packet, sizeof(packet));
}

MU_EXPORT void SendPickupItemRequest075(int32_t handle, uint16_t itemId)
{
    uint8_t packet[] = { 0xC1, 0x05, 0x22, 0x00, 0x00 };
    WriteUInt16BigEndian(packet + 3, itemId);
    NET_LOGI("[GS fd=%d] SendPickupItemRequest075 itemId=%u", handle, static_cast<unsigned int>(itemId));
    SendGameEncrypted(handle, packet, sizeof(packet));
}

MU_EXPORT void SendDropItemRequest(int32_t handle, BYTE targetX, BYTE targetY, BYTE itemSlot)
{
    const uint8_t packet[] = { 0xC3, 0x06, 0x23, targetX, targetY, itemSlot };
    NET_LOGI(
        "[GS fd=%d] SendDropItemRequest pos=(%u,%u) slot=%u",
        handle,
        static_cast<unsigned int>(targetX),
        static_cast<unsigned int>(targetY),
        static_cast<unsigned int>(itemSlot));
    SendGameEncrypted(handle, packet, sizeof(packet));
}

MU_EXPORT void SendItemMoveRequest(
    int32_t handle,
    uint32_t fromStorage,
    BYTE fromSlot,
    const BYTE* itemData,
    uint32_t itemDataByteLength,
    uint32_t toStorage,
    BYTE toSlot)
{
    uint8_t packet[19] = {};
    packet[0] = 0xC3;
    packet[1] = sizeof(packet);
    packet[2] = 0x24;
    packet[3] = static_cast<uint8_t>(fromStorage & 0xFF);
    packet[4] = fromSlot;

    if (itemData && itemDataByteLength > 0)
    {
        const size_t copySize = std::min<size_t>(12, static_cast<size_t>(itemDataByteLength));
        std::memcpy(packet + 5, itemData, copySize);
    }

    packet[17] = static_cast<uint8_t>(toStorage & 0xFF);
    packet[18] = toSlot;

    NET_LOGI(
        "[GS fd=%d] SendItemMoveRequest fromStorage=%u fromSlot=%u toStorage=%u toSlot=%u itemBytes=%u",
        handle,
        static_cast<unsigned int>(fromStorage),
        static_cast<unsigned int>(fromSlot),
        static_cast<unsigned int>(toStorage),
        static_cast<unsigned int>(toSlot),
        static_cast<unsigned int>(itemDataByteLength));
    SendGameEncrypted(handle, packet, sizeof(packet));
}

MU_EXPORT void SendItemMoveRequestExtended(int32_t handle, uint32_t fromStorage, BYTE fromSlot, uint32_t toStorage, BYTE toSlot)
{
    const uint8_t packet[] = {
        0xC3,
        0x07,
        0x24,
        static_cast<uint8_t>(fromStorage & 0xFF),
        fromSlot,
        static_cast<uint8_t>(toStorage & 0xFF),
        toSlot,
    };
    NET_LOGI(
        "[GS fd=%d] SendItemMoveRequestExtended fromStorage=%u fromSlot=%u toStorage=%u toSlot=%u",
        handle,
        static_cast<unsigned int>(fromStorage),
        static_cast<unsigned int>(fromSlot),
        static_cast<unsigned int>(toStorage),
        static_cast<unsigned int>(toSlot));
    SendGameEncrypted(handle, packet, sizeof(packet));
}

MU_EXPORT void SendConsumeItemRequest(int32_t handle, BYTE itemSlot, BYTE targetSlot, uint32_t fruitConsumption)
{
    const uint8_t packet[] = {
        0xC3,
        0x06,
        0x26,
        itemSlot,
        targetSlot,
        static_cast<uint8_t>(fruitConsumption & 0xFF),
    };
    NET_LOGI(
        "[GS fd=%d] SendConsumeItemRequest itemSlot=%u targetSlot=%u fruit=%u",
        handle,
        static_cast<unsigned int>(itemSlot),
        static_cast<unsigned int>(targetSlot),
        static_cast<unsigned int>(fruitConsumption));
    SendGameEncrypted(handle, packet, sizeof(packet));
}

MU_EXPORT void SendConsumeItemRequest075(int32_t handle, BYTE itemSlot, BYTE targetSlot)
{
    const uint8_t packet[] = { 0xC1, 0x05, 0x26, itemSlot, targetSlot };
    NET_LOGI(
        "[GS fd=%d] SendConsumeItemRequest075 itemSlot=%u targetSlot=%u",
        handle,
        static_cast<unsigned int>(itemSlot),
        static_cast<unsigned int>(targetSlot));
    SendGameEncrypted(handle, packet, sizeof(packet));
}

MU_EXPORT void SendWarpCommandRequest(int32_t handle, uint32_t commandKey, uint16_t warpInfoIndex)
{
    // Packet spec (pre S6): C1 0A 8E 02 <commandKey:LE32> <warpInfoIndex:LE16>
    uint8_t packet[] = { 0xC1, 0x0A, 0x8E, 0x02, 0, 0, 0, 0, 0, 0 };
    WriteUInt32LittleEndian(packet + 4, commandKey);
    WriteUInt16LittleEndian(packet + 8, warpInfoIndex);
    NET_LOGI(
        "[GS fd=%d] SendWarpCommandRequest key=%u warpInfoIndex=%u",
        handle,
        static_cast<unsigned int>(commandKey),
        static_cast<unsigned int>(warpInfoIndex));
    SendGameEncrypted(handle, packet, sizeof(packet));
}

MU_EXPORT void SendEnterGateRequest(int32_t handle, uint16_t gateNumber, BYTE teleportTargetX, BYTE teleportTargetY)
{
    // Packet spec: C3 08 1C 00 <gateNumber:LE16> <x> <y>
    uint8_t packet[] = { 0xC3, 0x08, 0x1C, 0x00, 0x00, 0x00, teleportTargetX, teleportTargetY };
    WriteUInt16LittleEndian(packet + 4, gateNumber);
    NET_LOGI(
        "[GS fd=%d] SendEnterGateRequest gate=%u pos=(%u,%u)",
        handle,
        static_cast<unsigned int>(gateNumber),
        static_cast<unsigned int>(teleportTargetX),
        static_cast<unsigned int>(teleportTargetY));
    SendGameEncrypted(handle, packet, sizeof(packet));
}

MU_EXPORT void SendEnterGateRequest075(int32_t handle, BYTE gateNumber, BYTE teleportTargetX, BYTE teleportTargetY)
{
    // Legacy 0.75 format.
    const uint8_t packet[] = { 0xC3, 0x06, 0x1C, gateNumber, teleportTargetX, teleportTargetY };
    NET_LOGI(
        "[GS fd=%d] SendEnterGateRequest075 gate=%u pos=(%u,%u)",
        handle,
        static_cast<unsigned int>(gateNumber),
        static_cast<unsigned int>(teleportTargetX),
        static_cast<unsigned int>(teleportTargetY));
    SendGameEncrypted(handle, packet, sizeof(packet));
}

MU_EXPORT void SendTeleportTarget(int32_t handle, uint16_t targetId, BYTE teleportTargetX, BYTE teleportTargetY)
{
    // Packet spec: C3 07 B0 <targetId:LE16> <x> <y>
    uint8_t packet[] = { 0xC3, 0x07, 0xB0, 0x00, 0x00, teleportTargetX, teleportTargetY };
    WriteUInt16LittleEndian(packet + 3, targetId);
    NET_LOGI(
        "[GS fd=%d] SendTeleportTarget target=%u pos=(%u,%u)",
        handle,
        static_cast<unsigned int>(targetId),
        static_cast<unsigned int>(teleportTargetX),
        static_cast<unsigned int>(teleportTargetY));
    SendGameEncrypted(handle, packet, sizeof(packet));
}

MU_EXPORT void SendServerChangeAuthentication(
    int32_t handle,
    const BYTE* accountXor3,
    uint32_t accountXor3ByteLength,
    const BYTE* characterNameXor3,
    uint32_t characterNameXor3ByteLength,
    uint32_t authCode1,
    uint32_t authCode2,
    uint32_t authCode3,
    uint32_t authCode4,
    uint32_t tickCount,
    const BYTE* clientVersion,
    uint32_t clientVersionByteLength,
    const BYTE* clientSerial,
    uint32_t clientSerialByteLength)
{
    // Packet spec: C3 45 B1 01
    // [4..15]=AccountXor3(12), [16..27]=CharacterNameXor3(12)
    // [28..47]=Auth/Tick (LE32), [48..52]=Version(5), [53..68]=Serial(16)
    uint8_t packet[69] = {};
    packet[0] = 0xC3;
    packet[1] = static_cast<uint8_t>(sizeof(packet));
    packet[2] = 0xB1;
    packet[3] = 0x01;

    if (accountXor3 && accountXor3ByteLength > 0)
    {
        const size_t copySize = std::min<size_t>(12u, static_cast<size_t>(accountXor3ByteLength));
        std::memcpy(packet + 4, accountXor3, copySize);
    }
    if (characterNameXor3 && characterNameXor3ByteLength > 0)
    {
        const size_t copySize = std::min<size_t>(12u, static_cast<size_t>(characterNameXor3ByteLength));
        std::memcpy(packet + 16, characterNameXor3, copySize);
    }

    WriteUInt32LittleEndian(packet + 28, authCode1);
    WriteUInt32LittleEndian(packet + 32, authCode2);
    WriteUInt32LittleEndian(packet + 36, authCode3);
    WriteUInt32LittleEndian(packet + 40, authCode4);
    WriteUInt32LittleEndian(packet + 44, tickCount);

    if (clientVersion && clientVersionByteLength > 0)
    {
        const size_t copySize = std::min<size_t>(5u, static_cast<size_t>(clientVersionByteLength));
        std::memcpy(packet + 48, clientVersion, copySize);
    }
    if (clientSerial && clientSerialByteLength > 0)
    {
        const size_t copySize = std::min<size_t>(16u, static_cast<size_t>(clientSerialByteLength));
        std::memcpy(packet + 53, clientSerial, copySize);
    }

    NET_LOGI(
        "[GS fd=%d] SendServerChangeAuthentication accLen=%u charLen=%u tick=%u",
        handle,
        static_cast<unsigned int>(accountXor3ByteLength),
        static_cast<unsigned int>(characterNameXor3ByteLength),
        static_cast<unsigned int>(tickCount));
    SendGameEncrypted(handle, packet, sizeof(packet));
}

MU_EXPORT void SendSelectCharacter(int32_t handle, const wchar_t* name)
{
    // Step 1: focus/select acknowledgement.
    SendCharacterSelectionPacket(handle, name, 0x15, "SendSelectCharacter focus-step");
}

MU_EXPORT void SendFocusCharacter(int32_t handle, const wchar_t* name)
{
    // Step 2: enter world with the selected character.
    SendCharacterSelectionPacket(handle, name, 0x03, "SendFocusCharacter enter-step");
}

MU_EXPORT void SendWalkRequest(
    int32_t handle,
    BYTE sourceX,
    BYTE sourceY,
    BYTE stepCount,
    BYTE targetRotation,
    const BYTE* directions,
    uint32_t directionsByteLength)
{
    const uint32_t directionCount = std::min<uint32_t>(directionsByteLength, 249u);
    std::vector<uint8_t> packet(static_cast<size_t>(6u + directionCount), 0);
    packet[0] = 0xC1;
    packet[1] = static_cast<uint8_t>(packet.size());
    packet[2] = 0xD4;
    packet[3] = sourceX;
    packet[4] = sourceY;
    packet[5] = static_cast<uint8_t>((stepCount & 0x0F) | ((targetRotation & 0x0F) << 4));
    if (directions && directionCount > 0)
    {
        std::memcpy(packet.data() + 6, directions, directionCount);
    }

    NET_LOGD(
        "[GS fd=%d] SendWalkRequest src=(%u,%u) steps=%u dirCount=%u rot=%u",
        handle,
        static_cast<unsigned int>(sourceX),
        static_cast<unsigned int>(sourceY),
        static_cast<unsigned int>(stepCount),
        static_cast<unsigned int>(directionCount),
        static_cast<unsigned int>(targetRotation));
    SendGameEncrypted(handle, packet.data(), packet.size());
}

MU_EXPORT void SendWalkRequest075(
    int32_t handle,
    BYTE sourceX,
    BYTE sourceY,
    BYTE stepCount,
    BYTE targetRotation,
    const BYTE* directions,
    uint32_t directionsByteLength)
{
    const uint32_t directionCount = std::min<uint32_t>(directionsByteLength, 249u);
    std::vector<uint8_t> packet(static_cast<size_t>(6u + directionCount), 0);
    packet[0] = 0xC1;
    packet[1] = static_cast<uint8_t>(packet.size());
    packet[2] = 0x10;
    packet[3] = sourceX;
    packet[4] = sourceY;
    packet[5] = static_cast<uint8_t>((stepCount & 0x0F) | ((targetRotation & 0x0F) << 4));
    if (directions && directionCount > 0)
    {
        std::memcpy(packet.data() + 6, directions, directionCount);
    }

    SendGameEncrypted(handle, packet.data(), packet.size());
}

MU_EXPORT void SendInstantMoveRequest(int32_t handle, BYTE targetX, BYTE targetY)
{
    const uint8_t packet[] = { 0xC1, 0x05, 0x15, targetX, targetY };
    SendGameEncrypted(handle, packet, sizeof(packet));
}

MU_EXPORT void SendAnimationRequest(int32_t handle, BYTE rotation, BYTE animationNumber)
{
    const uint8_t packet[] = { 0xC1, 0x05, 0x18, rotation, animationNumber };
    SendGameEncrypted(handle, packet, sizeof(packet));
}

MU_EXPORT void SendHitRequest(int32_t handle, uint16_t targetId, BYTE attackAnimation, BYTE lookingDirection)
{
    uint8_t packet[] = { 0xC1, 0x07, 0x11, 0, 0, attackAnimation, lookingDirection };
    WriteUInt16BigEndian(packet + 3, targetId);
    SendGameEncrypted(handle, packet, sizeof(packet));
}

MU_EXPORT void SendTargetedSkill(int32_t handle, uint16_t skillId, uint16_t targetId)
{
    uint8_t packet[] = { 0xC3, 0x07, 0x19, 0, 0, 0, 0 };
    WriteUInt16BigEndian(packet + 3, skillId);
    WriteUInt16BigEndian(packet + 5, targetId);
    NET_LOGI(
        "[GS fd=%d] SendTargetedSkill skillId=%u targetId=%u",
        handle,
        static_cast<unsigned int>(skillId),
        static_cast<unsigned int>(targetId));
    SendGameEncrypted(handle, packet, sizeof(packet));
}

MU_EXPORT void SendTargetedSkill075(int32_t handle, BYTE skillIndex, uint16_t targetId)
{
    uint8_t packet[] = { 0xC1, 0x06, 0x19, skillIndex, 0, 0 };
    WriteUInt16BigEndian(packet + 4, targetId);
    NET_LOGI(
        "[GS fd=%d] SendTargetedSkill075 skillIndex=%u targetId=%u",
        handle,
        static_cast<unsigned int>(skillIndex),
        static_cast<unsigned int>(targetId));
    SendGameEncrypted(handle, packet, sizeof(packet));
}

MU_EXPORT void SendTargetedSkill095(int32_t handle, BYTE skillIndex, uint16_t targetId)
{
    uint8_t packet[] = { 0xC3, 0x06, 0x19, skillIndex, 0, 0 };
    WriteUInt16BigEndian(packet + 4, targetId);
    NET_LOGI(
        "[GS fd=%d] SendTargetedSkill095 skillIndex=%u targetId=%u",
        handle,
        static_cast<unsigned int>(skillIndex),
        static_cast<unsigned int>(targetId));
    SendGameEncrypted(handle, packet, sizeof(packet));
}

MU_EXPORT void SendMagicEffectCancelRequest(int32_t handle, uint16_t skillId, uint16_t playerId)
{
    uint8_t packet[] = { 0xC1, 0x07, 0x1B, 0, 0, 0, 0 };
    WriteUInt16BigEndian(packet + 3, skillId);
    WriteUInt16BigEndian(packet + 5, playerId);
    SendGameEncrypted(handle, packet, sizeof(packet));
}

MU_EXPORT void SendAreaSkill(
    int32_t handle,
    uint16_t skillId,
    BYTE targetX,
    BYTE targetY,
    BYTE rotation,
    uint16_t extraTargetId,
    BYTE animationCounter)
{
    uint8_t packet[] = { 0xC3, 0x0D, 0x1E, 0, 0, targetX, targetY, rotation, 0, 0, 0, 0, animationCounter };
    WriteUInt16BigEndian(packet + 3, skillId);
    WriteUInt16BigEndian(packet + 10, extraTargetId);
    NET_LOGI(
        "[GS fd=%d] SendAreaSkill skillId=%u pos=(%u,%u) rot=%u extraTarget=%u anim=%u",
        handle,
        static_cast<unsigned int>(skillId),
        static_cast<unsigned int>(targetX),
        static_cast<unsigned int>(targetY),
        static_cast<unsigned int>(rotation),
        static_cast<unsigned int>(extraTargetId),
        static_cast<unsigned int>(animationCounter));
    SendGameEncrypted(handle, packet, sizeof(packet));
}

MU_EXPORT void SendAreaSkill075(int32_t handle, BYTE skillIndex, BYTE targetX, BYTE targetY, BYTE rotation)
{
    const uint8_t packet[] = { 0xC1, 0x07, 0x1E, skillIndex, targetX, targetY, rotation };
    NET_LOGI(
        "[GS fd=%d] SendAreaSkill075 skillIndex=%u pos=(%u,%u) rot=%u",
        handle,
        static_cast<unsigned int>(skillIndex),
        static_cast<unsigned int>(targetX),
        static_cast<unsigned int>(targetY),
        static_cast<unsigned int>(rotation));
    SendGameEncrypted(handle, packet, sizeof(packet));
}

MU_EXPORT void SendAreaSkill095(int32_t handle, BYTE skillIndex, BYTE targetX, BYTE targetY, BYTE rotation)
{
    const uint8_t packet[] = { 0xC3, 0x07, 0x1E, skillIndex, targetX, targetY, rotation };
    NET_LOGI(
        "[GS fd=%d] SendAreaSkill095 skillIndex=%u pos=(%u,%u) rot=%u",
        handle,
        static_cast<unsigned int>(skillIndex),
        static_cast<unsigned int>(targetX),
        static_cast<unsigned int>(targetY),
        static_cast<unsigned int>(rotation));
    SendGameEncrypted(handle, packet, sizeof(packet));
}

MU_EXPORT void SendRageAttackRequest(int32_t handle, uint16_t skillId, uint16_t targetId)
{
    uint8_t packet[] = { 0xC3, 0x08, 0x4A, 0, 0, 0, 0, 0 };
    WriteUInt16BigEndian(packet + 3, skillId);
    WriteUInt16BigEndian(packet + 6, targetId);
    SendGameEncrypted(handle, packet, sizeof(packet));
}

MU_EXPORT void SendRageAttackRangeRequest(int32_t handle, uint16_t skillId, uint16_t targetId)
{
    uint8_t packet[] = { 0xC1, 0x07, 0x4B, 0, 0, 0, 0 };
    WriteUInt16BigEndian(packet + 3, skillId);
    WriteUInt16BigEndian(packet + 5, targetId);
    SendGameEncrypted(handle, packet, sizeof(packet));
}

MU_EXPORT void SendPing(int32_t /*handle*/, uint32_t /*tickCount*/, uint16_t /*attackSpeed*/)
{
}

MU_EXPORT void SendChecksumResponse(int32_t /*handle*/, uint32_t /*checksum*/)
{
}

MU_EXPORT void SendPublicChatMessage(int32_t handle, const wchar_t* character, const wchar_t* message)
{
    constexpr size_t kNameLength = 10;
    constexpr size_t kHeaderAndNameLength = 13; // C1 + size + code + 10-byte character
    constexpr size_t kMaxPacketLength = 0xFF;
    constexpr size_t kMaxMessageLength = kMaxPacketLength - kHeaderAndNameLength;

    const std::string characterUtf8 = EncodeUtf8Bounded(character, kNameLength);
    const std::string messageUtf8 = EncodeUtf8Bounded(message, kMaxMessageLength);
    const size_t packetLength = kHeaderAndNameLength + messageUtf8.size();

    std::vector<uint8_t> packet(packetLength, 0);
    packet[0] = 0xC1;
    packet[1] = static_cast<uint8_t>(packetLength & 0xFF);
    packet[2] = 0x00;
    if (!characterUtf8.empty())
    {
        std::memcpy(packet.data() + 3, characterUtf8.data(), characterUtf8.size());
    }
    if (!messageUtf8.empty())
    {
        std::memcpy(packet.data() + 13, messageUtf8.data(), messageUtf8.size());
    }

    NET_LOGI(
        "[GS fd=%d] SendPublicChatMessage nameLen=%u msgLen=%u",
        handle,
        static_cast<unsigned int>(characterUtf8.size()),
        static_cast<unsigned int>(messageUtf8.size()));
    SendGameEncrypted(handle, packet.data(), packet.size());
}

MU_EXPORT void SendWhisperMessage(int32_t handle, const wchar_t* receiverName, const wchar_t* message)
{
    constexpr size_t kNameLength = 10;
    constexpr size_t kHeaderAndNameLength = 13; // C1 + size + code + 10-byte receiver
    constexpr size_t kMaxPacketLength = 0xFF;
    constexpr size_t kMaxMessageLength = kMaxPacketLength - kHeaderAndNameLength;

    const std::string receiverUtf8 = EncodeUtf8Bounded(receiverName, kNameLength);
    const std::string messageUtf8 = EncodeUtf8Bounded(message, kMaxMessageLength);
    const size_t packetLength = kHeaderAndNameLength + messageUtf8.size();

    std::vector<uint8_t> packet(packetLength, 0);
    packet[0] = 0xC1;
    packet[1] = static_cast<uint8_t>(packetLength & 0xFF);
    packet[2] = 0x02;
    if (!receiverUtf8.empty())
    {
        std::memcpy(packet.data() + 3, receiverUtf8.data(), receiverUtf8.size());
    }
    if (!messageUtf8.empty())
    {
        std::memcpy(packet.data() + 13, messageUtf8.data(), messageUtf8.size());
    }

    NET_LOGI(
        "[GS fd=%d] SendWhisperMessage recvLen=%u msgLen=%u",
        handle,
        static_cast<unsigned int>(receiverUtf8.size()),
        static_cast<unsigned int>(messageUtf8.size()));
    SendGameEncrypted(handle, packet.data(), packet.size());
}

MU_EXPORT void SendLogOut(int32_t handle, uint32_t type)
{
    const uint8_t packet[] = { 0xC1, 0x05, 0xF1, 0x02, static_cast<uint8_t>(type & 0xFF) };
    SendGameEncrypted(handle, packet, sizeof(packet));
}

MU_EXPORT void SendLogOutByCheatDetection(int32_t /*handle*/, BYTE /*param*/, BYTE /*type*/)
{
}

MU_EXPORT void SendLogin075(
    int32_t handle,
    const BYTE* username,
    uint32_t usernameLength,
    const BYTE* password,
    uint32_t passwordLength,
    uint32_t tickCount,
    const BYTE* clientVersion,
    uint32_t /*clientVersionByteLength*/,
    const BYTE* clientSerial,
    uint32_t /*clientSerialByteLength*/)
{
    wchar_t userWide[11] = {};
    wchar_t passWide[21] = {};
    for (uint32_t i = 0; username && i < usernameLength && i < 10; ++i)
    {
        userWide[i] = static_cast<wchar_t>(username[i]);
    }
    for (uint32_t i = 0; password && i < passwordLength && i < 20; ++i)
    {
        passWide[i] = static_cast<wchar_t>(password[i]);
    }

    ConnectionManager_SendLogin(handle, userWide, passWide, tickCount, clientVersion, clientSerial);
}

MU_EXPORT void SendLoginLongPassword(
    int32_t handle,
    const BYTE* username,
    uint32_t usernameLength,
    const BYTE* password,
    uint32_t passwordLength,
    uint32_t tickCount,
    const BYTE* clientVersion,
    uint32_t clientVersionByteLength,
    const BYTE* clientSerial,
    uint32_t clientSerialByteLength)
{
    SendLogin075(
        handle,
        username,
        usernameLength,
        password,
        passwordLength,
        tickCount,
        clientVersion,
        clientVersionByteLength,
        clientSerial,
        clientSerialByteLength);
}

MU_EXPORT void SendLoginShortPassword(
    int32_t handle,
    const BYTE* username,
    uint32_t usernameLength,
    const BYTE* password,
    uint32_t passwordLength,
    uint32_t tickCount,
    const BYTE* clientVersion,
    uint32_t clientVersionByteLength,
    const BYTE* clientSerial,
    uint32_t clientSerialByteLength)
{
    SendLogin075(
        handle,
        username,
        usernameLength,
        password,
        passwordLength,
        tickCount,
        clientVersion,
        clientVersionByteLength,
        clientSerial,
        clientSerialByteLength);
}

MU_EXPORT void SendTalkToNpcRequest(int32_t handle, uint16_t npcId)
{
    // OpenMU/PC-compatible talk packet format.
    uint8_t packet[] = { 0xC3, 0x05, 0x30, 0x00, 0x00 };
    WriteUInt16BigEndian(packet + 3, npcId);
    NET_LOGI(
        "[GS fd=%d] SendTalkToNpcRequest npcId=%u",
        handle,
        static_cast<unsigned int>(npcId));
    SendGameEncrypted(handle, packet, sizeof(packet));
}

MU_EXPORT void SendCloseNpcRequest(int32_t handle)
{
    const uint8_t packet[] = { 0xC1, 0x03, 0x31 };
    NET_LOGI("[GS fd=%d] SendCloseNpcRequest", handle);
    SendGameEncrypted(handle, packet, sizeof(packet));
}

MU_EXPORT void SendBuyItemFromNpcRequest(int32_t handle, BYTE itemSlot)
{
    const uint8_t packet[] = { 0xC3, 0x04, 0x32, itemSlot };
    NET_LOGI(
        "[GS fd=%d] SendBuyItemFromNpcRequest slot=%u",
        handle,
        static_cast<unsigned int>(itemSlot));
    SendGameEncrypted(handle, packet, sizeof(packet));
}

MU_EXPORT void SendSellItemToNpcRequest(int32_t handle, BYTE itemSlot)
{
    const uint8_t packet[] = { 0xC3, 0x04, 0x33, itemSlot };
    NET_LOGI(
        "[GS fd=%d] SendSellItemToNpcRequest slot=%u",
        handle,
        static_cast<unsigned int>(itemSlot));
    SendGameEncrypted(handle, packet, sizeof(packet));
}

MU_EXPORT void SendRepairItemRequest(int32_t handle, BYTE itemSlot, BYTE isSelfRepair)
{
    const uint8_t packet[] = { 0xC1, 0x05, 0x34, itemSlot, static_cast<uint8_t>(isSelfRepair ? 1 : 0) };
    NET_LOGI(
        "[GS fd=%d] SendRepairItemRequest slot=%u self=%u",
        handle,
        static_cast<unsigned int>(itemSlot),
        static_cast<unsigned int>(isSelfRepair ? 1 : 0));
    SendGameEncrypted(handle, packet, sizeof(packet));
}

MU_EXPORT void SendUnlockVault(int32_t handle, uint16_t pin)
{
    uint8_t packet[] = { 0xC1, 0x07, 0x83, 0x00, 0x00, 0x00, 0x00 };
    WriteUInt16LittleEndian(packet + 4, pin);
    NET_LOGI("[GS fd=%d] SendUnlockVault pin=%u", handle, static_cast<unsigned int>(pin));
    SendGameEncrypted(handle, packet, sizeof(packet));
}

MU_EXPORT void SendSetVaultPin(int32_t handle, uint16_t pin, const wchar_t* password)
{
    uint8_t packet[27] = { 0xC1, 0x1B, 0x83, 0x01 };
    WriteUInt16LittleEndian(packet + 4, pin);
    for (int i = 0; password && password[i] && i < 20; ++i)
    {
        packet[6 + i] = static_cast<uint8_t>(password[i] & 0xFF);
    }

    NET_LOGI("[GS fd=%d] SendSetVaultPin pin=%u", handle, static_cast<unsigned int>(pin));
    SendGameEncrypted(handle, packet, sizeof(packet));
}

MU_EXPORT void SendRemoveVaultPin(int32_t handle, const wchar_t* password)
{
    uint8_t packet[27] = { 0xC1, 0x1B, 0x83, 0x02 };
    for (int i = 0; password && password[i] && i < 20; ++i)
    {
        packet[6 + i] = static_cast<uint8_t>(password[i] & 0xFF);
    }

    NET_LOGI("[GS fd=%d] SendRemoveVaultPin", handle);
    SendGameEncrypted(handle, packet, sizeof(packet));
}

MU_EXPORT void SendVaultClosed(int32_t handle)
{
    const uint8_t packet[] = { 0xC1, 0x03, 0x82 };
    NET_LOGI("[GS fd=%d] SendVaultClosed", handle);
    SendGameEncrypted(handle, packet, sizeof(packet));
}

MU_EXPORT void SendVaultMoveMoneyRequest(int32_t handle, uint32_t direction, uint32_t amount)
{
    uint8_t packet[] = { 0xC1, 0x08, 0x81, 0x00, 0x00, 0x00, 0x00, 0x00 };
    packet[3] = static_cast<uint8_t>(direction & 0xFF);
    WriteUInt32LittleEndian(packet + 4, amount);
    NET_LOGI(
        "[GS fd=%d] SendVaultMoveMoneyRequest dir=%u amount=%u",
        handle,
        static_cast<unsigned int>(direction & 0xFF),
        static_cast<unsigned int>(amount));
    SendGameEncrypted(handle, packet, sizeof(packet));
}

MU_EXPORT void SendCrywolfInfoRequest(int32_t /*handle*/)
{
}

MU_EXPORT void SendClientReadyAfterMapChange(int32_t handle)
{
    const uint8_t packet[] = { 0xC1, 0x04, 0xF3, 0x12 };
    NET_LOGI("[GS fd=%d] SendClientReadyAfterMapChange", handle);
    SendGameEncrypted(handle, packet, sizeof(packet));
}

MU_EXPORT void SendPingResponse(int32_t /*handle*/)
{
}

MU_EXPORT void SendGuildListRequest(int32_t /*handle*/)
{
}

MU_EXPORT void SendGuildInfoRequest(int32_t /*handle*/, uint32_t /*guildKey*/)
{
}

MU_EXPORT void SendRequestAllianceList(int32_t /*handle*/)
{
}

MU_EXPORT void SendInventoryRequest(int32_t /*handle*/)
{
}

MU_EXPORT void SendSetFriendOnlineState(int32_t /*handle*/, BYTE /*onlineState*/)
{
}

MU_EXPORT void SendActiveQuestListRequest(int32_t /*handle*/)
{
}

MU_EXPORT void SendEventQuestStateListRequest(int32_t /*handle*/)
{
}

MU_EXPORT void SendCastleSiegeStatusRequest(int32_t /*handle*/)
{
}

MU_EXPORT void SendCastleSiegeRegistrationStateRequest(int32_t /*handle*/)
{
}

MU_EXPORT void SendCastleSiegeRegisteredGuildsListRequest(int32_t /*handle*/)
{
}

MU_EXPORT void SendCashShopPointInfoRequest(int32_t handle)
{
    const uint8_t packet[] = { 0xC1, 0x04, 0xD2, 0x01 };
    NET_LOGI("[GS fd=%d] SendCashShopPointInfoRequest", handle);
    SendGameEncrypted(handle, packet, sizeof(packet));
}

MU_EXPORT void SendCashShopOpenState(int32_t handle, BYTE isClosed)
{
    const uint8_t packet[] = { 0xC1, 0x05, 0xD2, 0x02, static_cast<uint8_t>(isClosed) };
    NET_LOGI(
        "[GS fd=%d] SendCashShopOpenState isClosed=%u",
        handle,
        static_cast<unsigned int>(isClosed));
    SendGameEncrypted(handle, packet, sizeof(packet));
}

MU_EXPORT void SendCashShopStorageListRequest(int32_t handle, uint32_t pageIndex, BYTE inventoryType)
{
    uint8_t packet[] = { 0xC1, 0x09, 0xD2, 0x05, 0x00, 0x00, 0x00, 0x00, inventoryType };
    WriteUInt32LittleEndian(packet + 4, pageIndex);
    NET_LOGI(
        "[GS fd=%d] SendCashShopStorageListRequest page=%u type=%u",
        handle,
        static_cast<unsigned int>(pageIndex),
        static_cast<unsigned int>(inventoryType));
    SendGameEncrypted(handle, packet, sizeof(packet));
}

MU_EXPORT void SendMuHelperStatusChangeRequest(int32_t handle, BYTE pauseStatus)
{
    const uint8_t packet[] = { 0xC1, 0x05, 0xBF, 0x51, static_cast<uint8_t>(pauseStatus ? 1 : 0) };
    NET_LOGI(
        "[GS fd=%d] SendMuHelperStatusChangeRequest pause=%u",
        handle,
        static_cast<unsigned int>(pauseStatus ? 1 : 0));
    SendGameEncrypted(handle, packet, sizeof(packet));
}

MU_EXPORT void SendMuHelperSaveDataRequest(int32_t handle, const BYTE* helperData, uint32_t helperDataByteLength)
{
    constexpr uint16_t packetLength = 261; // C2 + length + code + 257-byte helper payload
    uint8_t packet[packetLength] = {};
    packet[0] = 0xC2;
    WriteUInt16BigEndian(packet + 1, packetLength);
    packet[3] = 0xAE;

    uint32_t copyLength = helperDataByteLength;
    if (copyLength > 257u)
    {
        copyLength = 257u;
    }
    if (!helperData)
    {
        copyLength = 0u;
    }
    if (copyLength > 0u)
    {
        std::memcpy(packet + 4, helperData, copyLength);
    }

    NET_LOGI(
        "[GS fd=%d] SendMuHelperSaveDataRequest payload=%u",
        handle,
        static_cast<unsigned int>(copyLength));
    SendGameEncrypted(handle, packet, sizeof(packet));
}

MU_EXPORT void ConnectionManager_SendAuthenticateExt(int32_t /*handle*/, uint16_t /*roomId*/, uint32_t /*token*/)
{
}

MU_EXPORT void ConnectionManager_SendChatMessageExt(int32_t /*handle*/, BYTE /*senderIndex*/, const wchar_t* /*message*/)
{
}

#endif // __ANDROID__
