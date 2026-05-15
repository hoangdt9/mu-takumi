namespace Takumi.Server.Protocol;

/// <summary>Map teleport (<c>PMSG_TELEPORT_SEND</c> / head <c>0x1C</c>, Takumi client <c>PRECEIVE_TELEPORT_POSITION</c>).</summary>
public static class TeleportWire602
{
    public const byte HeadCode = 0x1C;

    public const int PacketLength = 9;

    /// <param name="mapChangeFlag">WORD <c>Flag</c> — non-zero triggers map reload on client (legacy <c>gate &gt; 0 ? 1 : 0</c>).</param>
    public static byte[] Build(ushort mapChangeFlag, byte map, byte x, byte y, byte dir)
    {
        var buf = new byte[PacketLength];
        buf[0] = 0xC1;
        buf[1] = PacketLength;
        buf[2] = HeadCode;
        buf[3] = (byte)(mapChangeFlag & 0xFF);
        buf[4] = (byte)((mapChangeFlag >> 8) & 0xFF);
        buf[5] = map;
        buf[6] = x;
        buf[7] = y;
        buf[8] = dir;
        return buf;
    }
}
