namespace Takumi.Server.Game.World;

/// <summary>Row from <c>MapManager.txt</c> (parity <c>MAP_MANAGER_INFO</c>).</summary>
public sealed class MapManagerEntry
{
    public byte MapId { get; init; }

    public int GensBattle { get; init; }
}
