namespace Takumi.Server.Protocol;

/// <summary>Season 6 <c>C1 0x14</c> delete viewport (<c>GCViewportDestroySend</c> / <c>ReceiveDeleteCharacterViewport</c>).</summary>
public static class MonsterViewportDestroyWire602
{
    public const byte HeadCode = 0x14;

    public static byte[] Build(IReadOnlyList<int> objectKeys)
    {
        if (objectKeys.Count == 0)
        {
            return Array.Empty<byte>();
        }

        var size = 4 + (objectKeys.Count * 2);
        var buf = new byte[size];
        buf[0] = 0xC1;
        buf[1] = (byte)size;
        buf[2] = HeadCode;
        buf[3] = (byte)objectKeys.Count;
        var o = 4;
        foreach (var key in objectKeys)
        {
            var k = key & 0x7FFF;
            buf[o++] = (byte)((k >> 8) & 0xFF);
            buf[o++] = (byte)(k & 0xFF);
        }

        return buf;
    }
}
