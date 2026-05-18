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
        Assert.Equal(2, pkt[5]);
    }

    sealed record Row(byte Slot, ushort Type, byte Level) : MagicListWire602.CharacterSkillRowLike;
}
