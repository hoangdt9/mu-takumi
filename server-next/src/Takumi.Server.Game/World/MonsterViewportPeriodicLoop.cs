using System.Globalization;
using Takumi.Server.Game.Networking;
using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>Legacy ~1s viewport rescan per connected player (parity <c>gObjViewportListProtocolCreate</c>).</summary>
public static class MonsterViewportPeriodicLoop
{
    static Task? _task;

    public static void Start(CancellationToken appCt)
    {
        var ms = ParseIntEnv("TAKUMI_MONSTER_VIEWPORT_PERIODIC_MS", 1000, 0, 60_000);
        if (ms <= 0 || _task is not null)
        {
            return;
        }

        _task = Task.Run(() => RunAsync(TimeSpan.FromMilliseconds(ms), appCt), appCt);
        Console.WriteLine("[m9-vp] periodic viewport loop started intervalMs={0}", ms);
    }

    static async Task RunAsync(TimeSpan interval, CancellationToken ct)
    {
        while (!ct.IsCancellationRequested)
        {
            try
            {
                await TickAllSessionsAsync(ct).ConfigureAwait(false);
            }
            catch (OperationCanceledException) when (ct.IsCancellationRequested)
            {
                break;
            }
            catch (Exception ex)
            {
                Console.WriteLine("[m9-vp] periodic tick error: {0}", ex.Message);
            }

            try
            {
                await Task.Delay(interval, ct).ConfigureAwait(false);
            }
            catch (OperationCanceledException) when (ct.IsCancellationRequested)
            {
                break;
            }
        }
    }

    static async Task TickAllSessionsAsync(CancellationToken ct)
    {
        MapMonsterWorld.EnsureInitialized();
        var viewRange = ParseIntEnv("TAKUMI_MONSTER_VIEW_RANGE", 15, 1, 32);
        var maxNpcs = ParseIntEnv("TAKUMI_MONSTER_VIEWPORT_MAX_NPC", 32, 1, 80);
        var maxMobs = ParseIntEnv("TAKUMI_MONSTER_VIEWPORT_MAX_MOB", 48, 1, 80);

        foreach (var session in MonsterViewerRegistry.GetAllSessions())
        {
            if (session.ViewportTracker is null)
            {
                continue;
            }

            var entities = MapMonsterWorld.GetViewportEntities(
                session.MapId,
                session.X,
                session.Y,
                viewRange,
                maxNpcs,
                maxMobs);
            var (fresh, left) = session.ViewportTracker.SyncView(entities);
            if (left.Count == 0 && fresh.Count == 0)
            {
                continue;
            }

            if (left.Count > 0)
            {
                var destroyPkt = MonsterViewportDestroyWire602.Build(left);
                await GamePortOutboundWire.WriteAsync(session.Connection, session.Protect, destroyPkt, ct)
                    .ConfigureAwait(false);
            }

            var spawnPkt = MapMonsterScopeSender.BuildViewportPacketForInstances(fresh);
            if (spawnPkt is not null && spawnPkt.Length > 0)
            {
                await GamePortOutboundWire.WriteAsync(session.Connection, session.Protect, spawnPkt, ct)
                    .ConfigureAwait(false);
            }
        }
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
