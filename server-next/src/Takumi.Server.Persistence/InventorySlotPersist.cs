using System.Text;

namespace Takumi.Server.Persistence;

/// <summary>Persist bag slots + zen after NPC shop (M9 P4.3).</summary>
public static class InventorySlotPersist
{
    public static bool IsEnabled =>
        TakumiPostgresMirror.InventorySlots is not null &&
        !string.Equals(
            Environment.GetEnvironmentVariable("TAKUMI_ROSTER_DB_SYNC")?.Trim(),
            "0",
            StringComparison.OrdinalIgnoreCase);

    public static async Task SaveSlotsAsync(
        string accountLogin,
        byte[] characterName10,
        IReadOnlyDictionary<byte, byte[]> slots,
        CancellationToken ct = default)
    {
        var repo = TakumiPostgresMirror.InventorySlots;
        if (repo is null || !IsEnabled)
        {
            return;
        }

        var name = NormaliseCharacterName(characterName10);
        var snapshot = new Dictionary<byte, byte[]>(slots.Count);
        foreach (var kv in slots)
        {
            if (kv.Value.Length != 12 || IsEmptyItem12(kv.Value))
            {
                continue;
            }

            snapshot[kv.Key] = kv.Value;
        }

        await repo.ReplaceCharacterSlotsAsync(accountLogin, name, snapshot, ct).ConfigureAwait(false);
    }

    static string NormaliseCharacterName(byte[] characterName10) =>
        CharacterRosterMerge.NormaliseName(
            Encoding.ASCII.GetString(characterName10.AsSpan(0, Math.Min(10, characterName10.Length))));

    static bool IsEmptyItem12(byte[] item12)
    {
        for (var i = 0; i < item12.Length; i++)
        {
            if (item12[i] != 0)
            {
                return false;
            }
        }

        return true;
    }

    public static async Task SaveZenAsync(
        string accountLogin,
        byte[] characterName10,
        long zen,
        CancellationToken ct = default)
    {
        var roster = TakumiPostgresMirror.CharacterRoster;
        if (roster is null || !IsEnabled)
        {
            return;
        }

        var name = NormaliseCharacterName(characterName10);
        await roster.UpdateZenAsync(accountLogin, name, zen, ct).ConfigureAwait(false);
    }
}
