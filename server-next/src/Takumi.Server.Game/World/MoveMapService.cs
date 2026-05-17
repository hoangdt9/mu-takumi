using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>Validates move-map requests and resolves gate destinations (parity <c>CMove::Move</c>).</summary>
public static class MoveMapService
{
    public enum DenyReason
    {
        None,
        UnknownIndex,
        LevelTooLow,
        LevelTooHigh,
        ResetTooLow,
        ResetTooHigh,
        AccountLevelTooLow,
        NotEnoughZen,
        GateFailed,
        PkMurderer,
        TeleportInProgress,
        UiBlocked,
        DeadRegen,
        EquipNotEnough,
        EquipWearing,
        GensRequired,
        CustomArenaBlocked,
        CustomArenaNotOpen,
        CustomArenaClass,
    }

    public static bool TryResolve(
        int moveIndex,
        MoveMapPlayerContext player,
        byte previousMap,
        out MapGateService.TeleportDestination dest,
        out DenyReason reason,
        out int zenCost)
    {
        dest = default;
        reason = DenyReason.None;
        zenCost = 0;

        if (!MoveMapCatalog.TryGet(moveIndex, out var move) || move is null)
        {
            reason = DenyReason.UnknownIndex;
            return false;
        }

        zenCost = move.Money;
        if (move.MinLevel >= 0 && player.Level < move.MinLevel)
        {
            reason = DenyReason.LevelTooLow;
            return false;
        }

        if (move.MaxLevel >= 0 && player.Level > move.MaxLevel)
        {
            reason = DenyReason.LevelTooHigh;
            return false;
        }

        if (move.MinReset >= 0 && player.Reset < move.MinReset)
        {
            reason = DenyReason.ResetTooLow;
            return false;
        }

        if (move.MaxReset >= 0 && player.Reset > move.MaxReset)
        {
            reason = DenyReason.ResetTooHigh;
            return false;
        }

        if (move.AccountLevel > 0 && player.AccountLevel < move.AccountLevel)
        {
            reason = DenyReason.AccountLevelTooLow;
            return false;
        }

        if (player.Zen < move.Money)
        {
            reason = DenyReason.NotEnoughZen;
            return false;
        }

        if (!MoveMapSessionRules.PkLimitFreeEnabled() && player.PkLevel >= 5)
        {
            reason = DenyReason.PkMurderer;
            return false;
        }

        if (player.TeleportInProgress)
        {
            reason = DenyReason.TeleportInProgress;
            return false;
        }

        if (player.ShopWarehouseOrTradeOpen)
        {
            reason = DenyReason.UiBlocked;
            return false;
        }

        if (player.IsDead)
        {
            reason = DenyReason.DeadRegen;
            return false;
        }

        if (!MapGateService.TryResolveWarpGate(move.Gate, player.Level, previousMap, out dest))
        {
            reason = DenyReason.GateFailed;
            return false;
        }

        if (player.GensFamily == 0 && MapManagerCatalog.IsGensBattleMap(dest.MapId))
        {
            reason = DenyReason.GensRequired;
            return false;
        }

        if (!CustomArenaMoveRules.TryAllowEntry(player, move.Gate, dest.MapId, out reason))
        {
            return false;
        }

        if (MoveMapEquipRules.BlocksWarpToMap(dest.MapId, player.PresenceSessionId, out var wearing))
        {
            reason = wearing ? DenyReason.EquipWearing : DenyReason.EquipNotEnough;
            return false;
        }

        return true;
    }

    public static byte ToWireResult(DenyReason reason) =>
        reason switch
        {
            DenyReason.None => MoveMapWire602.ResultSuccess,
            DenyReason.NotEnoughZen => MoveMapWire602.ResultNotEnoughZen,
            DenyReason.LevelTooLow or DenyReason.LevelTooHigh => MoveMapWire602.ResultNotEnoughLevel,
            DenyReason.PkMurderer => MoveMapWire602.ResultMurderer,
            DenyReason.TeleportInProgress => MoveMapWire602.ResultFailedTeleport,
            DenyReason.UiBlocked => MoveMapWire602.ResultFailedPShopOpen,
            DenyReason.DeadRegen => MoveMapWire602.ResultFailedRecalled,
            DenyReason.EquipNotEnough => MoveMapWire602.ResultNotEnoughEquip,
            DenyReason.EquipWearing => MoveMapWire602.ResultWearingEquip,
            DenyReason.GensRequired => MoveMapWire602.ResultGensRequired,
            DenyReason.CustomArenaBlocked or DenyReason.CustomArenaNotOpen or DenyReason.CustomArenaClass =>
                MoveMapWire602.ResultCustomArena,
            _ => MoveMapWire602.ResultFailed,
        };
}
