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

        var name = CharacterRosterMerge.NormaliseName(
            System.Text.Encoding.ASCII.GetString(characterName10).TrimEnd('\0'));
        await repo.ReplaceCharacterSlotsAsync(accountLogin, name, slots, ct).ConfigureAwait(false);
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

        var name = CharacterRosterMerge.NormaliseName(
            System.Text.Encoding.ASCII.GetString(characterName10).TrimEnd('\0'));
        await roster.UpdateZenAsync(accountLogin, name, zen, ct).ConfigureAwait(false);
    }
}
