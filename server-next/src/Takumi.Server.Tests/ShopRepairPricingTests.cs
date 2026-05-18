using Takumi.Server.Game.World;
using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class ShopRepairPricingTests
{
    [Fact]
    public void Compute_self_repair_adds_fifty_percent_markup()
    {
        var npc = ShopRepairPricing.Compute(10_000, 0, 79, 13 * 512 + 23, selfRepair: false);
        var self = ShopRepairPricing.Compute(10_000, 0, 79, 13 * 512 + 23, selfRepair: true);
        Assert.True(self > npc);
        Assert.True(npc > 0);
    }

    [Fact]
    public void RepairCost_returns_zero_when_item_is_full()
    {
        var item = new byte[ItemWire602.WireBytes];
        ItemWire602.WriteSeason6Item(item, 13, 23, 9, 40, false, false, 0, 0);
        var max = ItemSizeCatalog.GetMaxDurability(item);
        ItemWire602.SetDurability(item, max);
        Assert.Equal(0, ShopItemPricing.RepairCost(item, selfRepair: true));
    }

    [Fact]
    public void Pet_group13_can_equip_to_helper_slot_8()
    {
        var pet = new byte[ItemWire602.WireBytes];
        ItemWire602.WriteSeason6Item(pet, 13, 23, 0, 255, false, false, 0, 0);
        Assert.True(InventoryEquipRules.CanEquipToWearSlot(8, pet));
        Assert.False(InventoryEquipRules.CanEquipToWearSlot(7, pet));
    }
}
