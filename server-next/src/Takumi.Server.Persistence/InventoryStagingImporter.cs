using Npgsql;
using Takumi.Server.Protocol;

namespace Takumi.Server.Persistence;

/// <summary>M7: <c>inventory_staging</c> → <c>inventory_slot</c> (Postgres SSOT, no JSON).</summary>
public static class InventoryStagingImporter
{
    public readonly record struct ImportSummary(int Characters, int Slots);

    public static bool IsEnabled()
    {
        var raw = Environment.GetEnvironmentVariable("TAKUMI_IMPORT_INVENTORY_STAGING")?.Trim();
        return string.Equals(raw, "1", StringComparison.OrdinalIgnoreCase)
               || string.Equals(raw, "true", StringComparison.OrdinalIgnoreCase);
    }

    public static async Task<ImportSummary> TryImportAsync(CancellationToken ct = default)
    {
        if (!IsEnabled() || !CharacterRosterBootstrap.IsDbSyncEnabled())
        {
            return new ImportSummary(0, 0);
        }

        var invRepo = TakumiPostgresMirror.InventorySlots;
        if (invRepo is null)
        {
            Console.WriteLine("[inventory-staging] Postgres inventory mirror not initialized");
            return new ImportSummary(0, 0);
        }

        var cs = PostgresCharacterRosterRepository.BuildConnectionStringFromEnv();
        if (string.IsNullOrEmpty(cs))
        {
            return new ImportSummary(0, 0);
        }

        try
        {
            await using var conn = new NpgsqlConnection(cs);
            await conn.OpenAsync(ct).ConfigureAwait(false);
            if (!await TableExistsAsync(conn, "inventory_staging", ct).ConfigureAwait(false))
            {
                return new ImportSummary(0, 0);
            }

            var byCharacter = new Dictionary<string, List<InventorySlotRow>>(StringComparer.OrdinalIgnoreCase);
            await using var sel = new NpgsqlCommand(
                """
                SELECT account_login, character_name, slot_idx, item_index, item_level, durability,
                       skill, luck, item_option, excellent
                FROM inventory_staging
                ORDER BY account_login, character_name, slot_idx
                """,
                conn);
            await using var reader = await sel.ExecuteReaderAsync(ct).ConfigureAwait(false);
            while (await reader.ReadAsync(ct).ConfigureAwait(false))
            {
                var account = reader.GetString(0);
                var charName = CharacterRosterMerge.NormaliseName(reader.GetString(1));
                var slot = (byte)reader.GetInt16(2);
                var itemIndex = reader.GetInt32(3);
                if (itemIndex <= 0)
                {
                    continue;
                }

                var blob = EncodeItem(
                    itemIndex,
                    reader.GetInt16(4),
                    reader.GetInt16(5),
                    reader.GetBoolean(6),
                    reader.GetBoolean(7),
                    reader.GetInt16(8),
                    reader.GetInt16(9));
                if (ItemWire602.IsEmpty(blob))
                {
                    continue;
                }

                var key = $"{account}\0{charName}";
                if (!byCharacter.TryGetValue(key, out var list))
                {
                    list = new List<InventorySlotRow>();
                    byCharacter[key] = list;
                }

                list.Add(new InventorySlotRow { Slot = slot, Item12 = blob });
            }

            if (byCharacter.Count == 0)
            {
                Console.WriteLine("[inventory-staging] table empty — skip import");
                return new ImportSummary(0, 0);
            }

            var slotTotal = 0;
            foreach (var (key, rows) in byCharacter)
            {
                var sep = key.IndexOf('\0');
                var account = key[..sep];
                var charName = key[(sep + 1)..];
                await invRepo.ReplaceCharacterSlotsAsync(account, charName, rows, ct).ConfigureAwait(false);
                slotTotal += rows.Count;
                Console.WriteLine(
                    "[inventory-staging] {0}/{1} slots={2}",
                    account,
                    charName,
                    rows.Count);
            }

            Console.WriteLine(
                "[inventory-staging] imported characters={0} slots={1}",
                byCharacter.Count,
                slotTotal);
            return new ImportSummary(byCharacter.Count, slotTotal);
        }
        catch (Exception ex)
        {
            Console.WriteLine("[inventory-staging] import failed: {0}", ex.Message);
            return new ImportSummary(0, 0);
        }
    }

    internal static byte[] EncodeItem(
        int flatIndex,
        int level,
        int durability,
        bool skill,
        bool luck,
        int option,
        int excellent)
    {
        var group = flatIndex / 512;
        var index = flatIndex % 512;
        var blob = new byte[ItemWire602.WireBytes];
        ItemWire602.WriteSeason6Item(
            blob,
            group,
            index,
            level,
            durability,
            skill,
            luck,
            option,
            excellent);
        return blob;
    }

    static async Task<bool> TableExistsAsync(NpgsqlConnection conn, string table, CancellationToken ct)
    {
        await using var check = new NpgsqlCommand(
            """
            SELECT EXISTS (
                SELECT 1 FROM information_schema.tables
                WHERE table_schema = 'public' AND table_name = $1)
            """,
            conn);
        check.Parameters.Add(new NpgsqlParameter("t", NpgsqlTypes.NpgsqlDbType.Text) { Value = table });
        return (bool)(await check.ExecuteScalarAsync(ct).ConfigureAwait(false) ?? false);
    }
}
