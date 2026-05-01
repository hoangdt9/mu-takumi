using System.Globalization;
using System.Data.SqlTypes;
using Microsoft.Data.SqlClient;
using Npgsql;
using NpgsqlTypes;

namespace Takumi.Etl;

/// <summary>
/// Creates <c>takumi_staging.legacy_*</c> mirrors of MEMB_INFO and Character; optional full reload from MSSQL.
/// </summary>
internal static class StagingLoginPath
{
    public static async Task<int> RunAsync(string[] tail)
    {
        try
        {
            return await RunCoreAsync(tail);
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine("staging-login-path failed: " + ex.Message);
            return 1;
        }
    }

    private static async Task<int> RunCoreAsync(string[] tail)
    {
        var schema = EtlArgs.ArgValue(tail, "--schema") ?? "dbo";
        var recreate = tail.Contains("--recreate");
        var load = tail.Contains("--load");

        if (!recreate && !load)
        {
            Console.Error.WriteLine(
                "staging-login-path: use --recreate (DDL), --load (TRUNCATE+INSERT), or both. See takumi-etl --help.");
            return 2;
        }

        if (!LegacyMssql.IsSafeSqlIdentifier(schema))
        {
            Console.Error.WriteLine("Invalid --schema (use letters/digits/_ only).");
            return 2;
        }

        var mssql = Environment.GetEnvironmentVariable("TAKUMI_MSSQL_CONNECTION");
        var pgCs = Environment.GetEnvironmentVariable("TAKUMI_PG_CONNECTION");
        if (string.IsNullOrWhiteSpace(mssql) || string.IsNullOrWhiteSpace(pgCs))
        {
            Console.Error.WriteLine(
                "staging-login-path requires TAKUMI_MSSQL_CONNECTION and TAKUMI_PG_CONNECTION.\n" +
                "  • copy tools/db-migrate/db-migrate.env.sample → tools/db-migrate/.env rồi chỉnh port/mật khẩu\n" +
                "  • staging-login-sync.sh tự động source tools/db-migrate/.env");
            return 2;
        }

        await using var ms = new SqlConnection(mssql);
        await ms.OpenAsync();
        await using var pg = new NpgsqlConnection(pgCs);
        await pg.OpenAsync();

        var membResolved = await LegacyMssql.ResolveTableNameAsync(ms, schema, "MEMB_INFO");
        var charResolved = await LegacyMssql.ResolveTableNameAsync(ms, schema, "Character");
        if (membResolved is null || charResolved is null)
        {
            Console.Error.WriteLine("MEMB_INFO or Character not found in MSSQL; abort.");
            return 3;
        }

        var membCols = await LegacyMssql.GetColumnsAsync(ms, schema, membResolved);
        var charCols = await LegacyMssql.GetColumnsAsync(ms, schema, charResolved);
        if (membCols.Count == 0 || charCols.Count == 0)
        {
            Console.Error.WriteLine("No columns fetched for MEMB_INFO or Character; abort.");
            return 3;
        }

        await EnsureSchemaExistsAsync(pg);

        if (recreate)
        {
            var schQuote = LegacyMssql.QuotePgIdent(LegacyMssql.PGStagingSchema);
            await ExecAsync(pg, $"DROP TABLE IF EXISTS {schQuote}.{LegacyMssql.QuotePgIdent("legacy_memb_info")};");
            await ExecAsync(pg, $"DROP TABLE IF EXISTS {schQuote}.{LegacyMssql.QuotePgIdent("legacy_character")};");
            await CreateMirrorTableAsync(pg, "legacy_memb_info", membCols);
            await CreateMirrorTableAsync(pg, "legacy_character", charCols);
            Console.WriteLine("RECREATE: takumi_staging.legacy_memb_info + legacy_character");
        }
        else
            await GuardTablesExistAsync(pg);

        if (!load)
            return 0;

        var nMemb = await CopyTableAsync(ms, pg, schema, membResolved, membCols, "legacy_memb_info");
        var nChar = await CopyTableAsync(ms, pg, schema, charResolved, charCols, "legacy_character");
        Console.WriteLine($"LOAD: legacy_memb_info rows={nMemb}, legacy_character rows={nChar}");

        return 0;
    }

    private static async Task GuardTablesExistAsync(NpgsqlConnection pg)
    {
        await using var cmd = new NpgsqlCommand(
            """
            SELECT NOT EXISTS (SELECT 1 FROM information_schema.tables t
              WHERE t.table_schema = 'takumi_staging' AND t.table_name = 'legacy_memb_info')
              OR NOT EXISTS (SELECT 1 FROM information_schema.tables t
              WHERE t.table_schema = 'takumi_staging' AND t.table_name = 'legacy_character')
            """,
            pg);
        var missing = (bool)(await cmd.ExecuteScalarAsync() ?? true);
        if (missing)
        {
            throw new InvalidOperationException(
                "Staging tables missing; run: takumi-etl staging-login-path --recreate first.");
        }
    }

    private static async Task EnsureSchemaExistsAsync(NpgsqlConnection pg) =>
        await ExecAsync(pg, $"CREATE SCHEMA IF NOT EXISTS {LegacyMssql.QuotePgIdent(LegacyMssql.PGStagingSchema)};");

    private static async Task ExecAsync(NpgsqlConnection pg, string sql)
    {
        await using var cmd = new NpgsqlCommand(sql, pg);
        await cmd.ExecuteNonQueryAsync();
    }

    private static async Task CreateMirrorTableAsync(
        NpgsqlConnection pg,
        string stagingTable,
        IReadOnlyList<LegacyMssql.ColumnInfo> cols)
    {
        var parts = new List<string>();
        foreach (var c in cols)
        {
            var ty = c.StoreAsBytea ? "bytea" : "text";
            parts.Add($"{LegacyMssql.QuotePgIdent(c.Name)} {ty} NULL");
        }

        parts.Add("etl_staged_at timestamptz NOT NULL DEFAULT now()");

        var fq = $"{LegacyMssql.QuotePgIdent(LegacyMssql.PGStagingSchema)}.{LegacyMssql.QuotePgIdent(stagingTable)}";
        var ddl = $"CREATE TABLE {fq} (\n  {string.Join(",\n  ", parts)}\n);";
        await ExecAsync(pg, ddl);
        Console.WriteLine($"CREATE TABLE {fq} ({cols.Count} legacy columns + etl_staged_at)");
    }

    private static async Task<int> CopyTableAsync(
        SqlConnection ms,
        NpgsqlConnection pg,
        string mssqlSchema,
        string mssqlTable,
        IReadOnlyList<LegacyMssql.ColumnInfo> cols,
        string stagingTable)
    {
        await using var trunc = new NpgsqlCommand(
            $"TRUNCATE TABLE {LegacyMssql.QuotePgIdent(LegacyMssql.PGStagingSchema)}.{LegacyMssql.QuotePgIdent(stagingTable)};",
            pg);
        await trunc.ExecuteNonQueryAsync();

        var colList = string.Join(", ", cols.Select(c => LegacyMssql.QuotePgIdent(c.Name)));
        var fq = $"{LegacyMssql.QuotePgIdent(LegacyMssql.PGStagingSchema)}.{LegacyMssql.QuotePgIdent(stagingTable)}";

        var selectSql = $"SELECT {string.Join(", ", cols.Select(c => LegacyMssql.Bracket(c.Name)))} " +
                        $"FROM {LegacyMssql.Bracket(mssqlSchema)}.{LegacyMssql.Bracket(mssqlTable)}";

        await using var readCmd = new SqlCommand(selectSql, ms) { CommandTimeout = 0 };
        await using var reader = await readCmd.ExecuteReaderAsync();

        var placeholders = string.Join(", ", cols.Select((_, i) => "$" + (i + 1)));
        var insertSql = $"INSERT INTO {fq} ({colList}, etl_staged_at) VALUES ({placeholders}, now())";
        var count = 0;
        await using var tx = await pg.BeginTransactionAsync();
        await using var writeCmd = new NpgsqlCommand(insertSql, pg, tx);

        while (await reader.ReadAsync())
        {
            writeCmd.Parameters.Clear();
            for (var i = 0; i < cols.Count; i++)
            {
                var c = cols[i];
                object? val;
                if (reader.IsDBNull(i))
                    val = DBNull.Value;
                else if (c.StoreAsBytea)
                {
                    var raw = reader.GetValue(i);
                    val = raw switch
                    {
                        byte[] b => b,
                        SqlBinary sb => sb.Value,
                        _ => throw new InvalidCastException(
                            $"Column {cols[i].Name}: expected binary, got {(raw?.GetType().FullName ?? "null")}."),
                    };
                }
                else
                {
                    var v = reader.GetValue(i);
                    val = v switch
                    {
                        string s => s,
                        bool b => b ? "true" : "false",
                        DateTime dt => dt.ToString("o", CultureInfo.InvariantCulture),
                        Guid g => g.ToString("D", CultureInfo.InvariantCulture),
                        byte or sbyte or short or int or long or decimal or double or float =>
                            Convert.ToString(v, CultureInfo.InvariantCulture) ?? "",
                        _ => v.ToString() ?? "",
                    };
                }

                writeCmd.Parameters.Add(new NpgsqlParameter
                {
                    NpgsqlDbType = c.StoreAsBytea ? NpgsqlDbType.Bytea : NpgsqlDbType.Text,
                    Value = val ?? DBNull.Value,
                });
            }

            await writeCmd.ExecuteNonQueryAsync();
            count++;
            if (count % 500 == 0)
                Console.WriteLine($"  ... {stagingTable} inserted {count}");
        }

        await tx.CommitAsync();
        return count;
    }
}
