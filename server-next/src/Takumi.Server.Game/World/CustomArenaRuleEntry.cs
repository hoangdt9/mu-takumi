namespace Takumi.Server.Game.World;

/// <summary>Rule row from <c>Custom/CustomArena.txt</c> section 1 (parity <c>CUSTOM_ARENA_RULE_INFO</c>).</summary>
public sealed class CustomArenaRuleEntry
{
    public int Index { get; init; }

    public int AlarmTime { get; init; }

    public int StandTime { get; init; }

    public int EventTime { get; init; }

    public int CloseTime { get; init; }

    public int StartGate { get; init; }

    public int MinLevel { get; init; } = -1;

    public int MaxLevel { get; init; } = -1;

    public int MinReset { get; init; } = -1;

    public int MaxReset { get; init; } = -1;

    /// <summary>DW, DK, FE, MG, DL, SU, RF — 1 = allowed.</summary>
    public int[] RequireClass { get; init; } = { 1, 1, 1, 1, 1, 1, 1 };
}
