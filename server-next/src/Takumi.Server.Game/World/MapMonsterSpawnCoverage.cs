namespace Takumi.Server.Game.World;

/// <summary>M8×M9: spawn coverage vs <c>Move.txt</c> destinations (ops + dev smoke).</summary>
public static class MapMonsterSpawnCoverage
{
    /// <summary>Move targets with no field mobs by design (siege / market).</summary>
    static readonly HashSet<byte> MoveMapsWithoutFieldMobs = new() { 30, 79 };
    public readonly struct MapSpawnSummary
    {
        public MapSpawnSummary(int total, int npcCount, int fieldCount)
        {
            Total = total;
            NpcCount = npcCount;
            FieldCount = fieldCount;
        }

        public int Total { get; }

        public int NpcCount { get; }

        public int FieldCount { get; }
    }

    public static IReadOnlyDictionary<byte, MapSpawnSummary> BuildSummaryByMap()
    {
        MapMonsterWorld.EnsureInitialized();
        var raw = MapMonsterWorld.GetInstanceCountByMap();
        var result = new Dictionary<byte, MapSpawnSummary>();
        foreach (var (mapId, total) in raw)
        {
            var npc = MapMonsterWorld.GetNpcCountOnMap(mapId);
            result[mapId] = new MapSpawnSummary(total, npc, Math.Max(0, total - npc));
        }

        return result;
    }

    /// <summary>Logs per-map counts and warns when a <c>Move.txt</c> warp target has no field mobs.</summary>
    public static void LogStartupReport()
    {
        var byMap = BuildSummaryByMap();
        if (byMap.Count == 0)
        {
            Console.WriteLine("[m8-m9] monster spawn coverage: no instances (check MonsterSetBase mount / ETL)");
            return;
        }

        var totalInst = byMap.Values.Sum(s => s.Total);
        var mapCount = byMap.Count;
        Console.WriteLine(
            "[m8-m9] monster spawn coverage: {0} instances on {1} map(s)",
            totalInst,
            mapCount);

        foreach (var mapId in byMap.Keys.OrderBy(static id => id))
        {
            var s = byMap[mapId];
            Console.WriteLine(
                "[m8-m9]   map {0,3}: total={1,4} npc={2,4} field={3,4}",
                mapId,
                s.Total,
                s.NpcCount,
                s.FieldCount);
        }

        LogMoveDestinationGaps(byMap);
    }

    static void LogMoveDestinationGaps(IReadOnlyDictionary<byte, MapSpawnSummary> byMap)
    {
        MoveMapCatalog.EnsureInitialized();
        MapGateCatalog.EnsureInitialized();
        if (!MoveMapCatalog.TryGetAllForCoverage(out var moves) || moves.Count == 0)
        {
            return;
        }

        var warned = 0;
        foreach (var move in moves)
        {
            if (!MapGateCatalog.TryGetGate(move.Gate, out var gate) || gate is null)
            {
                Console.WriteLine(
                    "[m8-m9] WARN move index {0}: gate {1} not in Gate.txt",
                    move.Index,
                    move.Gate);
                warned++;
                continue;
            }

            var mapId = gate.MapId;
            if (!byMap.TryGetValue(mapId, out var summary))
            {
                if (MoveMapsWithoutFieldMobs.Contains(mapId))
                {
                    Console.WriteLine(
                        "[m8-m9] move index {0} (gate {1}) -> map {2}: no spawns by design (siege/market map)",
                        move.Index,
                        move.Gate,
                        mapId);
                    continue;
                }

                Console.WriteLine(
                    "[m8-m9] WARN move index {0} (gate {1}) -> map {2}: no spawns in MonsterSetBase",
                    move.Index,
                    move.Gate,
                    mapId);
                warned++;
                continue;
            }

            if (summary.FieldCount == 0)
            {
                if (MoveMapsWithoutFieldMobs.Contains(mapId))
                {
                    Console.WriteLine(
                        "[m8-m9] move index {0} (gate {1}) -> map {2}: NPC only by design ({3} rows)",
                        move.Index,
                        move.Gate,
                        mapId,
                        summary.Total);
                    continue;
                }

                Console.WriteLine(
                    "[m8-m9] WARN move index {0} (gate {1}) -> map {2}: NPC only ({3} rows, 0 field mobs)",
                    move.Index,
                    move.Gate,
                    mapId,
                    summary.Total);
                warned++;
            }
        }

        if (warned == 0)
        {
            Console.WriteLine("[m8-m9] move-map destinations: all resolved gates have field spawns");
        }
    }
}
