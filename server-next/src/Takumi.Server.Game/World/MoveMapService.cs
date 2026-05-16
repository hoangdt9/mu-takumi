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
        NotEnoughZen,
        GateFailed,
    }

    public static bool TryResolve(
        int moveIndex,
        int playerLevel,
        long playerZen,
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
        if (move.MinLevel >= 0 && playerLevel < move.MinLevel)
        {
            reason = DenyReason.LevelTooLow;
            return false;
        }

        if (move.MaxLevel >= 0 && playerLevel > move.MaxLevel)
        {
            reason = DenyReason.LevelTooHigh;
            return false;
        }

        if (playerZen < move.Money)
        {
            reason = DenyReason.NotEnoughZen;
            return false;
        }

        if (!MapGateService.TryResolveWarpGate(move.Gate, playerLevel, previousMap, out dest))
        {
            reason = DenyReason.GateFailed;
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
            _ => MoveMapWire602.ResultFailed,
        };
}
