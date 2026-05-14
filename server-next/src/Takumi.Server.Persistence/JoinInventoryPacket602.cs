using System.Text;
using Npgsql;
using Takumi.Server.Protocol;

namespace Takumi.Server.Persistence;

/// <summary>Build Season 6 <c>F3 10</c> (<see cref="InventoryListWire602"/>) from <c>inventory_slot</c> or an empty list.</summary>
public static class JoinInventoryPacket602
{
    /// <summary>
    /// Loads slots from Postgres when <paramref name="inventoryRepo"/> and <paramref name="accountLogin"/> are set;
    /// otherwise returns <see cref="InventoryListWire602.BuildEmpty"/>. Resilient to missing table or DB errors (logs to stderr).
    /// </summary>
    public static async Task<byte[]> BuildAsync(
        PostgresInventorySlotRepository? inventoryRepo,
        string? accountLogin,
        byte[]? characterName10,
        CancellationToken ct)
    {
        if (inventoryRepo is null || string.IsNullOrEmpty(accountLogin))
        {
            return InventoryListWire602.BuildEmpty();
        }

        if (characterName10 is null || characterName10.Length < 10)
        {
            return InventoryListWire602.BuildEmpty();
        }

        var charName = CharacterRosterMerge.NormaliseName(Encoding.ASCII.GetString(characterName10.AsSpan(0, 10)));
        if (charName.Length == 0)
        {
            return InventoryListWire602.BuildEmpty();
        }

        try
        {
            var rows = await inventoryRepo.LoadByCharacterAsync(accountLogin, charName, ct).ConfigureAwait(false);
            if (rows.Count == 0)
            {
                return InventoryListWire602.BuildEmpty();
            }

            var buf = new byte[rows.Count * 13];
            var o = 0;
            foreach (var r in rows)
            {
                buf[o++] = r.Slot;
                r.Item12.AsSpan(0, InventoryListWire602.ItemWireBytes).CopyTo(buf.AsSpan(o, InventoryListWire602.ItemWireBytes));
                o += InventoryListWire602.ItemWireBytes;
            }

            return InventoryListWire602.Build(buf);
        }
        catch (PostgresException ex) when (ex.SqlState == "42P01")
        {
            Console.Error.WriteLine(
                "[inventory-db] table inventory_slot missing — apply sql/init/002_inventory_slot.sql ({0})",
                ex.Message);
            return InventoryListWire602.BuildEmpty();
        }
        catch (Exception ex)
        {
            Console.Error.WriteLine("[inventory-db] load failed for account={0} char={1}: {2}", accountLogin, charName, ex.Message);
            return InventoryListWire602.BuildEmpty();
        }
    }
}
