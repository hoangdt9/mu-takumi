namespace Takumi.Server.Game.Networking;

/// <summary>Wire bytes for periodic game-port keepalive (C1 03 71 ping request).</summary>
public static class GamePortKeepAliveWire
{
    public static ReadOnlyMemory<byte> PingRequest { get; } = new byte[] { 0xC1, 0x03, 0x71 };
}
