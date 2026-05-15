using Takumi.Server.Persistence;

namespace Takumi.Server.Game.World;

/// <summary>ETL <c>Data/Custom/*</c> into <c>custom_world_config</c> (parity <c>CServerInfo::ReadCustomInfo</c> snapshot).</summary>
public static class CustomWorldConfigDbImporter
{
    static readonly string[] TableExtensions = [".txt"];

    static readonly string[] RawExtensions = [".ini", ".xml"];

    public static async Task<int> ImportDirectoryAsync(string? customDir = null, CancellationToken ct = default)
    {
        var cs = PostgresCharacterRosterRepository.BuildConnectionStringFromEnv()
                 ?? throw new InvalidOperationException(
                     "No Postgres connection: set TAKUMI_PG_CONNECTION_STRING or TAKUMI_PG_HOST.");

        var dir = customDir;
        if (string.IsNullOrWhiteSpace(dir))
        {
            var root = WorldDataPathResolver.ResolveOrThrow();
            dir = Path.Combine(root, "Custom");
        }

        if (!Directory.Exists(dir))
        {
            throw new DirectoryNotFoundException($"Custom folder not found: {dir}");
        }

        var rows = new List<CustomWorldConfigRow>();
        foreach (var file in Directory.EnumerateFiles(dir, "*.*", SearchOption.TopDirectoryOnly))
        {
            var ext = Path.GetExtension(file).ToLowerInvariant();
            var key = "Custom/" + Path.GetFileName(file);
            if (TableExtensions.Contains(ext))
            {
                rows.Add(CustomWorldConfigBuilder.FromTableFile(key, file));
            }
            else if (RawExtensions.Contains(ext))
            {
                rows.Add(CustomWorldConfigBuilder.FromRawFile(key, file, ext.TrimStart('.')));
            }
        }

        await using var repo = new PostgresCustomWorldConfigRepository(cs);
        await repo.ReplaceAllAsync(rows, ct).ConfigureAwait(false);
        return rows.Count;
    }
}
