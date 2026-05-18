namespace Takumi.Server.Protocol;

/// <summary>Full character combat preview: base stats + worn equipment + item/set/harmony/buff options.</summary>
public static class CharacterCombatCalculator602
{
    public readonly record struct CalcResult(
        CharacterCombatPreview602.Preview Combat,
        uint AddStrength,
        uint AddDexterity,
        uint AddVitality,
        uint AddEnergy,
        uint AddLeadership,
        uint MulPhysiDamage,
        uint DivPhysiDamage,
        uint MulMagicDamage,
        uint DivMagicDamage,
        uint MulCurseDamage,
        uint DivCurseDamage,
        uint MagicDamageRate,
        uint CurseDamageRate,
        uint CurseDamageMin,
        uint CurseDamageMax);

    public static CalcResult Compute(
        byte serverClass,
        ushort level,
        CharacterSheetStats sheet,
        IReadOnlyDictionary<byte, byte[]>? wearSlots,
        CombatEffectState602? activeEffects = null)
    {
        ItemCombatStatCatalog.EnsureInitialized();
        ItemOptionCatalog.EnsureInitialized();
        SetItemTypeCatalog.EnsureInitialized();
        SetItemOptionCatalog.EnsureInitialized();
        JewelOfHarmonyOptionCatalog.EnsureInitialized();
        ItemOptionBexCatalog.EnsureInitialized();

        var acc = new CharacterCombatAccumulator();
        byte[]? right = null;
        byte[]? left = null;

        if (wearSlots is not null)
        {
            wearSlots.TryGetValue(0, out right);
            wearSlots.TryGetValue(1, out left);
            SetItemCombatApplicator.ApplyStatBonuses(acc, wearSlots);
        }

        var effectiveSheet = ApplySheetAdds(sheet, acc);
        SeedBaseCombat(acc, serverClass, level, effectiveSheet);

        var strength = effectiveSheet.Strength;
        var dexterity = effectiveSheet.Dexterity;
        var vitality = effectiveSheet.Vitality;
        var energy = effectiveSheet.Energy;

        if (wearSlots is not null)
        {
            ApplyWearEquipment(acc, serverClass, level, wearSlots);
            SetItemCombatApplicator.ApplySetCountBonuses(
                acc, wearSlots, level, strength, dexterity, vitality, energy);
            JewelOfHarmonyCombatApplicator.ApplyPreviewBonuses(acc, wearSlots);
            ItemOptionBexCombatApplicator.Apply(acc, level, wearSlots);
        }

        var preview = acc.ToPreview(serverClass, right, left);
        if (activeEffects is not null && activeEffects != CombatEffectState602.Empty)
        {
            SkillBuffPreview602.ApplyToPreview(ref preview, activeEffects, effectiveSheet, acc);
        }

        return new CalcResult(
            preview,
            acc.AddStrength,
            acc.AddDexterity,
            acc.AddVitality,
            acc.AddEnergy,
            acc.AddLeadership,
            acc.MulPhysiDamage,
            acc.DivPhysiDamage,
            acc.MulMagicDamage,
            acc.DivMagicDamage,
            acc.MulCurseDamage,
            acc.DivCurseDamage,
            acc.MagicDamageRate,
            acc.CurseDamageRate,
            (uint)Math.Max(0, acc.CurseDamageMin),
            (uint)Math.Max(0, acc.CurseDamageMax));
    }

    static CharacterSheetStats ApplySheetAdds(CharacterSheetStats sheet, CharacterCombatAccumulator acc) =>
        sheet with
        {
            Strength = (ushort)Math.Min(ushort.MaxValue, sheet.Strength + acc.AddStrength),
            Dexterity = (ushort)Math.Min(ushort.MaxValue, sheet.Dexterity + acc.AddDexterity),
            Vitality = (ushort)Math.Min(ushort.MaxValue, sheet.Vitality + acc.AddVitality),
            Energy = (ushort)Math.Min(ushort.MaxValue, sheet.Energy + acc.AddEnergy),
            Leadership = (ushort)Math.Min(ushort.MaxValue, sheet.Leadership + acc.AddLeadership),
        };

    static void SeedBaseCombat(
        CharacterCombatAccumulator acc,
        byte serverClass,
        ushort level,
        CharacterSheetStats sheet)
    {
        var basePreview = CharacterCombatPreview602.FromSheet(serverClass, level, sheet);
        acc.PhysiDamageMinRight = (int)basePreview.PhysiDamageMin;
        acc.PhysiDamageMaxRight = (int)basePreview.PhysiDamageMax;
        acc.PhysiDamageMinLeft = (int)basePreview.PhysiDamageMin;
        acc.PhysiDamageMaxLeft = (int)basePreview.PhysiDamageMax;
        acc.MagicDamageMin = (int)basePreview.MagicDamageMin;
        acc.MagicDamageMax = (int)basePreview.MagicDamageMax;
        acc.CurseDamageMin = (int)basePreview.MagicDamageMin;
        acc.CurseDamageMax = (int)basePreview.MagicDamageMax;
        acc.Defense = (int)basePreview.Defense;
        acc.AttackSuccessRate = (int)basePreview.AttackSuccessRate;
        acc.AttackSuccessRatePvP = (int)basePreview.AttackSuccessRatePvP;
        acc.DefenseSuccessRate = (int)basePreview.DefenseSuccessRate;
        acc.DefenseSuccessRatePvP = (int)basePreview.DefenseSuccessRatePvP;
        acc.PhysiSpeed = (int)basePreview.PhysiSpeed;
        acc.MagicSpeed = (int)basePreview.MagicSpeed;
    }

    static void ApplyWearEquipment(
        CharacterCombatAccumulator acc,
        byte serverClass,
        ushort level,
        IReadOnlyDictionary<byte, byte[]> wearSlots)
    {
        for (byte slot = 0; slot <= ItemWire602.LastWearSlot; slot++)
        {
            if (!wearSlots.TryGetValue(slot, out var item12) || ItemWire602.IsEmpty(item12))
            {
                continue;
            }

            var stats = ItemInstanceCombatCalculator.Compute(item12);
            var index = ItemWire602.DecodeItemIndex(item12);

            if (slot == 0)
            {
                AddWeaponDamage(acc, stats, toRight: true, index);
            }
            else if (slot == 1)
            {
                if (ItemWireDecode602.IsShieldIndex(index))
                {
                    acc.Defense += stats.Defense;
                    acc.DefenseSuccessRate += stats.DefenseSuccessRate;
                }
                else
                {
                    AddWeaponDamage(acc, stats, toRight: false, index);
                }
            }
            else if (slot is >= 2 and <= 7)
            {
                acc.Defense += stats.Defense;
                acc.DefenseSuccessRate += stats.DefenseSuccessRate;
            }

            if (slot == 5 || slot == 8 || slot == 9 || slot == 10 || slot == 11)
            {
                acc.PhysiSpeed += stats.AttackSpeed;
                acc.MagicSpeed += stats.AttackSpeed;
            }

            if (slot == 7 && stats.MagicDamageRate > 0)
            {
                acc.MagicDamageRate = (uint)stats.MagicDamageRate;
            }

            foreach (var (optionIndex, optionValue) in ItemOptionCatalog.ResolveOptions(item12))
            {
                acc.ApplyItemOption(optionIndex, optionValue, level);
            }
        }
    }

    static void AddWeaponDamage(
        CharacterCombatAccumulator acc,
        ItemInstanceCombatCalculator.ItemCombatStats stats,
        bool toRight,
        int index)
    {
        var divisor = ItemWireDecode602.IsStaffIndex(index) ? 2 : 1;
        var min = stats.DamageMin / divisor;
        var max = stats.DamageMax / divisor;

        if (toRight)
        {
            acc.PhysiDamageMinRight += min;
            acc.PhysiDamageMaxRight += max;
        }
        else
        {
            acc.PhysiDamageMinLeft += min;
            acc.PhysiDamageMaxLeft += max;
        }

        if (ItemWireDecode602.IsStaffIndex(index))
        {
            acc.MagicDamageMin += min;
            acc.MagicDamageMax += max;
        }
    }
}
