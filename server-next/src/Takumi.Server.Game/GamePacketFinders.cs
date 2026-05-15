using System.Buffers.Binary;

namespace Takumi.Server.Game;

/// <summary>Packet scanners shared with <c>LegacyLoginHost</c> (F3 list / join).</summary>
public static class GamePacketFinders
{
    /// <summary>Detects PMSG_CHARACTER_LIST_REQ (F3 / 00).</summary>
    public static bool TryFindCharacterListRequest(ReadOnlySpan<byte> packet, out int frameOffset)
    {
        frameOffset = -1;
        Span<byte> scratch = stackalloc byte[8];

        for (var i = 0; i <= packet.Length - 4; i++)
        {
            if (packet[i] is not (0xC1 or 0xC3))
            {
                continue;
            }

            // PMSG_CHARACTER_LIST is a small control frame; C3 account login from Android is ~90 bytes and can
            // contain incidental F3/00 pairs at +2/+3 — do not treat those as list requests.
            if (packet[i] == 0xC3 && (int)packet[i + 1] > 48)
            {
                continue;
            }

            if (packet[i + 2] == 0xF3 && packet[i + 3] == 0x00)
            {
                frameOffset = i;
                return true;
            }

            if (packet[i] == 0xC1 && packet[i + 2] == 0xF3 && i + 5 <= packet.Length)
            {
                packet.Slice(i, 5).CopyTo(scratch[..5]);
                if (scratch[3] != 0x00)
                {
                    TakumiStreamXorCodec.DecodeTakumiStreamXor(scratch[..5], firstXorIndex: 3);
                }

                if (scratch[3] == 0x00)
                {
                    frameOffset = i;
                    return true;
                }
            }
        }

        for (var i = 0; i <= packet.Length - 5; i++)
        {
            if (packet[i] != 0xC3)
            {
                continue;
            }

            var c3Len = (int)packet[i + 1];
            if (c3Len is < 5 or > 48)
            {
                continue;
            }

            if (packet[i + 3] == 0xF3 && packet[i + 4] == 0x00)
            {
                frameOffset = i;
                return true;
            }
        }

        for (var i = 0; i <= packet.Length - 3; i++)
        {
            if (packet[i] != 0x05 || packet[i + 1] != 0xF3 || packet[i + 2] != 0x00)
            {
                continue;
            }

            if (i >= 2 && packet[i - 2] is 0xC1 or 0xC3)
            {
                frameOffset = i - 2;
                return true;
            }
        }

        if (packet.Length <= 16)
        {
            var skipSubstringScan = packet.Length == 15
                && packet[0] == 0xC1
                && packet[1] == 0x0F
                && packet[2] == 0xF3;
            if (!skipSubstringScan)
            {
                for (var j = 0; j <= packet.Length - 2; j++)
                {
                    if (packet[j] == 0xF3 && packet[j + 1] == 0x00)
                    {
                        frameOffset = j >= 2 ? j - 2 : 0;
                        return true;
                    }
                }
            }
        }

        return false;
    }

    /// <summary>C1 0E F3 (03|15) + name[10].</summary>
    public static bool TryFindCharacterJoinRequest(ReadOnlySpan<byte> packet, out int frameOffset, out byte[] name10)
    {
        frameOffset = -1;
        name10 = Array.Empty<byte>();
        Span<byte> scratch = stackalloc byte[14];

        for (var i = 0; i <= packet.Length - 14; i++)
        {
            if (packet[i] != 0xC1 || packet[i + 1] != 0x0E)
            {
                continue;
            }

            packet.Slice(i, 14).CopyTo(scratch);
            if (scratch[2] != 0xF3)
            {
                continue;
            }

            for (var pass = 0; pass < 8 && scratch[3] != 0x03 && scratch[3] != 0x15; pass++)
            {
                TakumiStreamXorCodec.DecodeTakumiStreamXor(scratch, 3);
            }

            if (scratch[2] != 0xF3 || (scratch[3] != 0x03 && scratch[3] != 0x15))
            {
                continue;
            }

            frameOffset = i;
            name10 = new byte[10];
            scratch.Slice(4, 10).CopyTo(name10);
            return true;
        }

        return false;
    }

    /// <summary><c>C1|C3 … F1 02 &lt;flag&gt;</c> logout.</summary>
    public static bool TryFindGameLogoutRequest(ReadOnlySpan<byte> packet, out int frameOffset, out byte value)
    {
        frameOffset = -1;
        value = 0;
        Span<byte> xorScratch = stackalloc byte[64];
        for (var i = 0; i <= packet.Length - 5; i++)
        {
            var lead = packet[i];
            if (lead != 0xC1 && lead != 0xC3)
            {
                continue;
            }

            var size = packet[i + 1];
            if (size < 5 || i + size > packet.Length)
            {
                continue;
            }

            if (packet[i + 2] == 0xF1 && packet[i + 3] == 0x02)
            {
                frameOffset = i;
                value = packet[i + 4];
                return true;
            }

            if (lead == 0xC3 && size is >= 5 and <= 64 && i + size <= packet.Length)
            {
                var work = xorScratch[..(int)size];
                packet.Slice(i, size).CopyTo(work);
                for (var pass = 0; pass < 8 && (work[2] != 0xF1 || work[3] != 0x02); pass++)
                {
                    TakumiStreamXorCodec.DecodeTakumiStreamXor(work, 3);
                }

                if (work[2] == 0xF1 && work[3] == 0x02)
                {
                    frameOffset = i;
                    value = work[4];
                    return true;
                }
            }
        }

        return false;
    }

    /// <summary>
    /// Locates and normalizes account login (<c>F1 01</c>): plain <c>C1</c> with stream XOR on subcode, or <c>C3</c> envelope
    /// (same peel loop as <see cref="TryFindGameLogoutRequest"/>) until <c>[2]=0xF1</c> and <c>[3]=0x01</c>. Output length is at least 59 bytes for id/password/version/serial layout.
    /// </summary>
    public static bool TryUnpackAccountLoginFrame(ReadOnlySpan<byte> packet, out byte[] loginFrame)
    {
        loginFrame = Array.Empty<byte>();
        Span<byte> xorScratch = stackalloc byte[256];

        for (var i = 0; i <= packet.Length - 5; i++)
        {
            var lead = packet[i];
            if (lead is not (0xC1 or 0xC3))
            {
                continue;
            }

            var size = (int)packet[i + 1];
            if (size < 5 || i + size > packet.Length)
            {
                continue;
            }

            var frame = packet.Slice(i, size);

            if (lead == 0xC1 && size >= 59)
            {
                var buf = frame.ToArray();
                var sp = buf.AsSpan();
                for (var peel = 0; peel < 16 && (sp[2] != 0xF1 || sp[3] != 0x01); peel++)
                {
                    TakumiStreamXorCodec.DecodeTakumiStreamXor(sp, firstXorIndex: 3);
                }

                if (sp[2] != 0xF1 || sp[3] != 0x01 || sp.Length < 59)
                {
                    continue;
                }

                loginFrame = sp.Length == 59 ? buf : sp[..59].ToArray();
                return true;
            }

            if (lead == 0xC3 && size >= 5 && size <= 256)
            {
                var work = xorScratch.Slice(0, size);
                frame.CopyTo(work);
                for (var pass = 0; pass < 16 && (work[2] != 0xF1 || work[3] != 0x01); pass++)
                {
                    TakumiStreamXorCodec.DecodeTakumiStreamXor(work, firstXorIndex: 3);
                }

                if (work[2] != 0xF1 || work[3] != 0x01)
                {
                    continue;
                }

                if (work[0] == 0xC1 && work[1] >= 59 && work[1] <= size)
                {
                    loginFrame = work[..work[1]].ToArray();
                }
                else if (size >= 59)
                {
                    loginFrame = work[..size].ToArray();
                }
                else
                {
                    continue;
                }

                return loginFrame.Length >= 59;
            }
        }

        return false;
    }

    /// <summary>Move-map <c>8E 02</c>.</summary>
    public static bool TryFindMoveMapRequest(ReadOnlySpan<byte> packet, out int frameOffset, out uint blockKey, out ushort mapIndex)
    {
        frameOffset = -1;
        blockKey = 0;
        mapIndex = 0;
        for (var i = 0; i <= packet.Length - 10; i++)
        {
            var lead = packet[i];
            if (lead != 0xC1 && lead != 0xC3)
            {
                continue;
            }

            if (packet[i + 1] != 0x0A || packet[i + 2] != 0x8E || packet[i + 3] != 0x02)
            {
                continue;
            }

            frameOffset = i;
            blockKey = BinaryPrimitives.ReadUInt32LittleEndian(packet.Slice(i + 4));
            mapIndex = BinaryPrimitives.ReadUInt16LittleEndian(packet.Slice(i + 8));
            return true;
        }

        return false;
    }

    /// <summary>PMSG_CREATE_CHARACTER (<c>F3 01</c>): <c>C1 0F F3 …</c> + name[10] + packed class.</summary>
    public static bool TryFindCreateCharacterRequest(
        ReadOnlySpan<byte> packet,
        string remote,
        bool verbose,
        out int frameOffset,
        out byte[] name10,
        out byte packedClass)
    {
        frameOffset = -1;
        name10 = Array.Empty<byte>();
        packedClass = 0;
        Span<byte> scratch = stackalloc byte[15];

        for (var i = 0; i <= packet.Length - 15; i++)
        {
            if (packet[i] != 0xC1 || packet[i + 1] != 0x0F)
            {
                continue;
            }

            packet.Slice(i, 15).CopyTo(scratch);
            if (scratch[2] != 0xF3)
            {
                if (verbose)
                {
                    Console.WriteLine(
                        "[{0}] create-candidate rejected — head@2=0x{1:X2} (want F3) hex={2}",
                        remote,
                        scratch[2],
                        Convert.ToHexString(packet.Slice(i, 15)));
                }

                continue;
            }

            for (var pass = 0; pass < 8 && scratch[3] != 0x01; pass++)
            {
                TakumiStreamXorCodec.DecodeTakumiStreamXor(scratch, firstXorIndex: 3);
            }

            if (scratch[3] != 0x01)
            {
                if (verbose)
                {
                    Console.WriteLine(
                        "[{0}] create F3/01 still not normalized (sub@3=0x{1:X2}) — hex={2}",
                        remote,
                        scratch[3],
                        Convert.ToHexString(packet.Slice(i, 15)));
                }

                continue;
            }

            frameOffset = i;
            name10 = new byte[10];
            scratch.Slice(4, 10).CopyTo(name10);
            packedClass = scratch[14];
            return true;
        }

        return false;
    }

    /// <summary>PMSG_REQUEST_DELETE_CHARACTER (<c>F3 02</c>).</summary>
    public static bool TryFindDeleteCharacterRequest(
        ReadOnlySpan<byte> packet,
        out int frameOffset,
        out byte[] name10,
        out byte[] resident20)
    {
        frameOffset = -1;
        name10 = Array.Empty<byte>();
        resident20 = Array.Empty<byte>();
        const int kFrameLen = 34;
        Span<byte> scratch = stackalloc byte[kFrameLen];

        for (var i = 0; i <= packet.Length - kFrameLen; i++)
        {
            if (packet[i] != 0xC1)
            {
                continue;
            }

            if (packet[i + 1] == kFrameLen
                && packet[i + 2] == 0xF3
                && packet[i + 3] == 0x02
                && i + kFrameLen <= packet.Length)
            {
                frameOffset = i;
                name10 = packet.Slice(i + 4, 10).ToArray();
                resident20 = packet.Slice(i + 14, 20).ToArray();
                return true;
            }

            packet.Slice(i, kFrameLen).CopyTo(scratch);
            for (var pass = 0; pass < 8 && (scratch[2] != 0xF3 || scratch[3] != 0x02); pass++)
            {
                TakumiStreamXorCodec.DecodeTakumiStreamXor(scratch, firstXorIndex: 3);
            }

            if (scratch[2] != 0xF3 || scratch[3] != 0x02 || scratch[1] != kFrameLen)
            {
                continue;
            }

            frameOffset = i;
            name10 = new byte[10];
            scratch.Slice(4, 10).CopyTo(name10);
            resident20 = new byte[20];
            scratch.Slice(14, 20).CopyTo(resident20);
            return true;
        }

        return false;
    }
}
