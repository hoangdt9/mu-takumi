using MUnique.OpenMU.Network;
using Takumi.Server.Game.Networking;

namespace Takumi.Server.Game.World;

/// <summary>Handles custom NPC-position warps before shop/quest talk.</summary>
public static class CustomNpcMoveHandler
{
    public static async Task<bool> TryHandleNpcTalkWarpAsync(
        GameRosterEntry player,
        MonsterViewportTracker tracker,
        Connection? connection,
        (byte K1, byte K2)? clientProtectOutbound,
        string? accountId,
        byte[] characterName10,
        Guid presenceSessionId,
        int monsterClass,
        byte npcMap,
        byte npcX,
        byte npcY,
        Func<ReadOnlyMemory<byte>, CancellationToken, Task> writeAsync,
        Action? onRosterDirty,
        Action? onRosterSave,
        string remote,
        CancellationToken ct)
    {
        if (!CustomNpcMoveService.TryMatchNpc(monsterClass, npcMap, npcX, npcY, out var move) || move is null)
        {
            return false;
        }

        var prevMap = player.MapId;
        var ctx = MoveMapSessionRules.BuildContext(player, presenceSessionId);
        if (!CustomNpcMoveService.TryValidate(move, ctx, masterReset: 0, out var deny))
        {
            Console.WriteLine(
                "[m8] custom npc move denied index={0} npc={1} map={2} ({3},{4}) reason={5} {6}",
                move.Index,
                monsterClass,
                npcMap,
                npcX,
                npcY,
                deny,
                remote);
            return true;
        }

        if (!CustomNpcMoveService.TryResolveDestination(move, prevMap, out var dest))
        {
            Console.WriteLine(
                "[m8] custom npc move blocked destination index={0} map={1} ({2},{3}) {4}",
                move.Index,
                move.DestinationMap,
                move.DestinationX,
                move.DestinationY,
                remote);
            return true;
        }

        MoveMapSessionState.SetTeleportInProgress(presenceSessionId, true);
        try
        {
            await MoveWarpExecutor.ApplyAsync(
                    player,
                    tracker,
                    connection,
                    clientProtectOutbound,
                    accountId,
                    characterName10,
                    presenceSessionId,
                    dest,
                    prevMap,
                    writeAsync,
                    remote,
                    onRosterDirty,
                    onRosterSave,
                    reseedMoveMapKey: false,
                    logPrefix: $"custom npc move index={move.Index}",
                    ct)
                .ConfigureAwait(false);
        }
        finally
        {
            MoveMapSessionState.SetTeleportInProgress(presenceSessionId, false);
        }

        return true;
    }
}
