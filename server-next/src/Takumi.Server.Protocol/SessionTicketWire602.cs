using System.Buffers.Binary;

namespace Takumi.Server.Protocol;

/// <summary>Wire layout for signed session ticket (M6 split-stack).</summary>
public static class SessionTicketWire602
{
    public const byte ServerPushSubCode = 0xA5;

    public const byte ClientAttachSubCode = 0xA6;

    /// <summary>After <c>C1 len F1 sub</c>: ticket + exp + account + mac.</summary>
    public const int BodyBytes = 16 + 8 + SessionTicketSignature602.AccountWireBytes + SessionTicketSignature602.MacLength;

    public static int PushPacketTotalLength => 4 + BodyBytes;

    /// <summary>Plain <c>C1</c> packet (server sends unencrypted on Takumi login TCP).</summary>
    public static byte[] BuildPushC1(Guid ticketId, long expiresUnixUtc, ReadOnlySpan<byte> account10Wire, ReadOnlySpan<byte> mac32)
    {
        if (account10Wire.Length != SessionTicketSignature602.AccountWireBytes || mac32.Length != SessionTicketSignature602.MacLength)
        {
            throw new ArgumentException("Invalid account or MAC length.");
        }

        var pkt = new byte[PushPacketTotalLength];
        pkt[0] = 0xC1;
        pkt[1] = (byte)PushPacketTotalLength;
        pkt[2] = 0xF1;
        pkt[3] = ServerPushSubCode;
        ticketId.TryWriteBytes(pkt.AsSpan(4, 16));
        BinaryPrimitives.WriteInt64BigEndian(pkt.AsSpan(20, 8), expiresUnixUtc);
        account10Wire.CopyTo(pkt.AsSpan(28, 10));
        mac32.CopyTo(pkt.AsSpan(38, 32));
        return pkt;
    }

    /// <summary>Locate client attach <c>F1 0xA6</c> in a decrypted buffer (C1 or C3 single-frame).</summary>
    public static bool TryFindClientAttach(ReadOnlySpan<byte> packet, out ReadOnlySpan<byte> body66)
    {
        body66 = default;
        if (packet.Length < 4 + BodyBytes)
        {
            return false;
        }

        byte head;
        byte sub;
        int payloadStart;
        if (packet[0] == 0xC1 && packet.Length >= 4)
        {
            head = packet[2];
            sub = packet[3];
            payloadStart = 4;
        }
        else if (packet[0] == 0xC3 && packet.Length >= 4)
        {
            head = packet[2];
            sub = packet[3];
            payloadStart = 4;
        }
        else
        {
            return false;
        }

        if (head != 0xF1 || sub != ClientAttachSubCode)
        {
            return false;
        }

        if (packet.Length - payloadStart < BodyBytes)
        {
            return false;
        }

        body66 = packet.Slice(payloadStart, BodyBytes);
        return true;
    }

    public static bool TryReadBody(
        ReadOnlySpan<byte> body66,
        out Guid ticketId,
        out long expiresUnixUtc,
        out ReadOnlySpan<byte> account10,
        out ReadOnlySpan<byte> mac32)
    {
        ticketId = default;
        expiresUnixUtc = 0;
        account10 = default;
        mac32 = default;
        if (body66.Length != BodyBytes)
        {
            return false;
        }

        ticketId = new Guid(body66.Slice(0, 16));
        expiresUnixUtc = BinaryPrimitives.ReadInt64BigEndian(body66.Slice(16, 8));
        account10 = body66.Slice(24, 10);
        mac32 = body66.Slice(34, 32);
        return true;
    }
}
