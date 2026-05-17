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
    public void TryFindItemMoveRequest_parses_C3_0x24_android_wire()
    {
        var pkt = Convert.FromHexString("C31324002C14CF007F00400000000000000022");
        Assert.True(ClientGameplayPackets602.TryFindItemMoveRequest(pkt, out _, out var sf, out var ss, out var tf, out var ts));
        Assert.Equal(0, sf);
        Assert.Equal((byte)44, ss);
        Assert.Equal(0, tf);
        Assert.Equal((byte)34, ts);
    }

    [Fact]
    public void TryMoveInventorySlot_equips_sword_to_wear_slot()
    {
        var sid = Guid.NewGuid();
        var sword = new byte[ItemWire602.WireBytes];
        ItemWire602.WriteSeason6Item(sword, 0, 5, 0, 40, false, false, 0, 0);
        PlayerShopSession.SetSlot(sid, 12, sword);

        Assert.True(PlayerShopSession.TryMoveInventorySlot(sid, 12, 0, out _, out _, out _));
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

        Assert.True(PlayerShopSession.TryMoveInventorySlot(sid, 12, 13, out var at13, out var swappedAt12, out _));
        Assert.NotNull(swappedAt12);
        Assert.True(PlayerShopSession.TryGetSlot(sid, 12, out var now12));
        Assert.Equal(b[0], now12[0]);
        Assert.Equal(a[0], at13[0]);
    }

    [Fact]
    public void TryMoveInventorySlot_to_empty_slot_clears_source_anchor()
    {
        var sid = Guid.NewGuid();
        var bow = new byte[ItemWire602.WireBytes];
        ItemWire602.WriteSeason6Item(bow, 4, 7, 0, 40, false, false, 0, 0);
        PlayerShopSession.SetSlot(sid, 12, bow);

        Assert.True(PlayerShopSession.TryMoveInventorySlot(sid, 12, 40, out _, out var swapped, out _));
        Assert.Null(swapped);
        Assert.False(PlayerShopSession.TryGetSlot(sid, 12, out var ghost) && !ItemWire602.IsEmpty(ghost));
        Assert.True(PlayerShopSession.TryGetSlot(sid, 40, out var at40));
        Assert.False(ItemWire602.IsEmpty(at40));
    }

    [Fact]
    public void BuildInventoryListPacket_after_bag_move_keeps_target_anchor_not_repacked()
    {
        var sid = Guid.NewGuid();
        var bow = new byte[ItemWire602.WireBytes];
        ItemWire602.WriteSeason6Item(bow, 4, 7, 0, 40, false, false, 0, 0);
        PlayerShopSession.SetSlot(sid, 12, bow);

        Assert.True(PlayerShopSession.TryMoveInventorySlot(sid, 12, 44, out _, out _, out _));

        var pkt = PlayerShopSession.BuildInventoryListPacket(sid);
        var slotsInWire = new List<byte>();
        for (var o = 6; o + 13 <= pkt.Length; o += 13)
        {
            slotsInWire.Add(pkt[o]);
        }

        Assert.Contains((byte)44, slotsInWire);
        Assert.DoesNotContain((byte)12, slotsInWire);

        PlayerShopSession.CompactBagForPlacement(sid);
        Assert.True(PlayerShopSession.TryGetSlot(sid, 12, out var repacked));
        Assert.False(ItemWire602.IsEmpty(repacked));
        Assert.False(PlayerShopSession.TryGetSlot(sid, 44, out var at44) && !ItemWire602.IsEmpty(at44));
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
    public void InventoryBagGrid_places_2x2_items_without_anchor_overlap()
    {
        var sid = Guid.NewGuid();
        var armor = new byte[ItemWire602.WireBytes];
        ItemWire602.WriteSeason6Item(armor, 7, 0, 0, 40, false, false, 0, 0);
        PlayerShopSession.SetSlot(sid, 12, armor);
        PlayerShopSession.SetSlot(sid, 13, armor);
        PlayerShopSession.RepackLoadedBag(sid);

        Assert.True(PlayerShopSession.TryGetSessionSlots(sid, out var slots));
        var anchors = slots.Keys.Where(ItemWire602.IsBagSlot).OrderBy(x => x).ToList();
        Assert.Equal(2, anchors.Count);
        Assert.Equal(12, anchors[0]);
        Assert.Equal(14, anchors[1]);
    }

    [Fact]
    public void TryFindEmptyBagSlot_respects_item_footprint()
    {
        var sid = Guid.NewGuid();
        var wide = new byte[ItemWire602.WireBytes];
        ItemWire602.WriteSeason6Item(wide, 7, 0, 0, 40, false, false, 0, 0);
        PlayerShopSession.SetSlot(sid, 12, wide);

        Assert.True(PlayerShopSession.TryFindEmptyBagSlot(sid, wide, out var slot));
        Assert.Equal((byte)14, slot);
    }

    [Fact]
    public void BuildInventoryListPacket_emits_C4_F3_10_with_slot_count()
    {
        var sid = Guid.NewGuid();
        var potion = new byte[ItemWire602.WireBytes];
        ItemWire602.WriteSeason6Item(potion, 14, 0, 0, 255, false, false, 0, 0);
        PlayerShopSession.SetSlot(sid, 59, potion);
        PlayerShopSession.SetSlot(sid, 60, potion);

        var pkt = PlayerShopSession.BuildInventoryListPacket(sid);
        Assert.Equal(0xC4, pkt[0]);
        Assert.Equal(0xF3, pkt[3]);
        Assert.Equal(0x10, pkt[4]);
        Assert.Equal(2, pkt[5]);
    }

    [Fact]
    public void CompactBagSlots_places_bow_after_small_items_at_first_fit()
    {
        var slots = new Dictionary<byte, byte[]>();
        var potion = new byte[ItemWire602.WireBytes];
        ItemWire602.WriteSeason6Item(potion, 14, 0, 0, 255, false, false, 0, 0);
        slots[12] = potion.ToArray();
        slots[13] = potion.ToArray();
        slots[14] = potion.ToArray();
        slots[15] = potion.ToArray();

        var bow = new byte[ItemWire602.WireBytes];
        ItemWire602.WriteSeason6Item(bow, 4, 20, 9, 80, false, false, 0, 0);
        InventoryBagGrid.CompactBagSlots(slots);
        Assert.True(InventoryBagGrid.TryFindEmptyAnchor(slots, bow, out var anchor));
        Assert.True(anchor is >= 12 and <= 20, $"bow should land in top rows, got {anchor}");
    }

    [Fact]
    public void TryFindEmptyBagSlot_with_scattered_items_does_not_repack_existing_anchors()
    {
        var sid = Guid.NewGuid();
        var potion = new byte[ItemWire602.WireBytes];
        ItemWire602.WriteSeason6Item(potion, 14, 0, 0, 255, false, false, 0, 0);
        PlayerShopSession.SetSlot(sid, 29, potion);
        PlayerShopSession.SetSlot(sid, 24, potion);

        var armor = new byte[ItemWire602.WireBytes];
        ItemWire602.WriteSeason6Item(armor, 7, 0, 0, 40, false, false, 0, 0);
        Assert.True(PlayerShopSession.TryFindEmptyBagSlot(sid, armor, out var freeSlot));

        Assert.True(PlayerShopSession.TryGetSlot(sid, 29, out var still29));
        Assert.True(PlayerShopSession.TryGetSlot(sid, 24, out var still24));
        Assert.False(ItemWire602.IsEmpty(still29));
        Assert.False(ItemWire602.IsEmpty(still24));
        Assert.NotEqual((byte)29, freeSlot);
        Assert.NotEqual((byte)24, freeSlot);
    }

    [Fact]
    public void TryFindEmptyBagSlot_after_repack_uses_top_left_not_high_anchor()
    {
        var sid = Guid.NewGuid();
        var armor = new byte[ItemWire602.WireBytes];
        ItemWire602.WriteSeason6Item(armor, 7, 0, 0, 40, false, false, 0, 0);
        PlayerShopSession.SetSlot(sid, 20, armor);
        PlayerShopSession.SetSlot(sid, 28, armor);
        PlayerShopSession.SetSlot(sid, 36, armor);
        PlayerShopSession.SetSlot(sid, 44, armor);
        PlayerShopSession.CompactBagForPlacement(sid);

        var potion = new byte[ItemWire602.WireBytes];
        ItemWire602.WriteSeason6Item(potion, 14, 0, 0, 255, false, false, 0, 0);
        Assert.True(PlayerShopSession.TryFindEmptyBagSlot(sid, potion, out var freeSlot));
        Assert.True(freeSlot < 36, $"expected compact bag slot after repack, got {freeSlot}");
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
    public void ClientItemFootprintCatalog_reads_Item_eng_bmd_when_present()
    {
        var candidates = new[]
        {
            Environment.GetEnvironmentVariable("TAKUMI_ITEM_BMD_PATH"),
            Path.GetFullPath(Path.Combine(
                AppContext.BaseDirectory,
                "..", "..", "..", "..", "..", "..",
                "docker", "data-zip", "host", "Data", "Local", "Eng", "Item_eng.bmd")),
        };

        string? path = null;
        foreach (var c in candidates)
        {
            if (!string.IsNullOrWhiteSpace(c) && File.Exists(c))
            {
                path = c;
                break;
            }
        }

        if (path is null)
        {
            return;
        }

        var map = ClientItemFootprintCatalog.TryLoadFromBmd(path);
        Assert.NotNull(map);
        Assert.True(map!.Count > 1000);

        // Group 4 index 4 — "Cung Hổ" bow (client + Item.txt: 2×4).
        const int tigerBow = (4 * 512) + 4;
        Assert.True(map.TryGetValue(tigerBow, out var size));
        Assert.Equal(2, size.Width);
        Assert.Equal(4, size.Height);
    }

    [Fact]
    public void WarehouseBagGrid_compact_dedupes_duplicate_anchors()
    {
        ItemSizeCatalog.EnsureInitialized();
        var bow = new byte[ItemWire602.WireBytes];
        ItemWire602.WriteSeason6Item(bow, 4, 4, 9, 40, false, false, 0, 0);
        var slots = new Dictionary<byte, byte[]>
        {
            [21] = bow.ToArray(),
            [22] = bow.ToArray(),
            [52] = bow.ToArray(),
        };

        Assert.True(WarehouseBagGrid.CompactWarehouseSlots(slots));
        Assert.Single(slots);
        Assert.True(slots.ContainsKey(0) || slots.ContainsKey(21));
    }

    [Fact]
    public void WarehouseBagGrid_maps_main_and_extended_pages()
    {
        Assert.True(WarehouseBagGrid.WireToCell(0, out var c0, out var r0));
        Assert.Equal(0, c0);
        Assert.Equal(0, r0);
        Assert.True(WarehouseBagGrid.WireToCell(120, out var c1, out var r1));
        Assert.Equal(0, c1);
        Assert.Equal(0, r1);
        Assert.False(WarehouseBagGrid.WireToCell(240, out _, out _));
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
