using System.Net;
using System.Net.Sockets;

namespace Takumi.Server.Connect;

/// <summary>Stable string for handoff / logs (IPv4 or IPv6 textual form).</summary>
public static class ConnectClientIp
{
    public static string? TryFormatIp(EndPoint? remote)
    {
        if (remote is IPEndPoint ipe)
        {
            return ipe.Address.ToString();
        }

        return null;
    }
}
