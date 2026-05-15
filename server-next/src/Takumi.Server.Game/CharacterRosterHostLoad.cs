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
                roster.Add(ToGameEntry(row));
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
                static (e, d) =>
                {
                    e.MapId = d.MapId;
                    e.PosX = d.PosX;
                    e.PosY = d.PosY;
                    e.Angle = d.Angle;
                    e.Level = d.Level;
                    e.ServerClass = d.ServerClass;
                    e.CurrentHp = d.CurrentHp;
                    e.MaxHp = d.MaxHp;
                    e.CurrentMp = d.CurrentMp;
                    e.MaxMp = d.MaxMp;
                    e.Zen = d.Zen;
                });
            CharacterRosterMirrorHealth.RecordMergeSuccess();
        }
        catch (Exception ex)
        {
            CharacterRosterMirrorHealth.RecordMergeFail();
            Console.WriteLine("[roster-db] merge after login failed for {0}: {1}", accountId, ex.Message);
        }
    }

    public static GameRosterEntry ToGameEntry(CharacterRosterRow row)
    {
        var nm = new byte[10];
        var enc = Encoding.ASCII.GetBytes(row.Name.Trim());
        Buffer.BlockCopy(enc, 0, nm, 0, Math.Min(10, enc.Length));
        var entry = new GameRosterEntry
        {
            Name10 = nm,
            ServerClass = row.ServerClass,
            Level = row.Level,
            MapId = row.MapId,
            PosX = row.PosX,
            PosY = row.PosY,
            Angle = row.Angle,
            CurrentHp = row.CurrentHp,
            MaxHp = row.MaxHp,
            CurrentMp = row.CurrentMp,
            MaxMp = row.MaxMp,
            Zen = row.Zen,
        };
        GameRosterDisk.ApplyLegacySpawnIfUnset(entry);
        return entry;
    }
}
