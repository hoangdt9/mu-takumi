namespace Takumi.Server.Protocol;

/// <summary>Season 6 skill list (<c>C1 F3 11</c> / <c>GCSkillListSend</c>).</summary>
public static class MagicListWire602
{
    public const byte ListTypeMaster = 0;
    public const byte ListTypeNormal = 1;

    /// <summary>Empty skill list (client accepts count=0).</summary>
    public static byte[] BuildEmpty(byte listType = ListTypeNormal)
    {
        var buf = new byte[7];
        buf[0] = 0xC1;
        buf[1] = 7;
        buf[2] = 0xF3;
        buf[3] = 0x11;
        buf[4] = listType;
        buf[5] = 0;
        return buf;
    }
}
