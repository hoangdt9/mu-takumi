using System.Collections.Concurrent;

namespace Takumi.Server.Game.World;

/// <summary>Per-session UI block flags for move-map (parity <c>CMove::Move</c> Interface / PShopOpen).</summary>
public static class PlayerUiSession
{
    sealed class UiState
    {
        public bool NpcShopOpen;
        public bool WarehouseOpen;
        public bool TradeOpen;
        public bool PersonalShopOpen;
        public int GenericInterfaceRefs;
    }

    static int BumpRefs(UiState state) => Interlocked.Increment(ref state.GenericInterfaceRefs);

    static int DropRefs(UiState state)
    {
        while (true)
        {
            var current = state.GenericInterfaceRefs;
            if (current <= 0)
            {
                return 0;
            }

            if (Interlocked.CompareExchange(ref state.GenericInterfaceRefs, current - 1, current) == current)
            {
                return current - 1;
            }
        }
    }

    static readonly ConcurrentDictionary<Guid, UiState> Sessions = new();

    public static bool IsMoveBlocked(Guid sessionId)
    {
        if (!Sessions.TryGetValue(sessionId, out var s))
        {
            return false;
        }

        return s.NpcShopOpen
               || s.WarehouseOpen
               || s.TradeOpen
               || s.PersonalShopOpen
               || s.GenericInterfaceRefs > 0;
    }

    /// <summary>Warehouse is the only UI flag blocking move-map (client closed UI without <c>0x82</c>).</summary>
    public static bool IsWarehouseOnlyMoveBlock(Guid sessionId)
    {
        if (!Sessions.TryGetValue(sessionId, out var s))
        {
            return false;
        }

        return s.WarehouseOpen
               && !s.NpcShopOpen
               && !s.TradeOpen
               && !s.PersonalShopOpen
               && s.GenericInterfaceRefs <= 0;
    }

    public static bool IsPersonalShopOpen(Guid sessionId) =>
        Sessions.TryGetValue(sessionId, out var s) && s.PersonalShopOpen;

    public static void SetNpcShop(Guid sessionId, bool open) =>
        GetOrAdd(sessionId).NpcShopOpen = open;

    public static void SetWarehouse(Guid sessionId, bool open) =>
        GetOrAdd(sessionId).WarehouseOpen = open;

    public static void SetTrade(Guid sessionId, bool open) =>
        GetOrAdd(sessionId).TradeOpen = open;

    public static void SetPersonalShop(Guid sessionId, bool open) =>
        GetOrAdd(sessionId).PersonalShopOpen = open;

    /// <summary>Chaos box, trainer, guild interface, etc. (ref-counted).</summary>
    public static void AddGenericInterface(Guid sessionId) =>
        BumpRefs(GetOrAdd(sessionId));

    public static void RemoveGenericInterface(Guid sessionId)
    {
        if (Sessions.TryGetValue(sessionId, out var s))
        {
            DropRefs(s);
        }
    }

    public static void Clear(Guid sessionId) => Sessions.TryRemove(sessionId, out _);

    static UiState GetOrAdd(Guid sessionId) =>
        Sessions.GetOrAdd(sessionId, _ => new UiState());
}
