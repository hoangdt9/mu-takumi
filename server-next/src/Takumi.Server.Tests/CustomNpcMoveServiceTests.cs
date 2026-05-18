using Takumi.Server.Game.World;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class CustomNpcMoveServiceTests
{
    [Fact]
    public void TryValidate_blocks_pk_when_pk_move_disabled()
    {
        var move = new CustomNpcMoveEntry
        {
            Index = 0,
            MonsterClass = 602,
            NpcMap = 0,
            NpcX = 130,
            NpcY = 135,
            DestinationMap = 0,
            DestinationX = 140,
            DestinationY = 140,
            PkMove = 0,
        };
        var ctx = new MoveMapPlayerContext
        {
            Level = 50,
            Reset = 0,
            AccountLevel = 0,
            PkLevel = 5,
            GensFamily = 1,
            ShopWarehouseOrTradeOpen = false,
            IsDead = false,
            TeleportInProgress = false,
            PresenceSessionId = Guid.NewGuid(),
        };

        Assert.False(CustomNpcMoveService.TryValidate(move, ctx, masterReset: 0, out var reason));
        Assert.Equal(CustomNpcMoveService.DenyReason.PkMurderer, reason);
    }

    [Fact]
    public void TryResolveDestination_sets_map_changed_when_dest_map_differs()
    {
        var move = new CustomNpcMoveEntry
        {
            Index = 1,
            MonsterClass = 603,
            DestinationMap = 2,
            DestinationX = 100,
            DestinationY = 100,
        };

        Assert.True(CustomNpcMoveService.TryResolveDestination(move, previousMap: 0, out var dest));
        Assert.Equal((byte)2, dest.MapId);
        Assert.True(dest.MapChanged);
    }

    [Fact]
    public void Catalog_lookup_matches_npc_tile()
    {
        CustomNpcMoveCatalog.LoadForTests(
        [
            new CustomNpcMoveEntry
            {
                Index = 2,
                MonsterClass = 602,
                NpcMap = 0,
                NpcX = 130,
                NpcY = 135,
                DestinationMap = 0,
                DestinationX = 140,
                DestinationY = 140,
            },
        ]);

        Assert.True(CustomNpcMoveService.TryMatchNpc(602, 0, 130, 135, out var entry));
        Assert.NotNull(entry);
        Assert.Equal(140, entry.DestinationX);
    }
}
