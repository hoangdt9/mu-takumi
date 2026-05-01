using Npgsql;

/// Read-only Postgres information_schema dump (same CSV shape as takumi-mssql-inspect).
/// Connection: env TAKUMI_PG_CONNECTION or args --connection "Host=...;Database=...;..."
/// From repo root: dotnet run --project tools/db-migrate/dotnet/Takumi.PgInspect -- --help
internal static class Program
{
    private const string Help = """
        takumi-pg-inspect — dump columns (read-only) for OpenMU / Postgres Phase 2 parity.

        Connection (first match wins):
          env TAKUMI_PG_CONNECTION
          arg  --connection "Host=...;Port=5432;Database=...;Username=...;Password=..."

        Options:
          --tables              List schema.table_name only.
          --mapping-rows        CSV: header + TABLE row per base table in --schema (fill mapping columns).
          --mapping-openmu-all  Same CSV for schemas data,config,friend,guild (legacy_name = schema.table).
          --table Name          Columns for one table (schema.Name; default schema: public).
          --markdown            Column detail as markdown.
          --schema public       Table schema filter (default: public). OpenMU: data / config / friend / guild.

        Example (OpenMU Postgres — schema tên data/config, c.f. SchemaNames.cs):
          TAKUMI_PG_CONNECTION="Host=127.0.0.1;Port=5433;..." \\
            dotnet run --project tools/db-migrate/dotnet/Takumi.PgInspect -- --schema data --tables
          dotnet run --project tools/db-migrate/dotnet/Takumi.PgInspect -- --mapping-openmu-all \\
            > docs/takumi-game-spec/PHASE2-MAPPING-OPENMU-EF-TABLES-FULL.csv
        """;

    public static async Task<int> Main(string[] args)
    {
        if (args.Contains("-h") || args.Contains("--help"))
        {
            Console.WriteLine(Help);
            return 0;
        }

        var schema = ArgValue(args, "--schema") ?? "public";
        var connStr = Environment.GetEnvironmentVariable("TAKUMI_PG_CONNECTION")
            ?? ArgValue(args, "--connection");

        if (string.IsNullOrWhiteSpace(connStr))
        {
            Console.Error.WriteLine("Missing connection: set TAKUMI_PG_CONNECTION or use --connection\n");
            Console.WriteLine(Help);
            return 2;
        }

        var tablesOnly = args.Contains("--tables");
        var mappingRows = args.Contains("--mapping-rows");
        var mappingOpenMuAll = args.Contains("--mapping-openmu-all");
        var tableFilter = ArgValue(args, "--table");
        var markdown = args.Contains("--markdown");

        try
        {
            await using var conn = new NpgsqlConnection(connStr);
            await conn.OpenAsync();

            if (mappingOpenMuAll)
            {
                await EmitOpenMuAllMappingRowsAsync(conn);
                return 0;
            }

            if (mappingRows)
            {
                await EmitMappingTemplateRowsAsync(conn, schema);
                return 0;
            }

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

    private static async Task EmitMappingTemplateRowsAsync(NpgsqlConnection conn, string schema)
    {
        const string sql = """
            SELECT table_name
            FROM information_schema.tables
            WHERE table_type = 'BASE TABLE'
              AND table_schema = @schema
            ORDER BY table_name
            """;

        Console.WriteLine("kind,legacy_name,openmu_or_plugin,parity_status,notes");

        await using var cmd = new NpgsqlCommand(sql, conn);
        cmd.Parameters.AddWithValue("schema", schema);
        await using var r = await cmd.ExecuteReaderAsync();
        while (await r.ReadAsync())
        {
            var t = r.GetString(0);
            var qualified = $"{schema}.{t}";
            var note = $"{qualified} — OpenMU Postgres; map từ takumi-pg-inspect --table {t}";
            Console.WriteLine($"TABLE,{EscapeCsv(qualified)},,todo,{EscapeCsv(note)}");
        }
    }

    private static async Task EmitOpenMuAllMappingRowsAsync(NpgsqlConnection conn)
    {
        const string sql = """
            SELECT table_schema, table_name
            FROM information_schema.tables
            WHERE table_type = 'BASE TABLE'
              AND table_schema IN ('data', 'config', 'friend', 'guild')
            ORDER BY table_schema, table_name
            """;

        Console.WriteLine("kind,legacy_name,openmu_or_plugin,parity_status,notes");

        await using var cmd = new NpgsqlCommand(sql, conn);
        await using var r = await cmd.ExecuteReaderAsync();
        while (await r.ReadAsync())
        {
            var sch = r.GetString(0);
            var t = r.GetString(1);
            var qualified = $"{sch}.{t}";
            var note = $"{qualified} — OpenMU EF snapshot; đối chiếu legacy MSSQL";
            Console.WriteLine($"TABLE,{EscapeCsv(qualified)},,todo,{EscapeCsv(note)}");
        }
    }

    private static async Task ListTablesAsync(NpgsqlConnection conn, string schema)
    {
        const string sql = """
            SELECT table_schema, table_name
            FROM information_schema.tables
            WHERE table_type = 'BASE TABLE'
              AND table_schema = @schema
            ORDER BY table_name
            """;

        await using var cmd = new NpgsqlCommand(sql, conn);
        cmd.Parameters.AddWithValue("schema", schema);
        await using var r = await cmd.ExecuteReaderAsync();
        while (await r.ReadAsync())
            Console.WriteLine($"{r.GetString(0)}.{r.GetString(1)}");
    }

    private static async Task<bool> DumpTableColumnsAsync(
        NpgsqlConnection conn,
        string schema,
        string table,
        bool markdown)
    {
        const string sql = """
            SELECT column_name, data_type, character_maximum_length, is_nullable, column_default
            FROM information_schema.columns
            WHERE table_schema = @schema AND table_name = @tbl
            ORDER BY ordinal_position
            """;

        await using var cmd = new NpgsqlCommand(sql, conn);
        cmd.Parameters.AddWithValue("schema", schema);
        cmd.Parameters.AddWithValue("tbl", table);
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

    private static async Task DumpSchemaColumnsCsvAsync(NpgsqlConnection conn, string schema)
    {
        const string sql = """
            SELECT table_name
            FROM information_schema.tables
            WHERE table_type = 'BASE TABLE'
              AND table_schema = @schema
            ORDER BY table_name
            """;

        await using var cmd = new NpgsqlCommand(sql, conn);
        cmd.Parameters.AddWithValue("schema", schema);
        await using var reader = await cmd.ExecuteReaderAsync();
        var tables = new List<string>();
        while (await reader.ReadAsync())
            tables.Add(reader.GetString(0));
        await reader.CloseAsync();

        Console.WriteLine("table_schema,table_name,column_name,data_type,max_len,nullable,default");

        const string colSql = """
            SELECT column_name, data_type, character_maximum_length, is_nullable, column_default
            FROM information_schema.columns
            WHERE table_schema = @schema AND table_name = @tbl
            ORDER BY ordinal_position
            """;

        foreach (var t in tables)
        {
            await using var c2 = new NpgsqlCommand(colSql, conn);
            c2.Parameters.AddWithValue("schema", schema);
            c2.Parameters.AddWithValue("tbl", t);
            await using var r = await c2.ExecuteReaderAsync();
            while (await r.ReadAsync())
                WriteColumnRow(r, schema, t, markdown: false);
        }
    }

    private static void WriteColumnRow(NpgsqlDataReader r, string schema, string table, bool markdown)
    {
        var col = r.GetString(0);
        var dtype = r.GetString(1);
        var maxlen = r.IsDBNull(2) ? "" : FormatMaxLen(r.GetValue(2));
        var nullab = r.GetString(3);
        var def = r.IsDBNull(4) ? "" : FormatDefault(r.GetValue(4));

        if (markdown)
            Console.WriteLine($"| {col} | {dtype} | {maxlen} | {nullab} | {def.Replace('|', '/')} |");
        else
            Console.WriteLine($"{schema},{table},{EscapeCsv(col)},{EscapeCsv(dtype)},{EscapeCsv(maxlen)},{EscapeCsv(nullab)},{EscapeCsv(def)}");
    }

    private static string FormatMaxLen(object v) => v switch
    {
        int i => i.ToString(),
        long l => l.ToString(),
        short s => s.ToString(),
        decimal d => d.ToString("0", System.Globalization.CultureInfo.InvariantCulture),
        _ => v.ToString() ?? "",
    };

    private static string FormatDefault(object v) => v switch
    {
        string s => s,
        byte[] bytes => Convert.ToHexString(bytes),
        _ => v.ToString() ?? "",
    };

    private static string EscapeCsv(string s)
    {
        if (s.Contains(',') || s.Contains('"') || s.Contains('\n'))
            return $"\"{s.Replace("\"", "\"\"")}\"";
        return s;
    }
}
