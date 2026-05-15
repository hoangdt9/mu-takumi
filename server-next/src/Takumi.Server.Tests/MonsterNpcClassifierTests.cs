using Takumi.Server.Game.World;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class MonsterNpcClassifierTests
{
    [Theory]
    [InlineData(226, true)]
    [InlineData(257, true)]
    [InlineData(479, true)]
    [InlineData(3, false)]
    [InlineData(0, false)]
    public void IsNpc_matches_season6_shop_and_mob_classes(int monsterClass, bool expected) =>
        Assert.Equal(expected, MonsterNpcClassifier.IsNpc(monsterClass));
}
