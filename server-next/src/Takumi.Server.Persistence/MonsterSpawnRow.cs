namespace Takumi.Server.Persistence;

/// <summary>One row in <c>monster_spawn</c> (parity <c>MONSTER_SET_BASE_INFO</c> / M8 ETL).</summary>
public sealed class MonsterSpawnRow
{
    public int Id { get; init; }

    public short SpawnType { get; init; }

    public int MonsterClass { get; init; }

    public byte MapId { get; init; }

    public int Dis { get; init; }

    public short PosX { get; init; }

    public short PosY { get; init; }

    public short RangeTx { get; init; }

    public short RangeTy { get; init; }

    public short Dir { get; init; }

    public int SpawnValue { get; init; }
}
