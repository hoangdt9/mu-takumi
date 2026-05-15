namespace Takumi.Server.Persistence;

public sealed class MapGateRow
{
    public int Id { get; init; }

    public int GateIndex { get; init; }

    public short Flag { get; init; }

    public byte MapId { get; init; }

    public short PosX { get; init; }

    public short PosY { get; init; }

    public short RangeTx { get; init; }

    public short RangeTy { get; init; }

    public int TargetGate { get; init; }

    public short Dir { get; init; }

    public int MinLevel { get; init; }

    public int MaxLevel { get; init; }

    public int MinReset { get; init; }

    public int MaxReset { get; init; }

    public short AccountLevel { get; init; }
}
