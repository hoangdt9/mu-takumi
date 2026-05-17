namespace Takumi.Server.Game.World;

/// <summary>Subset of <c>CCustomArena::CheckEnterEnabled</c> for move-map warps to map 40.</summary>
public static class CustomArenaMoveRules
{
    public static bool TryAllowEntry(
        MoveMapPlayerContext player,
        int gate,
        byte destinationMapId,
        out MoveMapService.DenyReason reason)
    {
        reason = MoveMapService.DenyReason.None;

        if (!MapManagerCatalog.IsCustomArenaMap(destinationMapId))
        {
            return true;
        }

        if (player.ShopWarehouseOrTradeOpen)
        {
            reason = MoveMapService.DenyReason.UiBlocked;
            return false;
        }

        if (MapManagerCatalog.IsCustomArenaMap(player.CurrentMapId))
        {
            reason = MoveMapService.DenyReason.CustomArenaBlocked;
            return false;
        }

        if (!CustomArenaCatalog.TryGetByStartGate(gate, out var rule) || rule is null)
        {
            reason = MoveMapService.DenyReason.CustomArenaBlocked;
            return false;
        }

        if (!CustomArenaCatalog.SkipScheduleCheck()
            && !CustomArenaScheduleFsm.IsEnterEnabled(rule.Index))
        {
            reason = MoveMapService.DenyReason.CustomArenaNotOpen;
            return false;
        }

        if (rule.MinLevel >= 0 && player.Level < rule.MinLevel)
        {
            reason = MoveMapService.DenyReason.LevelTooLow;
            return false;
        }

        if (rule.MaxLevel >= 0 && player.Level > rule.MaxLevel)
        {
            reason = MoveMapService.DenyReason.LevelTooHigh;
            return false;
        }

        if (rule.MinReset >= 0 && player.Reset < rule.MinReset)
        {
            reason = MoveMapService.DenyReason.ResetTooLow;
            return false;
        }

        if (rule.MaxReset >= 0 && player.Reset > rule.MaxReset)
        {
            reason = MoveMapService.DenyReason.ResetTooHigh;
            return false;
        }

        var classIndex = ClassIndexFromServerClass(player.ServerClass);
        if (classIndex < 0
            || classIndex >= rule.RequireClass.Length
            || rule.RequireClass[classIndex] == 0)
        {
            reason = MoveMapService.DenyReason.CustomArenaClass;
            return false;
        }

        return true;
    }

    static int ClassIndexFromServerClass(byte serverClass) =>
        serverClass switch
        {
            < 16 => 0,
            < 32 => 1,
            < 48 => 2,
            < 64 => 3,
            < 80 => 4,
            < 96 => 5,
            _ => 6,
        };
}
