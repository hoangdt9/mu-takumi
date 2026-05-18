using Takumi.Server.Game.World;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class NpcQuestCatalogTests
{
    [Fact]
    public void BuildQuestInfoMask_marks_active_slot()
    {
        var mask = NpcQuestCatalog.BuildQuestInfoMask(3);
        Assert.Equal((byte)0, mask[3]);
        Assert.Equal((byte)0xFF, mask[0]);
        Assert.Equal((byte)0xFF, mask[4]);
    }

    [Fact]
    public void DefaultQuestIndex_maps_season_npc_classes()
    {
        Assert.Equal(0, NpcQuestCatalog.DefaultQuestIndexForClass(229));
        Assert.Equal(1, NpcQuestCatalog.DefaultQuestIndexForClass(569));
    }
}
