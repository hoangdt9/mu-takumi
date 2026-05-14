using Takumi.Server.Persistence;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class CharacterRosterMergeTests
{
    [Fact]
    public void ApplyDbOverlay_updates_matching_name()
    {
        var roster = new List<CharacterRosterRow>
        {
            new()
            {
                Name = "Hero",
                ServerClass = 0x20,
                Level = 1,
                MapId = 0,
                PosX = 135,
                PosY = 122,
                Angle = 1,
            },
        };

        var db = new List<CharacterRosterRow>
        {
            new()
            {
                Name = "Hero",
                ServerClass = 0x20,
                Level = 50,
                MapId = 2,
                PosX = 10,
                PosY = 20,
                Angle = 3,
            },
        };

        CharacterRosterMerge.ApplyDbOverlay(roster, db, static x => x.Name, static (x, d) =>
        {
            x.MapId = d.MapId;
            x.PosX = d.PosX;
            x.PosY = d.PosY;
            x.Angle = d.Angle;
            x.Level = d.Level;
            x.ServerClass = d.ServerClass;
        });

        Assert.Equal(2, roster[0].MapId);
        Assert.Equal(10, roster[0].PosX);
        Assert.Equal(20, roster[0].PosY);
        Assert.Equal(3, roster[0].Angle);
        Assert.Equal((ushort)50, roster[0].Level);
    }
}
