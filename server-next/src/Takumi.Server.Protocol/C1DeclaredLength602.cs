namespace Takumi.Server.Protocol;

/// <summary>Helpers for Season6 <c>C1</c> length byte at offset+1 (total frame length from <paramref name="frameStart"/>).</summary>
public static class C1DeclaredLength602
{
    public static bool TryGetDeclaredTotalLength(ReadOnlySpan<byte> buffer, int frameStart, out int declaredTotalLen)
    {
        declaredTotalLen = 0;
        if (frameStart < 0 || buffer.Length - frameStart < 2)
        {
            return false;
        }

        if (buffer[frameStart] != 0xC1)
        {
            return false;
        }

        declaredTotalLen = buffer[frameStart + 1];
        return declaredTotalLen >= 2;
    }

    /// <summary>Returns false when the declared length exceeds the buffer (truncated / malformed frame).</summary>
    public static bool IsDeclaredLengthWithinBuffer(ReadOnlySpan<byte> buffer, int frameStart, out int declaredTotalLen)
    {
        if (!TryGetDeclaredTotalLength(buffer, frameStart, out declaredTotalLen))
        {
            return false;
        }

        return frameStart + declaredTotalLen <= buffer.Length;
    }
}
