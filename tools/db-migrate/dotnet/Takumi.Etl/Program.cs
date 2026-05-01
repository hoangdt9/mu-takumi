using Microsoft.Data.SqlClient;
using Npgsql;

/// <summary>
/// Takumi MSSQL → OpenMU Postgres ETL scaffolding (loads still TODO).
/// </summary>
internal static class Program
{
    private const string Help = """
        takumi-etl — Phase 2 scaffold. Read-only MSSQL previews; Postgres loads TODO.

        Env (inspectors reuse the same vars):
          TAKUMI_MSSQL_CONNECTION    Server=...;Database=...;
          TAKUMI_PG_CONNECTION       optional for future writer steps

        Commands:
          check-sources          OK/SKIP/FAIL per connection in env.

          preview-login-path     MSSQL-only: dbo MEMB_INFO + Character — row counts, columns,
                                 heuristic lines for OpenMU data.Account / Character (no writes).

          Options after command:
          --schema dbo           Table schema filter (default: dbo).

        From repo root:
          dotnet run --project tools/db-migrate/dotnet/Takumi.Etl -- check-sources
          dotnet run --project tools/db-migrate/dotnet/Takumi.Etl -- preview-login-path --schema dbo
        """;

    public static async Task<int> Main(string[] args)
    {
        if (args.Length == 0 || args.Contains("-h") || args.Contains("--help"))
        {
            Console.WriteLine(Help);
            return 0;
        }

        if (args[0] == "check-sources")
            return await CheckSourcesAsync();

        if (args[0] == "preview-login-path")
        {
            var tail = args.AsSpan(1).ToArray();
            var schema = ArgValue(tail, "--schema") ?? "dbo";
            return await PreviewLoginPathAsync(schema);
        }

        Console.Error.WriteLine($"Unknown command: {args[0]}\n");
        Console.WriteLine(Help);
        return 2;
    }

    private static string? ArgValue(IReadOnlyList<string> args, string name)
    {
        for (var i = 0; i < args.Count - 1; i++)
        {
            if (string.Equals(args[i], name, StringComparison.OrdinalIgnoreCase))
                return args[i + 1];
        }

        return null;
    }

    private static async Task<int> PreviewLoginPathAsync(string schema)
    {
        if (!IsSafeSqlIdentifier(schema))
        {
            Console.Error.WriteLine("Invalid --schema (use letters/digits/_ only).");
            return 2;
        }

        var mssql = Environment.GetEnvironmentVariable("TAKUMI_MSSQL_CONNECTION");
        if (string.IsNullOrWhiteSpace(mssql))
        {
            Console.Error.WriteLine("preview-login-path requires TAKUMI_MSSQL_CONNECTION.");
            return 2;
        }

        await using var conn = new SqlConnection(mssql);
        await conn.OpenAsync();

        var membName = await ResolveTableNameAsync(conn, schema, "MEMB_INFO");
        var charName = await ResolveTableNameAsync(conn, schema, "Character");

        Console.WriteLine($"Legacy schema [{schema}], login-path preview (read-only)");

        await DumpLegacyTableOutlineAsync(conn, schema, membName, "MEMB_INFO (accounts — Join/DataServer)");
        await DumpLegacyTableOutlineAsync(conn, schema, charName, "[Character] (per Takumi queries)");

        if (membName is null || charName is null)
        {
            Console.Error.WriteLine("\nWARN: Missing MEMB_INFO or Character — check restore / schema (Gate 2 needs both). Table names matched case-insensitively.");
            return 3;
        }

        Console.WriteLine("""
            
            --- Heuristic targets (verify OpenMU EF + PHASE2 doc) ---
            MSSQL memb___id        -> data.Account.LoginName (PK not 1:1; OpenMU may use GUID keys — keep id map table)
            MSSQL memb__pwd        -> data.Account.PasswordHash (legacy may be plaintext/MD5; OpenMU prefers BCrypt — re-hash policy Gate 2)
            MSSQL bloc_code (+ TTL) -> data.Account.State (AccountState.Normal/Banned/TemporaryBanned)
            Character.AccountID    -> must match memb___id (string) linking to migrated Account.LoginName / map table
            Character.Name         -> data.Character.Name
            Character.cLevel (+ exp fields if any) -> Level / Experience (OpenMU model differs — normalize)
            Character.Class        -> CharacterClass FK via config.CharacterClass.Number
            Character.Inventory    -> ItemStorage / parse blob → Item slots (no raw copy)
            
            Docs: docs/takumi-game-spec/PHASE2-OPENMU-DATA-MODEL-MAP.md §2, §3.
            """);

        return 0;
    }

    private static async Task DumpLegacyTableOutlineAsync(
        SqlConnection conn,
        string schema,
        string? actualTableName,
        string label)
    {
        Console.WriteLine();
        Console.WriteLine($"== {label} ==");
        if (actualTableName is null)
        {
            Console.WriteLine("  (table not found)");
            return;
        }

        Console.WriteLine($"  resolved name: {schema}.{actualTableName}");

        await using var countCmd = new SqlCommand(
            $"SELECT COUNT(*) FROM {Bracket(schema)}.{Bracket(actualTableName)}",
            conn);
        var cntObj = await countCmd.ExecuteScalarAsync();
        Console.WriteLine($"  row_count: {cntObj}");

        const string colSql = """
            SELECT COLUMN_NAME, DATA_TYPE, CHARACTER_MAXIMUM_LENGTH, IS_NULLABLE
            FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_SCHEMA = @sch AND TABLE_NAME = @tbl
            ORDER BY ORDINAL_POSITION
            """;

        await using var colCmd = new SqlCommand(colSql, conn);
        colCmd.Parameters.AddWithValue("@sch", schema);
        colCmd.Parameters.AddWithValue("@tbl", actualTableName);
        await using var r = await colCmd.ExecuteReaderAsync();

        Console.WriteLine("  columns:");
        while (await r.ReadAsync())
        {
            var cname = r.GetString(0);
            var dt = r.GetString(1);
            var maxlen = r.IsDBNull(2) ? "" : r.GetInt32(2).ToString();
            var nullable = r.GetString(3);
            Console.WriteLine($"    {cname}\t{dt}\t{(maxlen == "" ? "(n/a)" : maxlen)}\t{nullable}");
        }
    }

    private static async Task<string?> ResolveTableNameAsync(
        SqlConnection conn,
        string schema,
        string logicalLower)
    {
        const string sql = """
            SELECT t.name
            FROM sys.tables t
            INNER JOIN sys.schemas s ON t.schema_id = s.schema_id
            WHERE s.name = @sch AND lower(t.name) = @logical
            """;
        await using var cmd = new SqlCommand(sql, conn);
        cmd.Parameters.AddWithValue("@sch", schema);
        cmd.Parameters.AddWithValue("@logical", logicalLower.ToLowerInvariant());
        var o = await cmd.ExecuteScalarAsync();
        return o is string n ? n : null;
    }

    private static bool IsSafeSqlIdentifier(string id) =>
        id.Length is > 0 and <= 128 &&
        id.All(ch => char.IsLetterOrDigit(ch) || ch == '_');

    private static string Bracket(string id)
    {
        if (!IsSafeSqlIdentifier(id))
            throw new ArgumentException("Unsafe identifier.", nameof(id));
        return "[" + id.Replace("]", "]]", StringComparison.Ordinal) + "]";
    }

    private static async Task<int> CheckSourcesAsync()
    {
        var ok = true;
        var mssql = Environment.GetEnvironmentVariable("TAKUMI_MSSQL_CONNECTION");
        if (string.IsNullOrWhiteSpace(mssql))
            Console.WriteLine("[MSSQL] SKIP — TAKUMI_MSSQL_CONNECTION not set");
        else
        {
            try
            {
                await using var conn = new SqlConnection(mssql);
                await conn.OpenAsync();
                await using var cmd = new SqlCommand("SELECT DB_NAME(), @@VERSION", conn);
                await using var r = await cmd.ExecuteReaderAsync();
                if (await r.ReadAsync())
                {
                    var db = r.IsDBNull(0) ? "?" : r.GetString(0);
                    var ver = r.IsDBNull(1) ? "?" : r.GetString(1).Split('\n')[0].Trim();
                    Console.WriteLine($"[MSSQL] OK — database={db}; {ver}");
                }
                else
                    Console.WriteLine("[MSSQL] OK — connected (no metadata row)");
            }
            catch (Exception ex)
            {
                ok = false;
                Console.Error.WriteLine("[MSSQL] FAIL — " + ex.Message);
            }
        }

        var pg = Environment.GetEnvironmentVariable("TAKUMI_PG_CONNECTION");
        if (string.IsNullOrWhiteSpace(pg))
            Console.WriteLine("[PG]    SKIP — TAKUMI_PG_CONNECTION not set");
        else
        {
            try
            {
                await using var conn = new NpgsqlConnection(pg);
                await conn.OpenAsync();
                await using var cmd = new NpgsqlCommand(
                    "SELECT current_database(), version()",
                    conn);
                await using var r = await cmd.ExecuteReaderAsync();
                if (await r.ReadAsync())
                {
                    var db = r.GetFieldValue<string>(0);
                    var ver = r.GetFieldValue<string>(1).Split('\n')[0].Trim();
                    Console.WriteLine($"[PG]    OK — database={db}; {ver}");
                }
                else
                    Console.WriteLine("[PG]    OK — connected");
            }
            catch (Exception ex)
            {
                ok = false;
                Console.Error.WriteLine("[PG]    FAIL — " + ex.Message);
            }
        }

        return ok ? 0 : 1;
    }
}
