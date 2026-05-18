namespace Takumi.Server.Protocol;

/// <summary>Applies ancient/set bonuses (parity <c>CSetItemOption::CalcSetItemStat/Option</c>).</summary>
public static class SetItemCombatApplicator
{
    public static void ApplyStatBonuses(
        CharacterCombatAccumulator acc,
        IReadOnlyDictionary<byte, byte[]> wearSlots)
    {
        SetItemTypeCatalog.EnsureInitialized();
        foreach (var (_, item12) in wearSlots)
        {
            if (ItemWire602.IsEmpty(item12))
            {
                continue;
            }

            if (!IsSetItem(item12))
            {
                continue;
            }

            var index = ItemWire602.DecodeItemIndex(item12);
            if (SetItemTypeCatalog.GetStatType(index) < 0)
            {
                continue;
            }

            var tier = ((ItemWireDecode602.DecodeSetOption(item12) >> 2) & 3) * 5;
            CombatOptionApplicator602.ApplySetOption(acc, SetItemTypeCatalog.GetStatType(index), tier, 0, 0, 0, 0, 0);
        }
    }

    public static void ApplySetCountBonuses(
        CharacterCombatAccumulator acc,
        IReadOnlyDictionary<byte, byte[]> wearSlots,
        ushort playerLevel,
        int strength,
        int dexterity,
        int vitality,
        int energy)
    {
        SetItemTypeCatalog.EnsureInitialized();
        SetItemOptionCatalog.EnsureInitialized();

        var counts = new Dictionary<int, int>();
        var weaponIndex = 0;
        var ringIndex = 0;
        foreach (var (slot, item12) in wearSlots)
        {
            if (ItemWire602.IsEmpty(item12) || !IsSetItem(item12))
            {
                continue;
            }

            var itemIndex = ItemWire602.DecodeItemIndex(item12);
            if (!SetItemTypeCatalog.TryGet(itemIndex, out _))
            {
                continue;
            }

            var setId = SetItemTypeCatalog.GetSetOptionIndex(itemIndex, (ItemWireDecode602.DecodeSetOption(item12) & 3) - 1);
            if (setId <= 0)
            {
                continue;
            }

            if (slot is 0 or 1)
            {
                if (weaponIndex == 0)
                {
                    weaponIndex = setId;
                }
                else if (weaponIndex == setId && wearSlots.TryGetValue(0, out var r) && wearSlots.TryGetValue(1, out var l)
                         && ItemWire602.DecodeItemIndex(r) == ItemWire602.DecodeItemIndex(l))
                {
                    continue;
                }
            }
            else if (slot is 10 or 11)
            {
                if (ringIndex == 0)
                {
                    ringIndex = setId;
                }
                else if (ringIndex == setId && wearSlots.TryGetValue(10, out var a) && wearSlots.TryGetValue(11, out var b)
                         && ItemWire602.DecodeItemIndex(a) == ItemWire602.DecodeItemIndex(b))
                {
                    continue;
                }
            }

            counts.TryGetValue(setId, out var c);
            counts[setId] = c + 1;
        }

        foreach (var (setId, count) in counts)
        {
            if (!SetItemOptionCatalog.TryGet(setId, out var def))
            {
                continue;
            }

            var maxPieces = CountMaxTableOptions(def);
            for (var tier = 0; tier < Math.Min(count - 1, TableTierCount(def)); tier++)
            {
                foreach (var entry in def.Tables[tier])
                {
                    if (entry.Index >= 0)
                    {
                        CombatOptionApplicator602.ApplySetOption(
                            acc, entry.Index, entry.Value, playerLevel, strength, dexterity, vitality, energy);
                    }
                }
            }

            if (count - 1 >= maxPieces)
            {
                foreach (var entry in def.FullOptions)
                {
                    if (entry.Index >= 0)
                    {
                        CombatOptionApplicator602.ApplySetOption(
                            acc, entry.Index, entry.Value, playerLevel, strength, dexterity, vitality, energy);
                    }
                }
            }
        }
    }

    static bool IsSetItem(ReadOnlySpan<byte> item12) => (ItemWireDecode602.DecodeSetOption(item12) & 3) != 0;

    static int CountMaxTableOptions(SetItemOptionCatalog.SetOptionDef def)
    {
        for (var t = def.Tables.Length - 1; t >= 0; t--)
        {
            foreach (var e in def.Tables[t])
            {
                if (e.Index >= 0)
                {
                    return t + 1;
                }
            }
        }

        return 0;
    }

    static int TableTierCount(SetItemOptionCatalog.SetOptionDef def) => def.Tables.Length;
}
