using System.Text;
using Takumi.Server.Game;
using Takumi.Server.Game.World;
using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class ItemWorldHandlerTests
{
    [Fact]
    public void TryFindItemMoveRequest_parses_C1_0x24_layout()
    {
        var pkt = new byte[ClientGameplayPackets602.ItemMoveFrameLength];
        pkt[0] = 0xC1;
        pkt[1] = (byte)ClientGameplayPackets602.ItemMoveFrameLength;
        pkt[2] = 0x24;
        pkt[3] = 0;
        pkt[4] = 20;
        pkt[15] = 0;
        pkt[18] = 30;

        Assert.True(ClientGameplayPackets602.TryFindItemMoveRequest(pkt, out _, out var sf, out var ss, out var tf, out var ts));
        Assert.Equal(0, sf);
        Assert.Equal((byte)20, ss);
        Assert.Equal(0, tf);
        Assert.Equal((byte)30, ts);
    }

    [Fact]
    public void TryMoveInventorySlot_equips_sword_to_wear_slot()
    {
        var sid = Guid.NewGuid();
        var sword = new byte[ItemWire602.WireBytes];
        ItemWire602.WriteSeason6Item(sword, 0, 5, 0, 40, false, false, 0, 0);
        PlayerShopSession.SetSlot(sid, 12, sword);

        Assert.True(PlayerShopSession.TryMoveInventorySlot(sid, 12, 0, out _));
        Assert.True(PlayerShopSession.TryGetSlot(sid, 0, out var worn));
        Assert.Equal(sword[0], worn[0]);
        Assert.False(PlayerShopSession.TryGetSlot(sid, 12, out _));
    }

    [Fact]
    public void IsSupportedItemStorage_respects_open_interfaces()
    {
        Assert.True(ClientGameplayPackets602.IsSupportedItemStorage(0, warehouseOpen: false, tradeOpen: false));
        Assert.True(ClientGameplayPackets602.IsSupportedItemStorage(2, warehouseOpen: true, tradeOpen: false));
        Assert.False(ClientGameplayPackets602.IsSupportedItemStorage(2, warehouseOpen: false, tradeOpen: false));
        Assert.True(ClientGameplayPackets602.IsSupportedItemStorage(1, warehouseOpen: false, tradeOpen: true));
    }

    [Fact]
    public void TryMoveInventorySlot_swaps_two_bag_slots()
    {
        var sid = Guid.NewGuid();
        var a = new byte[ItemWire602.WireBytes];
        var b = new byte[ItemWire602.WireBytes];
        ItemWire602.WriteSeason6Item(a, 14, 3, 5, 40, false, false, 0, 0);
        ItemWire602.WriteSeason6Item(b, 14, 4, 6, 50, false, false, 0, 0);
        PlayerShopSession.SetSlot(sid, 12, a);
        PlayerShopSession.SetSlot(sid, 13, b);

        Assert.True(PlayerShopSession.TryMoveInventorySlot(sid, 12, 13, out var at13));
        Assert.True(PlayerShopSession.TryGetSlot(sid, 12, out var now12));
        Assert.Equal(b[0], now12[0]);
        Assert.Equal(a[0], at13[0]);
    }

    [Fact]
    public void MapGroundItemStore_drop_and_take_roundtrip()
    {
        var item = new byte[ItemWire602.WireBytes];
        ItemWire602.WriteSeason6Item(item, 14, 3, 1, 10, false, false, 0, 0);
        var idx = MapGroundItemStore.Drop(0, 100, 100, item);
        Assert.True(idx > 0);
        Assert.True(MapGroundItemStore.TryTake(0, idx, 100, 100, out var picked));
        Assert.Equal(item[0], picked[0]);
        Assert.False(MapGroundItemStore.TryTake(0, idx, 100, 100, out _));
    }

    [Fact]
    public void ItemWorldWire602_move_fail_uses_0xFF_subcode()
    {
        var pkt = ItemWorldWire602.BuildMoveFail(22);
        Assert.Equal(0xFF, pkt[3]);
        Assert.Equal((byte)22, pkt[4]);
    }

    [Fact]
    public void PlayerDieWire602_layout_matches_GCUserDieSend()
    {
        var pkt = PlayerDieWire602.Build(victimObjectKey: 0x1234, killerObjectKey: 0x5678, skill: 0);
        Assert.Equal(0xC1, pkt[0]);
        Assert.Equal(PlayerDieWire602.PacketLength, pkt[1]);
        Assert.Equal((byte)0x17, pkt[2]);
        Assert.Equal(0x12, pkt[3]);
        Assert.Equal(0x34, pkt[4]);
        Assert.Equal(0x56, pkt[7]);
        Assert.Equal(0x78, pkt[8]);
    }

    [Fact]
    public void InventoryEquipRules_allows_sword_to_weapon_slots()
    {
        var item = new byte[ItemWire602.WireBytes];
        ItemWire602.WriteSeason6Item(item, 0, 0, 0, 40, false, false, 0, 0);
        Assert.True(InventoryEquipRules.CanEquipToWearSlot(0, item));
        Assert.True(InventoryEquipRules.CanMoveBetweenSlots(12, 0, item));
    }

    [Fact]
    public void InventoryEquipRules_rejects_zen_to_wear()
    {
        var zen = new byte[ItemWire602.WireBytes];
        ItemWire602.WriteSeason6Item(zen, 14, 15, 0, 1, false, false, 0, 0);
        Assert.False(InventoryEquipRules.CanEquipToWearSlot(0, zen));
    }

    [Fact]
    public void PlayerVitalsState_dead_until_revive_delay()
    {
        var id = Guid.NewGuid();
        PlayerVitalsState.MarkDead(id, TimeSpan.FromSeconds(30));
        Assert.True(PlayerVitalsState.IsDead(id));
        PlayerVitalsState.TryClearDead(id);
        Assert.False(PlayerVitalsState.IsDead(id));
    }

    [Fact]
    public void TryFindItemUseRequest_parses_C1_0x26()
    {
        var pkt = new byte[] { 0xC1, 0x05, 0x26, 14, 0 };
        Assert.True(ClientGameplayPackets602.TryFindItemUseRequest(pkt, out _, out var src, out var tgt));
        Assert.Equal((byte)14, src);
        Assert.Equal((byte)0, tgt);
    }

    [Fact]
    public void NormalizeItemUseSlot_maps_relative_bag_index()
    {
        Assert.Equal((byte)14, ClientGameplayPackets602.NormalizeItemUseSlot(2));
        Assert.Equal((byte)20, ClientGameplayPackets602.NormalizeItemUseSlot(20));
    }

    [Fact]
    public void InventoryConsumableRules_apple_heals_hp()
    {
        Assert.True(InventoryConsumableRules.TryGetPotionHeal((14 * 512) + 0, 1000, 500, 500, out var heal));
        Assert.Equal(1000, heal.Hp);
        Assert.Equal(0, heal.Mp);
        Assert.Equal(0, heal.Shield);
    }

    [Fact]
    public void InventoryConsumableRules_sd_potion_heals_shield()
    {
        Assert.True(InventoryConsumableRules.TryGetPotionHeal((14 * 512) + 35, 1000, 500, 800, out var heal));
        Assert.Equal(0, heal.Hp);
        Assert.Equal(400, heal.Shield);
    }

    [Fact]
    public void ItemWorldWire602_item_delete_and_dur_layout()
    {
        var del = ItemWorldWire602.BuildItemDelete(14);
        Assert.Equal(0x28, del[2]);
        Assert.Equal((byte)14, del[3]);
        var dur = ItemWorldWire602.BuildItemDur(14, 9);
        Assert.Equal(0x2A, dur[2]);
        Assert.Equal((byte)9, dur[4]);
    }

    [Fact]
    public void ItemViewportWire602_create_sets_fresh_drop_high_bit()
    {
        var item = new byte[ItemWire602.WireBytes];
        ItemWire602.WriteSeason6Item(item, 14, 3, 1, 10, false, false, 0, 0);
        var pkt = ItemViewportWire602.BuildCreateSingle(42, 10, 11, item, freshDrop: true);
        Assert.Equal(0x20, pkt[3]);
        Assert.Equal(1, pkt[4]);
        Assert.Equal(0x80 | 0, pkt[5]);
        Assert.Equal(42, pkt[6]);
        Assert.Equal((byte)10, pkt[7]);
        Assert.Equal((byte)11, pkt[8]);
    }
}
