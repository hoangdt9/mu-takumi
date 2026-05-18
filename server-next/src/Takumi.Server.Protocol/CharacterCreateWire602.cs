using System.Buffers.Binary;

namespace Takumi.Server.Protocol;

/// <summary><c>F3 01</c> create result and <c>F3 02</c> delete result; packed class mapping for create requests.</summary>
public static class CharacterCreateWire602
{
    /// <summary>Takumi PRECEIVE_CREATE_CHARACTER: PBMSG + SubCode + Result + ID[10] + Index + Level + Class (19 bytes).</summary>
    public static ReadOnlyMemory<byte> BuildCreateSuccess(ReadOnlySpan<byte> name10, byte slot, ushort level, byte serverClass)
    {
        var p = new byte[19];
        p[0] = 0xC1;
        p[1] = 19;
        p[2] = 0xF3;
        p[3] = 0x01;
        p[4] = 1;
        p.AsSpan(5, 10).Clear();
        var n = Math.Min(10, name10.Length);
        name10[..n].CopyTo(p.AsSpan(5));

        p[15] = slot;
        BinaryPrimitives.WriteUInt16LittleEndian(p.AsSpan(16, 2), level);
        p[18] = serverClass;
        return p;
    }

    public static ReadOnlyMemory<byte> BuildCreateFailure(byte resultCode, ReadOnlySpan<byte> name10)
    {
        var p = new byte[19];
        p[0] = 0xC1;
        p[1] = 19;
        p[2] = 0xF3;
        p[3] = 0x01;
        p[4] = resultCode;
        p.AsSpan(5, 10).Clear();
        var n = Math.Min(10, name10.Length);
        name10[..n].CopyTo(p.AsSpan(5));
        return p;
    }

    /// <summary>PRECEIVE delete character: Value 1=success, 2=resident wrong, …</summary>
    public static byte[] BuildDeleteResponse(byte value) => new byte[] { 0xC1, 0x05, 0xF3, 0x02, value };

    /// <summary>Maps client create byte <c>(CLASS_TYPE &lt;&lt; 4) | skin</c> to Season 6 wire class.</summary>
    public static byte MapPackedClassToServerProtocol(byte packed)
    {
        var jobNibble = (byte)(packed >> 4);
        return jobNibble switch
        {
            0 => 0x00,
            1 => 0x20,
            2 => 0x40,
            3 => 0x60,
            4 => 0x80,
            5 => 0xA0,
            6 => 0xC0,
            _ => 0x00,
        };
    }
}
