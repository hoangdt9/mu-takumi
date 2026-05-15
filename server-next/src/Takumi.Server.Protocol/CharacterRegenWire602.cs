namespace Takumi.Server.Protocol;

/// <summary>Season 6 <c>GCCharacterRegenSend</c> — <c>C1 F3 04</c>.</summary>
public static class CharacterRegenWire602
{
    public static byte[] Build(
        byte mapId,
        byte x,
        byte y,
        byte dir,
        ushort life,
        ushort mana,
        ushort shield = 0,
        ushort bp = 0)
    {
        var buf = new byte[24];
        buf[0] = 0xC1;
        buf[1] = 24;
        buf[2] = 0xF3;
        buf[3] = 0x04;
        buf[4] = x;
        buf[5] = y;
        buf[6] = mapId;
        buf[7] = dir;
        buf[8] = (byte)(life >> 8);
        buf[9] = (byte)(life & 0xFF);
        buf[10] = (byte)(mana >> 8);
        buf[11] = (byte)(mana & 0xFF);
        buf[12] = (byte)(shield >> 8);
        buf[13] = (byte)(shield & 0xFF);
        buf[14] = (byte)(bp >> 8);
        buf[15] = (byte)(bp & 0xFF);
        return buf;
    }
}
