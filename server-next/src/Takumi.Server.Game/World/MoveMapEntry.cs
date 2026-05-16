namespace Takumi.Server.Game.World;

/// <summary>Row from <c>Move/Move.txt</c> (parity <c>MOVE_INFO</c> / <c>CMove::GetInfo</c>).</summary>
public sealed class MoveMapEntry
{
    public int Index { get; init; }

    public int Money { get; init; }

    public int MinLevel { get; init; }

    public int MaxLevel { get; init; }

    public int MinReset { get; init; }

    public int MaxReset { get; init; }

    public int AccountLevel { get; init; }

    public int Gate { get; init; }
}
