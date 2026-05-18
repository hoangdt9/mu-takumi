namespace Takumi.Server.Protocol;

/// <summary>Mutable combat state while applying equipment (parity <c>LPOBJ</c> fields used by preview).</summary>
public sealed class CharacterCombatAccumulator
{
    public int PhysiDamageMinRight;
    public int PhysiDamageMaxRight;
    public int PhysiDamageMinLeft;
    public int PhysiDamageMaxLeft;
    public int MagicDamageMin;
    public int MagicDamageMax;
    public int CurseDamageMin;
    public int CurseDamageMax;
    public int Defense;
    public int AttackSuccessRate;
    public int AttackSuccessRatePvP;
    public int DefenseSuccessRate;
    public int DefenseSuccessRatePvP;
    public int PhysiSpeed;
    public int MagicSpeed;
    public uint MulPhysiDamage;
    public uint DivPhysiDamage;
    public uint MulMagicDamage;
    public uint DivMagicDamage;
    public uint MulCurseDamage;
    public uint DivCurseDamage;
    public uint MagicDamageRate;
    public uint CurseDamageRate;
    public uint AddStrength;
    public uint AddDexterity;
    public uint AddVitality;
    public uint AddEnergy;
    public uint AddLeadership;

    public void ApplyItemOption(int optionIndex, int value, ushort playerLevel) =>
        CombatOptionApplicator602.ApplyItemOption(this, optionIndex, value, playerLevel);

    public CharacterCombatPreview602.Preview ToPreview(
        byte serverClass,
        byte[]? rightItem12,
        byte[]? leftItem12)
    {
        var (physMin, physMax) = SelectPreviewPhysiDamage(serverClass, rightItem12, leftItem12);
        return new CharacterCombatPreview602.Preview
        {
            PhysiDamageMin = (uint)Math.Max(0, physMin),
            PhysiDamageMax = (uint)Math.Max(0, physMax),
            MagicDamageMin = (uint)Math.Max(0, MagicDamageMin),
            MagicDamageMax = (uint)Math.Max(0, MagicDamageMax),
            PhysiSpeed = (uint)Math.Max(0, PhysiSpeed),
            MagicSpeed = (uint)Math.Max(0, MagicSpeed),
            AttackSuccessRate = (uint)Math.Max(0, AttackSuccessRate),
            AttackSuccessRatePvP = (uint)Math.Max(0, AttackSuccessRatePvP),
            Defense = (uint)Math.Max(0, Defense),
            DefenseSuccessRate = (uint)Math.Max(0, DefenseSuccessRate),
            DefenseSuccessRatePvP = (uint)Math.Max(0, DefenseSuccessRatePvP),
        };
    }

    (int Min, int Max) SelectPreviewPhysiDamage(
        byte serverClass,
        byte[]? rightItem12,
        byte[]? leftItem12)
    {
        var cls = CharacterSheetCalculator.ClassIndex(serverClass);
        var rightIdx = rightItem12 is not null && !ItemWire602.IsEmpty(rightItem12)
            ? ItemWire602.DecodeItemIndex(rightItem12)
            : -1;
        var leftIdx = leftItem12 is not null && !ItemWire602.IsEmpty(leftItem12)
            ? ItemWire602.DecodeItemIndex(leftItem12)
            : -1;

        var dualHand = cls is 1 or 3 or 4 or 6
                       && IsMeleeWeapon(rightIdx)
                       && IsMeleeWeapon(leftIdx);

        if (dualHand)
        {
            return (
                PhysiDamageMinRight + PhysiDamageMinLeft,
                PhysiDamageMaxRight + PhysiDamageMaxLeft);
        }

        if (IsMeleeWeapon(rightIdx) || (rightIdx is >= (5 * 512) and < (6 * 512) && rightIdx != (4 * 512) + 15))
        {
            return (PhysiDamageMinRight, PhysiDamageMaxRight);
        }

        if (rightIdx is >= (4 * 512) and < (5 * 512) && rightIdx != (4 * 512) + 15)
        {
            return (PhysiDamageMinRight, PhysiDamageMaxRight);
        }

        if (leftIdx is >= (4 * 512) and < (5 * 512) && leftIdx != (4 * 512) + 7)
        {
            return (PhysiDamageMinLeft, PhysiDamageMaxLeft);
        }

        return (PhysiDamageMinLeft, PhysiDamageMaxLeft);
    }

    static bool IsMeleeWeapon(int index) =>
        index is >= 0 and < (4 * 512);
}


