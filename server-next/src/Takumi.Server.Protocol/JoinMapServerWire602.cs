using System.Buffers.Binary;

namespace Takumi.Server.Protocol;

/// <summary><c>PRECEIVE_JOIN_MAP_SERVER</c> (Takumi WSclient.h) — plain <c>C1</c>, 131 bytes.</summary>
public static class JoinMapServerWire602
{
    public const int PacketLength = 131;

    /// <param name="mapId">Override map byte (default Lorencia stub).</param>
    public static byte[] Build(CharacterRosterWire r, byte mapId = 0)
    {
        var p = new byte[PacketLength];
        p[0] = 0xC1;
        p[1] = PacketLength;
        p[2] = 0xF3;
        p[3] = 0x03;
        p[4] = 135;
        p[5] = 122;
        p[6] = mapId;
        p[7] = 1; // angle byte 1 → 0° in client ((Angle-1)*45)

        _ = r;
        BinaryPrimitives.WriteUInt64BigEndian(p.AsSpan(8), 0UL);
        BinaryPrimitives.WriteUInt64BigEndian(p.AsSpan(16), 0UL);

        BinaryPrimitives.WriteUInt16LittleEndian(p.AsSpan(24), 0);
        for (var i = 26; i < 50; i += 2)
        {
            BinaryPrimitives.WriteUInt16LittleEndian(p.AsSpan(i), 0);
        }

        BinaryPrimitives.WriteUInt32LittleEndian(p.AsSpan(50), 0);
        p[54] = 0;
        p[55] = 0;
        BinaryPrimitives.WriteInt16LittleEndian(p.AsSpan(56), 0);
        BinaryPrimitives.WriteInt16LittleEndian(p.AsSpan(58), 0);
        BinaryPrimitives.WriteUInt16LittleEndian(p.AsSpan(60), 0);
        BinaryPrimitives.WriteUInt16LittleEndian(p.AsSpan(62), 0);
        BinaryPrimitives.WriteUInt16LittleEndian(p.AsSpan(64), 0);
        p[66] = 0;
        p.AsSpan(67, 64).Clear();
        return p;
    }
}
