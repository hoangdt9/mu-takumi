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
    public void MagicGladiator_defaults_are_combat_qa_kit_only()
    {
        var skills = CharacterSkillCatalog.GetDefaultEntries(96);
        Assert.Equal(30, skills.Count);
        Assert.Contains(skills, s => s.Type == 55);
        Assert.Contains(skills, s => s.Type == 39);
        Assert.Contains(skills, s => s.Type == 76);
        Assert.DoesNotContain(skills, s => s.Type == 490);
        Assert.DoesNotContain(skills, s => s.Type == 48);
        Assert.DoesNotContain(skills, s => s.Type == 238);

        for (var i = 0; i < skills.Count; i++)
        {
            Assert.Equal((byte)(i + 1), skills[i].Index);
            Assert.Equal(CharacterSkillCatalog.MagicGladiatorCombatSkillTypes[i], skills[i].Type);
        }
    }

    [Fact]
    public void NormalizeMagicGladiatorForClientWire_remaps_legacy_slot_equals_type()
    {
        var legacy =
            CharacterSkillCatalog.ToMagicGladiatorCombatEntries(20)
                .Select(e => new MagicListWire602.Entry((byte)e.Type, e.Type, e.Level))
                .ToArray();

        var normalized = CharacterSkillCatalog.NormalizeMagicGladiatorForClientWire(legacy, 20);

        Assert.Equal(30, normalized.Length);
        Assert.Equal((byte)29, normalized[28].Index);
        Assert.Equal((ushort)236, normalized[28].Type);
        Assert.Equal((byte)30, normalized[29].Index);
        Assert.Equal((ushort)237, normalized[29].Type);
    }

    [Fact]
    public void NormalizeMagicGladiatorForClientWire_strips_master_rows()
    {
        var rows = CharacterSkillCatalog.ToMagicGladiatorCombatEntries(20).ToList();
        rows.Add(new MagicListWire602.Entry(120, 490, 20));

        var normalized = CharacterSkillCatalog.NormalizeMagicGladiatorForClientWire(rows, 20);

        Assert.Equal(30, normalized.Length);
        Assert.DoesNotContain(normalized, e => e.Type == 490);
    }

    [Fact]
    public void BuildForServerClass_elf_sends_skills()
    {
        var pkt = MagicListWire602.BuildForServerClass(64);
        Assert.True(pkt.Length > 7);
        Assert.Equal(0x11, pkt[3]);
    }

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
            new Row(24, 55, 15),
        ];
        var pkt = MagicListWire602.BuildFromRows(rows);
        Assert.Equal(0xC1, pkt[0]);
        Assert.Equal(2, pkt[4]);
        Assert.Equal(MagicListWire602.ListTypeNormal, pkt[5]);
    }

    sealed record Row(byte Slot, ushort Type, byte Level) : MagicListWire602.CharacterSkillRowLike;
}
