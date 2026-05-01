using Microsoft.Data.SqlClient;
using Npgsql;

/// <summary>
/// Placeholder ETL CLI: MSSQL legacy → Postgres OpenMU. Loads not implemented yet.
/// Use <c>check-sources</c> before running inspectors or future migration jobs.
/// </summary>
internal static class Program
{
    private const string Help = """
        takumi-etl — Phase 2 scaffold (loads TODO). Validates env connections.

        Env (same as inspectors):
          TAKUMI_MSSQL_CONNECTION    Server=...;Database=...;
          TAKUMI_PG_CONNECTION       Host=...;Database=...;...

        Commands:
          check-sources   Print OK/SKIP for each connection present in env (--help skips run).

        From repo root:
          dotnet run --project tools/db-migrate/dotnet/Takumi.Etl -- check-sources
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

        Console.Error.WriteLine($"Unknown command: {args[0]}\n");
        Console.WriteLine(Help);
        return 2;
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
                {
                    Console.WriteLine("[MSSQL] OK — connected (no metadata row)");
                }
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
                {
                    Console.WriteLine("[PG]    OK — connected");
                }
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
