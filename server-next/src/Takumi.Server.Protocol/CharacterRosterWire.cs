using System.Buffers.Binary;

namespace Takumi.Server.Protocol;

/// <summary>Character row for Season 6 list / join wire builders (Takumi <c>PRECEIVE_CHARACTER_LIST</c> / join stub).</summary>
public sealed class CharacterRosterWire
{
    public CharacterRosterWire(ReadOnlySpan<byte> name10, byte serverClass, ushort level)
    {
        this.Name10 = new byte[10];
        var n = Math.Min(10, name10.Length);
        name10[..n].CopyTo(this.Name10);
        this.ServerClass = serverClass;
        this.Level = level;
    }

    public byte[] Name10 { get; }

    public byte ServerClass { get; }

    public ushort Level { get; }
}
