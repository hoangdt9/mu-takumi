namespace Takumi.Server.Protocol;

/// <summary>30-byte client option blob (skill hotkeys + QWER + flags). Parity <c>PMSG_OPTION_DATA</c> / <c>SaveOptions</c>.</summary>
public static class CharacterKeyConfiguration
{
    public const int Length = 30;

    public static byte[] CreateDefault()
    {
        var data = new byte[Length];
        for (var i = 0; i < 20; i += 2)
        {
            data[i] = 0xFF;
            data[i + 1] = 0xFF;
        }

        return data;
    }

    public static byte[] Normalize(ReadOnlySpan<byte> source)
    {
        if (source.Length == Length)
        {
            return source.ToArray();
        }

        var data = CreateDefault();
        if (source.Length > 0)
        {
            source.Slice(0, Math.Min(Length, source.Length)).CopyTo(data);
        }

        return data;
    }
}
