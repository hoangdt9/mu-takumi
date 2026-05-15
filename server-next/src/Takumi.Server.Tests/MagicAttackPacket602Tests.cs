using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class MagicAttackPacket602Tests
{
    [Fact]
    public void Magic_attack_0xDB_parses_targets()
    {
        var pkt = new byte[]
        {
            0xC1, 0x0F, 0xDB, 0x00, 0x0A, 120, 130, 0x01, 0x02,
            0x2E, 0xE1, 0x00,
            0x2F, 0xE2, 0x00,
        };
        Assert.True(ClientHitPackets602.TryFindMagicAttack(pkt, out _, out var skill, out var x, out var y, out var ids));
        Assert.Equal(10, skill);
        Assert.Equal(120, x);
        Assert.Equal(130, y);
        Assert.Equal(2, ids.Count);
        Assert.Equal(12001, ids[0]);
    }
}
