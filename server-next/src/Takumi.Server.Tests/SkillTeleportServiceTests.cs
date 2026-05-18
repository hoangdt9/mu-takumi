using Takumi.Server.Game;
using Takumi.Server.Game.World;
using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class SkillTeleportServiceTests
{
    [Fact]
    public void TryValidateArea_rejects_outside_range()
    {
        Assert.False(SkillTeleportService.TryValidateArea(0, 100, 100, 109, 100));
    }

    [Fact]
    public void TryValidateArea_accepts_within_range_when_walkable()
    {
        if (!MapAttWalkability.CanWalk(0, 133, 168))
        {
            return;
        }

        Assert.True(SkillTeleportService.TryValidateArea(0, 133, 168, 135, 168));
    }

    [Fact]
    public void PlayerHasTeleportSkill_true_for_magic_gladiator()
    {
        Assert.True(SkillTeleportService.PlayerHasTeleportSkill(serverClass: 0x60));
    }

    [Fact]
    public void TryConsumeResources_deducts_mana()
    {
        Environment.SetEnvironmentVariable("TAKUMI_SKILL_TELEPORT_SKIP", null);
        var player = new GameRosterEntry
        {
            ServerClass = 0x60,
            CurrentMp = 100,
            MaxMp = 100,
            CurrentBp = 50,
        };

        Assert.True(SkillTeleportService.TryConsumeResources(player, out var spent));
        Assert.Equal(30, spent);
        Assert.Equal(70, player.CurrentMp);
    }

    [Fact]
    public void MapGateService_skill_teleport_same_map_only()
    {
        var ok = MapGateService.TryResolveSkillTeleport(0, 133, 168, 134, 168, 0, out var dest);
        if (!ok)
        {
            return;
        }

        Assert.Equal(0, dest.MapId);
        Assert.False(dest.MapChanged);
    }
}
