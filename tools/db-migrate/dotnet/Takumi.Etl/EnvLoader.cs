namespace Takumi.Etl;

/// <summary>
/// Loads <c>tools/db-migrate/.env</c> first, then repo-root <c>.env</c> (same directory rules as bash wrappers).
/// Only sets vars that are not already present in <see cref="Environment.GetEnvironmentVariable"/>.
/// Lines: KEY=VALUE (# comments, optional surrounding quotes stripped).
/// </summary>
internal static class EnvLoader
{
    internal static void ApplyRepoLocalDotEnv(string? currentWorkingDirectory = null)
    {
        var baseDir = currentWorkingDirectory ?? Environment.CurrentDirectory;
        foreach (var path in CandidatePaths(baseDir))
        {
            if (!File.Exists(path))
                continue;
            ApplyFile(path);
            return;
        }
    }

    private static IEnumerable<string> CandidatePaths(string baseDir)
    {
        yield return Path.Combine(baseDir, "tools", "db-migrate", ".env");
        yield return Path.Combine(baseDir, ".env");
    }

    private static void ApplyFile(string path)
    {
        foreach (var raw in File.ReadAllLines(path))
        {
            var line = raw.Trim();
            if (line.Length == 0 || line.StartsWith('#'))
                continue;
            var eq = line.IndexOf('=');
            if (eq <= 0)
                continue;
            var key = line[..eq].Trim();
            if (key.Length == 0)
                continue;
            var val = line[(eq + 1)..].Trim();
            if (val.Length >= 2 && val[0] == '"' && val[^1] == '"')
                val = val[1..^1].Replace("\\\"", "\"", StringComparison.Ordinal);
            if (Environment.GetEnvironmentVariable(key) is null or "" && val.Length > 0)
                Environment.SetEnvironmentVariable(key, val);
        }
    }
}
