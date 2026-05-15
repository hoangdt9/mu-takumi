using Takumi.Server.Persistence;

namespace Takumi.Server.Game.World;

/// <summary>M8: in-memory map gates for runtime lookup (DB or <c>Gate.txt</c>).</summary>
public static class MapGateCatalog
{
    static readonly object InitLock = new();
    static bool _initialized;
    static Dictionary<int, MapGateEntry> _byIndex = new();
    static Dictionary<byte, List<MapGateEntry>> _byMap = new();

    public static void EnsureInitialized()
    {
        if (_initialized)
        {
            return;
        }

        lock (InitLock)
        {
            if (_initialized)
            {
                return;
            }

            IReadOnlyList<MapGateEntry> gates;
            if (TryLoadFromPostgres(out gates))
            {
                Console.WriteLine("[m8] MapGateCatalog: {0} gates from Postgres", gates.Count);
            }
            else
            {
                var path = ResolveGatePath();
                if (path is not null && File.Exists(path))
                {
                    gates = GateLoader.LoadFromFile(path);
                    Console.WriteLine("[m8] MapGateCatalog: {0} gates from {1}", gates.Count, path);
                }
                else
                {
                    gates = Array.Empty<MapGateEntry>();
                    Console.WriteLine("[m8] MapGateCatalog: no Gate.txt ({0})", path ?? "(unset)");
                }
            }

            Rebuild(gates);
            _initialized = true;
        }
    }

    public static bool TryGetGate(int gateIndex, out MapGateEntry? entry)
    {
        EnsureInitialized();
        if (_byIndex.TryGetValue(gateIndex, out var g))
        {
            entry = g;
            return true;
        }

        entry = null;
        return false;
    }

    public static IReadOnlyList<MapGateEntry> GetGatesOnMap(byte mapId)
    {
        EnsureInitialized();
        return _byMap.TryGetValue(mapId, out var list) ? list : Array.Empty<MapGateEntry>();
    }

    static bool TryLoadFromPostgres(out IReadOnlyList<MapGateEntry> gates)
    {
        gates = Array.Empty<MapGateEntry>();
        var repo = TakumiPostgresMirror.MapGate;
        if (repo is null)
        {
            return false;
        }

        try
        {
            var rows = repo.LoadAllAsync().GetAwaiter().GetResult();
            if (rows.Count == 0)
            {
                return false;
            }

            gates = rows.Select(MapGateRowMapping.FromRow).ToList();
            return true;
        }
        catch (Exception ex)
        {
            Console.WriteLine("[m8] map_gate load failed: {0}", ex.Message);
            return false;
        }
    }

    static void Rebuild(IReadOnlyList<MapGateEntry> gates)
    {
        _byIndex = gates.ToDictionary(g => g.GateIndex);
        _byMap = new Dictionary<byte, List<MapGateEntry>>();
        foreach (var g in gates)
        {
            if (!_byMap.TryGetValue(g.MapId, out var bucket))
            {
                bucket = new List<MapGateEntry>();
                _byMap[g.MapId] = bucket;
            }

            bucket.Add(g);
        }
    }

    static string? ResolveGatePath()
    {
        var env = Environment.GetEnvironmentVariable("TAKUMI_GATE_PATH")?.Trim();
        if (!string.IsNullOrEmpty(env))
        {
            return Path.GetFullPath(env);
        }

        var root = WorldDataPathResolver.ResolveDataRoot();
        return root is null ? null : Path.Combine(root, "Move", "Gate.txt");
    }
}
