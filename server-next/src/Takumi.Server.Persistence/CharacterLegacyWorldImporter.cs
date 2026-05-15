using Npgsql;

namespace Takumi.Server.Persistence;

/// <summary>M4: optional import <c>character_staging</c> → <c>character_domain</c> + <c>character_roster</c> at host startup.</summary>
public static class CharacterLegacyWorldImporter
{
    public static bool IsEnabled()
    {
        var raw = Environment.GetEnvironmentVariable("TAKUMI_IMPORT_CHARACTER_STAGING")?.Trim();
        return string.Equals(raw, "1", StringComparison.OrdinalIgnoreCase)
               || string.Equals(raw, "true", StringComparison.OrdinalIgnoreCase);
    }

    public static async Task TryImportAsync(CancellationToken ct = default)
    {
        if (!IsEnabled() || !CharacterRosterBootstrap.IsDbSyncEnabled())
        {
            return;
        }

        var rosterRepo = TakumiPostgresMirror.CharacterRoster;
        var domainRepo = TakumiPostgresMirror.CharacterDomain;
        if (rosterRepo is null || domainRepo is null)
        {
            return;
        }

        var cs = PostgresCharacterRosterRepository.BuildConnectionStringFromEnv();
        if (string.IsNullOrEmpty(cs))
        {
            return;
        }

        try
        {
            await using var conn = new NpgsqlConnection(cs);
            await conn.OpenAsync(ct).ConfigureAwait(false);
            await using var check = new NpgsqlCommand(
                """
                SELECT EXISTS (
                    SELECT 1 FROM information_schema.tables
                    WHERE table_schema = 'public' AND table_name = 'character_staging')
                """,
                conn);
            var exists = (bool)(await check.ExecuteScalarAsync(ct).ConfigureAwait(false) ?? false);
            if (!exists)
            {
                return;
            }

            var byAccount = new Dictionary<string, List<CharacterRosterRow>>(StringComparer.OrdinalIgnoreCase);
            await using var sel = new NpgsqlCommand(
                """
                SELECT account_login, character_name, server_class, level, map_id, pos_x, pos_y, angle,
                       current_hp, max_hp, current_mp, max_mp, zen
                FROM character_staging
                """,
                conn);
            await using var reader = await sel.ExecuteReaderAsync(ct).ConfigureAwait(false);
            while (await reader.ReadAsync(ct).ConfigureAwait(false))
            {
                var account = reader.GetString(0);
                if (!byAccount.TryGetValue(account, out var list))
                {
                    list = new List<CharacterRosterRow>();
                    byAccount[account] = list;
                }

                list.Add(
                    new CharacterRosterRow
                    {
                        Name = reader.GetString(1),
                        ServerClass = (byte)reader.GetInt16(2),
                        Level = (ushort)reader.GetInt32(3),
                        MapId = (byte)reader.GetInt16(4),
                        PosX = (byte)reader.GetInt16(5),
                        PosY = (byte)reader.GetInt16(6),
                        Angle = (byte)reader.GetInt16(7),
                        CurrentHp = reader.GetInt32(8),
                        MaxHp = reader.GetInt32(9),
                        CurrentMp = reader.GetInt32(10),
                        MaxMp = reader.GetInt32(11),
                        Zen = reader.GetInt64(12),
                    });
            }

            if (byAccount.Count == 0)
            {
                Console.WriteLine("[character-staging] table empty — skip import");
                return;
            }

            foreach (var (account, rows) in byAccount)
            {
                await rosterRepo.ReplaceAccountRosterAsync(account, rows, ct).ConfigureAwait(false);
                await domainRepo.ReplaceAccountAsync(account, rows, ct).ConfigureAwait(false);
            }

            Console.WriteLine("[character-staging] imported {0} account(s) → character_roster + character_domain", byAccount.Count);
        }
        catch (Exception ex)
        {
            Console.WriteLine("[character-staging] import failed: {0}", ex.Message);
        }
    }
}
