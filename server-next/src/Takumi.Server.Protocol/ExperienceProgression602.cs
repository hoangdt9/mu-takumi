namespace Takumi.Server.Protocol;

/// <summary>Server-side EXP / level-up (parity client <c>ReceiveDieExp</c> + <c>TakumiOnHeroLevelGained</c>).</summary>
public static class ExperienceProgression602
{
    public const int MaxLevel = 400;

    /// <summary>Add kill EXP; returns number of levels gained.</summary>
    public static int ApplyKillExperience(
        ref ushort level,
        ref uint experience,
        ref ushort levelUpPoint,
        byte serverClass,
        int expGain,
        Action<CharacterComputedVitals>? onLevelUpVitals = null)
    {
        if (expGain <= 0 || level >= MaxLevel)
        {
            return 0;
        }

        experience = (uint)Math.Min((ulong)experience + (ulong)expGain, uint.MaxValue);
        var levelsGained = 0;
        while (level < MaxLevel)
        {
            var threshold = ExperienceFormula602.CumulativeForLevel(level);
            if (experience < threshold)
            {
                break;
            }

            level++;
            levelsGained++;
            var addPts = CharacterSheetCalculator.LevelUpPointsPerLevel(serverClass);
            levelUpPoint = (ushort)Math.Min(levelUpPoint + addPts, ushort.MaxValue);

            var sheet = CharacterSheetCalculator.DefaultSheet(serverClass, level);
            var vitals = CharacterSheetCalculator.ComputeMaxVitals(serverClass, level, sheet);
            onLevelUpVitals?.Invoke(vitals);
        }

        return levelsGained;
    }
}
