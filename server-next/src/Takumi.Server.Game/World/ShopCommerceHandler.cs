using Takumi.Server.Game;
using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>NPC shop buy/sell/repair (<c>0x32</c>–<c>0x34</c>) stub.</summary>
public static class ShopCommerceHandler
{
    public static async Task<bool> TryHandlePacketAsync(
        GameRosterEntry player,
        Guid presenceSessionId,
        string? accountId,
        byte[] characterName10,
        byte[] packet,
        string remote,
        Func<ReadOnlyMemory<byte>, CancellationToken, Task> writeAsync,
        Action? onRosterDirty,
        CancellationToken ct)
    {
        if (PlayerVitalsState.IsDead(presenceSessionId))
        {
            return false;
        }

        if (ClientGameplayPackets602.TryFindBuyRequest(packet, out _, out var shopSlot))
        {
            return await HandleBuyAsync(
                    player,
                    presenceSessionId,
                    accountId,
                    characterName10,
                    shopSlot,
                    writeAsync,
                    onRosterDirty,
                    remote,
                    ct)
                .ConfigureAwait(false);
        }

        if (ClientGameplayPackets602.TryFindSellRequest(packet, out _, out var invSlot))
        {
            return await HandleSellAsync(
                    player,
                    presenceSessionId,
                    accountId,
                    characterName10,
                    invSlot,
                    writeAsync,
                    onRosterDirty,
                    remote,
                    ct)
                .ConfigureAwait(false);
        }

        if (ClientGameplayPackets602.TryFindRepairRequest(packet, out _, out var repairSlot, out _))
        {
            return await HandleRepairAsync(
                    player,
                    presenceSessionId,
                    accountId,
                    characterName10,
                    repairSlot,
                    writeAsync,
                    onRosterDirty,
                    remote,
                    ct)
                .ConfigureAwait(false);
        }

        return false;
    }

    static async Task<bool> HandleBuyAsync(
        GameRosterEntry player,
        Guid presenceSessionId,
        string? accountId,
        byte[] characterName10,
        byte shopSlot,
        Func<ReadOnlyMemory<byte>, CancellationToken, Task> writeAsync,
        Action? onRosterDirty,
        string remote,
        CancellationToken ct)
    {
        if (!PlayerShopSession.TryGetShopIndex(presenceSessionId, out var shopIndex))
        {
            await writeAsync(ShopCommerceWire602.BuildBuy(ShopCommerceWire602.BuyShopClosedIndex, stackalloc byte[ItemWire602.WireBytes]), ct)
                .ConfigureAwait(false);
            return true;
        }

        if (!NpcShopCatalog.TryGetShopItem(shopIndex, shopSlot, out var shopItem) || shopItem is null)
        {
            await writeAsync(ShopCommerceWire602.BuildBuyFail(), ct).ConfigureAwait(false);
            return true;
        }

        await PlayerShopSession.EnsureInventoryLoadedAsync(presenceSessionId, accountId, characterName10, ct)
            .ConfigureAwait(false);

        var price = ShopItemPricing.BuyPrice(shopItem);
        if (player.Zen < price)
        {
            await writeAsync(ShopCommerceWire602.BuildBuyFail(), ct).ConfigureAwait(false);
            Console.WriteLine("[m8] shop buy no zen need={0} have={1} {2}", price, player.Zen, remote);
            return true;
        }

        if (!PlayerShopSession.TryFindEmptyBagSlot(presenceSessionId, out var bagSlot))
        {
            await writeAsync(ShopCommerceWire602.BuildBuyFail(), ct).ConfigureAwait(false);
            return true;
        }

        var blob = new byte[ItemWire602.WireBytes];
        ItemWire602.WriteSeason6Item(
            blob,
            shopItem.ItemGroup,
            shopItem.ItemIndex,
            shopItem.ItemLevel,
            shopItem.Durability,
            shopItem.Skill != 0,
            shopItem.Luck != 0,
            shopItem.Option,
            shopItem.ExcOpt);

        player.Zen -= price;
        onRosterDirty?.Invoke();
        PlayerShopSession.SetSlot(presenceSessionId, bagSlot, blob);
        PlayerShopSession.PersistSlotToMirror(accountId, characterName10, bagSlot, blob);

        await writeAsync(ShopCommerceWire602.BuildBuy(bagSlot, blob), ct).ConfigureAwait(false);
        Console.WriteLine(
            "[m8] shop buy shop={0} slot={1} → inv={2} zen={3} {4}",
            shopIndex,
            shopSlot,
            bagSlot,
            player.Zen,
            remote);
        return true;
    }

    static async Task<bool> HandleSellAsync(
        GameRosterEntry player,
        Guid presenceSessionId,
        string? accountId,
        byte[] characterName10,
        byte invSlot,
        Func<ReadOnlyMemory<byte>, CancellationToken, Task> writeAsync,
        Action? onRosterDirty,
        string remote,
        CancellationToken ct)
    {
        if (!PlayerShopSession.TryGetShopIndex(presenceSessionId, out _))
        {
            await writeAsync(ShopCommerceWire602.BuildSell(0, (uint)Math.Max(0, player.Zen)), ct).ConfigureAwait(false);
            return true;
        }

        await PlayerShopSession.EnsureInventoryLoadedAsync(presenceSessionId, accountId, characterName10, ct)
            .ConfigureAwait(false);

        if (!PlayerShopSession.TryGetSlot(presenceSessionId, invSlot, out var blob) || ItemWire602.IsEmpty(blob))
        {
            await writeAsync(ShopCommerceWire602.BuildSell(0, (uint)Math.Max(0, player.Zen)), ct).ConfigureAwait(false);
            return true;
        }

        var price = ShopItemPricing.SellPrice(blob);
        player.Zen += price;
        onRosterDirty?.Invoke();
        var empty = new byte[ItemWire602.WireBytes];
        PlayerShopSession.SetSlot(presenceSessionId, invSlot, empty);
        PlayerShopSession.PersistSlotToMirror(accountId, characterName10, invSlot, empty);

        await writeAsync(ShopCommerceWire602.BuildSell(1, (uint)Math.Clamp(player.Zen, 0, uint.MaxValue)), ct)
            .ConfigureAwait(false);
        Console.WriteLine("[m8] shop sell inv={0} +{1} zen={2} {3}", invSlot, price, player.Zen, remote);
        return true;
    }

    static async Task<bool> HandleRepairAsync(
        GameRosterEntry player,
        Guid presenceSessionId,
        string? accountId,
        byte[] characterName10,
        byte slot,
        Func<ReadOnlyMemory<byte>, CancellationToken, Task> writeAsync,
        Action? onRosterDirty,
        string remote,
        CancellationToken ct)
    {
        if (!PlayerShopSession.TryGetShopIndex(presenceSessionId, out _))
        {
            await writeAsync(ShopCommerceWire602.BuildRepair((uint)Math.Max(0, player.Zen)), ct).ConfigureAwait(false);
            return true;
        }

        await PlayerShopSession.EnsureInventoryLoadedAsync(presenceSessionId, accountId, characterName10, ct)
            .ConfigureAwait(false);

        long totalCost = 0;
        if (slot == 0xFF)
        {
            for (byte s = 0; s <= ItemWire602.LastBagSlot; s++)
            {
                if (!PlayerShopSession.TryGetSlot(presenceSessionId, s, out var blob) || ItemWire602.IsEmpty(blob))
                {
                    continue;
                }

                var cost = ShopItemPricing.RepairCost(blob);
                if (cost <= 0 || player.Zen < cost)
                {
                    continue;
                }

                player.Zen -= cost;
                totalCost += cost;
                ItemWire602.SetDurability(blob, 255);
                PlayerShopSession.SetSlot(presenceSessionId, s, blob);
                PlayerShopSession.PersistSlotToMirror(accountId, characterName10, s, blob);
            }
        }
        else if (PlayerShopSession.TryGetSlot(presenceSessionId, slot, out var one) && !ItemWire602.IsEmpty(one))
        {
            totalCost = ShopItemPricing.RepairCost(one);
            if (totalCost > 0 && player.Zen >= totalCost)
            {
                player.Zen -= totalCost;
                ItemWire602.SetDurability(one, 255);
                PlayerShopSession.SetSlot(presenceSessionId, slot, one);
                PlayerShopSession.PersistSlotToMirror(accountId, characterName10, slot, one);
            }
            else
            {
                totalCost = 0;
            }
        }

        if (totalCost > 0)
        {
            onRosterDirty?.Invoke();
        }

        await writeAsync(ShopCommerceWire602.BuildRepair((uint)Math.Clamp(player.Zen, 0, uint.MaxValue)), ct)
            .ConfigureAwait(false);
        Console.WriteLine("[m8] shop repair slot={0} cost={1} zen={2} {3}", slot, totalCost, player.Zen, remote);
        return true;
    }
}
