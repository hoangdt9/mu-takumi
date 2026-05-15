using System.Collections.Concurrent;
using Takumi.Server.Persistence;
using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>Per-connection NPC shop + bag inventory (stub; SSOT inventory DB = M4b).</summary>
public static class PlayerShopSession
{
    static readonly ConcurrentDictionary<Guid, SessionState> Sessions = new();

    public readonly record struct SessionState(int ShopIndex, Dictionary<byte, byte[]> Slots);

    public static void OpenShop(Guid sessionId, int shopIndex) =>
        Sessions.AddOrUpdate(
            sessionId,
            _ => new SessionState(shopIndex, new Dictionary<byte, byte[]>()),
            (_, existing) => existing with { ShopIndex = shopIndex });

    public static void CloseShop(Guid sessionId)
    {
        if (Sessions.TryGetValue(sessionId, out var s))
        {
            Sessions[sessionId] = s with { ShopIndex = -1 };
        }
    }

    public static bool TryGetShopIndex(Guid sessionId, out int shopIndex)
    {
        if (Sessions.TryGetValue(sessionId, out var s) && s.ShopIndex >= 0)
        {
            shopIndex = s.ShopIndex;
            return true;
        }

        shopIndex = -1;
        return false;
    }

    public static async Task EnsureInventoryLoadedAsync(
        Guid sessionId,
        string? accountId,
        byte[] characterName10,
        CancellationToken ct)
    {
        if (!Sessions.TryGetValue(sessionId, out var s))
        {
            s = new SessionState(-1, new Dictionary<byte, byte[]>());
            Sessions[sessionId] = s;
        }

        if (s.Slots.Count > 0)
        {
            return;
        }

        var rows = await JoinInventoryPacket602
            .LoadRowsAsync(TakumiPostgresMirror.InventorySlots, accountId, characterName10, ct)
            .ConfigureAwait(false);
        foreach (var row in rows)
        {
            s.Slots[row.Slot] = row.Item12.ToArray();
        }
    }

    public static bool TryGetSlot(Guid sessionId, byte slot, out byte[] item12)
    {
        item12 = Array.Empty<byte>();
        if (!Sessions.TryGetValue(sessionId, out var s))
        {
            return false;
        }

        if (s.Slots.TryGetValue(slot, out var blob))
        {
            item12 = blob;
            return true;
        }

        return false;
    }

    public static void SetSlot(Guid sessionId, byte slot, byte[] item12)
    {
        var s = Sessions.GetOrAdd(sessionId, _ => new SessionState(-1, new Dictionary<byte, byte[]>()));
        if (ItemWire602.IsEmpty(item12))
        {
            s.Slots.Remove(slot);
        }
        else
        {
            s.Slots[slot] = item12;
        }
    }

    public static bool TryFindEmptyBagSlot(Guid sessionId, out byte slot)
    {
        if (!Sessions.TryGetValue(sessionId, out var s))
        {
            slot = 0;
            return false;
        }

        for (var i = ItemWire602.FirstBagSlot; i <= ItemWire602.LastBagSlot; i++)
        {
            if (!s.Slots.TryGetValue((byte)i, out var blob) || ItemWire602.IsEmpty(blob))
            {
                slot = (byte)i;
                return true;
            }
        }

        slot = 0;
        return false;
    }

    public static void RemoveSession(Guid sessionId) => Sessions.TryRemove(sessionId, out _);
}
