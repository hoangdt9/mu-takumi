using System.Buffers.Binary;

namespace Takumi.Server.Protocol;

/// <summary>M7d: copy HP/MP/zen from a sent <c>F3 03</c> join packet into roster storage when vitals are still unset.</summary>
public static class JoinMapVitalsSeed
{
    public static bool TryReadFromJoinPacket(ReadOnlySpan<byte> joinPkt, out CharacterRosterVitals vitals)
    {
        vitals = default;
        if (joinPkt.Length < JoinMapServerWire602.PacketLength)
        {
            return false;
        }

        if (joinPkt[0] != 0xC1 || joinPkt[2] != 0xF3 || joinPkt[3] != 0x03)
        {
            return false;
        }

        var life = BinaryPrimitives.ReadUInt16LittleEndian(joinPkt.Slice(34));
        var lifeMax = BinaryPrimitives.ReadUInt16LittleEndian(joinPkt.Slice(36));
        var mana = BinaryPrimitives.ReadUInt16LittleEndian(joinPkt.Slice(38));
        var manaMax = BinaryPrimitives.ReadUInt16LittleEndian(joinPkt.Slice(40));
        var shield = BinaryPrimitives.ReadUInt16LittleEndian(joinPkt.Slice(42));
        var shieldMax = BinaryPrimitives.ReadUInt16LittleEndian(joinPkt.Slice(44));
        var gold = BinaryPrimitives.ReadUInt32LittleEndian(joinPkt.Slice(50));

        if (lifeMax == 0)
        {
            return false;
        }

        vitals = CharacterRosterVitals.FromInts(life, lifeMax, mana, manaMax, gold, shield, shieldMax);
        return true;
    }

    public static bool TryReadShieldFromJoinPacket(ReadOnlySpan<byte> joinPkt, out int currentShield, out int maxShield)
    {
        currentShield = 0;
        maxShield = 0;
        if (joinPkt.Length < JoinMapServerWire602.PacketLength)
        {
            return false;
        }

        maxShield = BinaryPrimitives.ReadUInt16LittleEndian(joinPkt.Slice(44));
        currentShield = BinaryPrimitives.ReadUInt16LittleEndian(joinPkt.Slice(42));
        return maxShield > 0;
    }

    /// <summary>When <paramref name="maxHpAlreadySet"/> is false, reads join wire stats into <paramref name="vitals"/>.</summary>
    public static bool TryApplyFromJoinPacketIfUnset(bool maxHpAlreadySet, ReadOnlySpan<byte> joinPkt, out CharacterRosterVitals vitals)
    {
        vitals = default;
        if (maxHpAlreadySet)
        {
            return false;
        }

        return TryReadFromJoinPacket(joinPkt, out vitals);
    }

    public static bool TryReadBpFromJoinPacket(ReadOnlySpan<byte> joinPkt, out int currentBp, out int maxBp)
    {
        currentBp = 0;
        maxBp = 0;
        if (joinPkt.Length < JoinMapServerWire602.PacketLength)
        {
            return false;
        }

        if (joinPkt[0] != 0xC1 || joinPkt[2] != 0xF3 || joinPkt[3] != 0x03)
        {
            return false;
        }

        currentBp = BinaryPrimitives.ReadUInt16LittleEndian(joinPkt.Slice(46));
        maxBp = BinaryPrimitives.ReadUInt16LittleEndian(joinPkt.Slice(48));
        return maxBp > 0;
    }

    public static bool TryReadSheetFromJoinPacket(ReadOnlySpan<byte> joinPkt, out CharacterSheetStats sheet)
    {
        sheet = default;
        if (joinPkt.Length < JoinMapServerWire602.PacketLength)
        {
            return false;
        }

        if (joinPkt[0] != 0xC1 || joinPkt[2] != 0xF3 || joinPkt[3] != 0x03)
        {
            return false;
        }

        sheet = CharacterSheetStats.FromInts(
            BinaryPrimitives.ReadUInt16LittleEndian(joinPkt.Slice(26)),
            BinaryPrimitives.ReadUInt16LittleEndian(joinPkt.Slice(28)),
            BinaryPrimitives.ReadUInt16LittleEndian(joinPkt.Slice(30)),
            BinaryPrimitives.ReadUInt16LittleEndian(joinPkt.Slice(32)),
            BinaryPrimitives.ReadUInt16LittleEndian(joinPkt.Slice(60)),
            BinaryPrimitives.ReadUInt16LittleEndian(joinPkt.Slice(24)));
        return sheet.HasBaseStats || sheet.LevelUpPoint > 0;
    }
}
