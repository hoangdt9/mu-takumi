namespace Takumi.Server.Game;

/// <summary>
/// Loads <c>server-next/env.defaults</c> (committed, machine-agnostic) then <c>server-next/.env</c> (local, gitignored).
/// Defaults only set variables that are not already present in the process environment; .env always applies non-empty values.
/// </summary>
public static class RepoEnvLoader
{
    public static void ApplyDefaultsAndLocalEnv()
    {
        var serverNext = FindServerNextDirectory();
        if (serverNext is null)
        {
            return;
        }

        var defaultsPath = Path.Combine(serverNext, "env.defaults");
        if (File.Exists(defaultsPath))
        {
            ApplyFile(defaultsPath, overrideExisting: false);
        }

        var localPath = Path.Combine(serverNext, ".env");
        if (File.Exists(localPath))
        {
            ApplyFile(localPath, overrideExisting: true);
        }
    }

    static string? FindServerNextDirectory()
    {
        var dir = new DirectoryInfo(Directory.GetCurrentDirectory());
        for (var i = 0; i < 14 && dir != null; i++)
        {
            if (File.Exists(Path.Combine(dir.FullName, "env.defaults"))
                && File.Exists(Path.Combine(dir.FullName, "docker-compose.yml")))
            {
                return dir.FullName;
            }

            var nested = Path.Combine(dir.FullName, "server-next");
            if (File.Exists(Path.Combine(nested, "env.defaults"))
                && File.Exists(Path.Combine(nested, "docker-compose.yml")))
            {
                return nested;
            }

            dir = dir.Parent;
        }

        return null;
    }

    static void ApplyFile(string path, bool overrideExisting)
    {
        foreach (var rawLine in File.ReadAllLines(path))
        {
            var line = rawLine.Trim();
            if (line.Length == 0 || line.StartsWith("#", StringComparison.Ordinal))
            {
                continue;
            }

            if (line.StartsWith("export ", StringComparison.OrdinalIgnoreCase))
            {
                line = line[7..].TrimStart();
            }

            var eq = line.IndexOf('=');
            if (eq <= 0)
            {
                continue;
            }

            var key = line[..eq].Trim();
            if (key.Length == 0)
            {
                continue;
            }

            var val = line[(eq + 1)..].Trim();
            if (val.Length >= 2
                && ((val[0] == '"' && val[^1] == '"') || (val[0] == '\'' && val[^1] == '\'')))
            {
                val = val[1..^1];
            }

            if (string.IsNullOrEmpty(val))
            {
                continue;
            }

            if (!overrideExisting && !string.IsNullOrEmpty(Environment.GetEnvironmentVariable(key)))
            {
                continue;
            }

            Environment.SetEnvironmentVariable(key, val);
        }
    }
}
