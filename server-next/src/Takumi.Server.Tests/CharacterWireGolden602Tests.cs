using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

/// <summary>Golden vectors for M2 — must match Takumi client expectations (WSclient.h / LegacyLoginHost prior wire).</summary>
public sealed class CharacterWireGolden602Tests
{
    [Fact]
    public void Empty_character_list_F3_00_matches_fixed_octets()
    {
        var actual = CharacterListWire602.BuildEmpty();
        Assert.Equal(
            new byte[] { 0xC1, 0x08, 0xF3, 0x00, 0x00, 0x00, 0x00, 0x00 },
            actual);
    }

    [Fact]
    public void Login_result_F1_01_success_byte()
    {
        var mem = LoginAccountWire602.BuildLoginResult(0x01);
        Assert.Equal(new byte[] { 0xC1, 0x05, 0xF1, 0x01, 0x01 }, mem.ToArray());
    }

    [Fact]
    public void Delete_character_response_value_1()
    {
        var b = CharacterCreateWire602.BuildDeleteResponse(1);
        Assert.Equal(new byte[] { 0xC1, 0x05, 0xF3, 0x02, 0x01 }, b);
    }

    [Fact]
    public void Join_map_131_bytes_header_and_coords_match_default_spawn()
    {
        var name = new byte[10];
        System.Text.Encoding.ASCII.GetBytes("HERO").AsSpan().CopyTo(name);
        var pkt = JoinMapServerWire602.Build(new CharacterRosterWire(name, serverClass: 0x20, level: 1));
        Assert.Equal(131, pkt.Length);
        Assert.Equal(0xC1, pkt[0]);
        Assert.Equal(131, pkt[1]);
        Assert.Equal(0xF3, pkt[2]);
        Assert.Equal(0x03, pkt[3]);
        Assert.Equal(135, pkt[4]);
        Assert.Equal(122, pkt[5]);
        Assert.Equal(0, pkt[6]); // default map id
        Assert.Equal(1, pkt[7]); // angle
        var life = pkt[34] | (pkt[35] << 8);
        Assert.True(life > 0, "Life LE @34 should be non-zero for client world bootstrap");
    }

    [Fact]
    public void Join_map_custom_spawn_overrides_defaults()
    {
        var name = new byte[10];
        System.Text.Encoding.ASCII.GetBytes("X").AsSpan().CopyTo(name);
        var spawn = new JoinMapSpawnWire(3, 40, 50, 2);
        var pkt = JoinMapServerWire602.Build(new CharacterRosterWire(name, serverClass: 0x00, level: 10), spawn);
        Assert.Equal(3, pkt[6]);
        Assert.Equal(40, pkt[4]);
        Assert.Equal(50, pkt[5]);
        Assert.Equal(2, pkt[7]);
    }

    [Fact]
    public void Inventory_F3_10_empty_is_C4_with_zero_count()
    {
        var pkt = InventoryListWire602.BuildEmpty();
        Assert.Equal(6, pkt.Length);
        Assert.Equal(new byte[] { 0xC4, 0x00, 0x06, 0xF3, 0x10, 0x00 }, pkt);
    }

    [Fact]
    public void Inventory_F3_10_one_slot_matches_GameServer_layout()
    {
        var payload = new byte[13];
        payload[0] = 12; // slot index (main inventory region in Takumi client)
        for (var i = 0; i < 12; i++)
        {
            payload[1 + i] = (byte)(0x10 + i);
        }

        var pkt = InventoryListWire602.Build(payload);
        Assert.Equal(6 + 13, pkt.Length);
        Assert.Equal(new byte[] { 0xC4, 0x00, 0x13, 0xF3, 0x10, 0x01 }, pkt.AsSpan(0, 6).ToArray());
        Assert.Equal(payload, pkt.AsSpan(6).ToArray());
    }

    [Fact]
    public void Single_slot_character_list_length_and_padding()
    {
        var name = new byte[10];
        System.Text.Encoding.ASCII.GetBytes("ADMIN").AsSpan().CopyTo(name);
        var roster = new List<CharacterRosterWire> { new(name, serverClass: 0x20, level: 400) };
        var pkt = CharacterListWire602.Build(roster);
        const int expectedLen = 8 + 34;
        Assert.Equal(expectedLen, pkt.Length);
        Assert.Equal(0xC1, pkt[0]);
        Assert.Equal(expectedLen, pkt[1]);
        Assert.Equal(0xF3, pkt[2]);
        Assert.Equal(0x00, pkt[3]);
        Assert.Equal(1, pkt[6]); // count
        // slot index
        Assert.Equal(0, pkt[8]);
        // name
        Assert.Equal((byte)'A', pkt[9]);
        // padding before LE level @ 8+1+10 = 19
        Assert.Equal(0, pkt[19]);
        Assert.Equal(400, pkt[20] | (pkt[21] << 8)); // LE
        Assert.Equal(0, pkt[22]); // CtlCode
        Assert.Equal(0x20, pkt[23]); // ServerClass
    }

    [Fact]
    public void Create_success_packet_layout()
    {
        var name = new byte[10];
        System.Text.Encoding.ASCII.GetBytes("NEWBIE").AsSpan().CopyTo(name);
        var mem = CharacterCreateWire602.BuildCreateSuccess(name, slot: 0, level: 1, serverClass: 0x20);
        var p = mem.ToArray();
        Assert.Equal(19, p.Length);
        Assert.Equal(0xC1, p[0]);
        Assert.Equal(19, p[1]);
        Assert.Equal(0xF3, p[2]);
        Assert.Equal(0x01, p[3]);
        Assert.Equal(1, p[4]);
        Assert.Equal(0, p[15]); // slot
        Assert.Equal(1, p[16]);
        Assert.Equal(0, p[17]);
        Assert.Equal(0x20, p[18]);
    }

    [Fact]
    public void Connect_server_list_one_id_wire()
    {
        var pkt = ConnectServerList602.Build(connectBase: 20, serverCount: 1, loadPercent: 0x0A);
        Assert.Equal(new byte[] { 0xC2, 0x00, 0x0B, 0xF4, 0x06, 0x00, 0x01, 0x14, 0x00, 0x0A, 0x00 }, pkt);
    }

    [Fact]
    public void Map_packed_class_Dark_Knight_nibble_1_to_0x20()
    {
        Assert.Equal(0x20, CharacterCreateWire602.MapPackedClassToServerProtocol(0x10)); // job<<4
    }
}
