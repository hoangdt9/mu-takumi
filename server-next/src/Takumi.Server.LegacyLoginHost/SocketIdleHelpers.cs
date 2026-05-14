// OS-level TCP keepalive for LegacyLoginHost sockets (NAT / idle middleboxes).
// Complements application-level C1 03 71 pings (see RunGamePortKeepAliveAsync in Program.cs).

using System.Net.Sockets;

internal static class SocketIdleHelpers
{
    /// <summary>
    /// Best-effort: enables SO_KEEPALIVE and short idle probes where the runtime/OS supports it.
    /// </summary>
    internal static void TryApplyGamePortTcpKeepAlive(Socket socket)
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
            // .NET 6+ — values are in seconds on Linux/macOS; milliseconds on Windows (runtime maps as needed).
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
