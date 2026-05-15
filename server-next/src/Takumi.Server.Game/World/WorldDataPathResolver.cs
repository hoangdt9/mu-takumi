namespace Takumi.Server.Game.World;

/// <summary>Resolves MuServer <c>4.GameServer/Data</c> paths from env or cwd (shared by M8 importers).</summary>
public static class WorldDataPathResolver
{
    public static string? ResolveDataRoot()
    {
        var env = Environment.GetEnvironmentVariable("TAKUMI_GAMESERVER_DATA_PATH")?.Trim();
        if (!string.IsNullOrEmpty(env))
        {
            return Path.GetFullPath(env);
        }

        var candidates = new[]
        {
            Path.Combine(Environment.CurrentDirectory, "Data"),
            Path.Combine(Environment.CurrentDirectory, "..", "MuServer", "4.GameServer", "Data"),
            Path.Combine(Environment.CurrentDirectory, "..", "MuServer", "4.GameServer", "Sub 1", "Data"),
            Path.Combine(Environment.CurrentDirectory, "..", "..", "MuServer", "4.GameServer", "Sub 1", "Data"),
        };

        foreach (var c in candidates)
        {
            var full = Path.GetFullPath(c);
            if (Directory.Exists(full) && File.Exists(Path.Combine(full, "ShopManager.txt")))
            {
                return full;
            }
        }

        return null;
    }

    public static string ResolveOrThrow(string? overridePath = null)
    {
        if (!string.IsNullOrWhiteSpace(overridePath))
        {
            return Path.GetFullPath(overridePath);
        }

        return ResolveDataRoot()
               ?? throw new InvalidOperationException(
                   "GameServer Data folder not found: set TAKUMI_GAMESERVER_DATA_PATH to MuServer/4.GameServer/Data.");
    }
}
