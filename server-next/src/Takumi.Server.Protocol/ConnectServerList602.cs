using System.Buffers.Binary;

namespace Takumi.Server.Protocol;

/// <summary>Season 6 connect-server list <c>C2 F4 06</c> (Takumi <c>ReceiveServerList</c> / OpenMU server list).</summary>
public static class ConnectServerList602
{
    /// <summary>
    /// Each entry is 4 bytes: server id (LE), load %, padding. Client maps id → ServerList.bmd via index/20.
    /// </summary>
    public static byte[] Build(int connectBase, int serverCount, byte loadPercent)
    {
        if (connectBase < 0 || connectBase > 65535)
        {
            throw new ArgumentOutOfRangeException(nameof(connectBase));
        }

        if (serverCount is < 1 or > 32)
        {
            throw new ArgumentOutOfRangeException(nameof(serverCount));
        }

        var len = 7 + serverCount * 4;
        var p = new byte[len];
        p[0] = 0xC2;
        BinaryPrimitives.WriteUInt16BigEndian(p.AsSpan(1, 2), (ushort)len);
        p[3] = 0xF4;
        p[4] = 0x06;
        BinaryPrimitives.WriteUInt16BigEndian(p.AsSpan(5, 2), (ushort)serverCount);
        var off = 7;
        for (var i = 0; i < serverCount; i++)
        {
            var id = connectBase + i;
            if (id > 65535)
            {
                throw new ArgumentException("connectBase + count exceeds ushort server id range.", nameof(connectBase));
            }

            BinaryPrimitives.WriteUInt16LittleEndian(p.AsSpan(off, 2), (ushort)id);
            p[off + 2] = loadPercent;
            p[off + 3] = 0;
            off += 4;
        }

        return p;
    }

    /// <summary>Same wire as <see cref="Build"/> but with an explicit list of connect indices.</summary>
    public static byte[] BuildFromIds(ReadOnlySpan<int> connectIds, byte loadPercent)
    {
        if (connectIds.Length is < 1 or > 32)
        {
            throw new ArgumentOutOfRangeException(nameof(connectIds));
        }

        var len = 7 + connectIds.Length * 4;
        var p = new byte[len];
        p[0] = 0xC2;
        BinaryPrimitives.WriteUInt16BigEndian(p.AsSpan(1, 2), (ushort)len);
        p[3] = 0xF4;
        p[4] = 0x06;
        BinaryPrimitives.WriteUInt16BigEndian(p.AsSpan(5, 2), (ushort)connectIds.Length);
        var off = 7;
        for (var i = 0; i < connectIds.Length; i++)
        {
            var id = connectIds[i];
            if (id is < 0 or > 65535)
            {
                throw new ArgumentOutOfRangeException(nameof(connectIds));
            }

            BinaryPrimitives.WriteUInt16LittleEndian(p.AsSpan(off, 2), (ushort)id);
            p[off + 2] = loadPercent;
            p[off + 3] = 0;
            off += 4;
        }

        return p;
    }
}
