using System.Globalization;
using System.Text;
using System.Text.Json;
using Takumi.Server.Protocol;

namespace Takumi.Server.Game;

/// <summary>Roster row (same JSON as <c>LegacyLoginHost</c> <c>takumi-roster/*.json</c>).</summary>
public sealed class GameRosterEntry
{
    public byte[] Name10 { get; set; } = new byte[10];
    public byte ServerClass { get; set; }
    public ushort Level { get; set; }

    public uint Experience { get; set; }

    public byte MapId { get; set; }
    public byte PosX { get; set; }
    public byte PosY { get; set; }
    public byte Angle { get; set; }

    public int CurrentHp { get; set; }

    public int MaxHp { get; set; }

    public int CurrentMp { get; set; }

    public int MaxMp { get; set; }

    public long Zen { get; set; }

    /// <summary>Session SD (not persisted to roster DB yet).</summary>
    public int CurrentShield { get; set; }

    public int MaxShield { get; set; }

    public ushort Strength { get; set; }

    public ushort Dexterity { get; set; }

    public ushort Vitality { get; set; }

    public ushort Energy { get; set; }

    public ushort Leadership { get; set; }

    public ushort LevelUpPoint { get; set; }

    public int CurrentBp { get; set; }

    public int MaxBp { get; set; }
}

public static class GameSpawnEnv
{
    public static JoinMapSpawnWire ReadNewCharacterSpawnDefaultsFromEnv()
    {
        var d = JoinMapSpawnWire.LorenciaDefault;
        if (byte.TryParse(
                Environment.GetEnvironmentVariable("TAKUMI_DEFAULT_SPAWN_MAP"),
                NumberStyles.Integer,
                CultureInfo.InvariantCulture,
                out var m))
        {
            d = d with { Map = m };
        }

        if (byte.TryParse(
                Environment.GetEnvironmentVariable("TAKUMI_DEFAULT_SPAWN_X"),
                NumberStyles.Integer,
                CultureInfo.InvariantCulture,
                out var x))
        {
            d = d with { PositionX = x };
        }

        if (byte.TryParse(
                Environment.GetEnvironmentVariable("TAKUMI_DEFAULT_SPAWN_Y"),
                NumberStyles.Integer,
                CultureInfo.InvariantCulture,
                out var y))
        {
            d = d with { PositionY = y };
        }

        if (byte.TryParse(
                Environment.GetEnvironmentVariable("TAKUMI_DEFAULT_SPAWN_ANGLE"),
                NumberStyles.Integer,
                CultureInfo.InvariantCulture,
                out var a))
        {
            d = d with { Angle = a };
        }

        return d;
    }
}

public static class GameRosterDisk
{
    public static readonly object JsonFileLock = new();

    public static JsonSerializerOptions JsonOptions { get; } = new()
    {
        PropertyNamingPolicy = JsonNamingPolicy.CamelCase,
        WriteIndented = true,
    };

    public static string GetRosterRoot()
    {
        var env = Environment.GetEnvironmentVariable("TAKUMI_ROSTER_DIR")?.Trim();
        if (!string.IsNullOrEmpty(env))
        {
            return Path.GetFullPath(env);
        }

        return Path.GetFullPath(Path.Combine(Environment.CurrentDirectory, "takumi-roster"));
    }

    public static string SanitizeAccountForFile(string accountId)
    {
        Span<char> buf = stackalloc char[Math.Min(accountId.Length, 48)];
        var n = 0;
        foreach (var c in accountId)
        {
            if (char.IsAsciiLetterOrDigit(c) || c == '_')
            {
                if (n < buf.Length)
                {
                    buf[n++] = c;
                }
            }
        }

        return n == 0 ? "account" : new string(buf[..n]);
    }

    public static string GetRosterFilePath(string accountId) =>
        Path.Combine(GetRosterRoot(), SanitizeAccountForFile(accountId) + ".json");

    public static void ApplyLegacySpawnIfUnset(GameRosterEntry e)
    {
        if (e.PosX != 0 || e.PosY != 0 || e.Angle != 0)
        {
            return;
        }

        var d = GameSpawnEnv.ReadNewCharacterSpawnDefaultsFromEnv();
        e.MapId = d.Map;
        e.PosX = d.PositionX;
        e.PosY = d.PositionY;
        e.Angle = d.Angle;
    }

    public static List<GameRosterEntry> LoadEntries(string accountId)
    {
        var list = new List<GameRosterEntry>();
        var path = GetRosterFilePath(accountId);
        if (!File.Exists(path))
        {
            return list;
        }

        try
        {
            string json;
            lock (JsonFileLock)
            {
                json = File.ReadAllText(path);
            }

            var root = JsonSerializer.Deserialize<RosterPersistRoot>(json, JsonOptions);
            if (root?.Characters is null)
            {
                return list;
            }

            foreach (var c in root.Characters)
            {
                if (string.IsNullOrWhiteSpace(c.Name))
                {
                    continue;
                }

                var nm = new byte[10];
                var enc = Encoding.ASCII.GetBytes(c.Name.Trim());
                Buffer.BlockCopy(enc, 0, nm, 0, Math.Min(10, enc.Length));
                var entry = new GameRosterEntry
                {
                    Name10 = nm,
                    ServerClass = c.ServerClass,
                    Level = c.Level,
                    Experience = c.Experience,
                    MapId = c.MapId,
                    PosX = c.PosX,
                    PosY = c.PosY,
                    Angle = c.Angle,
                    CurrentHp = c.CurrentHp,
                    MaxHp = c.MaxHp,
                    CurrentMp = c.CurrentMp,
                    MaxMp = c.MaxMp,
                    Zen = c.Zen,
                    CurrentShield = c.CurrentShield,
                    MaxShield = c.MaxShield,
                    Strength = c.Strength,
                    Dexterity = c.Dexterity,
                    Vitality = c.Vitality,
                    Energy = c.Energy,
                    Leadership = c.Leadership,
                    LevelUpPoint = c.LevelUpPoint,
                    CurrentBp = c.CurrentBp,
                    MaxBp = c.MaxBp,
                };
                ApplyLegacySpawnIfUnset(entry);
                list.Add(entry);
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine("[game-roster] load failed {0}: {1}", path, ex.Message);
        }

        return list;
    }

    sealed class RosterPersistRoot
    {
        public List<RosterPersistChar> Characters { get; set; } = new();
    }

    sealed class RosterPersistChar
    {
        public string Name { get; set; } = "";
        public byte ServerClass { get; set; }
        public ushort Level { get; set; }

        public uint Experience { get; set; }

        public byte MapId { get; set; }
        public byte PosX { get; set; }
        public byte PosY { get; set; }
        public byte Angle { get; set; }

        public int CurrentHp { get; set; }

        public int MaxHp { get; set; }

        public int CurrentMp { get; set; }

        public int MaxMp { get; set; }

        public long Zen { get; set; }

        public int CurrentShield { get; set; }

        public int MaxShield { get; set; }

        public ushort Strength { get; set; }

        public ushort Dexterity { get; set; }

        public ushort Vitality { get; set; }

        public ushort Energy { get; set; }

        public ushort Leadership { get; set; }

        public ushort LevelUpPoint { get; set; }

        public int CurrentBp { get; set; }

        public int MaxBp { get; set; }
    }
}

public static class GameNameUtil
{
    public static ReadOnlySpan<byte> TrimName10(ReadOnlySpan<byte> name10)
    {
        var len = Math.Min(10, name10.Length);
        while (len > 0 && name10[len - 1] == 0)
        {
            len--;
        }

        return name10[..len];
    }

    public static bool NameBytesEqual(ReadOnlySpan<byte> a, ReadOnlySpan<byte> b) =>
        TrimName10(a).SequenceEqual(TrimName10(b));
}
