using System.Buffers.Binary;

namespace Takumi.Server.Protocol;

/// <summary><c>PMSG_LEVEL_UP_POINT_SEND</c> / client <c>ReceiveAddPoint</c> — <c>C1 F3 06</c>.</summary>
public static class LevelUpPointWire602
{
    public const byte Head = 0xF3;
    public const byte Sub = 0x06;
    public const int PacketLength = 48;

    public static byte[] BuildSuccess(
        byte statType,
        CharacterSheetStats sheet,
        CharacterComputedVitals vitals)
    {
        var p = new byte[PacketLength];
        p[0] = 0xC1;
        p[1] = PacketLength;
        p[2] = Head;
        p[3] = Sub;
        p[4] = (byte)(16 + (statType & 0x0F));
        BinaryPrimitives.WriteUInt16LittleEndian(p.AsSpan(5), vitals.LifeMax);
        BinaryPrimitives.WriteUInt16LittleEndian(p.AsSpan(7), vitals.ShieldMax);
        BinaryPrimitives.WriteUInt16LittleEndian(p.AsSpan(9), vitals.SkillManaMax);

        var o = 11;
        BinaryPrimitives.WriteUInt32LittleEndian(p.AsSpan(o), sheet.LevelUpPoint);
        BinaryPrimitives.WriteUInt32LittleEndian(p.AsSpan(o + 4), vitals.LifeMax);
        BinaryPrimitives.WriteUInt32LittleEndian(p.AsSpan(o + 8), vitals.ManaMax);
        BinaryPrimitives.WriteUInt32LittleEndian(p.AsSpan(o + 12), vitals.SkillManaMax);
        BinaryPrimitives.WriteUInt32LittleEndian(p.AsSpan(o + 16), vitals.ShieldMax);
        BinaryPrimitives.WriteUInt32LittleEndian(p.AsSpan(o + 20), sheet.Strength);
        BinaryPrimitives.WriteUInt32LittleEndian(p.AsSpan(o + 24), sheet.Dexterity);
        BinaryPrimitives.WriteUInt32LittleEndian(p.AsSpan(o + 28), sheet.Vitality);
        BinaryPrimitives.WriteUInt32LittleEndian(p.AsSpan(o + 32), sheet.Energy);
        BinaryPrimitives.WriteUInt32LittleEndian(p.AsSpan(o + 36), sheet.Leadership);
        return p;
    }

    public static byte[] BuildFail() =>
        new byte[] { 0xC1, PacketLength, Head, Sub, 0x00, 0, 0, 0, 0, 0, 0 };
}
