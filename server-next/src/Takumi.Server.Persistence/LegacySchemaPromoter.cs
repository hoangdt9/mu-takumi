using Npgsql;

namespace Takumi.Server.Persistence;

/// <summary>
/// Promotes <c>takumi_legacy</c> / <c>takumi_staging</c> mirrors into <c>public</c> runtime tables;
/// migrates legacy <c>takumi_runtime.account</c> (EF) then drops that schema when enabled.
/// </summary>
public static class LegacySchemaPromoter
{
    static readonly string[] StagingSchemas = ["takumi_legacy", "takumi_staging"];

    static readonly string[] MembTableNames =
    [
        "legacy_memb_info",
        "memb_info_staging",
        "memb_info",
        "MEMB_INFO",
    ];

    static readonly string[] CharacterTableNames =
    [
        "legacy_character",
        "character_staging",
        "Character",
    ];

    public static bool IsEnabled()
    {
        var raw = Environment.GetEnvironmentVariable("TAKUMI_LEGACY_PROMOTE")?.Trim();
        if (string.Equals(raw, "0", StringComparison.OrdinalIgnoreCase)
            || string.Equals(raw, "false", StringComparison.OrdinalIgnoreCase))
        {
            return false;
        }

        return CharacterRosterBootstrap.IsDbSyncEnabled();
    }

    public static bool DropEfRuntimeSchemaAfterPromote() =>
        !string.Equals(Environment.GetEnvironmentVariable("TAKUMI_DROP_EF_RUNTIME_SCHEMA")?.Trim(), "0", StringComparison.OrdinalIgnoreCase)
        && !string.Equals(Environment.GetEnvironmentVariable("TAKUMI_DROP_EF_RUNTIME_SCHEMA")?.Trim(), "false", StringComparison.OrdinalIgnoreCase);

    public static bool IsPromoteOnlyMode() =>
        string.Equals(Environment.GetEnvironmentVariable("TAKUMI_LEGACY_PROMOTE_ONLY")?.Trim(), "1", StringComparison.OrdinalIgnoreCase)
        || string.Equals(Environment.GetEnvironmentVariable("TAKUMI_LEGACY_PROMOTE_ONLY")?.Trim(), "true", StringComparison.OrdinalIgnoreCase);

    public static async Task TryPromoteAllAsync(CancellationToken ct = default)
    {
        if (!IsEnabled())
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

            var accountsMigrated = await MigrateEfRuntimeAccountsAsync(conn, ct).ConfigureAwait(false);
            var membPromoted = 0;
            var charsPromoted = 0;

            foreach (var schema in StagingSchemas)
            {
                if (!await SchemaExistsAsync(conn, schema, ct).ConfigureAwait(false))
                {
                    continue;
                }

                membPromoted += await PromoteMembTablesAsync(conn, schema, ct).ConfigureAwait(false);
                charsPromoted += await PromoteCharacterTablesAsync(conn, schema, ct).ConfigureAwait(false);
            }

            await PromotePublicCharacterStagingAsync(conn, ct).ConfigureAwait(false);

            if (DropEfRuntimeSchemaAfterPromote())
            {
                await DropEfRuntimeSchemaAsync(conn, ct).ConfigureAwait(false);
            }

            Console.WriteLine(
                "[legacy-promote] done ef_accounts={0} memb_rows={1} character_accounts={2} drop_ef_schema={3}",
                accountsMigrated,
                membPromoted,
                charsPromoted,
                DropEfRuntimeSchemaAfterPromote());
        }
        catch (Exception ex)
        {
            Console.WriteLine("[legacy-promote] failed: {0}", ex.Message);
        }
    }

    static async Task<int> MigrateEfRuntimeAccountsAsync(NpgsqlConnection conn, CancellationToken ct)
    {
        if (!await SchemaExistsAsync(conn, "takumi_runtime", ct).ConfigureAwait(false))
        {
            return 0;
        }

        if (!await TableExistsAsync(conn, "takumi_runtime", "account", ct).ConfigureAwait(false))
        {
            return 0;
        }

        var cols = await GetColumnsAsync(conn, "takumi_runtime", "account", ct).ConfigureAwait(false);
        var loginCol = PickColumn(cols, "UserName", "username", "account_login", "memb___id");
        var passCol = PickColumn(cols, "PasswordHash", "password_hash", "memb__pwd", "password");
        if (loginCol is null || passCol is null)
        {
            Console.WriteLine("[legacy-promote] takumi_runtime.account: unknown columns, skip");
            return 0;
        }

        var rows = new List<(string Login, string Hash)>();
        await using (var sel = new NpgsqlCommand(
                         $"""
                          SELECT {QuoteIdent(loginCol)}, {QuoteIdent(passCol)}
                          FROM takumi_runtime.account
                          WHERE {QuoteIdent(loginCol)} IS NOT NULL AND TRIM({QuoteIdent(loginCol)}::text) <> ''
                          """,
                         conn))
        {
            await using var reader = await sel.ExecuteReaderAsync(ct).ConfigureAwait(false);
            while (await reader.ReadAsync(ct).ConfigureAwait(false))
            {
                var login = reader.GetString(0).Trim();
                if (login.Length == 0)
                {
                    continue;
                }

                var pass = reader.IsDBNull(1) ? string.Empty : reader.GetString(1);
                var hash = pass.StartsWith("$2", StringComparison.Ordinal) ? pass : AccountPasswordHasher.Hash(pass);
                rows.Add((login, hash));
            }
        }

        var count = 0;
        foreach (var (login, hash) in rows)
        {
            if (await UpsertAccountAsync(conn, login, hash, string.Empty, string.Empty, ct).ConfigureAwait(false))
            {
                count++;
            }
        }

        if (count > 0)
        {
            Console.WriteLine("[legacy-promote] migrated {0} row(s) from takumi_runtime.account → public.account", count);
        }

        return count;
    }

    static async Task<int> PromoteMembTablesAsync(NpgsqlConnection conn, string schema, CancellationToken ct)
    {
        var total = 0;
        foreach (var table in MembTableNames)
        {
            if (!await TableExistsAsync(conn, schema, table, ct).ConfigureAwait(false))
            {
                continue;
            }

            var cols = await GetColumnsAsync(conn, schema, table, ct).ConfigureAwait(false);
            var loginCol = PickColumn(cols, "memb___id", "account_login", "UserName", "username", "login");
            var passCol = PickColumn(cols, "memb__pwd", "password", "PasswordHash", "password_hash");
            var secCol = PickColumn(cols, "sno__numb", "security_code", "numcode", "sno_numb");
            var phoneCol = PickColumn(cols, "tel__numb", "phone", "phon_numb", "sodienthoai", "tel_numb");
            if (loginCol is null || passCol is null)
            {
                Console.WriteLine("[legacy-promote] {0}.{1}: no login/password columns, skip", schema, table);
                continue;
            }

            var secExpr = secCol is not null ? QuoteIdent(secCol) : "''";
            var phoneExpr = phoneCol is not null ? QuoteIdent(phoneCol) : "''";
            await using var sel = new NpgsqlCommand(
                $"""
                 SELECT {QuoteIdent(loginCol)}, {QuoteIdent(passCol)}, {secExpr}, {phoneExpr}
                 FROM {QuoteIdent(schema)}.{QuoteIdent(table)}
                 WHERE {QuoteIdent(loginCol)} IS NOT NULL AND TRIM({QuoteIdent(loginCol)}::text) <> ''
                 """,
                conn);
            var membRows = new List<(string Login, string Hash, string Sec, string Phone)>();
            await using (var reader = await sel.ExecuteReaderAsync(ct).ConfigureAwait(false))
            {
                while (await reader.ReadAsync(ct).ConfigureAwait(false))
                {
                    var login = reader.GetString(0).Trim();
                    var pass = reader.IsDBNull(1) ? string.Empty : reader.GetString(1);
                    var sec = reader.IsDBNull(2) ? string.Empty : reader.GetString(2).Trim();
                    var phone = reader.IsDBNull(3) ? string.Empty : reader.GetString(3).Trim();
                    var hash = pass.StartsWith("$2", StringComparison.Ordinal) ? pass : AccountPasswordHasher.Hash(pass);
                    membRows.Add((login, hash, sec, phone));
                }
            }

            var n = 0;
            foreach (var (login, hash, sec, phone) in membRows)
            {
                if (await UpsertAccountAsync(conn, login, hash, sec, phone, ct).ConfigureAwait(false))
                {
                    n++;
                }
            }

            if (n > 0)
            {
                Console.WriteLine("[legacy-promote] {0}.{1} → public.account ({2} row(s))", schema, table, n);
                total += n;
            }
        }

        return total;
    }

    static async Task<int> PromoteCharacterTablesAsync(NpgsqlConnection conn, string schema, CancellationToken ct)
    {
        var rosterRepo = TakumiPostgresMirror.CharacterRoster;
        var domainRepo = TakumiPostgresMirror.CharacterDomain;
        if (rosterRepo is null)
        {
            return 0;
        }

        var accounts = 0;
        foreach (var table in CharacterTableNames)
        {
            if (!await TableExistsAsync(conn, schema, table, ct).ConfigureAwait(false))
            {
                continue;
            }

            var cols = await GetColumnsAsync(conn, schema, table, ct).ConfigureAwait(false);
            var accountCol = PickColumn(cols, "account_login", "AccountID", "account", "memb___id", "Account");
            var nameCol = PickColumn(cols, "character_name", "Name", "CharacterName");
            if (accountCol is null || nameCol is null)
            {
                Console.WriteLine("[legacy-promote] {0}.{1}: no account/character columns, skip", schema, table);
                continue;
            }

            var classCol = PickColumn(cols, "server_class", "Class", "class");
            var levelCol = PickColumn(cols, "level", "cLevel", "Level");
            var mapCol = PickColumn(cols, "map_id", "MapNumber", "Map", "map");
            var xCol = PickColumn(cols, "pos_x", "MapPosX", "X");
            var yCol = PickColumn(cols, "pos_y", "MapPosY", "Y");
            var angleCol = PickColumn(cols, "angle", "Angle");
            var hpCol = PickColumn(cols, "current_hp", "Life", "HP");
            var maxHpCol = PickColumn(cols, "max_hp", "MaxLife");
            var mpCol = PickColumn(cols, "current_mp", "Mana", "MP");
            var maxMpCol = PickColumn(cols, "max_mp", "MaxMana");
            var zenCol = PickColumn(cols, "zen", "Money", "Zen");

            var byAccount = new Dictionary<string, List<CharacterRosterRow>>(StringComparer.OrdinalIgnoreCase);
            await using var sel = new NpgsqlCommand($"SELECT * FROM {QuoteIdent(schema)}.{QuoteIdent(table)}", conn);
            await using var reader = await sel.ExecuteReaderAsync(ct).ConfigureAwait(false);
            while (await reader.ReadAsync(ct).ConfigureAwait(false))
            {
                var account = ReadString(reader, accountCol)?.Trim();
                var name = ReadString(reader, nameCol)?.Trim();
                if (string.IsNullOrEmpty(account) || string.IsNullOrEmpty(name))
                {
                    continue;
                }

                if (!byAccount.TryGetValue(account, out var list))
                {
                    list = new List<CharacterRosterRow>();
                    byAccount[account] = list;
                }

                list.Add(
                    new CharacterRosterRow
                    {
                        Name = name,
                        ServerClass = (byte)ReadInt(reader, classCol, 0),
                        Level = (ushort)Math.Clamp(ReadInt(reader, levelCol, 1), 1, ushort.MaxValue),
                        MapId = (byte)ReadInt(reader, mapCol, 0),
                        PosX = (byte)Math.Clamp(ReadInt(reader, xCol, 125), 0, 255),
                        PosY = (byte)Math.Clamp(ReadInt(reader, yCol, 125), 0, 255),
                        Angle = (byte)ReadInt(reader, angleCol, 0),
                        CurrentHp = ReadInt(reader, hpCol, 100),
                        MaxHp = ReadInt(reader, maxHpCol, 100),
                        CurrentMp = ReadInt(reader, mpCol, 50),
                        MaxMp = ReadInt(reader, maxMpCol, 50),
                        Zen = ReadLong(reader, zenCol, 0),
                    });
            }

            foreach (var (account, rows) in byAccount)
            {
                await rosterRepo.ReplaceAccountRosterAsync(account, rows, ct).ConfigureAwait(false);
                if (domainRepo is not null)
                {
                    await domainRepo.ReplaceAccountAsync(account, rows, ct).ConfigureAwait(false);
                }
            }

            if (byAccount.Count > 0)
            {
                Console.WriteLine(
                    "[legacy-promote] {0}.{1} → character_roster ({2} account(s), {3} char(s))",
                    schema,
                    table,
                    byAccount.Count,
                    byAccount.Sum(static kv => kv.Value.Count));
                accounts += byAccount.Count;
            }
        }

        return accounts;
    }

    static async Task PromotePublicCharacterStagingAsync(NpgsqlConnection conn, CancellationToken ct)
    {
        if (CharacterLegacyWorldImporter.IsEnabled())
        {
            await CharacterLegacyWorldImporter.TryImportAsync(ct).ConfigureAwait(false);
        }
    }

    static async Task DropEfRuntimeSchemaAsync(NpgsqlConnection conn, CancellationToken ct)
    {
        if (!await SchemaExistsAsync(conn, "takumi_runtime", ct).ConfigureAwait(false))
        {
            return;
        }

        await using var cmd = new NpgsqlCommand("DROP SCHEMA IF EXISTS takumi_runtime CASCADE", conn);
        await cmd.ExecuteNonQueryAsync(ct).ConfigureAwait(false);
        Console.WriteLine("[legacy-promote] dropped schema takumi_runtime (EF duplicate)");
    }

    static async Task<bool> UpsertAccountAsync(
        NpgsqlConnection conn,
        string login,
        string passwordHash,
        string securityCode,
        string phone,
        CancellationToken ct)
    {
        var norm = PostgresAccountRepository.NormaliseLogin(login);
        await using var cmd = new NpgsqlCommand(
            """
            INSERT INTO public.account (account_login, password_hash, security_code, phone)
            VALUES (@login, @hash, @sec, @phone)
            ON CONFLICT (account_login) DO UPDATE SET
                password_hash = EXCLUDED.password_hash,
                security_code = CASE WHEN EXCLUDED.security_code <> '' THEN EXCLUDED.security_code ELSE account.security_code END,
                phone = CASE WHEN EXCLUDED.phone <> '' THEN EXCLUDED.phone ELSE account.phone END,
                updated_at = now()
            """,
            conn);
        cmd.Parameters.AddWithValue("login", norm);
        cmd.Parameters.AddWithValue("hash", passwordHash);
        cmd.Parameters.AddWithValue("sec", securityCode ?? string.Empty);
        cmd.Parameters.AddWithValue("phone", phone ?? string.Empty);
        await cmd.ExecuteNonQueryAsync(ct).ConfigureAwait(false);
        return true;
    }

    static async Task<bool> SchemaExistsAsync(NpgsqlConnection conn, string schema, CancellationToken ct)
    {
        await using var cmd = new NpgsqlCommand(
            "SELECT EXISTS (SELECT 1 FROM information_schema.schemata WHERE schema_name = @s)",
            conn);
        cmd.Parameters.AddWithValue("s", schema);
        return (bool)(await cmd.ExecuteScalarAsync(ct).ConfigureAwait(false) ?? false);
    }

    static async Task<bool> TableExistsAsync(NpgsqlConnection conn, string schema, string table, CancellationToken ct)
    {
        await using var cmd = new NpgsqlCommand(
            """
            SELECT EXISTS (
              SELECT 1 FROM information_schema.tables
              WHERE table_schema = @s AND table_name = @t)
            """,
            conn);
        cmd.Parameters.AddWithValue("s", schema);
        cmd.Parameters.AddWithValue("t", table);
        return (bool)(await cmd.ExecuteScalarAsync(ct).ConfigureAwait(false) ?? false);
    }

    static async Task<List<string>> GetColumnsAsync(NpgsqlConnection conn, string schema, string table, CancellationToken ct)
    {
        var list = new List<string>();
        await using var cmd = new NpgsqlCommand(
            """
            SELECT column_name FROM information_schema.columns
            WHERE table_schema = @s AND table_name = @t
            ORDER BY ordinal_position
            """,
            conn);
        cmd.Parameters.AddWithValue("s", schema);
        cmd.Parameters.AddWithValue("t", table);
        await using var reader = await cmd.ExecuteReaderAsync(ct).ConfigureAwait(false);
        while (await reader.ReadAsync(ct).ConfigureAwait(false))
        {
            list.Add(reader.GetString(0));
        }

        return list;
    }

    static string? PickColumn(IReadOnlyList<string> cols, params string[] names)
    {
        foreach (var name in names)
        {
            foreach (var col in cols)
            {
                if (col.Equals(name, StringComparison.OrdinalIgnoreCase))
                {
                    return col;
                }
            }
        }

        return null;
    }

    static string QuoteIdent(string ident) => "\"" + ident.Replace("\"", "\"\"", StringComparison.Ordinal) + "\"";

    static int ColumnOrdinal(NpgsqlDataReader reader, string? col)
    {
        if (col is null)
        {
            return -1;
        }

        for (var i = 0; i < reader.FieldCount; i++)
        {
            if (reader.GetName(i).Equals(col, StringComparison.OrdinalIgnoreCase))
            {
                return i;
            }
        }

        return -1;
    }

    static string? ReadString(NpgsqlDataReader reader, string? col)
    {
        var idx = ColumnOrdinal(reader, col);
        if (idx < 0 || reader.IsDBNull(idx))
        {
            return null;
        }

        return reader.GetValue(idx)?.ToString();
    }

    static int ReadInt(NpgsqlDataReader reader, string? col, int defaultValue)
    {
        var s = ReadString(reader, col);
        return int.TryParse(s, out var v) ? v : defaultValue;
    }

    static long ReadLong(NpgsqlDataReader reader, string? col, long defaultValue)
    {
        var s = ReadString(reader, col);
        return long.TryParse(s, out var v) ? v : defaultValue;
    }
}
