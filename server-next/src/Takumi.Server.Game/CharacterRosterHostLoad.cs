using System.Text;
using Takumi.Server.Persistence;

namespace Takumi.Server.Game;

/// <summary>M4b: shared login roster load for game TCP hosts.</summary>
public static class CharacterRosterHostLoad
{
    public static async Task LoadOnLoginAsync(
        string accountId,
        List<GameRosterEntry> roster,
        bool rosterDbMergeOverlay,
        CancellationToken ct)
    {
        roster.Clear();
        var dbPrimary = new List<CharacterRosterRow>();
        if (await CharacterRosterBootstrap.TryLoadDbPrimaryAsync(accountId, dbPrimary, ct).ConfigureAwait(false))
        {
            foreach (var row in dbPrimary)
            {
                roster.Add(CharacterRosterEntryMapping.ToGameEntry(row));
            }

            return;
        }

        roster.AddRange(GameRosterDisk.LoadEntries(accountId));
        if (!rosterDbMergeOverlay || TakumiPostgresMirror.CharacterRoster is null)
        {
            return;
        }

        try
        {
            var dbRows = await TakumiPostgresMirror.CharacterRoster.LoadByAccountAsync(accountId, ct).ConfigureAwait(false);
            CharacterRosterBootstrap.ApplyDbOverlayToRows(
                dbRows,
                roster,
                static e => Encoding.ASCII.GetString(e.Name10).TrimEnd('\0', ' '),
                CharacterRosterEntryMapping.ApplyDbOverlay);
            CharacterRosterMirrorHealth.RecordMergeSuccess();
        }
        catch (Exception ex)
        {
            CharacterRosterMirrorHealth.RecordMergeFail();
            Console.WriteLine("[roster-db] merge after login failed for {0}: {1}", accountId, ex.Message);
        }
    }
}
