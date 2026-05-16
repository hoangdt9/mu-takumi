using Microsoft.Data.SqlClient;

namespace Takumi.Etl;

internal static class LegacyMssql
{
    public sealed record ColumnInfo(string Name, string DataType, bool StoreAsBytea);

    public static bool IsSafeSqlIdentifier(string id) =>
        id.Length is > 0 and <= 128 &&
        id.All(ch => char.IsLetterOrDigit(ch) || ch == '_');

    public static string Bracket(string id)
    {
        if (!IsSafeSqlIdentifier(id))
            throw new ArgumentException("Unsafe identifier.", nameof(id));
        return "[" + id.Replace("]", "]]", StringComparison.Ordinal) + "]";
    }

    public static async Task<string?> ResolveTableNameAsync(
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

    public static async Task<IReadOnlyList<ColumnInfo>> GetColumnsAsync(
        SqlConnection conn,
        string schema,
        string actualTableName)
    {
        const string colSql = """
            SELECT COLUMN_NAME, DATA_TYPE
            FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_SCHEMA = @sch AND TABLE_NAME = @tbl
            ORDER BY ORDINAL_POSITION
            """;

        var list = new List<ColumnInfo>();
        await using var colCmd = new SqlCommand(colSql, conn);
        colCmd.Parameters.AddWithValue("@sch", schema);
        colCmd.Parameters.AddWithValue("@tbl", actualTableName);
        await using var r = await colCmd.ExecuteReaderAsync();
        while (await r.ReadAsync())
        {
            var cname = r.GetString(0);
            var dt = r.GetString(1);
            list.Add(new ColumnInfo(cname, dt, IsBinarySqlType(dt)));
        }

        return list;
    }

    public static bool IsBinarySqlType(string dataType) =>
        dataType.Equals("binary", StringComparison.OrdinalIgnoreCase)
        || dataType.Equals("varbinary", StringComparison.OrdinalIgnoreCase)
        || dataType.Equals("image", StringComparison.OrdinalIgnoreCase)
        || dataType.Equals("timestamp", StringComparison.OrdinalIgnoreCase); // SQL Server rowversion

    /// <summary>PostgreSQL double-quoted identifier.</summary>
    public static string QuotePgIdent(string name) =>
        "\"" + name.Replace("\"", "\"\"", StringComparison.Ordinal) + "\"";

    public static string PGStagingSchema => "takumi_legacy";
}
