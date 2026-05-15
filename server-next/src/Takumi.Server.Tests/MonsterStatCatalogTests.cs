using Takumi.Server.Game.World;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class MonsterStatCatalogTests
{
    [Fact]
    public void TryParseRow_handles_quoted_name_with_space()
    {
        var parts = new[]
        {
            "0", "1", "\"Bull", "Fighter\"", "119", "100", "0", "16", "20", "6",
        };

        Assert.True(MonsterStatCatalog.TryParseRow(parts, out var stat));
        Assert.Equal(0, stat.Index);
        Assert.Equal(119, stat.Level);
        Assert.Equal(100, stat.Life);
    }

    [Fact]
    public void LoadFromFile_parses_real_Monster_txt_when_present()
    {
        var path = "/Users/admin/Project/GitHub/mu-takumi/MuServer/4.GameServer/Data/Monster/Monster.txt";
        if (!File.Exists(path))
        {
            return;
        }

        var cat = MonsterStatCatalog.LoadFromFile(path);
        var stat = cat.GetOrDefault(0);
        Assert.Equal(119, stat.Level);
        Assert.Equal(100, stat.Life);
    }
}
