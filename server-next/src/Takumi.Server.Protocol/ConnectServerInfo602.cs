using System.Buffers.Binary;
using System.Text;

namespace Takumi.Server.Protocol;

/// <summary><c>C1 F4 03</c> server info (IP ASCII up to 16 + game port LE).</summary>
public static class ConnectServerInfo602
{
    public static byte[] Build(string ip, ushort gamePort)
    {
        var pkt = new byte[22];
        pkt[0] = 0xC1;
        pkt[1] = 22;
        pkt[2] = 0xF4;
        pkt[3] = 0x03;
        var ipBytes = Encoding.ASCII.GetBytes(ip);
        if (ipBytes.Length > 16)
        {
            throw new ArgumentException("Host must be at most 16 ASCII characters for ServerInfo packet.", nameof(ip));
        }

        ipBytes.CopyTo(pkt.AsSpan(4));
        BinaryPrimitives.WriteUInt16LittleEndian(pkt.AsSpan(20, 2), gamePort);
        return pkt;
    }
}
