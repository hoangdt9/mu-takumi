using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class C1DeclaredLength602Tests
{
    [Fact]
    public void TryGetDeclaredTotalLength_valid_C1()
    {
        ReadOnlySpan<byte> p = stackalloc byte[] { 0xC1, 0x0A, 0xF1, 0x01, 0, 0, 0, 0, 0, 0 };
        Assert.True(C1DeclaredLength602.TryGetDeclaredTotalLength(p, 0, out var len));
        Assert.Equal(10, len);
        Assert.True(C1DeclaredLength602.IsDeclaredLengthWithinBuffer(p, 0, out _));
    }

    [Fact]
    public void IsDeclaredLengthWithinBuffer_false_when_length_exceeds_buffer()
    {
        ReadOnlySpan<byte> p = stackalloc byte[] { 0xC1, 0xFF, 0x01 };
        Assert.True(C1DeclaredLength602.TryGetDeclaredTotalLength(p, 0, out var len));
        Assert.Equal(0xFF, len);
        Assert.False(C1DeclaredLength602.IsDeclaredLengthWithinBuffer(p, 0, out _));
    }

    [Fact]
    public void TryGetDeclaredTotalLength_rejects_short_buffer()
    {
        ReadOnlySpan<byte> p = stackalloc byte[] { 0xC1 };
        Assert.False(C1DeclaredLength602.TryGetDeclaredTotalLength(p, 0, out _));
    }

    [Fact]
    public void TryGetDeclaredTotalLength_rejects_non_C1()
    {
        ReadOnlySpan<byte> p = stackalloc byte[] { 0xC3, 0x05, 0x01 };
        Assert.False(C1DeclaredLength602.TryGetDeclaredTotalLength(p, 0, out _));
    }
}
