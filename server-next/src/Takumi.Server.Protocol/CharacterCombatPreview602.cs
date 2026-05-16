namespace Takumi.Server.Protocol;

/// <summary>Client-facing combat preview (<c>PMSG_NEW_CHARACTER_CALC_RECV</c> / <c>GCNewCharacterCalcRecv</c>).</summary>
public static class CharacterCombatPreview602
{
    public readonly struct Preview
    {
        public uint PhysiDamageMin { get; init; }
        public uint PhysiDamageMax { get; init; }
        public uint MagicDamageMin { get; init; }
        public uint MagicDamageMax { get; init; }
        public uint PhysiSpeed { get; init; }
        public uint MagicSpeed { get; init; }
        public uint AttackSuccessRate { get; init; }
        public uint AttackSuccessRatePvP { get; init; }
        public uint Defense { get; init; }
        public uint DefenseSuccessRate { get; init; }
        public uint DefenseSuccessRatePvP { get; init; }
    }

    public static Preview FromSheet(byte serverClass, ushort level, CharacterSheetStats sheet)
    {
        var s = CharacterSheetCalculator.ResolveSheet(serverClass, level, sheet);
        var lv = Math.Max((ushort)1, level);
        var cls = CharacterSheetCalculator.ClassIndex(serverClass);

        var (physMin, physMax) = ComputePhysiDamage(cls, s.Strength, s.Dexterity, s.Energy);
        var (magMin, magMax) = ComputeMagicDamage(cls, s.Energy);
        var defense = ComputeDefense(cls, lv, s.Dexterity);
        var atkRate = ComputeAttackRate(cls, lv, s.Strength, s.Dexterity, s.Leadership);
        var atkRatePvp = ComputeAttackRatePvp(cls, lv, s.Dexterity);
        var defRate = ComputeDefenseRate(cls, lv, s.Dexterity);
        var defRatePvp = ComputeDefenseRatePvp(cls, lv, s.Dexterity);
        var (physSpeed, magicSpeed) = ComputeSpeed(cls, s.Dexterity);

        return new Preview
        {
            PhysiDamageMin = physMin,
            PhysiDamageMax = physMax,
            MagicDamageMin = magMin,
            MagicDamageMax = magMax,
            PhysiSpeed = physSpeed,
            MagicSpeed = magicSpeed,
            AttackSuccessRate = atkRate,
            AttackSuccessRatePvP = atkRatePvp,
            Defense = defense,
            DefenseSuccessRate = defRate,
            DefenseSuccessRatePvP = defRatePvp,
        };
    }

    public static int ResolvePlayerDefense(byte serverClass, ushort level, CharacterSheetStats sheet)
    {
        var s = CharacterSheetCalculator.ResolveSheet(serverClass, level, sheet);
        var cls = CharacterSheetCalculator.ClassIndex(serverClass);
        return (int)ComputeDefense(cls, Math.Max((ushort)1, level), s.Dexterity);
    }

    static (uint Min, uint Max) ComputePhysiDamage(int cls, ushort strength, ushort dexterity, ushort energy) =>
        cls switch
        {
            1 => ((uint)Math.Max(1, strength / 6), (uint)Math.Max(1, strength / 4)), // DK
            2 => ((uint)Math.Max(1, (strength + dexterity) / 7), (uint)Math.Max(1, (strength + dexterity) / 4)), // FE
            3 => ((uint)Math.Max(1, (strength / 6) + (energy / 12)), (uint)Math.Max(1, (strength / 4) + (energy / 9))), // MG
            4 => ((uint)Math.Max(1, strength / 7 + energy / 14), (uint)Math.Max(1, strength / 5 + energy / 10)), // DL
            6 => ((uint)Math.Max(1, strength / 7 + dexterity / 15), (uint)Math.Max(1, strength / 5 + dexterity / 12)), // RF
            _ => ((uint)Math.Max(1, strength / 8), (uint)Math.Max(1, strength / 4)),
        };

    static (uint Min, uint Max) ComputeMagicDamage(int cls, ushort energy) =>
        cls switch
        {
            0 => ((uint)Math.Max(1, energy / 9), (uint)Math.Max(1, energy / 4)), // DW
            5 => ((uint)Math.Max(1, energy / 9), (uint)Math.Max(1, energy / 4)), // SU
            _ => (0, 0),
        };

    static uint ComputeDefense(int cls, ushort level, ushort dexterity)
    {
        var lv = Math.Max((ushort)1, level);
        return cls switch
        {
            1 => (uint)Math.Max(1, dexterity / 3 + lv / 2), // DK simplified
            2 => (uint)Math.Max(1, dexterity / 4),
            _ => (uint)Math.Max(1, dexterity / 3 + lv / 3),
        };
    }

    static uint ComputeAttackRate(int cls, ushort level, ushort strength, ushort dexterity, ushort leadership)
    {
        var lv = Math.Max((ushort)1, level);
        return cls switch
        {
            4 => (uint)(((lv * 5) + (dexterity * 5) / 2) + (strength / 6) + (leadership / 10)),
            6 => (uint)((lv * 3) + (dexterity * 5) / 4 + strength / 6),
            _ => (uint)(((lv * 5) + (dexterity * 3) / 2) + (strength / 4)),
        };
    }

    static uint ComputeAttackRatePvp(int cls, ushort level, ushort dexterity)
    {
        var lv = Math.Max((ushort)1, level);
        return cls switch
        {
            1 => (uint)(lv * 3 + dexterity * 4.5f),
            2 => (uint)(lv * 3 + dexterity * 0.6f),
            3 => (uint)(lv * 3 + dexterity * 3.5f),
            4 => (uint)(lv * 3 + dexterity * 4f),
            6 => (uint)(lv * 2.6f + dexterity * 3.6f),
            _ => (uint)(lv * 3 + dexterity * 4f),
        };
    }

    static uint ComputeDefenseRate(int cls, ushort level, ushort dexterity)
    {
        var lv = Math.Max((ushort)1, level);
        return cls switch
        {
            2 => (uint)(dexterity / 4 + lv * 2),
            _ => (uint)(dexterity / 3 + lv * 2),
        };
    }

    static uint ComputeDefenseRatePvp(int cls, ushort level, ushort dexterity)
    {
        var lv = Math.Max((ushort)1, level);
        return (uint)(dexterity / 2 + lv);
    }

    static (uint Physi, uint Magic) ComputeSpeed(int cls, ushort dexterity) =>
        cls switch
        {
            2 => ((uint)Math.Max(1, dexterity / 50), (uint)Math.Max(1, dexterity / 50)),
            1 or 3 => ((uint)Math.Max(1, dexterity / 15), (uint)Math.Max(1, dexterity / 20)),
            _ => ((uint)Math.Max(1, dexterity / 20), (uint)Math.Max(1, dexterity / 10)),
        };
}
