// TcpKeepAlive.h — OS-level TCP keepalive for MU game sockets (NAT / idle middleboxes).
// Used by Windows (WSctlc), Android (AndroidNetwork), and Asio client (ProtocolAsio / NEW_PROTOCOL_SYSTEM).

#pragma once

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <WinSock2.h>
#include <mstcpip.h>
#endif

#if defined(__linux__) || defined(__ANDROID__)
#include <netinet/tcp.h>
#endif

#if defined(__APPLE__)
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#endif

namespace TakumiNet
{

/// First keepalive probe after ~25s idle; then probes every ~8s (platform-dependent).
#if defined(_WIN32)
inline void ApplyGameTcpKeepAlive(SOCKET s)
{
    if (s == INVALID_SOCKET)
    {
        return;
    }

    BOOL on = TRUE;
    (void)setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<const char*>(&on), sizeof(on));

    struct tcp_keepalive ka = {};
    ka.onoff = 1;
    ka.keepalivetime = 25 * 1000;     // ms until first probe
    ka.keepaliveinterval = 8 * 1000; // ms between probes
    DWORD bytesReturned = 0;
    (void)WSAIoctl(
        s,
        SIO_KEEPALIVE_VALS,
        &ka,
        sizeof(ka),
        nullptr,
        0,
        &bytesReturned,
        nullptr,
        nullptr);
}
#else
inline void ApplyGameTcpKeepAlive(int fd)
{
    if (fd < 0)
    {
        return;
    }

    int yes = 1;
    (void)setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(yes));

#if defined(__linux__) || defined(__ANDROID__)
    int idleSec = 25;
    (void)setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &idleSec, sizeof(idleSec));
    int intvl = 8;
    (void)setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &intvl, sizeof(intvl));
    int cnt = 4;
    (void)setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, &cnt, sizeof(cnt));
#elif defined(__APPLE__)
    int idleSec = 25;
    (void)setsockopt(fd, IPPROTO_TCP, TCP_KEEPALIVE, &idleSec, sizeof(idleSec));
    int intvl = 8;
    (void)setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &intvl, sizeof(intvl));
    int cnt = 4;
    (void)setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, &cnt, sizeof(cnt));
#endif
}
#endif

} // namespace TakumiNet
