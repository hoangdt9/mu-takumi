namespace Takumi.Server.Game.World;

/// <summary>One row from <c>MonsterSetBase.txt</c> (parity <c>MONSTER_SET_BASE_INFO</c>).</summary>
public sealed class MonsterSetBaseEntry
{
    public int SpawnType { get; init; }
    public int MonsterClass { get; init; }
    public byte Map { get; init; }
    public int Dis { get; init; }
    public int X { get; init; }
    public int Y { get; init; }
    public int Tx { get; init; }
    public int Ty { get; init; }
    public byte Dir { get; init; }
    public int Value { get; init; }
}
