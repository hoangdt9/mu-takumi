using System.Collections.Concurrent;
using System.Globalization;
using MUnique.OpenMU.Network;
using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

public static class MapMonsterScopeSender
{
    static readonly ConcurrentDictionary<string, long> s_lastMoveSyncTickMs = new(StringComparer.Ordinal);
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
        var maxNpcs = ParseIntEnv("TAKUMI_MONSTER_VIEWPORT_MAX_NPC", 32, 1, 80);
        var maxMobs = ParseIntEnv("TAKUMI_MONSTER_VIEWPORT_MAX_MOB", 48, 1, 80);
        var monsters = onlyNew
            ?? MapMonsterWorld.GetViewportEntities(mapId, playerX, playerY, viewRange, maxNpcs, maxMobs);
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

    public static byte[]? BuildViewportPacketForInstances(IReadOnlyList<MapMonsterInstance> monsters)
    {
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
        var maxNpcs = ParseIntEnv("TAKUMI_MONSTER_VIEWPORT_MAX_NPC", 32, 1, 80);
        var maxMobs = ParseIntEnv("TAKUMI_MONSTER_VIEWPORT_MAX_MOB", 48, 1, 80);
        var monsters = MapMonsterWorld.GetViewportEntities(mapId, playerX, playerY, viewRange, maxNpcs, maxMobs);
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

        var minIntervalMs = ParseIntEnv("TAKUMI_MONSTER_VIEWPORT_MIN_INTERVAL_MS", 400, 0, 5000);
        if (minIntervalMs > 0)
        {
            var nowMs = Environment.TickCount64;
            if (s_lastMoveSyncTickMs.TryGetValue(remote, out var lastMs) && nowMs - lastMs < minIntervalMs)
            {
                return;
            }

            s_lastMoveSyncTickMs[remote] = nowMs;
        }

        var viewRange = ParseIntEnv("TAKUMI_MONSTER_VIEW_RANGE", 15, 1, 32);
        var maxNpcs = ParseIntEnv("TAKUMI_MONSTER_VIEWPORT_MAX_NPC", 32, 1, 80);
        var maxMobs = ParseIntEnv("TAKUMI_MONSTER_VIEWPORT_MAX_MOB", 48, 1, 80);
        var monsters = MapMonsterWorld.GetViewportEntities(mapId, playerX, playerY, viewRange, maxNpcs, maxMobs);
        var (fresh, left) = tracker.SyncView(monsters);
        tracker.NoteAnchor(mapId, playerX, playerY);
        var destroyKeys = FilterViewportDestroyKeys(left);
        await SendDestroyAsync(connection, clientProtectOutbound, destroyKeys, remote, ct).ConfigureAwait(false);
        await SendPacketAsync(connection, clientProtectOutbound, mapId, playerX, playerY, fresh, remote, "move", ct)
            .ConfigureAwait(false);
    }

    static async Task SendDestroyAsync(
        Connection connection,
        (byte K1, byte K2)? clientProtectOutbound,
        IReadOnlyList<int> objectKeys,
        string remote,
        CancellationToken ct)
    {
        if (objectKeys.Count == 0)
        {
            return;
        }

        var pkt = MonsterViewportDestroyWire602.Build(objectKeys);
        await GamePortOutboundWire.WriteAsync(connection, clientProtectOutbound, pkt, ct).ConfigureAwait(false);
        Console.WriteLine(
            "[{0}] [m9] sent C1 0x14 destroy viewport count={1}",
            remote,
            objectKeys.Count);
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

    internal static IReadOnlyList<int> FilterViewportDestroyKeysForPeriodic(IReadOnlyList<int> objectKeys) =>
        FilterViewportDestroyKeys(objectKeys);

    /// <summary>Town NPCs stay visible — only field mobs get <c>0x14</c> on walk (parity client safe-zone vendors).</summary>
    static IReadOnlyList<int> FilterViewportDestroyKeys(IReadOnlyList<int> objectKeys)
    {
        if (objectKeys.Count == 0)
        {
            return objectKeys;
        }

        if (string.Equals(
                Environment.GetEnvironmentVariable("TAKUMI_MONSTER_VIEWPORT_DESTROY_NPC")?.Trim(),
                "1",
                StringComparison.OrdinalIgnoreCase))
        {
            return objectKeys;
        }

        var filtered = new List<int>(objectKeys.Count);
        foreach (var key in objectKeys)
        {
            if (MapMonsterWorld.TryGetMonster(key, out var mob) && mob is { IsNpc: true })
            {
                continue;
            }

            filtered.Add(key);
        }

        return filtered;
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
