namespace Takumi.Server.Protocol;

/// <summary>Applies <c>ItemOptionBEx.xml</c> bonuses on worn items.</summary>
public static class ItemOptionBexCombatApplicator
{
    public static void Apply(
        CharacterCombatAccumulator acc,
        ushort playerLevel,
        IReadOnlyDictionary<byte, byte[]> wearSlots)
    {
        ItemOptionBexCatalog.EnsureInitialized();

        for (byte slot = 0; slot <= ItemWire602.LastWearSlot; slot++)
        {
            if (!wearSlots.TryGetValue(slot, out var item12) || ItemWire602.IsEmpty(item12))
            {
                continue;
            }

            var index = ItemWire602.DecodeItemIndex(item12);
            var level = ItemWire602.DecodeLevel(item12);
            var exc = ItemWireDecode602.DecodeNewOption(item12);

            foreach (var row in ItemOptionBexCatalog.Rows)
            {
                if (index < row.ItemMinIndex || index > row.ItemMaxIndex)
                {
                    continue;
                }

                if (!LevelMatches(level, row.ItemLevelMin, row.ItemLevelMax))
                {
                    continue;
                }

                if (row.ItemExc >= 0 && exc < row.ItemExc)
                {
                    continue;
                }

                if (row.ItemSet == 1)
                {
                    if (!CheckFivePieceArmorSet(wearSlots, index % 512, row.ItemLevelMin, row.ItemExc))
                    {
                        continue;
                    }
                }
                else if (row.ItemSet != 0)
                {
                    continue;
                }

                CombatOptionApplicator602.ApplyItemOption(acc, row.OptionIndex, row.OptionValue, playerLevel);
            }
        }
    }

    static bool LevelMatches(int level, int min, int max) =>
        (min < 0 || level >= min) && (max < 0 || level <= max);

    static bool CheckFivePieceArmorSet(
        IReadOnlyDictionary<byte, byte[]> wearSlots,
        int armorNumber,
        int minLevel,
        int exc)
    {
        var count = 0;
        for (byte slot = 2; slot <= 6; slot++)
        {
            if (!wearSlots.TryGetValue(slot, out var item12) || ItemWire602.IsEmpty(item12))
            {
                continue;
            }

            if (ItemWire602.DecodeDurability(item12) == 0)
            {
                continue;
            }

            var index = ItemWire602.DecodeItemIndex(item12);
            if (index % 512 != armorNumber)
            {
                continue;
            }

            var group = index / 512;
            if (group is < 7 or > 11)
            {
                continue;
            }

            if (exc == 1 && ItemWireDecode602.DecodeNewOption(item12) == 0)
            {
                continue;
            }

            if (minLevel >= 0 && ItemWire602.DecodeLevel(item12) < minLevel)
            {
                continue;
            }

            count++;
        }

        return count >= 5;
    }
}
