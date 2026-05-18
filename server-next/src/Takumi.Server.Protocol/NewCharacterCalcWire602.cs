using System.Buffers.Binary;

namespace Takumi.Server.Protocol;

/// <summary><c>C1 F3 E1</c> — <c>PMSG_NEW_CHARACTER_CALC_RECV</c> (Takumi <c>GCNewCharacterCalcRecv</c>).</summary>
public static class NewCharacterCalcWire602
{
    public const byte HeadCode = 0xF3;
    public const byte SubCode = 0xE1;
    public const int PacketLength = 172;

    public static byte[] Build(
        CharacterRosterWire roster,
        CharacterComputedVitals vitals,
        CharacterCombatCalculator602.CalcResult calc)
    {
        var p = new byte[PacketLength];
        p[0] = 0xC1;
        p[1] = PacketLength;
        p[2] = HeadCode;
        p[3] = SubCode;

        var off = 4;
        var combat = calc.Combat;
        WriteU32(p, ref off, vitals.Life);
        WriteU32(p, ref off, vitals.LifeMax);
        WriteU32(p, ref off, vitals.Mana);
        WriteU32(p, ref off, vitals.ManaMax);
        WriteU32(p, ref off, vitals.SkillMana);
        WriteU32(p, ref off, vitals.SkillManaMax);
        WriteU32(p, ref off, vitals.Shield);
        WriteU32(p, ref off, vitals.ShieldMax);
        WriteU32(p, ref off, calc.AddStrength);
        WriteU32(p, ref off, calc.AddDexterity);
        WriteU32(p, ref off, calc.AddVitality);
        WriteU32(p, ref off, calc.AddEnergy);
        WriteU32(p, ref off, calc.AddLeadership);
        WriteU32(p, ref off, combat.PhysiDamageMin);
        WriteU32(p, ref off, combat.PhysiDamageMax);
        WriteU32(p, ref off, combat.MagicDamageMin);
        WriteU32(p, ref off, combat.MagicDamageMax);
        WriteU32(p, ref off, calc.CurseDamageMin);
        WriteU32(p, ref off, calc.CurseDamageMax);
        WriteU32(p, ref off, calc.MulPhysiDamage);
        WriteU32(p, ref off, calc.DivPhysiDamage);
        WriteU32(p, ref off, calc.MulMagicDamage);
        WriteU32(p, ref off, calc.DivMagicDamage);
        WriteU32(p, ref off, calc.MulCurseDamage);
        WriteU32(p, ref off, calc.DivCurseDamage);
        WriteU32(p, ref off, calc.MagicDamageRate);
        WriteU32(p, ref off, calc.CurseDamageRate);
        WriteU32(p, ref off, combat.PhysiSpeed);
        WriteU32(p, ref off, combat.MagicSpeed);
        WriteU32(p, ref off, combat.AttackSuccessRate);
        WriteU32(p, ref off, combat.AttackSuccessRatePvP);
        WriteU32(p, ref off, combat.Defense);
        WriteU32(p, ref off, combat.DefenseSuccessRate);
        WriteU32(p, ref off, combat.DefenseSuccessRatePvP);
        WriteU32(p, ref off, 0); // ViewDamageMultiplier
        WriteU32(p, ref off, 0); // ViewRFDamageMultiplierA
        WriteU32(p, ref off, 0); // ViewRFDamageMultiplierB
        WriteU32(p, ref off, 0); // ViewRFDamageMultiplierC
        WriteU32(p, ref off, 0); // ViewDarkSpiritAttackDamageMin
        WriteU32(p, ref off, 0); // ViewDarkSpiritAttackDamageMax
        WriteU32(p, ref off, 0); // ViewDarkSpiritAttackSpeed
        WriteU32(p, ref off, 0); // ViewDarkSpiritAttackSuccessRate

        _ = roster;
        return p;
    }

    public static byte[] Build(CharacterRosterWire roster, IReadOnlyDictionary<byte, byte[]>? wearSlots = null)
    {
        var lv = Math.Max((ushort)1, roster.Level);
        var sheet = CharacterSheetCalculator.ResolveSheet(roster.ServerClass, lv, roster.Sheet);
        var vitals = CharacterSheetCalculator.ComputeMaxVitals(roster.ServerClass, lv, sheet);
        var merged = CharacterSheetCalculator.MergeVitalsForJoin(roster.Vitals, vitals);
        vitals = vitals with
        {
            Life = merged.ClampU16(merged.CurrentHp > 0 ? merged.CurrentHp : merged.MaxHp),
            Mana = merged.ClampU16(merged.CurrentMp > 0 ? merged.CurrentMp : merged.MaxMp),
            Shield = merged.ClampU16(merged.CurrentShield),
        };
        var calc = CharacterCombatCalculator602.Compute(roster.ServerClass, lv, sheet, wearSlots);
        return Build(roster, vitals, calc);
    }

    static void WriteU32(byte[] p, ref int off, uint value)
    {
        BinaryPrimitives.WriteUInt32LittleEndian(p.AsSpan(off), value);
        off += 4;
    }
}
