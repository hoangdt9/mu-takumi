using System.Globalization;
using MUnique.OpenMU.Network;
using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

public static class MapMonsterScopeSender
{
    public static byte[]? BuildViewportPacket(byte mapId, byte playerX, byte playerY)
    {
        return BuildViewportPacket(mapId, playerX, playerY, onlyNew: null);
    }

    public static byte[]? BuildViewportPacket(
        byte mapId,
        byte playerX,
        byte playerY,
        IReadOnlyList<MapMonsterInstance>? onlyNew)
    {
        MapMonsterWorld.EnsureInitialized();

        var viewRange = ParseIntEnv("TAKUMI_MONSTER_VIEW_RANGE", 15, 1, 32);
        var maxCount = ParseIntEnv("TAKUMI_MONSTER_VIEWPORT_MAX", 64, 1, 120);
        var monsters = onlyNew ?? MapMonsterWorld.GetMonstersNear(mapId, playerX, playerY, viewRange, maxCount);
        if (monsters.Count == 0)
        {
            return null;
        }

        var entries = new List<MonsterViewportEntry>(monsters.Count);
        foreach (var m in monsters)
        {
            entries.Add(
                new MonsterViewportEntry(
                    m.ObjectKey,
                    m.MonsterClass,
                    m.X,
                    m.Y,
                    m.X,
                    m.Y,
                    m.Dir,
                    CreateFlag: true));
        }

        return MonsterViewportWire602.Build(entries);
    }

    public static async Task TrySendAfterJoinAsync(
        MonsterViewportTracker tracker,
        Connection connection,
        (byte K1, byte K2)? clientProtectOutbound,
        byte mapId,
        byte playerX,
        byte playerY,
        string remote,
        CancellationToken ct)
    {
        tracker.ResetForMap(mapId, playerX, playerY);
        var viewRange = ParseIntEnv("TAKUMI_MONSTER_VIEW_RANGE", 15, 1, 32);
        var maxCount = ParseIntEnv("TAKUMI_MONSTER_VIEWPORT_MAX", 64, 1, 120);
        var monsters = MapMonsterWorld.GetMonstersNear(mapId, playerX, playerY, viewRange, maxCount);
        var fresh = tracker.TakeNewInView(monsters);
        await SendPacketAsync(connection, clientProtectOutbound, mapId, playerX, playerY, fresh, remote, "join", ct)
            .ConfigureAwait(false);
    }

    public static async Task TrySendOnMoveAsync(
        MonsterViewportTracker tracker,
        Connection connection,
        (byte K1, byte K2)? clientProtectOutbound,
        byte mapId,
        byte playerX,
        byte playerY,
        string remote,
        CancellationToken ct)
    {
        var moveThreshold = ParseIntEnv("TAKUMI_MONSTER_VIEWPORT_MOVE_TILES", 4, 1, 16);
        if (!tracker.ShouldRescan(mapId, playerX, playerY, moveThreshold))
        {
            return;
        }

        var viewRange = ParseIntEnv("TAKUMI_MONSTER_VIEW_RANGE", 15, 1, 32);
        var maxCount = ParseIntEnv("TAKUMI_MONSTER_VIEWPORT_MAX", 64, 1, 120);
        var monsters = MapMonsterWorld.GetMonstersNear(mapId, playerX, playerY, viewRange, maxCount);
        var fresh = tracker.TakeNewInView(monsters);
        tracker.NoteAnchor(mapId, playerX, playerY);
        await SendPacketAsync(connection, clientProtectOutbound, mapId, playerX, playerY, fresh, remote, "move", ct)
            .ConfigureAwait(false);
    }

    static async Task SendPacketAsync(
        Connection connection,
        (byte K1, byte K2)? clientProtectOutbound,
        byte mapId,
        byte playerX,
        byte playerY,
        IReadOnlyList<MapMonsterInstance> monsters,
        string remote,
        string reason,
        CancellationToken ct)
    {
        var pkt = BuildViewportPacket(mapId, playerX, playerY, monsters);
        if (pkt is null || pkt.Length == 0)
        {
            Console.WriteLine(
                "[{0}] [m9] no new monsters in view ({1}) map={2} xy=({3},{4})",
                remote,
                reason,
                mapId,
                playerX,
                playerY);
            return;
        }

        await GamePortOutboundWire.WriteAsync(connection, clientProtectOutbound, pkt, ct).ConfigureAwait(false);
        var count = pkt[4];
        Console.WriteLine(
            "[{0}] [m9] sent C2 0x13 monster viewport ({1}) count={2} map={3} wireLen={4}",
            remote,
            reason,
            count,
            mapId,
            pkt.Length);
    }

    static int ParseIntEnv(string key, int defaultValue, int min, int max)
    {
        var raw = Environment.GetEnvironmentVariable(key);
        if (string.IsNullOrWhiteSpace(raw)
            || !int.TryParse(raw, NumberStyles.Integer, CultureInfo.InvariantCulture, out var v))
        {
            return defaultValue;
        }

        return Math.Clamp(v, min, max);
    }
}
