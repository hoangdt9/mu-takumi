using System.Globalization;
using Takumi.Server.Game.Crypto;
using Takumi.Server.Game.Crypto.ModulusCryptor;

namespace Takumi.Server.Game.World;

/// <summary>Minimal <c>Terrain.att</c> loader (parity client <c>ATTReader</c> — block NoMove/NoGround).</summary>
public static class MapAttWalkability
{
    const int TerrainSize = 256;
    static readonly byte[] XorMask = [0xFC, 0xCF, 0xAB];

    static readonly object LoadLock = new();
    static readonly Dictionary<byte, byte[]> WalkableByMap = new();
    static string? _dataRoot;

    public static void PreloadMaps(IEnumerable<byte> mapIds)
    {
        foreach (var mapId in mapIds)
        {
            EnsureMapLoaded(mapId);
        }
    }

    /// <summary>True when tile is walkable or ATT not loaded (permissive fallback).</summary>
    public static bool CanWalk(byte mapId, byte x, byte y)
    {
        if (!TryGetFlags(mapId, x, y, out var flags))
        {
            return true;
        }

        const ushort blocked = 0x0004 | 0x0008; // NoMove | NoGround
        return (flags & blocked) == 0;
    }

    public static bool IsAttLoaded(byte mapId)
    {
        EnsureMapLoaded(mapId);
        return WalkableByMap.TryGetValue(mapId, out var walls) && walls.Length > 0;
    }

    public static bool IsSafeZone(byte mapId, byte x, byte y)
    {
        if (TryGetFlags(mapId, x, y, out var flags))
        {
            return (flags & 0x0001) != 0; // TWFlags.SafeZone
        }

        // No terrain bytes (Docker / missing EncTerrain*.att): without this, IsSafeZone is always false,
        // field mobs can resolve inside town rectangles and show up in villages like Noria.
        return ApproxTownSafeWhenTerrainUnavailable(mapId, x, y);
    }

    /// <summary>
    /// Conservative town bubbles when <see cref="IsAttLoaded"/> is false. Centers follow
    /// <see cref="MapRespawnCatalog"/> / join defaults; margins are intentionally generous.
    /// </summary>
    internal static bool ApproxTownSafeWhenTerrainUnavailable(byte mapId, byte x, byte y)
    {
        if (IsAttLoaded(mapId))
        {
            return false;
        }

        static bool Box(byte x, byte y, int minX, int maxX, int minY, int maxY) =>
            x >= minX && x <= maxX && y >= minY && y <= maxY;

        return mapId switch
        {
            // Lorencia (join default 135,122)
            0 => Box(x, y, 110, 160, 100, 145),
            // Devias (183,32)
            2 => Box(x, y, 165, 205, 15, 52),
            // Noria (173,125) — must cover gate (172,113) and common town paths (~172,110).
            3 => Box(x, y, 138, 205, 88, 142),
            _ => false,
        };
    }

    /// <summary>Walkable tile outside safe zone (spawn heal / field mob placement).</summary>
    public static bool TryFindNearestNonSafeWalkable(byte mapId, byte x, byte y, out byte walkX, out byte walkY, int maxRadius = 24)
    {
        walkX = x;
        walkY = y;
        if (CanWalk(mapId, x, y) && !IsSafeZone(mapId, x, y))
        {
            return true;
        }

        for (var r = 1; r <= maxRadius; r++)
        {
            for (var dy = -r; dy <= r; dy++)
            {
                for (var dx = -r; dx <= r; dx++)
                {
                    if (Math.Abs(dx) != r && Math.Abs(dy) != r)
                    {
                        continue;
                    }

                    var px = x + dx;
                    var py = y + dy;
                    if (px is < 0 or > 255 || py is < 0 or > 255)
                    {
                        continue;
                    }

                    var bx = (byte)px;
                    var by = (byte)py;
                    if (CanWalk(mapId, bx, by) && !IsSafeZone(mapId, bx, by))
                    {
                        walkX = bx;
                        walkY = by;
                        return true;
                    }
                }
            }
        }

        return false;
    }

    /// <summary>When <paramref name="x"/>/<paramref name="y"/> are blocked, search outward for a walkable tile (spawn / warp heal).</summary>
    public static bool TryFindNearestWalkable(byte mapId, byte x, byte y, out byte walkX, out byte walkY, int maxRadius = 12)
    {
        walkX = x;
        walkY = y;
        if (CanWalk(mapId, x, y))
        {
            return true;
        }

        for (var r = 1; r <= maxRadius; r++)
        {
            for (var dy = -r; dy <= r; dy++)
            {
                for (var dx = -r; dx <= r; dx++)
                {
                    if (Math.Abs(dx) != r && Math.Abs(dy) != r)
                    {
                        continue;
                    }

                    var px = x + dx;
                    var py = y + dy;
                    if (px is < 0 or > 255 || py is < 0 or > 255)
                    {
                        continue;
                    }

                    if (CanWalk(mapId, (byte)px, (byte)py))
                    {
                        walkX = (byte)px;
                        walkY = (byte)py;
                        return true;
                    }
                }
            }
        }

        return false;
    }

    static bool TryGetFlags(byte mapId, byte x, byte y, out ushort flags)
    {
        flags = 0;
        EnsureMapLoaded(mapId);
        if (!WalkableByMap.TryGetValue(mapId, out var walls))
        {
            return false;
        }

        var idx = y * TerrainSize + x;
        if (idx < 0 || idx >= walls.Length)
        {
            return false;
        }

        flags = walls[idx];
        return true;
    }

    static void EnsureMapLoaded(byte mapId)
    {
        lock (LoadLock)
        {
            if (WalkableByMap.ContainsKey(mapId))
            {
                return;
            }

            var path = ResolveAttPath(mapId);
            if (path is null || !File.Exists(path))
            {
                WalkableByMap[mapId] = Array.Empty<byte>();
                return;
            }

            try
            {
                WalkableByMap[mapId] = LoadAttFile(path);
                Console.WriteLine("[m9-att] loaded map={0} from {1}", mapId, path);
            }
            catch (Exception ex)
            {
                Console.WriteLine("[m9-att] load failed map={0} ({1})", mapId, ex.Message);
                WalkableByMap[mapId] = Array.Empty<byte>();
            }
        }
    }

    static string? ResolveAttPath(byte mapId)
    {
        var root = Environment.GetEnvironmentVariable("TAKUMI_ATT_DATA_ROOT")?.Trim();
        if (string.IsNullOrEmpty(root))
        {
            root = Environment.GetEnvironmentVariable("TAKUMI_CLIENT_DATA_ROOT")?.Trim();
        }

        if (string.IsNullOrEmpty(root))
        {
            root = ResolveDefaultDataRoot();
        }

        if (string.IsNullOrEmpty(root))
        {
            return null;
        }

        if (!MapNames.TryGetValue(mapId, out var world))
        {
            world = $"World{mapId}";
        }

        foreach (var dir in ResolveWorldDirectories(root, mapId, world))
        {
            // Client parity: EncTerrain{WorldActive+1}.att (e.g. Noria map 3 → World4/EncTerrain4.att).
            foreach (var terrainIndex in new[] { (int)mapId + 1, mapId })
            {
                var enc = Path.Combine(dir, $"EncTerrain{terrainIndex}.att");
                if (File.Exists(enc))
                {
                    return enc;
                }
            }

            var plain = Path.Combine(dir, "Terrain.att");
            if (File.Exists(plain))
            {
                return plain;
            }
        }

        return null;
    }

    static IEnumerable<string> ResolveWorldDirectories(string root, byte mapId, string worldName)
    {
        yield return Path.Combine(root, worldName);
        yield return Path.Combine(root, $"World{mapId + 1}");
        yield return Path.Combine(root, "Terrain");
        yield return Path.Combine(root, "World75"); // Takumi client alias for Lorencia assets
    }

    static string? ResolveDefaultDataRoot()
    {
        if (_dataRoot is not null)
        {
            return _dataRoot;
        }

        var candidates = new[]
        {
            Path.Combine(Environment.CurrentDirectory, "Data"),
            Path.Combine(Environment.CurrentDirectory, "..", "docker", "data-zip", "host", "Data"),
            Path.Combine(Environment.CurrentDirectory, "..", "..", "docker", "data-zip", "host", "Data"),
            Path.Combine(Environment.CurrentDirectory, "..", "MuServer", "Data"),
            Path.Combine(Environment.CurrentDirectory, "..", "MuServer", "4.GameServer", "Data"),
        };

        foreach (var c in candidates)
        {
            var full = Path.GetFullPath(c);
            if (Directory.Exists(full))
            {
                _dataRoot = full;
                return full;
            }
        }

        return null;
    }

    static readonly Dictionary<byte, string> MapNames = new()
    {
        [0] = "Lorencia",
        [1] = "Dungeon",
        [2] = "Devias",
        [3] = "Noria",
        [4] = "LostTower",
        [6] = "Stadium",
        [7] = "Atlans",
        [8] = "Tarkan",
        [10] = "Icarus",
    };

    static byte[] LoadAttFile(string path)
    {
        var buffer = File.ReadAllBytes(path);
        if (buffer.Length > 4
            && buffer[0] == (byte)'A'
            && buffer[1] == (byte)'T'
            && buffer[2] == (byte)'T'
            && buffer[3] == 1)
        {
            var enc = new byte[buffer.Length - 4];
            Buffer.BlockCopy(buffer, 4, enc, 0, enc.Length);
            buffer = Crypto.ModulusCryptor.ModulusCryptor.Decrypt(enc);
        }
        else
        {
            buffer = TakumiFileCryptor.Decrypt(buffer);
        }

        for (var i = 0; i < buffer.Length; i++)
        {
            buffer[i] ^= XorMask[i % XorMask.Length];
        }

        var expected = TerrainSize * TerrainSize + 4;
        var extended = TerrainSize * TerrainSize * 2 + 4;
        if (buffer.Length != expected && buffer.Length != extended)
        {
            throw new InvalidDataException($"ATT size {buffer.Length}, expected {expected} or {extended}");
        }

        var isExtended = buffer.Length == extended;
        var walls = new byte[TerrainSize * TerrainSize];
        var offset = 4;
        for (var i = 0; i < walls.Length; i++)
        {
            ushort b;
            if (isExtended)
            {
                b = (ushort)(buffer[offset] | (buffer[offset + 1] << 8));
                offset += 2;
            }
            else
            {
                b = buffer[offset++];
            }

            walls[i] = (byte)(b & 0xFF);
        }

        return walls;
    }
}
