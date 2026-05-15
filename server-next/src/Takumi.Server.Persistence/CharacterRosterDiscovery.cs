using Npgsql;
using NpgsqlTypes;

namespace Takumi.Server.Persistence;

/// <summary>Discover characters from Postgres SSOT tables (no JSON required).</summary>
public static class CharacterRosterDiscovery
{
    public readonly record struct CharacterKey(string AccountLogin, string CharacterName);

    public static async Task<IReadOnlyList<CharacterKey>> ListCharactersFromPostgresAsync(CancellationToken ct = default)
    {
        if (!CharacterRosterBootstrap.IsDbSyncEnabled())
        {
            return Array.Empty<CharacterKey>();
        }

        var cs = PostgresCharacterRosterRepository.BuildConnectionStringFromEnv();
        if (string.IsNullOrEmpty(cs))
        {
            return Array.Empty<CharacterKey>();
        }

        var set = new Dictionary<string, CharacterKey>(StringComparer.OrdinalIgnoreCase);
        await using var conn = new NpgsqlConnection(cs);
        await conn.OpenAsync(ct).ConfigureAwait(false);

        await foreach (var key in ReadCharacterKeysAsync(
            conn,
            """
            SELECT account_login, character_name FROM character_roster
            UNION
            SELECT account_login, character_name FROM character_domain
            """,
            ct).ConfigureAwait(false))
        {
            set[Key(key.AccountLogin, key.CharacterName)] = key;
        }

        if (set.Count == 0)
        {
            await foreach (var key in ReadCharacterKeysAsync(
                conn,
                """
                SELECT DISTINCT account_login, character_name FROM inventory_slot
                """,
                ct).ConfigureAwait(false))
            {
                set[Key(key.AccountLogin, key.CharacterName)] = key;
            }
        }

        return set.Values.OrderBy(k => k.AccountLogin, StringComparer.OrdinalIgnoreCase)
            .ThenBy(k => k.CharacterName, StringComparer.OrdinalIgnoreCase)
            .ToList();
    }

    static async IAsyncEnumerable<CharacterKey> ReadCharacterKeysAsync(
        NpgsqlConnection conn,
        string sql,
        [System.Runtime.CompilerServices.EnumeratorCancellation] CancellationToken ct)
    {
        await using var cmd = new NpgsqlCommand(sql, conn);
        await using var reader = await cmd.ExecuteReaderAsync(ct).ConfigureAwait(false);
        while (await reader.ReadAsync(ct).ConfigureAwait(false))
        {
            var account = reader.GetString(0);
            var name = CharacterRosterMerge.NormaliseName(reader.GetString(1));
            if (string.IsNullOrWhiteSpace(account) || string.IsNullOrWhiteSpace(name))
            {
                continue;
            }

            yield return new CharacterKey(account, name);
        }
    }

    static string Key(string account, string name) => $"{account}\0{name}";
}
