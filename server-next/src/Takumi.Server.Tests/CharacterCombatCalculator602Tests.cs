using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class CharacterCombatCalculator602Tests
{
    [Fact]
    public void Equipped_weapon_increases_phys_damage_over_base_stats()
    {
        Environment.SetEnvironmentVariable(
            "TAKUMI_ITEM_OPTION_PATH",
            Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", "..", "Data", "Item", "ItemOption.txt")));

        var sheet = CharacterSheetStats.FromInts(100, 100, 100, 100, 0, 0);
        var baseOnly = CharacterCombatCalculator602.Compute(0x00, 50, sheet, null).Combat;

        var weapon = new byte[ItemWire602.WireBytes];
        ItemWire602.WriteSeason6Item(weapon, 0, 5, level: 9, durability: 255, skill: false, luck: false, option: 0, excellent: 0);

        var wear = new Dictionary<byte, byte[]> { [0] = weapon };
        var withWeapon = CharacterCombatCalculator602.Compute(0x00, 50, sheet, wear).Combat;

        Assert.True(
            withWeapon.PhysiDamageMax > baseOnly.PhysiDamageMax,
            $"expected weapon to raise damage: base={baseOnly.PhysiDamageMax} with={withWeapon.PhysiDamageMax}");
    }
}
