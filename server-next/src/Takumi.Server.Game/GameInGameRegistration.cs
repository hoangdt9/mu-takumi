using System.Buffers.Binary;
using System.Text;

namespace Takumi.Server.Game;

/// <summary>
/// In-game account registration (<c>C1 D3 05</c>, client <c>CB_DangKyInGame</c> / legacy JoinServer forward).
/// </summary>
public static class GameInGameRegistration
{
    public const byte HeadCode = 0xD3;
    public const byte SubCode = 0x05;
    public const int RequestFrameLength = 59;

    public const byte ResultSuccess = 1;
    public const byte ResultAccountExists = 2;
    public const byte ResultInvalidInput = 3;

    public readonly record struct RegisterRequest(
        string Account,
        string Password,
        string SecurityCode,
        string Phone);

    public static bool TryFindRequest(ReadOnlySpan<byte> packet, out int frameOffset)
    {
        frameOffset = -1;
        const int frameLen = RequestFrameLength;
        Span<byte> scratch = stackalloc byte[frameLen];

        for (var i = 0; i <= packet.Length - frameLen; i++)
        {
            if (packet[i] != 0xC1 || packet[i + 1] != frameLen)
            {
                continue;
            }

            if (packet[i + 2] == HeadCode && packet[i + 3] == SubCode)
            {
                frameOffset = i;
                return true;
            }

            packet.Slice(i, frameLen).CopyTo(scratch);
            for (var pass = 0; pass < 8 && (scratch[2] != HeadCode || scratch[3] != SubCode); pass++)
            {
                TakumiStreamXorCodec.DecodeTakumiStreamXor(scratch, firstXorIndex: 3);
            }

            if (scratch[2] == HeadCode && scratch[3] == SubCode && scratch[1] == frameLen)
            {
                frameOffset = i;
                return true;
            }
        }

        return false;
    }

    public static bool TryParseRequest(ReadOnlySpan<byte> packet, int frameOffset, out RegisterRequest request)
    {
        request = default;
        if (frameOffset < 0 || frameOffset + RequestFrameLength > packet.Length)
        {
            return false;
        }

        var body = packet.Slice(frameOffset, RequestFrameLength);
        if (body[0] != 0xC1 || body[1] != RequestFrameLength || body[2] != HeadCode || body[3] != SubCode)
        {
            return false;
        }

        if (body[4] != 0x01)
        {
            return false;
        }

        var account = ReadAsciiField(body.Slice(5, 11));
        var password = ReadAsciiField(body.Slice(16, 21));
        var security = ReadAsciiField(body.Slice(37, 8));
        var phone = ReadAsciiField(body.Slice(45, 14));
        if (account.Length == 0 || password.Length == 0)
        {
            return false;
        }

        request = new RegisterRequest(account, password, security, phone);
        return true;
    }

    public static bool IsValidRequest(in RegisterRequest request) =>
        IsValidAccountId(request.Account)
        && IsValidPassword(request.Password)
        && IsDigits(request.SecurityCode, 7)
        && IsDigits(request.Phone, 10, 14);

    public static byte RegisterAccount(IDictionary<string, string> accounts, in RegisterRequest request)
    {
        if (!IsValidRequest(request))
        {
            return ResultInvalidInput;
        }

        if (accounts.ContainsKey(request.Account))
        {
            return ResultAccountExists;
        }

        accounts[request.Account] = request.Password;
        return ResultSuccess;
    }

    public static byte[] BuildResponse(byte resultCode)
    {
        var packet = new byte[8];
        packet[0] = 0xC1;
        packet[1] = 8;
        packet[2] = HeadCode;
        packet[3] = SubCode;
        BinaryPrimitives.WriteUInt32LittleEndian(packet.AsSpan(4), resultCode);
        return packet;
    }

    static string ReadAsciiField(ReadOnlySpan<byte> field)
    {
        var len = field.IndexOf((byte)0);
        if (len < 0)
        {
            len = field.Length;
        }

        return Encoding.ASCII.GetString(field[..len]).Trim();
    }

    static bool IsValidAccountId(string value)
    {
        if (value.Length is < 1 or > 10)
        {
            return false;
        }

        foreach (var c in value)
        {
            if (!char.IsAsciiLetterOrDigit(c))
            {
                return false;
            }
        }

        return true;
    }

    static bool IsValidPassword(string value)
    {
        if (value.Length is < 1 or > 20)
        {
            return false;
        }

        foreach (var c in value)
        {
            if (c < 32 || c > 126)
            {
                return false;
            }
        }

        return true;
    }

    static bool IsDigits(string value, int minLen, int maxLen = int.MaxValue)
    {
        if (value.Length < minLen || value.Length > maxLen)
        {
            return false;
        }

        return value.All(char.IsAsciiDigit);
    }
}
