using System.Collections.Concurrent;
using System.Text;
using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>Per-connection trade window state (storage flag <see cref="ItemStorageFlags602.Trade"/>).</summary>
public static class PlayerTradeSession
{
    public const byte MaxTradeSlot = 31;

    static readonly ConcurrentDictionary<Guid, TradeState> Sessions = new();

    public readonly record struct TradeState(bool IsOpen, Guid? PartnerSessionId, Dictionary<byte, byte[]> Slots);

    public static bool IsOpen(Guid sessionId) =>
        Sessions.TryGetValue(sessionId, out var s) && s.IsOpen;

    public static void OpenPair(Guid sessionA, Guid sessionB)
    {
        Sessions[sessionA] = new TradeState(true, sessionB, new Dictionary<byte, byte[]>());
        Sessions[sessionB] = new TradeState(true, sessionA, new Dictionary<byte, byte[]>());
    }

    public static void Close(Guid sessionId)
    {
        if (!Sessions.TryRemove(sessionId, out var s))
        {
            return;
        }

        if (s.PartnerSessionId is { } partner && Sessions.TryGetValue(partner, out _))
        {
            Sessions.TryRemove(partner, out _);
        }
    }

    public static bool TryApplyMove(
        Guid sessionId,
        string? accountId,
        byte[] characterName10,
        byte srcFlag,
        byte srcSlot,
        byte dstFlag,
        byte dstSlot,
        out byte[] targetItem)
    {
        targetItem = Array.Empty<byte>();
        _ = accountId;
        _ = characterName10;

        if (!IsOpen(sessionId))
        {
            return false;
        }

        if (srcFlag == ItemStorageFlags602.Trade && dstFlag == ItemStorageFlags602.Trade)
        {
            return TryMoveTradeSlot(sessionId, srcSlot, dstSlot, out targetItem);
        }

        if (srcFlag == ItemStorageFlags602.Inventory && dstFlag == ItemStorageFlags602.Trade)
        {
            return TryMoveInventoryToTrade(sessionId, srcSlot, dstSlot, out targetItem);
        }

        if (srcFlag == ItemStorageFlags602.Trade && dstFlag == ItemStorageFlags602.Inventory)
        {
            return TryMoveTradeToInventory(sessionId, srcSlot, dstSlot, out targetItem);
        }

        return false;
    }

    static bool TryMoveTradeSlot(Guid sessionId, byte srcSlot, byte dstSlot, out byte[] targetItem)
    {
        targetItem = Array.Empty<byte>();
        if (srcSlot > MaxTradeSlot || dstSlot > MaxTradeSlot)
        {
            return false;
        }

        var s = Sessions[sessionId];
        s.Slots.TryGetValue(srcSlot, out var source);
        source ??= Array.Empty<byte>();
        if (ItemWire602.IsEmpty(source))
        {
            return false;
        }

        s.Slots.TryGetValue(dstSlot, out var dest);
        dest ??= Array.Empty<byte>();
        var moved = source.ToArray();
        if (ItemWire602.IsEmpty(dest))
        {
            s.Slots.Remove(srcSlot);
            s.Slots[dstSlot] = moved;
        }
        else
        {
            s.Slots[srcSlot] = dest.ToArray();
            s.Slots[dstSlot] = moved;
        }

        targetItem = moved;
        return true;
    }

    static bool TryMoveInventoryToTrade(Guid sessionId, byte srcSlot, byte dstSlot, out byte[] targetItem)
    {
        targetItem = Array.Empty<byte>();
        if (dstSlot > MaxTradeSlot || !PlayerShopSession.IsInventorySlot(srcSlot))
        {
            return false;
        }

        if (!PlayerShopSession.TryGetSlot(sessionId, srcSlot, out var item) || ItemWire602.IsEmpty(item))
        {
            return false;
        }

        var s = Sessions[sessionId];
        if (s.Slots.TryGetValue(dstSlot, out var dest) && !ItemWire602.IsEmpty(dest))
        {
            return false;
        }

        var moved = item.ToArray();
        PlayerShopSession.SetSlot(sessionId, srcSlot, Array.Empty<byte>());
        s.Slots[dstSlot] = moved;
        targetItem = moved;
        return true;
    }

    static bool TryMoveTradeToInventory(Guid sessionId, byte srcSlot, byte dstSlot, out byte[] targetItem)
    {
        targetItem = Array.Empty<byte>();
        if (srcSlot > MaxTradeSlot || !PlayerShopSession.IsInventorySlot(dstSlot))
        {
            return false;
        }

        var s = Sessions[sessionId];
        if (!s.Slots.TryGetValue(srcSlot, out var item) || ItemWire602.IsEmpty(item))
        {
            return false;
        }

        if (PlayerShopSession.TryGetSlot(sessionId, dstSlot, out var dest) && !ItemWire602.IsEmpty(dest))
        {
            return false;
        }

        var moved = item.ToArray();
        s.Slots.Remove(srcSlot);
        PlayerShopSession.SetSlot(sessionId, dstSlot, moved);
        targetItem = moved;
        return true;
    }

    public static string ReadName10(byte[] name10) =>
        Encoding.ASCII.GetString(name10.AsSpan(0, 10)).TrimEnd('\0', ' ');
}
