using MUnique.OpenMU.Network.SimpleModulus;

namespace Takumi.Server.Game;

/// <summary>Load server-side SimpleModulus decrypt keys from Takumi <c>Dec2.dat</c> (same search rules as legacy hosts).</summary>
public static class Dec2ServerDecryptKeysLoader
{
    /// <summary>Returns OpenMU default keys when no Dec2 is found (may not match a real client).</summary>
    public static (SimpleModulusKeys Keys, string SourceTag) Load()
    {
        var serializer = new SimpleModulusKeySerializer();
        var candidates = new List<string>();

        var envPath = Environment.GetEnvironmentVariable("TAKUMI_DEC2_PATH");
        if (!string.IsNullOrWhiteSpace(envPath))
        {
            var trimmed = envPath.Trim();
            candidates.Add(trimmed);
            if (!File.Exists(trimmed))
            {
                Console.Error.WriteLine(
                    "[keys] ERROR: TAKUMI_DEC2_PATH is set but file does not exist:\n  {0}\n" +
                    "Use a real path (copy from phone: adb pull …/files/Data/Dec2.dat). Do not use placeholder text.",
                    Path.GetFullPath(trimmed));
            }
        }

        candidates.Add(Path.Combine(AppContext.BaseDirectory, "Data", "Dec2.dat"));
        candidates.Add(Path.Combine(Environment.CurrentDirectory, "Data", "Dec2.dat"));
        candidates.Add(Path.Combine(Environment.CurrentDirectory, "Dec2.dat"));

        foreach (var dir in new[] { AppContext.BaseDirectory, Environment.CurrentDirectory })
        {
            try
            {
                if (!string.IsNullOrEmpty(dir))
                {
                    candidates.AddRange(WalkParentsForDataDec2(dir));
                }
            }
            catch
            {
                // ignore invalid paths
            }
        }

        foreach (var path in candidates.Distinct(StringComparer.OrdinalIgnoreCase))
        {
            try
            {
                if (string.IsNullOrWhiteSpace(path) || !File.Exists(path))
                {
                    continue;
                }

                if (!serializer.TryDeserialize(path, out var modulusKey, out var cryptKey, out var xorKey))
                {
                    Console.WriteLine("[keys] TryDeserialize failed for: {0}", path);
                    continue;
                }

                if (modulusKey.Length != 4 || cryptKey.Length != 4 || xorKey.Length != 4)
                {
                    Console.WriteLine(
                        "[keys] Unexpected lengths in {0}: mod={1} key={2} xor={3}",
                        path,
                        modulusKey.Length,
                        cryptKey.Length,
                        xorKey.Length);
                    continue;
                }

                var combined = new uint[12];
                Array.Copy(modulusKey, 0, combined, 0, 4);
                Array.Copy(cryptKey, 0, combined, 4, 4);
                Array.Copy(xorKey, 0, combined, 8, 4);

                var keys = SimpleModulusKeys.CreateDecryptionKeys(combined);
                Console.WriteLine("[keys] Loaded Dec2.dat (server decrypt): {0}", path);
                return (keys, $"Dec2 ({path})");
            }
            catch (Exception ex)
            {
                Console.WriteLine("[keys] Error reading {0}: {1}", path, ex.Message);
            }
        }

        Console.WriteLine(
            "[keys] WARNING: Dec2.dat not loaded — using OpenMU default server keys. " +
            "If login never completes, copy the client Data/Dec2.dat beside the exe or set TAKUMI_DEC2_PATH.");
        return (PipelinedSimpleModulusDecryptor.DefaultServerKey, "OpenMU default (may NOT match your client)");
    }

    /// <summary>Looks for <c>Data/Dec2.dat</c> walking up from <paramref name="startDir"/>.</summary>
    public static IEnumerable<string> WalkParentsForDataDec2(string startDir)
    {
        var dir = Path.GetFullPath(startDir);
        for (var depth = 0; depth < 18 && !string.IsNullOrEmpty(dir); depth++)
        {
            yield return Path.Combine(dir, "Data", "Dec2.dat");
            dir = Directory.GetParent(dir)?.FullName ?? string.Empty;
        }
    }
}
