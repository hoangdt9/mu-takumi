using System.Buffers.Binary;

namespace Takumi.Server.Protocol;

/// <summary>Teleport result (<c>GCTeleportSend</c> / <c>PRECEIVE_TELEPORT_POSITION</c>, head <c>0x1C</c>).</summary>
public static class TeleportWire602
{
    public const byte HeadCode = 0x1C;
    public const int PacketLength = 9;

    public static byte[] Build(ushort flag, byte mapId, byte x, byte y, byte angle)
    {
        var buf = new byte[PacketLength];
        buf[0] = 0xC1;
        buf[1] = PacketLength;
        buf[2] = HeadCode;
        BinaryPrimitives.WriteUInt16LittleEndian(buf.AsSpan(3, 2), flag);
        buf[5] = mapId;
        buf[6] = x;
        buf[7] = y;
        buf[8] = angle;
        return buf;
    }
}
