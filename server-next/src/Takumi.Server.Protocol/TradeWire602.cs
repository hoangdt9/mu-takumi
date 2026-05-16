using System.Buffers.Binary;
using System.Text;

namespace Takumi.Server.Protocol;

/// <summary>Season 6 player trade wire (<c>0x36</c>–<c>0x3D</c>).</summary>
public static class TradeWire602
{
    public const byte HeadRequest = 0x36;
    public const byte HeadResponse = 0x37;
    public const byte HeadResult = 0x3A;
    public const byte HeadExit = 0x3D;

    public static bool TryFindTradeRequest(ReadOnlySpan<byte> packet, out int frameOffset, out ushort targetIndex)
    {
        frameOffset = -1;
        targetIndex = 0;
        for (var i = 0; i <= packet.Length - 5; i++)
        {
            if (packet[i] is not (0xC1 or 0xC3) || packet[i + 2] != HeadRequest)
            {
                continue;
            }

            frameOffset = i;
            targetIndex = (ushort)((packet[i + 3] << 8) | packet[i + 4]);
            return true;
        }

        return false;
    }

    public static bool TryFindTradeResponse(ReadOnlySpan<byte> packet, out int frameOffset, out byte response)
    {
        frameOffset = -1;
        response = 0;
        for (var i = 0; i <= packet.Length - 4; i++)
        {
            if (packet[i] != 0xC1 || packet[i + 2] != HeadResponse)
            {
                continue;
            }

            frameOffset = i;
            response = packet[i + 3];
            return true;
        }

        return false;
    }

    public static bool TryFindTradeExit(ReadOnlySpan<byte> packet, out int frameOffset)
    {
        frameOffset = -1;
        for (var i = 0; i <= packet.Length - 3; i++)
        {
            if (packet[i] == 0xC1 && packet[i + 2] == HeadExit)
            {
                frameOffset = i;
                return true;
            }
        }

        return false;
    }

    public static byte[] BuildTradeRequest(string name10)
    {
        var buf = new byte[13];
        buf[0] = 0xC3;
        buf[1] = 13;
        buf[2] = HeadRequest;
        var nameBytes = Encoding.ASCII.GetBytes(name10.PadRight(10)[..10]);
        nameBytes.CopyTo(buf.AsSpan(3, 10));
        return buf;
    }

    public static byte[] BuildTradeResponse(byte response, string name10, ushort level = 1, uint guildNumber = 0)
    {
        var buf = new byte[19];
        buf[0] = 0xC1;
        buf[1] = 19;
        buf[2] = HeadResponse;
        buf[3] = response;
        var nameBytes = Encoding.ASCII.GetBytes(name10.PadRight(10)[..10]);
        nameBytes.CopyTo(buf.AsSpan(4, 10));
        BinaryPrimitives.WriteUInt16LittleEndian(buf.AsSpan(14, 2), level);
        BinaryPrimitives.WriteUInt32LittleEndian(buf.AsSpan(16, 4), guildNumber);
        return buf;
    }

    public static byte[] BuildTradeResult(byte result)
    {
        var buf = new byte[4];
        buf[0] = 0xC1;
        buf[1] = 4;
        buf[2] = HeadResult;
        buf[3] = result;
        return buf;
    }

    public static byte[] BuildTradeExit(byte value = 0)
    {
        var buf = new byte[4];
        buf[0] = 0xC1;
        buf[1] = 4;
        buf[2] = HeadExit;
        buf[3] = value;
        return buf;
    }
}
