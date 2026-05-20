using Microsoft.Data.SqlClient;
using Npgsql;
using Takumi.Etl;

/// <summary>
/// Takumi MSSQL → Postgres (staging + OpenMU path). Loads: optional takumi_staging mirrors.
/// </summary>
internal static class Program
{
    private const string Help = """
        takumi-etl — Phase 2: MSSQL → Postgres staging for Gate 2 (dev only).

        Env files (optional; do not commit secrets):
          tools/db-migrate/.env — copy from db-migrate.env.sample; bash wrappers auto-source it.

        Env:
          TAKUMI_MSSQL_CONNECTION    legacy MuOnline
          TAKUMI_PG_CONNECTION       OpenMU Postgres (staging schema takumi_* does not collide with data/config)

        Commands:
          check-sources
             Connection smoke test.

          preview-login-path [--schema dbo]
             Read-only MSSQL: MEMB_INFO + Character outlines + heuristic OpenMU map.

          staging-login-path [--schema dbo] (--recreate | --load)+
             --recreate   DROP + CREATE takumi_staging.legacy_memb_info / legacy_character (columns from MSSQL).
             --load       TRUNCATE + copy all rows (passwords/binary land in staging — dev DB only).

        Full staging refresh:
          dotnet run --project tools/db-migrate/dotnet/Takumi.Etl -- staging-login-path --recreate --load

        NEVER point --load at production Postgres.
        """;

    public static async Task<int> Main(string[] args)
    {
        EnvLoader.ApplyRepoLocalDotEnv();

        if (args.Length == 0 || args.Contains("-h") || args.Contains("--help"))
        {
            Console.WriteLine(Help);
            return 0;
        }

        if (args[0] == "check-sources")
            return await CheckSourcesAsync();

        if (args[0] == "preview-login-path")
        {
            var tail = args.Skip(1).ToArray();
            var schema = EtlArgs.ArgValue(tail, "--schema") ?? "dbo";
            return await PreviewLoginPathAsync(schema);
        }

        if (args[0] == "staging-login-path")
            return await StagingLoginPath.RunAsync(args.Skip(1).ToArray());

        Console.Error.WriteLine($"Unknown command: {args[0]}\n");
        Console.WriteLine(Help);
        return 2;
    }

    private static async Task<int> PreviewLoginPathAsync(string schema)
    {
        if (!LegacyMssql.IsSafeSqlIdentifier(schema))
        {
            Console.Error.WriteLine("Invalid --schema (use letters/digits/_ only).");
            return 2;
        }

        var mssql = Environment.GetEnvironmentVariable("TAKUMI_MSSQL_CONNECTION");
        if (string.IsNullOrWhiteSpace(mssql))
        {
            Console.Error.WriteLine(
                "preview-login-path requires TAKUMI_MSSQL_CONNECTION (export hoặc file tools/db-migrate/.env từ db-migrate.env.sample — README).");
            return 2;
        }

        await using var conn = new SqlConnection(mssql);
        await conn.OpenAsync();

        var membName = await LegacyMssql.ResolveTableNameAsync(conn, schema, "MEMB_INFO");
        var charName = await LegacyMssql.ResolveTableNameAsync(conn, schema, "Character");

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
            MSSQL memb___id        -> data.Account.LoginName (Guid PK on Account — keep id mapping table if needed)
            MSSQL memb__pwd        -> data.Account.PasswordHash (BCrypt migration policy at Gate 2)
            MSSQL bloc_code (+ TTL) -> data.Account.State
            Character.AccountID    -> fk to memb___id / migrated login
            Character.Name         -> data.Character.Name
            Character.Class / cLevel / inventory blob -> EF shape + parsers (staging: takumi_staging.legacy_*)

            Docs: docs/game-spec/PHASE2-OPENMU-DATA-MODEL-MAP.md §2–§5.
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
            $"SELECT COUNT(*) FROM {LegacyMssql.Bracket(schema)}.{LegacyMssql.Bracket(actualTableName)}",
            conn);
        var cntObj = await countCmd.ExecuteScalarAsync();
        Console.WriteLine($"  row_count: {cntObj}");

        var cols = await LegacyMssql.GetColumnsAsync(conn, schema, actualTableName);

        Console.WriteLine("  columns:");
        foreach (var c in cols)
            Console.WriteLine($"    {c.Name}\t{c.DataType}\t{(c.StoreAsBytea ? "bytea→PG" : "text→PG")}");
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
