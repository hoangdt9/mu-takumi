namespace Takumi.Server.Game.World;

/// <summary>
/// Footprints from client <c>Data\Local\{locale}\Item_{locale}.bmd</c> (parity <c>OpenItemScript</c> / <c>ITEM_ATTRIBUTE.Width/Height</c>).
/// </summary>
internal static class ClientItemFootprintCatalog
{
    const int ItemAttributeBytes = 84;
    const int MaxItemCount = 8192;
    const int WidthOffset = 38;
    const int HeightOffset = 39;
    static readonly byte[] BuxCode = [0xFC, 0xCF, 0xAB];

    public static Dictionary<int, (int Width, int Height)>? TryLoadFromBmd(string path)
    {
        if (!File.Exists(path))
        {
            return null;
        }

        var fileLen = new FileInfo(path).Length;
        if (fileLen < ItemAttributeBytes * MaxItemCount)
        {
            Console.WriteLine("[m8] ClientItemFootprintCatalog: {0} too small ({1} bytes)", path, fileLen);
            return null;
        }

        var raw = File.ReadAllBytes(path);
        var map = new Dictionary<int, (int Width, int Height)>(MaxItemCount);
        for (var i = 0; i < MaxItemCount; i++)
        {
            var offset = i * ItemAttributeBytes;
            var w = (int)DecodeByte(raw, offset + WidthOffset);
            var h = (int)DecodeByte(raw, offset + HeightOffset);
            if (w < 1 || h < 1)
            {
                continue;
            }

            map[i] = (Math.Min(w, InventoryBagGrid.Columns), Math.Min(h, InventoryBagGrid.Rows));
        }

        return map;
    }

    static byte DecodeByte(byte[] file, int offset)
    {
        var b = file[offset];
        b ^= BuxCode[offset % 3];
        return b;
    }

    public static string? ResolveBmdPath()
    {
        var explicitPath = Environment.GetEnvironmentVariable("TAKUMI_ITEM_BMD_PATH")?.Trim();
        if (!string.IsNullOrEmpty(explicitPath) && File.Exists(explicitPath))
        {
            return Path.GetFullPath(explicitPath);
        }

        var locale = Environment.GetEnvironmentVariable("TAKUMI_ITEM_BMD_LOCALE")?.Trim();
        if (string.IsNullOrEmpty(locale))
        {
            locale = "Eng";
        }

        var roots = new List<string?>();
        var clientRoot = Environment.GetEnvironmentVariable("TAKUMI_CLIENT_DATA_ROOT")?.Trim();
        if (!string.IsNullOrEmpty(clientRoot))
        {
            roots.Add(clientRoot);
        }

        roots.Add("/att-data");
        roots.Add(Path.Combine(Environment.CurrentDirectory, "Data"));
        roots.Add(Path.Combine(Environment.CurrentDirectory, "..", "docker", "data-zip", "host", "Data"));
        roots.Add(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", "..", "..", "..", "docker", "data-zip", "host", "Data"));

        var nameVariants = new[]
        {
            $"Item_{locale}.bmd",
            $"Item_{locale.ToLowerInvariant()}.bmd",
            $"Item_{locale.ToUpperInvariant()}.bmd",
        };

        foreach (var root in roots)
        {
            if (string.IsNullOrEmpty(root))
            {
                continue;
            }

            var localDir = Path.Combine(root, "Local");
            if (!Directory.Exists(localDir))
            {
                continue;
            }

            foreach (var sub in Directory.EnumerateDirectories(localDir))
            {
                var dirName = Path.GetFileName(sub);
                if (!dirName.Equals(locale, StringComparison.OrdinalIgnoreCase))
                {
                    continue;
                }

                foreach (var name in nameVariants)
                {
                    var candidate = Path.Combine(sub, name);
                    if (File.Exists(candidate))
                    {
                        return Path.GetFullPath(candidate);
                    }
                }

                foreach (var file in Directory.EnumerateFiles(sub, "Item*.bmd", SearchOption.TopDirectoryOnly))
                {
                    return Path.GetFullPath(file);
                }
            }
        }

        return null;
    }
}
