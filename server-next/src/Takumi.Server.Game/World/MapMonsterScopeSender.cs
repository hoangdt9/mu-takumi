using System.Globalization;
using MUnique.OpenMU.Network;
using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

public static class MapMonsterScopeSender
{
    public static byte[]? BuildViewportPacket(byte mapId, byte playerX, byte playerY)
    {
        MapMonsterWorld.EnsureInitialized();

        var viewRange = ParseIntEnv("TAKUMI_MONSTER_VIEW_RANGE", 15, 1, 32);
        var maxCount = ParseIntEnv("TAKUMI_MONSTER_VIEWPORT_MAX", 64, 1, 120);
        var monsters = MapMonsterWorld.GetMonstersNear(mapId, playerX, playerY, viewRange, maxCount);
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
        Connection connection,
        (byte K1, byte K2)? clientProtectOutbound,
        byte mapId,
        byte playerX,
        byte playerY,
        string remote,
        CancellationToken ct)
    {
        var pkt = BuildViewportPacket(mapId, playerX, playerY);
        if (pkt is null || pkt.Length == 0)
        {
            Console.WriteLine("[{0}] [m9] no monsters in view for map={1} xy=({2},{3})", remote, mapId, playerX, playerY);
            return;
        }

        await GamePortOutboundWire.WriteAsync(connection, clientProtectOutbound, pkt, ct).ConfigureAwait(false);
        var count = pkt[4];
        Console.WriteLine(
            "[{0}] [m9] sent C2 0x13 monster viewport count={1} map={2} wireLen={3}",
            remote,
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
