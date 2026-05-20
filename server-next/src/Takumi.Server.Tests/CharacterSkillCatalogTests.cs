using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class CharacterSkillCatalogTests
{
    [Fact]
    public void Elf_defaults_include_poison()
    {
        var skills = CharacterSkillCatalog.GetDefaultEntries(64);
        Assert.Contains(skills, s => s.Type == 1);
    }

    [Fact]
    public void BuildForServerClass_elf_sends_skills()
    {
        var pkt = MagicListWire602.BuildForServerClass(64);
        Assert.True(pkt.Length > 7);
        Assert.Equal(0x11, pkt[3]);
    }

    /// <summary>Mỗi class cơ sở (wire = index×32) có bộ skill seed join &gt; 0.</summary>
    [Theory]
    [InlineData(0)]
    [InlineData(32)]
    [InlineData(64)]
    [InlineData(96)]
    [InlineData(128)]
    [InlineData(160)]
    [InlineData(192)]
    public void BuildForServerClass_each_core_class_sends_non_empty_skill_count(byte serverClass)
    {
        var pkt = MagicListWire602.BuildForServerClass(serverClass, maxLevel: 1);
        Assert.Equal(0xC1, pkt[0]);
        Assert.Equal(0xF3, pkt[2]);
        Assert.Equal(0x11, pkt[3]);
        Assert.True(pkt[4] > 0, $"join skill count for class wire {serverClass}");
        Assert.Equal(MagicListWire602.ListTypeNormal, pkt[5]);
    }

    [Fact]
    public void BuildFromRows_round_trip()
    {
        IReadOnlyList<MagicListWire602.CharacterSkillRowLike> rows =
        [
            new Row(1, 1, 20),
            new Row(52, 52, 15),
        ];
        var pkt = MagicListWire602.BuildFromRows(rows);
        Assert.Equal(0xC1, pkt[0]);
        Assert.Equal(2, pkt[4]);
        Assert.Equal(MagicListWire602.ListTypeNormal, pkt[5]);
    }

    sealed record Row(byte Slot, ushort Type, byte Level) : MagicListWire602.CharacterSkillRowLike;
}
