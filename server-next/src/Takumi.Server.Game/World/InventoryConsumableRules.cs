using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>M7d: HP/MP potion heal amounts (parity <c>CObjectManager::CharacterUsePotion</c> rate tables).</summary>
public static class InventoryConsumableRules
{
    public readonly record struct PotionHeal(int Hp, int Mp, int Shield);

    public static bool TryGetPotionHeal(int itemIndex, int maxHp, int maxMp, int maxShield, out PotionHeal heal)
    {
        heal = default;
        if (maxHp <= 0 && maxMp <= 0 && maxShield <= 0)
        {
            return false;
        }

        var (hpPct, mpPct, sdPct) = itemIndex switch
        {
            (14 * 512) + 0 => (100, 0, 0),
            (14 * 512) + 1 => (50, 0, 0),
            (14 * 512) + 2 => (70, 0, 0),
            (14 * 512) + 3 => (100, 0, 0),
            (14 * 512) + 4 => (0, 50, 0),
            (14 * 512) + 5 => (0, 70, 0),
            (14 * 512) + 6 => (0, 100, 0),
            (14 * 512) + 35 => (0, 0, 50),
            (14 * 512) + 36 => (0, 0, 70),
            (14 * 512) + 37 => (0, 0, 100),
            (14 * 512) + 38 => (30, 0, 0),
            (14 * 512) + 39 => (50, 0, 0),
            (14 * 512) + 40 => (70, 0, 0),
            (14 * 512) + 70 => (100, 0, 0),
            (14 * 512) + 71 => (0, 100, 0),
            (14 * 512) + 133 => (0, 0, 100),
            _ => (-1, -1, -1),
        };

        if (hpPct < 0)
        {
            return false;
        }

        var hp = hpPct > 0 && maxHp > 0 ? Math.Max(1, maxHp * hpPct / 100) : 0;
        var mp = mpPct > 0 && maxMp > 0 ? Math.Max(1, maxMp * mpPct / 100) : 0;
        var sd = sdPct > 0 && maxShield > 0 ? Math.Max(1, maxShield * sdPct / 100) : 0;
        if (hp == 0 && mp == 0 && sd == 0)
        {
            return false;
        }

        heal = new PotionHeal(hp, mp, sd);
        return true;
    }

    public static bool IsConsumablePotion(ReadOnlySpan<byte> item12) =>
        TryGetPotionHeal(ItemWire602.DecodeItemIndex(item12), 1, 1, 1, out _);

    public static int EnsureMaxShield(int maxHp, int maxShield) =>
        maxShield > 0 ? maxShield : Math.Max(1, maxHp);
}
