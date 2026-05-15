namespace Takumi.Server.Game.World;

public sealed class MapMonsterInstance
{
    public required int ObjectKey { get; init; }
    public required int MonsterClass { get; init; }
    public required byte Map { get; init; }
    public required byte X { get; init; }
    public required byte Y { get; init; }
    public required byte Dir { get; init; }
    public required int Life { get; init; }
    public required int Level { get; init; }
}
