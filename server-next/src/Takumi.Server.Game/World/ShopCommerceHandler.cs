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

        if (ClientGameplayPackets602.TryFindBuyConfirmRequest(packet, out _, out var confirmSlot))
        {
            return await HandleBuyAsync(
                    player,
                    presenceSessionId,
                    accountId,
                    characterName10,
                    confirmSlot,
                    writeAsync,
                    onRosterDirty,
                    remote,
                    ct,
                    fromConfirm: true)
                .ConfigureAwait(false);
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
                    ct,
                    fromConfirm: false)
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

        if (ClientGameplayPackets602.TryFindRepairRequest(packet, out _, out var repairSlot, out var repairType))
        {
            return await HandleRepairAsync(
                    player,
                    presenceSessionId,
                    accountId,
                    characterName10,
                    repairSlot,
                    repairType,
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
        CancellationToken ct,
        bool fromConfirm = false)
    {
        if (!fromConfirm && IsShopBuyConfirmEnabled())
        {
            if (!PlayerShopSession.TryGetShopIndex(presenceSessionId, out var shopIndexForPrompt))
            {
                await writeAsync(ShopCommerceWire602.BuildBuy(ShopCommerceWire602.BuyShopClosedIndex, stackalloc byte[ItemWire602.WireBytes]), ct)
                    .ConfigureAwait(false);
                return true;
            }

            if (!NpcShopCatalog.TryGetShopItem(shopIndexForPrompt, shopSlot, out var preview) || preview is null)
            {
                await writeAsync(ShopCommerceWire602.BuildBuyFail(), ct).ConfigureAwait(false);
                return true;
            }

            PlayerShopSession.SetPendingBuy(presenceSessionId, shopSlot);
            await writeAsync(ShopBuyConfirmWire602.Build(shopSlot), ct).ConfigureAwait(false);
            Console.WriteLine("[m8] shop buy confirm prompt slot={0} shop={1} {2}", shopSlot, shopIndexForPrompt, remote);
            return true;
        }

        if (fromConfirm && PlayerShopSession.TryGetPendingBuy(presenceSessionId, out var pending) && pending != shopSlot)
        {
            await writeAsync(ShopCommerceWire602.BuildBuyFail(), ct).ConfigureAwait(false);
            return true;
        }

        PlayerShopSession.ClearPendingBuy(presenceSessionId);

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

        var indexForCoin = (shopItem.ItemGroup * 512) + shopItem.ItemIndex;
        var usesCoin = ItemValueCatalog.IsCoinOnlyPrice(indexForCoin, shopItem.ItemLevel, shopItem.ExcOpt);
        long price;
        int coinPriceType;
        if (usesCoin)
        {
            if (!ItemValueCatalog.TryGetWirePrice(indexForCoin, shopItem.ItemLevel, shopItem.ExcOpt, out coinPriceType, out var coinPrice, out _)
                || coinPriceType is < 1 or > 3
                || coinPrice <= 0)
            {
                await writeAsync(ShopCommerceWire602.BuildBuyFail(), ct).ConfigureAwait(false);
                Console.WriteLine("[m8] shop buy coin price missing index={0} {1}", indexForCoin, remote);
                return true;
            }

            await AccountWalletSession.EnsureLoadedAsync(accountId, ct).ConfigureAwait(false);
            if (!AccountWalletSession.TryDebitCoin(accountId, coinPriceType, coinPrice, out var coinReason))
            {
                await writeAsync(ShopCommerceWire602.BuildBuyFail(), ct).ConfigureAwait(false);
                Console.WriteLine(
                    "[m8] shop buy coin debit fail type={0} need={1} reason={2} {3}",
                    coinPriceType,
                    coinPrice,
                    coinReason,
                    remote);
                return true;
            }

            price = 0;
        }
        else
        {
            coinPriceType = 0;
            price = ShopItemValueResolver.ResolveChargedBuy(
                shopItem,
                PlayerShopSession.GetTaxRatePercent(presenceSessionId));
            if (player.Zen < price)
            {
                await writeAsync(ShopCommerceWire602.BuildBuyFail(), ct).ConfigureAwait(false);
                Console.WriteLine("[m8] shop buy no zen need={0} have={1} {2}", price, player.Zen, remote);
                return true;
            }
        }

        var blob = new byte[ItemWire602.WireBytes];
        ShopItemWireEncoding.WriteShopEntry(blob, shopItem);

        // NPC shop: always a new bag anchor (no durability merge — repeat buys stack visually as one item).
        if (!PlayerShopSession.TryFindEmptyBagSlot(presenceSessionId, blob, out var bagSlot))
        {
            await writeAsync(ShopCommerceWire602.BuildBuyFail(), ct).ConfigureAwait(false);
            return true;
        }

        PlayerShopSession.SetSlot(presenceSessionId, bagSlot, blob);
        var buyWire = blob;

        var zenBeforeBuy = player.Zen;
        player.Zen -= price;
        onRosterDirty?.Invoke();

        ItemSizeCatalog.GetSize(buyWire, out var iw, out var ih);
        // Parity OpenMU BuyNpcItemAction + legacy ReceiveBuy: C1 0x32 InsertItem at slot only (no F3 10 wipe).
        await writeAsync(ShopCommerceWire602.BuildBuy(bagSlot, buyWire), ct).ConfigureAwait(false);
        var wireGold = (uint)Math.Clamp(player.Zen, 0, uint.MaxValue);
        await writeAsync(ItemWorldWire602.BuildInventoryMoneyUpdate(wireGold), ct).ConfigureAwait(false);
        await writeAsync(ShopCommerceWire602.BuildRepair(wireGold), ct).ConfigureAwait(false);
        await PlayerShopSession.PersistAsync(presenceSessionId, accountId, characterName10, player.Zen, ct)
            .ConfigureAwait(false);
        if (!usesCoin)
        {
            ShopPriceTrace.BuyTransaction(
                remote,
                shopSlot,
                zenBeforeBuy,
                price,
                player.Zen,
                shopItem,
                PlayerShopSession.GetTaxRatePercent(presenceSessionId));
        }

        if (PlayerShopSession.TryGetSessionSlots(presenceSessionId, out var snap))
        {
            var cells = InventoryBagGrid.CountOccupiedBagCells(snap);
            Console.WriteLine(
                usesCoin
                    ? "[m8] shop buy shop={0} slot={1} → inv={2} ({3}x{4}) zen={5} bagCells={6}/{7} coinType={8} {9}"
                    : "[m8] shop buy shop={0} slot={1} → inv={2} ({3}x{4}) zen={5} bagCells={6}/{7} price={8} {9}",
                shopIndex,
                shopSlot,
                bagSlot,
                iw,
                ih,
                player.Zen,
                cells,
                InventoryBagGrid.CellCount,
                usesCoin ? coinPriceType : price,
                remote);
        }
        return true;
    }

    static bool IsShopBuyConfirmEnabled()
    {
        var raw = Environment.GetEnvironmentVariable("TAKUMI_SHOP_BUY_CONFIRM");
        if (string.IsNullOrWhiteSpace(raw))
        {
            return false;
        }

        return !string.Equals(raw.Trim(), "0", StringComparison.OrdinalIgnoreCase)
               && !string.Equals(raw.Trim(), "false", StringComparison.OrdinalIgnoreCase);
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

        var zenBefore = player.Zen;
        var price = ShopItemPricing.SellPrice(blob);
        player.Zen += price;
        onRosterDirty?.Invoke();
        var empty = new byte[ItemWire602.WireBytes];
        PlayerShopSession.SetSlot(presenceSessionId, invSlot, empty);

        await writeAsync(ShopCommerceWire602.BuildSell(1, (uint)Math.Clamp(player.Zen, 0, uint.MaxValue)), ct)
            .ConfigureAwait(false);
        await PlayerShopSession.PersistAsync(presenceSessionId, accountId, characterName10, player.Zen, ct)
            .ConfigureAwait(false);
        ShopPriceTrace.SellTransaction(remote, invSlot, zenBefore, price, player.Zen, blob);
        Console.WriteLine("[m8] shop sell inv={0} +{1} zen={2} {3}", invSlot, price, player.Zen, remote);
        return true;
    }

    static async Task<bool> HandleRepairAsync(
        GameRosterEntry player,
        Guid presenceSessionId,
        string? accountId,
        byte[] characterName10,
        byte slot,
        byte repairType,
        Func<ReadOnlyMemory<byte>, CancellationToken, Task> writeAsync,
        Action? onRosterDirty,
        string remote,
        CancellationToken ct)
    {
        var inNpcShop = PlayerShopSession.TryGetShopIndex(presenceSessionId, out _);
        // C1 0x34 byte2: 0 = NPC shop hammer, 1 = self-repair (level 6+). Outside shop only type 1 is valid.
        if (!inNpcShop && repairType == 0)
        {
            await writeAsync(ShopCommerceWire602.BuildRepair((uint)Math.Max(0, player.Zen)), ct).ConfigureAwait(false);
            return true;
        }

        await PlayerShopSession.EnsureInventoryLoadedAsync(presenceSessionId, accountId, characterName10, ct)
            .ConfigureAwait(false);

        var selfRepair = repairType != 0;
        long totalCost = 0;
        var repairedAny = false;
        if (slot == 0xFF)
        {
            for (byte s = 0; s <= ItemWire602.LastBagSlot; s++)
            {
                if (!TryRepairSlot(presenceSessionId, player, s, selfRepair, ref totalCost))
                {
                    continue;
                }

                repairedAny = true;
            }
        }
        else if (TryRepairSlot(presenceSessionId, player, slot, selfRepair, ref totalCost))
        {
            repairedAny = true;
        }

        if (totalCost > 0)
        {
            onRosterDirty?.Invoke();
        }

        await writeAsync(ShopCommerceWire602.BuildRepair((uint)Math.Clamp(player.Zen, 0, uint.MaxValue)), ct)
            .ConfigureAwait(false);
        if (repairedAny)
        {
            var invSync = PlayerShopSession.BuildInventoryListPacket(presenceSessionId);
            if (invSync.Length > 0)
            {
                await writeAsync(invSync, ct).ConfigureAwait(false);
            }

            await PlayerShopSession.PersistAsync(presenceSessionId, accountId, characterName10, player.Zen, ct)
                .ConfigureAwait(false);
        }

        Console.WriteLine(
            "[m8] repair slot={0} type={1} shop={2} cost={3} zen={4} synced={5} {6}",
            slot,
            repairType,
            inNpcShop,
            totalCost,
            player.Zen,
            repairedAny,
            remote);
        return true;
    }

    static bool TryRepairSlot(
        Guid presenceSessionId,
        GameRosterEntry player,
        byte slot,
        bool selfRepair,
        ref long totalCost)
    {
        if (!PlayerShopSession.TryGetSlot(presenceSessionId, slot, out var blob) || ItemWire602.IsEmpty(blob))
        {
            return false;
        }

        var cost = ShopItemPricing.RepairCost(blob, selfRepair);
        if (cost <= 0 || player.Zen < cost)
        {
            return false;
        }

        player.Zen -= cost;
        totalCost += cost;
        var maxDur = ItemSizeCatalog.GetMaxDurability(blob);
        ItemWire602.SetDurability(blob, maxDur);
        PlayerShopSession.SetSlot(presenceSessionId, slot, blob);
        return true;
    }
}
