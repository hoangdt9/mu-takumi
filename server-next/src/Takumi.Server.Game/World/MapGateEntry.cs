namespace Takumi.Server.Game.World;

/// <summary>One row from <c>Move/Gate.txt</c> (parity <c>GATE_INFO</c> / <c>CGate::Load</c>).</summary>
public sealed class MapGateEntry
{
    public int GateIndex { get; init; }

    public int Flag { get; init; }

    public byte MapId { get; init; }

    public short PosX { get; init; }

    public short PosY { get; init; }

    public short RangeTx { get; init; }

    public short RangeTy { get; init; }

    public int TargetGate { get; init; }

    public short Dir { get; init; }

    public int MinLevel { get; init; } = -1;

    public int MaxLevel { get; init; } = -1;

    public int MinReset { get; init; } = -1;

    public int MaxReset { get; init; } = -1;

    public short AccountLevel { get; init; }
}
