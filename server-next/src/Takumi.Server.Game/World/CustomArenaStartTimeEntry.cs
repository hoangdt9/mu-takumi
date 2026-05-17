namespace Takumi.Server.Game.World;

/// <summary>Schedule row from <c>Custom/CustomArena.txt</c> section 0.</summary>
public sealed class CustomArenaStartTimeEntry
{
    public int ArenaIndex { get; init; }

    public int Year { get; init; } = -1;

    public int Month { get; init; } = -1;

    public int Day { get; init; } = -1;

    public int DayOfWeek { get; init; } = -1;

    public int Hour { get; init; } = -1;

    public int Minute { get; init; } = -1;

    public int Second { get; init; } = -1;
}
