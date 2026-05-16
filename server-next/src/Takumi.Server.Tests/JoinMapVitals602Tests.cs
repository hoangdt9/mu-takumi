using System.Buffers.Binary;
using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class JoinMapVitals602Tests
{
    [Fact]
    public void Join_recomputes_max_vitals_from_class_keeps_partial_current_and_zen()
    {
        var name = new byte[10];
        "Hero"u8.CopyTo(name);
        var vitals = CharacterRosterVitals.FromInts(currentHp: 321, maxHp: 500, currentMp: 80, maxMp: 120, zen: 1_234_567);
        var sheet = CharacterSheetCalculator.DefaultSheet(0x20, 10);
        var computed = CharacterSheetCalculator.ComputeMaxVitals(0x20, 10, sheet);
        var merged = CharacterSheetCalculator.MergeVitalsForJoin(vitals, computed);
        var pkt = JoinMapServerWire602.Build(new CharacterRosterWire(name, serverClass: 0x20, level: 10, vitals));

        Assert.Equal(merged.CurrentHp, BinaryPrimitives.ReadUInt16LittleEndian(pkt.AsSpan(34)));
        Assert.Equal(computed.LifeMax, BinaryPrimitives.ReadUInt16LittleEndian(pkt.AsSpan(36)));
        Assert.Equal(merged.CurrentMp, BinaryPrimitives.ReadUInt16LittleEndian(pkt.AsSpan(38)));
        Assert.Equal(computed.ManaMax, BinaryPrimitives.ReadUInt16LittleEndian(pkt.AsSpan(40)));
        Assert.Equal(1_234_567u, BinaryPrimitives.ReadUInt32LittleEndian(pkt.AsSpan(50)));
        Assert.Equal((uint)merged.CurrentHp, BinaryPrimitives.ReadUInt32LittleEndian(pkt.AsSpan(79)));
        Assert.Equal((uint)computed.LifeMax, BinaryPrimitives.ReadUInt32LittleEndian(pkt.AsSpan(83)));
    }

    [Fact]
    public void Join_recomputes_shield_max_from_stats()
    {
        var name = new byte[10];
        "Sd"u8.CopyTo(name);
        var vitals = CharacterRosterVitals.FromInts(100, 200, 30, 60, zen: 0, currentShield: 40, maxShield: 180);
        var sheet = CharacterSheetCalculator.DefaultSheet(0x20, 10);
        var computed = CharacterSheetCalculator.ComputeMaxVitals(0x20, 10, sheet);
        var merged = CharacterSheetCalculator.MergeVitalsForJoin(vitals, computed);
        var pkt = JoinMapServerWire602.Build(new CharacterRosterWire(name, serverClass: 0x20, level: 10, vitals));

        Assert.Equal(merged.CurrentShield, BinaryPrimitives.ReadUInt16LittleEndian(pkt.AsSpan(42)));
        Assert.Equal(computed.ShieldMax, BinaryPrimitives.ReadUInt16LittleEndian(pkt.AsSpan(44)));
        Assert.Equal((uint)merged.CurrentShield, BinaryPrimitives.ReadUInt32LittleEndian(pkt.AsSpan(103)));
        Assert.Equal((uint)computed.ShieldMax, BinaryPrimitives.ReadUInt32LittleEndian(pkt.AsSpan(107)));
    }

    [Fact]
    public void Join_rage_fighter_level1_uses_class_formula_vitals()
    {
        var name = new byte[10];
        "rf001"u8.CopyTo(name);
        var sheet = CharacterSheetCalculator.DefaultSheet(serverClass: 0xC0, level: 1);
        var computed = CharacterSheetCalculator.ComputeMaxVitals(0xC0, 1, sheet);
        var vitals = CharacterSheetCalculator.MergeVitalsForJoin(default, computed);
        var pkt = JoinMapServerWire602.Build(new CharacterRosterWire(name, serverClass: 0xC0, level: 1, vitals, sheet));

        Assert.Equal(32, BinaryPrimitives.ReadUInt16LittleEndian(pkt.AsSpan(26)));
        Assert.Equal(27, BinaryPrimitives.ReadUInt16LittleEndian(pkt.AsSpan(28)));
        Assert.Equal(25, BinaryPrimitives.ReadUInt16LittleEndian(pkt.AsSpan(30)));
        Assert.Equal(20, BinaryPrimitives.ReadUInt16LittleEndian(pkt.AsSpan(32)));
        Assert.Equal(120, BinaryPrimitives.ReadUInt16LittleEndian(pkt.AsSpan(34)));
        Assert.Equal(120, BinaryPrimitives.ReadUInt16LittleEndian(pkt.AsSpan(36)));
        Assert.Equal(30, BinaryPrimitives.ReadUInt16LittleEndian(pkt.AsSpan(38)));
        Assert.Equal(30, BinaryPrimitives.ReadUInt16LittleEndian(pkt.AsSpan(40)));
        Assert.Equal(120u, BinaryPrimitives.ReadUInt32LittleEndian(pkt.AsSpan(79)));
        Assert.Equal(120u, BinaryPrimitives.ReadUInt32LittleEndian(pkt.AsSpan(83)));
        Assert.Equal(30u, BinaryPrimitives.ReadUInt32LittleEndian(pkt.AsSpan(87)));
        Assert.Equal(30u, BinaryPrimitives.ReadUInt32LittleEndian(pkt.AsSpan(91)));
    }

    [Fact]
    public void Join_falls_back_to_stub_when_vitals_unset()
    {
        var name = new byte[10];
        "New"u8.CopyTo(name);
        var pkt = JoinMapServerWire602.Build(new CharacterRosterWire(name, serverClass: 0x00, level: 5));

        var life = BinaryPrimitives.ReadUInt16LittleEndian(pkt.AsSpan(34));
        var lifeMax = BinaryPrimitives.ReadUInt16LittleEndian(pkt.AsSpan(36));
        Assert.True(life > 0);
        Assert.Equal(lifeMax, life);
        Assert.Equal(0u, BinaryPrimitives.ReadUInt32LittleEndian(pkt.AsSpan(50)));
    }
}
