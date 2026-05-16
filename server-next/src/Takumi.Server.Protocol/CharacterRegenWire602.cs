using System.Buffers.Binary;

namespace Takumi.Server.Protocol;

/// <summary>Season 6 <c>GCCharacterRegenSend</c> — <c>C1 F3 04</c> matching client <c>PRECEIVE_REVIVAL</c> with <c>GAMESERVER_EXTRA</c> view DWORDs.</summary>
public static class CharacterRegenWire602
{
    public const int PacketLength = 44;

    public static byte[] Build(
        byte mapId,
        byte x,
        byte y,
        byte dir,
        ushort life,
        ushort mana,
        ushort shield = 0,
        ushort bp = 0,
        uint gold = 0,
        uint? viewCurHp = null,
        uint? viewCurMp = null,
        uint? viewCurBp = null,
        uint? viewCurSd = null)
    {
        var buf = new byte[PacketLength];
        buf[0] = 0xC1;
        buf[1] = PacketLength;
        buf[2] = 0xF3;
        buf[3] = 0x04;
        buf[4] = x;
        buf[5] = y;
        buf[6] = mapId;
        buf[7] = dir;
        WriteUInt16Le(buf, 8, life);
        WriteUInt16Le(buf, 10, mana);
        WriteUInt16Le(buf, 12, shield);
        WriteUInt16Le(buf, 14, bp);
        // Experience[8] — zeros (client rebuilds from join state on full revival path).
        BinaryPrimitives.WriteUInt32LittleEndian(buf.AsSpan(24), gold);
        BinaryPrimitives.WriteUInt32LittleEndian(buf.AsSpan(28), viewCurHp ?? life);
        BinaryPrimitives.WriteUInt32LittleEndian(buf.AsSpan(32), viewCurMp ?? mana);
        BinaryPrimitives.WriteUInt32LittleEndian(buf.AsSpan(36), viewCurBp ?? bp);
        BinaryPrimitives.WriteUInt32LittleEndian(buf.AsSpan(40), viewCurSd ?? shield);
        return buf;
    }

    static void WriteUInt16Le(byte[] buf, int offset, ushort value)
    {
        buf[offset] = (byte)(value & 0xFF);
        buf[offset + 1] = (byte)((value >> 8) & 0xFF);
    }
}
