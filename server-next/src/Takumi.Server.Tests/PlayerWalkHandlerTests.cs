using Takumi.Server.Game;
using Takumi.Server.Game.World;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class PlayerWalkHandlerTests
{
    [Fact]
    public void HealSpawnTile_leaves_walkable_unchanged()
    {
        var entry = new GameRosterEntry
        {
            MapId = 0,
            PosX = 135,
            PosY = 122,
            Name10 = "test"u8.ToArray(),
        };

        PlayerWalkHandler.HealSpawnTile(entry);
        Assert.Equal(135, entry.PosX);
        Assert.Equal(122, entry.PosY);
    }
}
