using Takumi.Server.Game.Networking;
using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>Broadcast viewport destroy/spawn to every viewer in range (parity regen + shared death).</summary>
public static class MonsterViewportBroadcast
{
    public static async Task BroadcastDestroyAsync(MapMonsterInstance monster, CancellationToken ct)
    {
        var pkt = MonsterViewportDestroyWire602.Build([monster.ObjectKey]);
        await MonsterViewerRegistry.BroadcastPacketInViewAsync(
                monster.Map,
                monster.X,
                monster.Y,
                pkt,
                ct)
            .ConfigureAwait(false);
    }

    public static async Task BroadcastSpawnAsync(MapMonsterInstance monster, CancellationToken ct)
    {
        var pkt = MapMonsterScopeSender.BuildViewportPacketForInstances([monster]);
        if (pkt is null || pkt.Length == 0)
        {
            return;
        }

        await MonsterViewerRegistry.BroadcastPacketInViewAsync(
                monster.Map,
                monster.X,
                monster.Y,
                pkt,
                ct)
            .ConfigureAwait(false);
    }

    public static async Task RegenMonsterAsync(MapMonsterInstance monster, CancellationToken ct)
    {
        await BroadcastDestroyAsync(monster, ct).ConfigureAwait(false);
        await BroadcastSpawnAsync(monster, ct).ConfigureAwait(false);
        Console.WriteLine(
            "[m9-vp] regen broadcast key={0} map={1} xy=({2},{3})",
            monster.ObjectKey,
            monster.Map,
            monster.X,
            monster.Y);
    }
}
