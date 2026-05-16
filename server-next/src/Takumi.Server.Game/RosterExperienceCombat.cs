using System.Text;
using Takumi.Server.Persistence;
using Takumi.Server.Protocol;

namespace Takumi.Server.Game;

/// <summary>M7d: grant kill EXP, level-up, mirror vitals + progress to Postgres.</summary>
public static class RosterExperienceCombat
{
    public static int GrantKillExperience(
        GameRosterEntry player,
        int expGain,
        string? accountId,
        Action? onRosterDirty)
    {
        if (expGain <= 0)
        {
            return 0;
        }

        var level = player.Level;
        var experience = player.Experience;
        var levelUpPoint = player.LevelUpPoint;
        var levelsGained = ExperienceProgression602.ApplyKillExperience(
            ref level,
            ref experience,
            ref levelUpPoint,
            player.ServerClass,
            expGain,
            vitals =>
            {
                player.MaxHp = vitals.LifeMax;
                player.CurrentHp = vitals.LifeMax;
                player.MaxMp = vitals.ManaMax;
                player.CurrentMp = vitals.ManaMax;
                player.MaxShield = vitals.ShieldMax;
                player.CurrentShield = vitals.ShieldMax;
                player.MaxBp = vitals.SkillManaMax;
                player.CurrentBp = vitals.SkillMana;
            });

        player.Level = level;
        player.Experience = experience;
        player.LevelUpPoint = levelUpPoint;

        onRosterDirty?.Invoke();

        RosterProgressMirror.ScheduleFromEntry(accountId, player);

        var charName = Encoding.ASCII.GetString(player.Name10).TrimEnd('\0', ' ');
        if (levelsGained > 0)
        {
            Console.WriteLine(
                "[m7-exp] level up name={0} lv={1} exp={2} statPts={3} (+{4} levels)",
                charName,
                player.Level,
                player.Experience,
                player.LevelUpPoint,
                levelsGained);
        }
        else
        {
            Console.WriteLine(
                "[m7-exp] kill grant name={0} +{1} exp lv={2} totalExp={3}",
                charName,
                expGain,
                player.Level,
                player.Experience);
        }

        return levelsGained;
    }
}
