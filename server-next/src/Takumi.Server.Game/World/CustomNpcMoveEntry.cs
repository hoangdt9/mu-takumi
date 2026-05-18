namespace Takumi.Server.Game.World;

/// <summary>One row from <c>Custom/CustomNpcMove.txt</c> (parity <c>NPC_MOVE_INFO</c>).</summary>
public sealed class CustomNpcMoveEntry
{
    public int Index { get; init; }

    public int MonsterClass { get; init; }

    public byte NpcMap { get; init; }

    public byte NpcX { get; init; }

    public byte NpcY { get; init; }

    public byte DestinationMap { get; init; }

    public byte DestinationX { get; init; }

    public byte DestinationY { get; init; }

    public int MinLevel { get; init; } = -1;

    public int MaxLevel { get; init; } = -1;

    public int MinReset { get; init; } = -1;

    public int MaxReset { get; init; } = -1;

    public int MinMasterReset { get; init; } = -1;

    public int MaxMasterReset { get; init; } = -1;

    public int AccountLevel { get; init; }

    /// <summary>1 = PK murderer may use; 0 = block PK ≥ 5.</summary>
    public int PkMove { get; init; } = 1;
}
