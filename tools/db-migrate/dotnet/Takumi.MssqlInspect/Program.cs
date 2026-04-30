using Microsoft.Data.SqlClient;

/// <summary>
/// Read-only MSSQL schema dump for Phase 2 (no credentials in repo).
/// Usage: TAKUMI_MSSQL_CONNECTION="Server=...;Database=MuOnline;User Id=...;Password=...;TrustServerCertificate=True" dotnet run
/// Or: dotnet run -- --connection "..."
/// </summary>
internal static class Program
{
    private const string Help = """
        takumi-mssql-inspect — dump dbo columns (read-only) for Takumi → OpenMU Phase 2.

        Connection (first match wins):
          env TAKUMI_MSSQL_CONNECTION
          arg  --connection "Server=...;Database=...;..."

        Options:
          --tables              List TABLE_SCHEMA.TABLE_NAME only.
          --table Name          Columns for one table (dbo.Name).
          --markdown            Column detail as markdown (default: CSV for --table / full dump uses one CSV header).
          --schema dbo          Table schema filter (default: dbo).

        Example:
          TAKUMI_MSSQL_CONNECTION="Server=127.0.0.1,1433;Database=MuOnline;User Id=sa;Password=***;TrustServerCertificate=True" \\
            dotnet run --project tools/db-migrate/dotnet/Takumi.MssqlInspect
        """;

    public static async Task<int> Main(string[] args)
    {
        if (args.Contains("-h") || args.Contains("--help"))
        {
            Console.WriteLine(Help);
            return 0;
        }

        var schema = ArgValue(args, "--schema") ?? "dbo";
        var connStr = Environment.GetEnvironmentVariable("TAKUMI_MSSQL_CONNECTION")
            ?? ArgValue(args, "--connection");

        if (string.IsNullOrWhiteSpace(connStr))
        {
            Console.Error.WriteLine("Missing connection: set TAKUMI_MSSQL_CONNECTION or use --connection\n");
            Console.WriteLine(Help);
            return 2;
        }

        var tablesOnly = args.Contains("--tables");
        var tableFilter = ArgValue(args, "--table");
        var markdown = args.Contains("--markdown");

        try
        {
            await using var conn = new SqlConnection(connStr);
            await conn.OpenAsync();

            if (tablesOnly)
            {
                await ListTablesAsync(conn, schema);
                return 0;
            }

            if (!string.IsNullOrEmpty(tableFilter))
                return await DumpTableColumnsAsync(conn, schema, tableFilter, markdown) ? 0 : 1;

            await DumpSchemaColumnsCsvAsync(conn, schema);
            return 0;
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine("Error: " + ex.Message);
            return 1;
        }
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

    private static async Task ListTablesAsync(SqlConnection conn, string schema)
    {
        const string sql = """
            SELECT TABLE_SCHEMA, TABLE_NAME
            FROM INFORMATION_SCHEMA.TABLES
            WHERE TABLE_TYPE = 'BASE TABLE' AND TABLE_SCHEMA = @schema
            ORDER BY TABLE_NAME
            """;

        await using var cmd = new SqlCommand(sql, conn);
        cmd.Parameters.AddWithValue("@schema", schema);
        await using var r = await cmd.ExecuteReaderAsync();
        while (await r.ReadAsync())
            Console.WriteLine($"{r.GetString(0)}.{r.GetString(1)}");
    }

    private static async Task<bool> DumpTableColumnsAsync(SqlConnection conn, string schema, string table, bool markdown)
    {
        const string sql = """
            SELECT COLUMN_NAME, DATA_TYPE, CHARACTER_MAXIMUM_LENGTH, IS_NULLABLE,
                   COLUMN_DEFAULT
            FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_SCHEMA = @schema AND TABLE_NAME = @table
            ORDER BY ORDINAL_POSITION
            """;

        await using var cmd = new SqlCommand(sql, conn);
        cmd.Parameters.AddWithValue("@schema", schema);
        cmd.Parameters.AddWithValue("@table", table);
        await using var r = await cmd.ExecuteReaderAsync();

        if (markdown)
        {
            Console.WriteLine($"# {schema}.{table}");
            Console.WriteLine("| column | type | len | nullable | default |");
            Console.WriteLine("|--------|------|-----|----------|---------|");
        }
        else
            Console.WriteLine("table_schema,table_name,column_name,data_type,max_len,nullable,default");

        var any = false;
        while (await r.ReadAsync())
        {
            any = true;
            WriteColumnRow(r, schema, table, markdown);
        }

        return any;
    }

    private static async Task DumpSchemaColumnsCsvAsync(SqlConnection conn, string schema)
    {
        const string sql = """
            SELECT TABLE_NAME
            FROM INFORMATION_SCHEMA.TABLES
            WHERE TABLE_TYPE = 'BASE TABLE' AND TABLE_SCHEMA = @schema
            ORDER BY TABLE_NAME
            """;

        await using var cmd = new SqlCommand(sql, conn);
        cmd.Parameters.AddWithValue("@schema", schema);
        await using var reader = await cmd.ExecuteReaderAsync();
        var tables = new List<string>();
        while (await reader.ReadAsync())
            tables.Add(reader.GetString(0));
        await reader.CloseAsync();

        Console.WriteLine("table_schema,table_name,column_name,data_type,max_len,nullable,default");

        const string colSql = """
            SELECT COLUMN_NAME, DATA_TYPE, CHARACTER_MAXIMUM_LENGTH, IS_NULLABLE,
                   COLUMN_DEFAULT
            FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_SCHEMA = @schema AND TABLE_NAME = @table
            ORDER BY ORDINAL_POSITION
            """;

        foreach (var t in tables)
        {
            await using var c2 = new SqlCommand(colSql, conn);
            c2.Parameters.AddWithValue("@schema", schema);
            c2.Parameters.AddWithValue("@table", t);
            await using var r = await c2.ExecuteReaderAsync();
            while (await r.ReadAsync())
                WriteColumnRow(r, schema, t, markdown: false);
        }
    }

    private static void WriteColumnRow(SqlDataReader r, string schema, string table, bool markdown)
    {
        var col = r.GetString(0);
        var dtype = r.GetString(1);
        var maxlen = r.IsDBNull(2) ? "" : r.GetInt32(2).ToString();
        var nullab = r.GetString(3);
        var def = FormatDefault(r, 4);

        if (markdown)
            Console.WriteLine($"| {col} | {dtype} | {maxlen} | {nullab} | {def.Replace('|', '/')} |");
        else
            Console.WriteLine($"{schema},{table},{EscapeCsv(col)},{EscapeCsv(dtype)},{EscapeCsv(maxlen)},{EscapeCsv(nullab)},{EscapeCsv(def)}");
    }

    private static string FormatDefault(SqlDataReader r, int ordinal)
    {
        if (r.IsDBNull(ordinal))
            return "";
        var v = r.GetValue(ordinal);
        return v switch
        {
            string s => s,
            byte[] bytes => Convert.ToHexString(bytes),
            _ => v.ToString() ?? "",
        };
    }

    private static string EscapeCsv(string s)
    {
        if (s.Contains(',') || s.Contains('"') || s.Contains('\n'))
            return $"\"{s.Replace("\"", "\"\"")}\"";
        return s;
    }
}
