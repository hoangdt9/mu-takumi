using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class WalkPackets602Tests
{
    [Fact]
    public void TryFindInstantMove_detects_C1_05_15()
    {
        ReadOnlySpan<byte> p = stackalloc byte[] { 0xC1, 0x05, 0x15, 0x2A, 0x3B };
        Assert.True(ClientWalkPackets602.TryFindInstantMove(p, out var off, out var x, out var y));
        Assert.Equal(0, off);
        Assert.Equal(0x2A, x);
        Assert.Equal(0x3B, y);
    }

    [Fact]
    public void TryFindWalkEndTile_one_step_west_from_10_10()
    {
        // stepCount=1, rot nibble 0 -> meta 0x01; first dir nibble 0 = West; payload 1 byte (high nibble used).
        ReadOnlySpan<byte> p = stackalloc byte[] { 0xC1, 0x07, 0xD4, 10, 10, 0x01, 0x00 };
        Assert.True(ClientWalkPackets602.TryFindWalkEndTile(p, out _, out var ex, out var ey, out var ang, out var moved));
        Assert.True(moved);
        Assert.Equal(9, ex);
        Assert.Equal(9, ey);
        Assert.Equal(1, ang);
    }

    [Fact]
    public void TryFindWalkEndTile_one_step_east_from_10_10()
    {
        // East dir nibble = 4 (OpenMU packet byte); packed as high nibble of payload[0].
        ReadOnlySpan<byte> p = stackalloc byte[] { 0xC1, 0x07, 0xD4, 10, 10, 0x01, 0x40 };
        Assert.True(ClientWalkPackets602.TryFindWalkEndTile(p, out _, out var ex, out var ey, out _, out var moved));
        Assert.True(moved);
        Assert.Equal(11, ex);
        Assert.Equal(11, ey);
    }

    [Fact]
    public void TryFindWalkEndTile_zero_steps_rotation_only()
    {
        ReadOnlySpan<byte> p = stackalloc byte[] { 0xC1, 0x06, 0xD4, 20, 30, 0x30 };
        Assert.True(ClientWalkPackets602.TryFindWalkEndTile(p, out _, out var ex, out var ey, out var ang, out var moved));
        Assert.False(moved);
        Assert.Equal(20, ex);
        Assert.Equal(30, ey);
        Assert.Equal(4, ang);
    }

    [Fact]
    public void TryFindWalkEndTile_accepts_075_opcode_0x10()
    {
        ReadOnlySpan<byte> p = stackalloc byte[] { 0xC1, 0x07, 0x10, 10, 10, 0x01, 0x00 };
        Assert.True(ClientWalkPackets602.TryFindWalkEndTile(p, out _, out var ex, out var ey, out _, out var moved));
        Assert.True(moved);
        Assert.Equal(9, ex);
        Assert.Equal(9, ey);
    }
}
