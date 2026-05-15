using System.Buffers.Binary;

namespace Takumi.Server.Protocol;

/// <summary>Season 6 <c>PMSG_LIFE_SEND</c> (0x26) / <c>PMSG_MANA_SEND</c> (0x27) — Takumi <c>PRECEIVE_LIFE</c> / <c>ReceiveMana</c>.</summary>
public static class LifeManaWire602
{
    public const byte HeadLife = 0x26;
    public const byte HeadMana = 0x27;
    public const byte TypeCurrent = 0xFF;
    public const byte TypeMax = 0xFE;
    public const int PacketLength = 9;

    public static byte[] BuildLife(byte type, ushort value, ushort shield = 0)
    {
        var p = new byte[PacketLength];
        p[0] = 0xC1;
        p[1] = PacketLength;
        p[2] = HeadLife;
        p[3] = type;
        p[4] = (byte)(value >> 8);
        p[5] = (byte)(value & 0xFF);
        p[6] = 0;
        p[7] = (byte)(shield >> 8);
        p[8] = (byte)(shield & 0xFF);
        return p;
    }

    public static byte[] BuildMana(byte type, ushort mana, ushort bp = 0)
    {
        var p = new byte[PacketLength];
        p[0] = 0xC1;
        p[1] = PacketLength;
        p[2] = HeadMana;
        p[3] = type;
        p[4] = (byte)(mana >> 8);
        p[5] = (byte)(mana & 0xFF);
        p[6] = (byte)(bp >> 8);
        p[7] = (byte)(bp & 0xFF);
        p[8] = 0;
        return p;
    }

    /// <summary>Scan outbound buffer for embedded C1 life/mana frames (M7d).</summary>
    public static bool TryApplyVitalsFromOutbound(
        ReadOnlySpan<byte> outbound,
        ref int currentHp,
        ref int maxHp,
        ref int currentMp,
        ref int maxMp)
    {
        var changed = false;
        for (var i = 0; i + PacketLength <= outbound.Length; i++)
        {
            if (outbound[i] != 0xC1 || outbound[i + 1] != PacketLength)
            {
                continue;
            }

            var head = outbound[i + 2];
            var type = outbound[i + 3];
            var v = (ushort)((outbound[i + 4] << 8) | outbound[i + 5]);
            if (head == HeadLife)
            {
                if (type == TypeCurrent && currentHp != v)
                {
                    currentHp = v;
                    changed = true;
                }
                else if (type == TypeMax && maxHp != v)
                {
                    maxHp = v;
                    changed = true;
                }
            }
            else if (head == HeadMana)
            {
                if (type == TypeCurrent && currentMp != v)
                {
                    currentMp = v;
                    changed = true;
                }
                else if (type == TypeMax && maxMp != v)
                {
                    maxMp = v;
                    changed = true;
                }
            }
        }

        return changed;
    }
}
