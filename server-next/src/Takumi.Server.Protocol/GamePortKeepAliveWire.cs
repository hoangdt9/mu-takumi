namespace Takumi.Server.Protocol;

/// <summary>Periodic game-port keepalive (<c>C1 0x71</c> ping).</summary>
public static class GamePortKeepAliveWire
{
    public static ReadOnlyMemory<byte> PingRequest { get; } = new byte[] { 0xC1, 0x03, 0x71 };
}
