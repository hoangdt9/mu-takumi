using Takumi.Server.Protocol;

namespace Takumi.Server.Persistence;

/// <summary>Build <c>C1 F3 00</c> with per-character equipment preview from <c>inventory_slot</c>.</summary>
public static class CharacterListPacket602
{
    public static async Task<byte[]> BuildAsync(
        string? accountLogin,
        IReadOnlyList<CharacterRosterWire> roster,
        CancellationToken ct = default)
    {
        if (roster.Count == 0)
        {
            return CharacterListWire602.BuildEmpty();
        }

        var previews = new byte[roster.Count][];
        var repo = TakumiPostgresMirror.InventorySlots;
        for (var i = 0; i < roster.Count; i++)
        {
            previews[i] = await BuildPreviewForCharacterAsync(repo, accountLogin, roster[i].Name10, ct).ConfigureAwait(false);
        }

        return CharacterListWire602.Build(roster, previews);
    }

    static async Task<byte[]> BuildPreviewForCharacterAsync(
        PostgresInventorySlotRepository? repo,
        string? accountLogin,
        byte[] name10,
        CancellationToken ct)
    {
        if (repo is null || string.IsNullOrEmpty(accountLogin))
        {
            return AllFfPreview();
        }

        var rows = await JoinInventoryPacket602.LoadRowsAsync(repo, accountLogin, name10, ct).ConfigureAwait(false);
        if (rows.Count == 0)
        {
            return AllFfPreview();
        }

        var wear = new byte[]?[12];
        foreach (var row in rows)
        {
            if (row.Slot <= ItemWire602.LastWearSlot)
            {
                wear[row.Slot] = row.Item12;
            }
        }

        return CharacterListEquipPreview602.BuildFromWearItems(wear);
    }

    static byte[] AllFfPreview()
    {
        var p = new byte[CharacterListEquipPreview602.PreviewLength];
        for (var i = 0; i < p.Length; i++)
        {
            p[i] = 0xFF;
        }

        return p;
    }
}
