using System.Globalization;
using System.Text.Json;
using Takumi.Server.Protocol;

namespace Takumi.Server.Persistence;

/// <summary>
/// M7g: bulk migrate <c>takumi-inventory/*.json</c> → <c>inventory_slot</c>.
/// One file per account; each character lists 12-byte item blobs per slot.
/// </summary>
public static class InventorySlotJsonMigrator
{
    public readonly record struct MigrateSummary(int Accounts, int Characters, int Slots, int SkippedFiles);

    static readonly JsonSerializerOptions JsonOptions = new()
    {
        PropertyNamingPolicy = JsonNamingPolicy.CamelCase,
        PropertyNameCaseInsensitive = true,
    };

    public static bool IsMigrateOnStartupEnabled()
    {
        var raw = Environment.GetEnvironmentVariable("TAKUMI_MIGRATE_INVENTORY_JSON")?.Trim();
        return string.Equals(raw, "1", StringComparison.OrdinalIgnoreCase)
               || string.Equals(raw, "true", StringComparison.OrdinalIgnoreCase);
    }

    public static bool IsMigrateOnlyMode()
    {
        var raw = Environment.GetEnvironmentVariable("TAKUMI_MIGRATE_INVENTORY_JSON_ONLY")?.Trim();
        return string.Equals(raw, "1", StringComparison.OrdinalIgnoreCase)
               || string.Equals(raw, "true", StringComparison.OrdinalIgnoreCase);
    }

    public static string ResolveInventoryDirectory()
    {
        var env = Environment.GetEnvironmentVariable("TAKUMI_INVENTORY_DIR")?.Trim();
        if (!string.IsNullOrEmpty(env))
        {
            return Path.GetFullPath(env);
        }

        return Path.GetFullPath(Path.Combine(Environment.CurrentDirectory, "takumi-inventory"));
    }

    public static async Task<MigrateSummary> MigrateAllJsonFilesAsync(CancellationToken ct = default)
    {
        if (!CharacterRosterBootstrap.IsDbSyncEnabled())
        {
            Console.WriteLine("[inventory-migrate] TAKUMI_ROSTER_DB_SYNC is off — nothing to do");
            return new MigrateSummary(0, 0, 0, 0);
        }

        var repo = TakumiPostgresMirror.InventorySlots;
        if (repo is null)
        {
            Console.WriteLine("[inventory-migrate] Postgres mirror not initialized — set TAKUMI_PG_* / TAKUMI_PG_CONNECTION_STRING");
            return new MigrateSummary(0, 0, 0, 0);
        }

        var dir = ResolveInventoryDirectory();
        if (!Directory.Exists(dir))
        {
            Console.WriteLine("[inventory-migrate] inventory dir missing: {0}", dir);
            return new MigrateSummary(0, 0, 0, 0);
        }

        var accounts = 0;
        var characters = 0;
        var slots = 0;
        var skipped = 0;

        foreach (var path in Directory.EnumerateFiles(dir, "*.json", SearchOption.TopDirectoryOnly).OrderBy(p => p, StringComparer.OrdinalIgnoreCase))
        {
            var accountId = Path.GetFileNameWithoutExtension(path);
            if (string.IsNullOrWhiteSpace(accountId))
            {
                skipped++;
                continue;
            }

            var perChar = TryLoadSlotsFromJsonFile(path);
            if (perChar.Count == 0)
            {
                Console.WriteLine("[inventory-migrate] skip empty {0}", path);
                skipped++;
                continue;
            }

            foreach (var (charName, rows) in perChar)
            {
                await repo.ReplaceCharacterSlotsAsync(accountId, charName, rows, ct).ConfigureAwait(false);
                characters++;
                slots += rows.Count;
            }

            accounts++;
            Console.WriteLine(
                "[inventory-migrate] account={0} characters={1} slots={2} file={3}",
                accountId,
                perChar.Count,
                perChar.Sum(kv => kv.Value.Count),
                path);
        }

        Console.WriteLine(
            "[inventory-migrate] done accounts={0} characters={1} slots={2} skippedFiles={3} dir={4}",
            accounts,
            characters,
            slots,
            skipped,
            dir);
        return new MigrateSummary(accounts, characters, slots, skipped);
    }

    public static IReadOnlyDictionary<string, List<InventorySlotRow>> TryLoadSlotsFromJsonFile(string jsonPath)
    {
        var map = new Dictionary<string, List<InventorySlotRow>>(StringComparer.OrdinalIgnoreCase);
        if (!File.Exists(jsonPath))
        {
            return map;
        }

        try
        {
            var json = File.ReadAllText(jsonPath);
            var root = JsonSerializer.Deserialize<InventoryJsonRoot>(json, JsonOptions);
            if (root?.Characters is null)
            {
                return map;
            }

            foreach (var c in root.Characters)
            {
                if (string.IsNullOrWhiteSpace(c.Name) || c.Slots is null || c.Slots.Count == 0)
                {
                    continue;
                }

                var name = CharacterRosterMerge.NormaliseName(c.Name);
                var rows = new List<InventorySlotRow>();
                foreach (var s in c.Slots)
                {
                    if (!TryParseItem12(s, out var item12))
                    {
                        continue;
                    }

                    rows.Add(new InventorySlotRow { Slot = s.Slot, Item12 = item12 });
                }

                if (rows.Count > 0)
                {
                    map[name] = rows;
                }
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine("[inventory-migrate] parse failed {0}: {1}", jsonPath, ex.Message);
        }

        return map;
    }

    static bool TryParseItem12(InventoryJsonSlot slot, out byte[] item12)
    {
        item12 = Array.Empty<byte>();
        if (!string.IsNullOrWhiteSpace(slot.ItemBase64))
        {
            try
            {
                var decoded = Convert.FromBase64String(slot.ItemBase64.Trim());
                if (decoded.Length == ItemWire602.WireBytes)
                {
                    item12 = decoded;
                    return true;
                }
            }
            catch (FormatException)
            {
            }
        }

        if (!string.IsNullOrWhiteSpace(slot.ItemHex))
        {
            var hex = slot.ItemHex.Trim();
            if (hex.StartsWith("0x", StringComparison.OrdinalIgnoreCase))
            {
                hex = hex[2..];
            }

            if (hex.Length == ItemWire602.WireBytes * 2
                && TryDecodeHex(hex, out item12))
            {
                return true;
            }
        }

        return false;
    }

    static bool TryDecodeHex(string hex, out byte[] bytes)
    {
        bytes = Array.Empty<byte>();
        if (hex.Length % 2 != 0)
        {
            return false;
        }

        var len = hex.Length / 2;
        var buf = new byte[len];
        for (var i = 0; i < len; i++)
        {
            if (!byte.TryParse(hex.AsSpan(i * 2, 2), NumberStyles.HexNumber, CultureInfo.InvariantCulture, out var b))
            {
                return false;
            }

            buf[i] = b;
        }

        bytes = buf;
        return bytes.Length == ItemWire602.WireBytes;
    }

    sealed class InventoryJsonRoot
    {
        public List<InventoryJsonChar>? Characters { get; set; }
    }

    sealed class InventoryJsonChar
    {
        public string Name { get; set; } = "";

        public List<InventoryJsonSlot>? Slots { get; set; }
    }

    sealed class InventoryJsonSlot
    {
        public byte Slot { get; set; }

        public string? ItemHex { get; set; }

        public string? ItemBase64 { get; set; }
    }
}
