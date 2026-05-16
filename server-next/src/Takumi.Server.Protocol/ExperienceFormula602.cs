namespace Takumi.Server.Protocol;

/// <summary>Season 6 cumulative experience thresholds (parity Takumi <c>CalculateNextExperince</c> / EXP bar).</summary>
public static class ExperienceFormula602
{
    /// <summary>Cumulative EXP required to advance past <paramref name="level"/> (client <c>NextExperince</c>).</summary>
    public static uint CumulativeForLevel(int level)
    {
        if (level < 1)
        {
            level = 1;
        }

        ulong v = (ulong)(9 + level) * (ulong)level * (ulong)level * 10;
        if (level > 255)
        {
            var over = level - 255;
            v += (ulong)(9 + over) * (ulong)over * (ulong)over * 1000;
        }

        return (uint)Math.Min(v, uint.MaxValue);
    }
}
