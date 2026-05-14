using System.Net.Sockets;

namespace Takumi.Server.Game.Networking;

/// <summary>OS-level TCP keepalive for game-port sockets (NAT / idle middleboxes).</summary>
public static class SocketIdleHelpers
{
    /// <summary>Best-effort: enables SO_KEEPALIVE and short idle probes where the runtime/OS supports it.</summary>
    public static void TryApplyGamePortTcpKeepAlive(Socket socket)
    {
        try
        {
            socket.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.KeepAlive, true);
        }
        catch
        {
            // ignore
        }

        try
        {
            socket.SetSocketOption(SocketOptionLevel.Tcp, SocketOptionName.TcpKeepAliveTime, 25);
            socket.SetSocketOption(SocketOptionLevel.Tcp, SocketOptionName.TcpKeepAliveInterval, 8);
            socket.SetSocketOption(SocketOptionLevel.Tcp, SocketOptionName.TcpKeepAliveRetryCount, 5);
        }
        catch
        {
            // Older stacks / restricted sockets — SO_KEEPALIVE alone still helps some environments.
        }
    }
}
