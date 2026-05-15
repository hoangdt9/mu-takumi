using System.Globalization;

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

    public static bool IsSafeZone(byte mapId, byte x, byte y)
    {
        if (!TryGetFlags(mapId, x, y, out var flags))
        {
            return false;
        }

        return (flags & 0x0001) != 0; // TWFlags.SafeZone
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

        return Path.Combine(root, world, "Terrain.att");
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
            buffer = TakumiAttDecrypt(enc);
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

    /// <summary>Placeholder — encrypted ATT on disk may need full Modulus decrypt; unencrypted ATT works.</summary>
    static byte[] TakumiAttDecrypt(byte[] enc) => enc;
}
