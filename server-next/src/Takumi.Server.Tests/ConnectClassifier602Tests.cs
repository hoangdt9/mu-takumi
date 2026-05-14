using Takumi.Server.Connect;
using Takumi.Server.Protocol;
using Xunit;

namespace Takumi.Server.Tests;

public sealed class ConnectClassifier602Tests
{
    [Fact]
    public void Classify_F4_06_Is_ServerList()
    {
        var pkt = new byte[] { 0xC1, 0x05, 0xF4, 0x06, 0x00 };
        Assert.Equal(TakumiConnectRequestKind.ServerList, ConnectServerPacketClassifier.Classify(pkt));
    }

    [Fact]
    public void Classify_F4_02_Is_ServerList()
    {
        var pkt = new byte[] { 0xC1, 0x05, 0xF4, 0x02, 0x00 };
        Assert.Equal(TakumiConnectRequestKind.ServerList, ConnectServerPacketClassifier.Classify(pkt));
    }

    [Fact]
    public void Classify_F4_03_Is_ServerInfo()
    {
        var pkt = new byte[] { 0xC1, 0x05, 0xF4, 0x03, 0x00 };
        Assert.Equal(TakumiConnectRequestKind.ServerInfo, ConnectServerPacketClassifier.Classify(pkt));
    }

    [Fact]
    public void Classify_Patch_OpenMU_style_Is_PatchCheck()
    {
        var pkt = new byte[] { 0xC1, 0x08, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00 };
        Assert.Equal(TakumiConnectRequestKind.PatchCheck, ConnectServerPacketClassifier.Classify(pkt));
    }

    [Fact]
    public void Classify_MainCode_05_Is_PatchCheck()
    {
        var pkt = new byte[] { 0xC1, 0x06, 0x05, 0x01, 0x00, 0x00 };
        Assert.Equal(TakumiConnectRequestKind.PatchCheck, ConnectServerPacketClassifier.Classify(pkt));
    }

    [Fact]
    public void TryFindFirstRequestOfKind_Finds_List_After_Leading_Info()
    {
        var info = new byte[] { 0xC1, 0x05, 0xF4, 0x03, 0x00 };
        var list = new byte[] { 0xC1, 0x05, 0xF4, 0x06, 0x00 };
        var buf = new byte[info.Length + list.Length];
        info.CopyTo(buf.AsSpan(0));
        list.CopyTo(buf.AsSpan(info.Length));
        Assert.True(
            ConnectServerPacketClassifier.TryFindFirstRequestOfKind(
                buf,
                TakumiConnectRequestKind.ServerList,
                out var off,
                out var frame));
        Assert.Equal(info.Length, off);
        Assert.Equal(list.Length, frame.Length);
    }

    [Fact]
    public void PatchVersionOkay_Golden()
    {
        Assert.Equal(new byte[] { 0xC1, 0x04, 0x02, 0x00 }, ConnectPatchWire602.BuildPatchVersionOkay());
    }

    [Fact]
    public void ServerBusy_Golden()
    {
        Assert.Equal(new byte[] { 0xC1, 0x05, 0xF4, 0x05, 7 }, ConnectServerBusy602.Build(7));
    }
}
