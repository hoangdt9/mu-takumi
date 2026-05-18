namespace Takumi.Server.Protocol;

/// <summary>Applies item / set / harmony option indices onto <see cref="CharacterCombatAccumulator"/> (parity GameServer option switches).</summary>
public static class CombatOptionApplicator602
{
    public static void ApplyItemOption(CharacterCombatAccumulator acc, int optionIndex, int value, ushort playerLevel) =>
        ApplyItemOption(acc, optionIndex, value, playerLevel, 0, 0, 0, 0);

    public static void ApplySetOption(
        CharacterCombatAccumulator acc,
        int setOptionIndex,
        int value,
        ushort playerLevel,
        int strength,
        int dexterity,
        int vitality,
        int energy)
    {
        switch (setOptionIndex)
        {
            case 0:
                acc.AddStrength += (uint)value;
                break;
            case 1:
                acc.AddDexterity += (uint)value;
                break;
            case 2:
                acc.AddEnergy += (uint)value;
                break;
            case 3:
                acc.AddVitality += (uint)value;
                break;
            case 4:
                acc.AddLeadership += (uint)value;
                break;
            case 5:
                acc.PhysiDamageMinLeft += value;
                acc.PhysiDamageMinRight += value;
                break;
            case 6:
                acc.PhysiDamageMaxLeft += value;
                acc.PhysiDamageMaxRight += value;
                break;
            case 7:
                acc.MagicDamageMin += (acc.MagicDamageMin * value) / 100;
                acc.MagicDamageMax += (acc.MagicDamageMax * value) / 100;
                break;
            case 8:
                acc.PhysiDamageMinLeft += value;
                acc.PhysiDamageMinRight += value;
                acc.PhysiDamageMaxLeft += value;
                acc.PhysiDamageMaxRight += value;
                break;
            case 9:
                acc.AttackSuccessRate += (acc.AttackSuccessRate * value) / 100;
                break;
            case 10:
                acc.Defense += value;
                break;
            case 24:
                AddPhysiByStat(acc, strength, value);
                break;
            case 25:
                AddPhysiByStat(acc, dexterity, value);
                break;
            case 26:
                acc.Defense += dexterity / Math.Max(1, value);
                break;
            case 27:
                acc.Defense += vitality / Math.Max(1, value);
                break;
            case 28:
                acc.MagicDamageMin += energy / Math.Max(1, value);
                acc.MagicDamageMax += energy / Math.Max(1, value);
                break;
            default:
                ApplyItemOption(acc, MapSetToItemOption(setOptionIndex), value, playerLevel, strength, dexterity, vitality, energy);
                break;
        }
    }

    public static void ApplyHarmony(int harmonyType, int harmonyIndex, int value, CharacterCombatAccumulator acc)
    {
        switch (harmonyType)
        {
            case 1:
                switch (harmonyIndex)
                {
                    case 1:
                        acc.PhysiDamageMinLeft += value;
                        acc.PhysiDamageMinRight += value;
                        break;
                    case 2:
                        acc.PhysiDamageMaxLeft += value;
                        acc.PhysiDamageMaxRight += value;
                        break;
                    case 5:
                        acc.PhysiDamageMinLeft += value;
                        acc.PhysiDamageMinRight += value;
                        acc.PhysiDamageMaxLeft += value;
                        acc.PhysiDamageMaxRight += value;
                        break;
                }

                break;
            case 2:
                if (harmonyIndex == 1)
                {
                    acc.MagicDamageMin += value;
                    acc.MagicDamageMax += value;
                    acc.CurseDamageMin += value;
                    acc.CurseDamageMax += value;
                }

                break;
            case 3:
                if (harmonyIndex == 1)
                {
                    acc.Defense += value;
                }

                break;
        }
    }

    static void ApplyItemOption(
        CharacterCombatAccumulator acc,
        int optionIndex,
        int value,
        ushort playerLevel,
        int strength,
        int dexterity,
        int vitality,
        int energy)
    {
        switch (optionIndex)
        {
            case 80:
            case 123:
                acc.PhysiDamageMinRight += value;
                acc.PhysiDamageMaxRight += value;
                acc.PhysiDamageMinLeft += value;
                acc.PhysiDamageMaxLeft += value;
                acc.MagicDamageMin += value;
                acc.MagicDamageMax += value;
                acc.CurseDamageMin += value;
                acc.CurseDamageMax += value;
                break;
            case 81:
            case 113:
                acc.MagicDamageMin += value;
                acc.MagicDamageMax += value;
                acc.CurseDamageMin += value;
                acc.CurseDamageMax += value;
                break;
            case 82:
                acc.DefenseSuccessRate += value;
                break;
            case 83:
            case 126:
                acc.Defense += value;
                break;
            case 87:
                acc.DefenseSuccessRate += (acc.DefenseSuccessRate * value) / 100;
                break;
            case 93:
            case 124:
                var byLv = playerLevel / Math.Max(1, value);
                acc.PhysiDamageMinRight += byLv;
                acc.PhysiDamageMaxRight += byLv;
                acc.PhysiDamageMinLeft += byLv;
                acc.PhysiDamageMaxLeft += byLv;
                acc.MagicDamageMin += byLv;
                acc.MagicDamageMax += byLv;
                break;
            case 94:
                acc.PhysiDamageMinRight += (acc.PhysiDamageMinRight * value) / 100;
                acc.PhysiDamageMaxRight += (acc.PhysiDamageMaxRight * value) / 100;
                acc.PhysiDamageMinLeft += (acc.PhysiDamageMinLeft * value) / 100;
                acc.PhysiDamageMaxLeft += (acc.PhysiDamageMaxLeft * value) / 100;
                acc.MulPhysiDamage += (uint)value;
                break;
            case 95:
                acc.MagicDamageMin += playerLevel / Math.Max(1, value);
                acc.MagicDamageMax += playerLevel / Math.Max(1, value);
                break;
            case 96:
                acc.MagicDamageMin += (acc.MagicDamageMin * value) / 100;
                acc.MagicDamageMax += (acc.MagicDamageMax * value) / 100;
                acc.MulMagicDamage += (uint)value;
                break;
            case 97:
                acc.PhysiSpeed += value;
                acc.MagicSpeed += value;
                break;
            case 114:
                acc.CurseDamageMin += playerLevel / Math.Max(1, value);
                acc.CurseDamageMax += playerLevel / Math.Max(1, value);
                break;
            case 115:
                acc.CurseDamageMin += (acc.CurseDamageMin * value) / 100;
                acc.CurseDamageMax += (acc.CurseDamageMax * value) / 100;
                acc.MulCurseDamage += (uint)value;
                break;
            case 127:
                acc.Defense += (acc.Defense * value) / 100;
                break;
        }
    }

    static int MapSetToItemOption(int setOptionIndex) =>
        setOptionIndex switch
        {
            15 or 16 or 17 or 18 => 84,
            19 => 123,
            21 or 22 => 102,
            _ => 0,
        };

    static void AddPhysiByStat(CharacterCombatAccumulator acc, int stat, int divisor)
    {
        if (divisor <= 0)
        {
            return;
        }

        var add = stat / divisor;
        acc.PhysiDamageMinLeft += add;
        acc.PhysiDamageMinRight += add;
        acc.PhysiDamageMaxLeft += add;
        acc.PhysiDamageMaxRight += add;
    }
}
