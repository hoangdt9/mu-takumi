using System.Text.Json;
using Takumi.Server.Protocol;

namespace Takumi.Server.Persistence;

/// <summary>M7: bulk migrate all <c>takumi-roster/*.json</c> accounts → <c>character_roster</c> (+ <c>character_domain</c>).</summary>
public static class CharacterRosterJsonMigrator
{
    public readonly record struct MigrateSummary(int Accounts, int Characters, int SkippedFiles);

    static readonly JsonSerializerOptions JsonOptions = new()
    {
        PropertyNamingPolicy = JsonNamingPolicy.CamelCase,
        PropertyNameCaseInsensitive = true,
    };

    public static bool IsMigrateOnStartupEnabled()
    {
        var raw = Environment.GetEnvironmentVariable("TAKUMI_MIGRATE_ROSTER_JSON")?.Trim();
        return string.Equals(raw, "1", StringComparison.OrdinalIgnoreCase)
               || string.Equals(raw, "true", StringComparison.OrdinalIgnoreCase);
    }

    public static bool IsMigrateOnlyMode()
    {
        var raw = Environment.GetEnvironmentVariable("TAKUMI_MIGRATE_ROSTER_JSON_ONLY")?.Trim();
        return string.Equals(raw, "1", StringComparison.OrdinalIgnoreCase)
               || string.Equals(raw, "true", StringComparison.OrdinalIgnoreCase);
    }

    public static string ResolveRosterDirectory()
    {
        var env = Environment.GetEnvironmentVariable("TAKUMI_ROSTER_DIR")?.Trim();
        if (!string.IsNullOrEmpty(env))
        {
            return Path.GetFullPath(env);
        }

        return Path.GetFullPath(Path.Combine(Environment.CurrentDirectory, "takumi-roster"));
    }

    public static async Task<MigrateSummary> MigrateAllJsonFilesAsync(CancellationToken ct = default)
    {
        if (!CharacterRosterBootstrap.IsDbSyncEnabled())
        {
            Console.WriteLine("[roster-migrate] TAKUMI_ROSTER_DB_SYNC is off — nothing to do");
            return new MigrateSummary(0, 0, 0);
        }

        var rosterRepo = TakumiPostgresMirror.CharacterRoster;
        if (rosterRepo is null)
        {
            Console.WriteLine("[roster-migrate] Postgres mirror not initialized — set TAKUMI_PG_* / TAKUMI_PG_CONNECTION_STRING");
            return new MigrateSummary(0, 0, 0);
        }

        var dir = ResolveRosterDirectory();
        if (!Directory.Exists(dir))
        {
            Console.WriteLine("[roster-migrate] roster dir missing: {0}", dir);
            return new MigrateSummary(0, 0, 0);
        }

        var domainRepo = TakumiPostgresMirror.CharacterDomain;
        var syncDomain = CharacterDomainMirrorWriter.IsEnabled() && domainRepo is not null;
        var accounts = 0;
        var characters = 0;
        var skipped = 0;

        foreach (var path in Directory.EnumerateFiles(dir, "*.json", SearchOption.TopDirectoryOnly).OrderBy(p => p, StringComparer.OrdinalIgnoreCase))
        {
            var accountId = Path.GetFileNameWithoutExtension(path);
            if (string.IsNullOrWhiteSpace(accountId))
            {
                skipped++;
                continue;
            }

            var rows = TryLoadRowsFromJsonFile(path);
            if (rows.Count == 0)
            {
                Console.WriteLine("[roster-migrate] skip empty {0}", path);
                skipped++;
                continue;
            }

            await rosterRepo.ReplaceAccountRosterAsync(accountId, rows, ct).ConfigureAwait(false);
            if (syncDomain)
            {
                await domainRepo!.ReplaceAccountAsync(accountId, rows, ct).ConfigureAwait(false);
            }

            accounts++;
            characters += rows.Count;
            Console.WriteLine(
                "[roster-migrate] account={0} characters={1} file={2}",
                accountId,
                rows.Count,
                path);
        }

        Console.WriteLine(
            "[roster-migrate] done accounts={0} characters={1} skippedFiles={2} dir={3} domain={4}",
            accounts,
            characters,
            skipped,
            dir,
            syncDomain);
        return new MigrateSummary(accounts, characters, skipped);
    }

    public static IReadOnlyList<CharacterRosterRow> TryLoadRowsFromJsonFile(string jsonPath)
    {
        var list = new List<CharacterRosterRow>();
        if (!File.Exists(jsonPath))
        {
            return list;
        }

        try
        {
            var json = File.ReadAllText(jsonPath);
            var root = JsonSerializer.Deserialize<RosterJsonRoot>(json, JsonOptions);
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

                var row = new CharacterRosterRow
                {
                    Name = CharacterRosterMerge.NormaliseName(c.Name),
                    ServerClass = c.ServerClass,
                    Level = c.Level,
                    MapId = c.MapId,
                    PosX = c.PosX,
                    PosY = c.PosY,
                    Angle = c.Angle,
                    CurrentHp = c.CurrentHp,
                    MaxHp = c.MaxHp,
                    CurrentMp = c.CurrentMp,
                    MaxMp = c.MaxMp,
                    Zen = c.Zen,
                };
                ApplySpawnDefaultsIfUnset(row);
                list.Add(row);
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine("[roster-migrate] parse failed {0}: {1}", jsonPath, ex.Message);
        }

        return list;
    }

    static void ApplySpawnDefaultsIfUnset(CharacterRosterRow row)
    {
        if (row.PosX != 0 || row.PosY != 0 || row.Angle != 0)
        {
            return;
        }

        var d = ReadSpawnDefaultsFromEnv();
        row.MapId = d.Map;
        row.PosX = d.PositionX;
        row.PosY = d.PositionY;
        row.Angle = d.Angle;
    }

    static JoinMapSpawnWire ReadSpawnDefaultsFromEnv()
    {
        var d = JoinMapSpawnWire.LorenciaDefault;
        if (byte.TryParse(Environment.GetEnvironmentVariable("TAKUMI_DEFAULT_SPAWN_MAP"), out var m))
        {
            d = d with { Map = m };
        }

        if (byte.TryParse(Environment.GetEnvironmentVariable("TAKUMI_DEFAULT_SPAWN_X"), out var x))
        {
            d = d with { PositionX = x };
        }

        if (byte.TryParse(Environment.GetEnvironmentVariable("TAKUMI_DEFAULT_SPAWN_Y"), out var y))
        {
            d = d with { PositionY = y };
        }

        if (byte.TryParse(Environment.GetEnvironmentVariable("TAKUMI_DEFAULT_SPAWN_ANGLE"), out var a))
        {
            d = d with { Angle = a };
        }

        return d;
    }

    sealed class RosterJsonRoot
    {
        public List<RosterJsonChar>? Characters { get; set; }
    }

    sealed class RosterJsonChar
    {
        public string Name { get; set; } = "";

        public byte ServerClass { get; set; }

        public ushort Level { get; set; }

        public byte MapId { get; set; }

        public byte PosX { get; set; }

        public byte PosY { get; set; }

        public byte Angle { get; set; }

        public int CurrentHp { get; set; }

        public int MaxHp { get; set; }

        public int CurrentMp { get; set; }

        public int MaxMp { get; set; }

        public long Zen { get; set; }
    }
}
