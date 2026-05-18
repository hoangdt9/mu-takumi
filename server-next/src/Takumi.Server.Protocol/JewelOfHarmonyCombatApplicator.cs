namespace Takumi.Server.Protocol;

/// <summary>Applies JoH options from worn items (parity <c>CalcJewelOfHarmonyOption</c>, preview path).</summary>
public static class JewelOfHarmonyCombatApplicator
{
    public static void ApplyPreviewBonuses(
        CharacterCombatAccumulator acc,
        IReadOnlyDictionary<byte, byte[]> wearSlots)
    {
        JewelOfHarmonyOptionCatalog.EnsureInitialized();

        for (byte slot = 0; slot <= ItemWire602.LastWearSlot; slot++)
        {
            if (!wearSlots.TryGetValue(slot, out var item12) || ItemWire602.IsEmpty(item12))
            {
                continue;
            }

            if (!TryGetHarmony(item12, out var harmonyType, out var harmonyIndex, out var harmonyLevel))
            {
                continue;
            }

            var def = JewelOfHarmonyOptionCatalog.Get(harmonyType, harmonyIndex);
            if (def is not { } harmonyDef)
            {
                continue;
            }

            var itemLevel = ItemWire602.DecodeLevel(item12);
            if (harmonyLevel > itemLevel)
            {
                continue;
            }

            var value = harmonyLevel < harmonyDef.ValueTable.Length ? harmonyDef.ValueTable[harmonyLevel] : 0;
            CombatOptionApplicator602.ApplyHarmony(harmonyType, harmonyIndex, value, acc);
        }
    }

    public static bool TryGetHarmony(
        ReadOnlySpan<byte> item12,
        out int harmonyType,
        out int harmonyIndex,
        out int harmonyLevel)
    {
        harmonyType = 0;
        harmonyIndex = 0;
        harmonyLevel = 0;

        if (item12.Length < 7 || ItemWireDecode602.IsSocketItem(item12))
        {
            return false;
        }

        var packed = item12[6];
        harmonyIndex = (packed >> 4) & 0x0F;
        harmonyLevel = packed & 0x0F;
        if (harmonyIndex == 0)
        {
            return false;
        }

        var index = ItemWire602.DecodeItemIndex(item12);
        harmonyType = ResolveHarmonyType(index);
        return harmonyType != 0;
    }

    static int ResolveHarmonyType(int index)
    {
        if (index is >= 0 and < 2560 && index != (4 * 512) + 15)
        {
            return 1;
        }

        if (index is >= 2560 and < 3072)
        {
            return 2;
        }

        if (index is >= 3072 and < 6144)
        {
            return 3;
        }

        return 0;
    }
}
