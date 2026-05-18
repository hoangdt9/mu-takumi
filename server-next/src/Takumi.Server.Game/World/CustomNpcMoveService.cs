namespace Takumi.Server.Game.World;

/// <summary>NPC talk warp (parity <c>CCustomNpcMove::GetNpcMove</c> + <c>gObjTeleport</c>).</summary>
public static class CustomNpcMoveService
{
    public enum DenyReason
    {
        None,
        LevelTooLow,
        LevelTooHigh,
        ResetTooLow,
        ResetTooHigh,
        MasterResetTooLow,
        MasterResetTooHigh,
        AccountLevelTooLow,
        PkMurderer,
        UiBlocked,
        DeadRegen,
        TeleportInProgress,
        EquipNotEnough,
        EquipWearing,
        GensRequired,
        DestinationBlocked,
    }

    public static bool TryMatchNpc(int monsterClass, byte map, byte x, byte y, out CustomNpcMoveEntry? entry) =>
        CustomNpcMoveCatalog.TryGetByNpc(monsterClass, map, x, y, out entry);

    public static bool TryValidate(
        CustomNpcMoveEntry move,
        MoveMapPlayerContext ctx,
        int masterReset,
        out DenyReason reason)
    {
        reason = DenyReason.None;
        if (move.MinLevel >= 0 && ctx.Level < move.MinLevel)
        {
            reason = DenyReason.LevelTooLow;
            return false;
        }

        if (move.MaxLevel >= 0 && ctx.Level > move.MaxLevel)
        {
            reason = DenyReason.LevelTooHigh;
            return false;
        }

        if (move.MinReset >= 0 && ctx.Reset < move.MinReset)
        {
            reason = DenyReason.ResetTooLow;
            return false;
        }

        if (move.MaxReset >= 0 && ctx.Reset > move.MaxReset)
        {
            reason = DenyReason.ResetTooHigh;
            return false;
        }

        if (move.MinMasterReset >= 0 && masterReset < move.MinMasterReset)
        {
            reason = DenyReason.MasterResetTooLow;
            return false;
        }

        if (move.MaxMasterReset >= 0 && masterReset > move.MaxMasterReset)
        {
            reason = DenyReason.MasterResetTooHigh;
            return false;
        }

        if (ctx.AccountLevel < move.AccountLevel)
        {
            reason = DenyReason.AccountLevelTooLow;
            return false;
        }

        if (move.PkMove == 0 && !MoveMapSessionRules.PkLimitFreeEnabled() && ctx.PkLevel >= 5)
        {
            reason = DenyReason.PkMurderer;
            return false;
        }

        if (ctx.ShopWarehouseOrTradeOpen)
        {
            reason = DenyReason.UiBlocked;
            return false;
        }

        if (ctx.IsDead)
        {
            reason = DenyReason.DeadRegen;
            return false;
        }

        if (ctx.TeleportInProgress)
        {
            reason = DenyReason.TeleportInProgress;
            return false;
        }

        if (MoveMapEquipRules.BlocksWarpToMap(move.DestinationMap, ctx.PresenceSessionId, out var wearing))
        {
            reason = wearing ? DenyReason.EquipWearing : DenyReason.EquipNotEnough;
            return false;
        }

        if (ctx.GensFamily == 0 && MapManagerCatalog.IsGensBattleMap(move.DestinationMap))
        {
            reason = DenyReason.GensRequired;
            return false;
        }

        if (!MapAttWalkability.CanWalk(move.DestinationMap, move.DestinationX, move.DestinationY)
            && !MapAttWalkability.TryFindNearestWalkable(
                move.DestinationMap,
                move.DestinationX,
                move.DestinationY,
                out _,
                out _))
        {
            reason = DenyReason.DestinationBlocked;
            return false;
        }

        return true;
    }

    public static bool TryResolveDestination(
        CustomNpcMoveEntry move,
        byte previousMap,
        out MapGateService.TeleportDestination dest)
    {
        dest = default;
        var x = move.DestinationX;
        var y = move.DestinationY;
        if (!MapAttWalkability.CanWalk(move.DestinationMap, x, y))
        {
            if (!MapAttWalkability.TryFindNearestWalkable(move.DestinationMap, x, y, out x, out y))
            {
                return false;
            }
        }

        dest = new MapGateService.TeleportDestination(
            move.DestinationMap,
            x,
            y,
            0,
            move.DestinationMap != previousMap);
        return true;
    }
}
