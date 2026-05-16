using System.Text;
using Takumi.Server.Persistence;

namespace Takumi.Server.Game;

/// <summary>M7: mirror level/EXP/vitals/stats to Postgres after gameplay mutations.</summary>
public static class RosterProgressMirror
{
    public static void ScheduleFromEntry(string? accountId, GameRosterEntry player)
    {
        if (string.IsNullOrEmpty(accountId))
        {
            return;
        }

        var charName = Encoding.ASCII.GetString(player.Name10).TrimEnd('\0', ' ');
        if (string.IsNullOrEmpty(charName))
        {
            return;
        }

        CharacterRosterMirrorWriter.ScheduleProgressUpsert(
            accountId,
            charName,
            player.Level,
            player.Experience,
            player.LevelUpPoint,
            player.CurrentHp,
            player.MaxHp,
            player.CurrentMp,
            player.MaxMp,
            player.CurrentShield,
            player.MaxShield,
            player.Strength,
            player.Dexterity,
            player.Vitality,
            player.Energy,
            player.Leadership,
            player.CurrentBp,
            player.MaxBp);
    }
}
