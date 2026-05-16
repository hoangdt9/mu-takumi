using System.Buffers.Binary;

namespace Takumi.Server.Protocol;

/// <summary>Season 6 <c>PMSG_LIFE_SEND</c> (0x26) / <c>PMSG_MANA_SEND</c> (0x27) — Takumi <c>PRECEIVE_LIFE</c> / <c>ReceiveMana</c>.</summary>
public static class LifeManaWire602
{
    public const byte HeadLife = 0x26;
    public const byte HeadMana = 0x27;
    public const byte TypeCurrent = 0xFF;
    public const byte TypeMax = 0xFE;

    /// <summary>Legacy 9-byte life (no View DWORDs). Client <c>ReceiveLife</c> does not read View fields.</summary>
    public const int PacketLengthLifeLegacy = 9;

    /// <summary>Takumi client always reads <c>ViewHP</c>/<c>ViewSD</c> on mana 0xFE even when size is 9 — use extended wire.</summary>
    public const int PacketLengthLife = 17;

    public const int PacketLengthMana = 16;

    public static byte[] BuildLife(byte type, ushort value, ushort shield = 0)
    {
        var p = new byte[PacketLengthLife];
        p[0] = 0xC1;
        p[1] = PacketLengthLife;
        p[2] = HeadLife;
        p[3] = type;
        p[4] = (byte)(value >> 8);
        p[5] = (byte)(value & 0xFF);
        p[6] = 0;
        p[7] = (byte)(shield >> 8);
        p[8] = (byte)(shield & 0xFF);
        BinaryPrimitives.WriteUInt32LittleEndian(p.AsSpan(9), value);
        BinaryPrimitives.WriteUInt32LittleEndian(p.AsSpan(13), shield);
        return p;
    }

    public static byte[] BuildMana(byte type, ushort mana, ushort bp = 0)
    {
        var p = new byte[PacketLengthMana];
        p[0] = 0xC1;
        p[1] = PacketLengthMana;
        p[2] = HeadMana;
        p[3] = type;
        p[4] = (byte)(mana >> 8);
        p[5] = (byte)(mana & 0xFF);
        p[6] = (byte)(bp >> 8);
        p[7] = (byte)(bp & 0xFF);
        // ReceiveMana 0xFE always assigns ViewMaxMP/ViewMaxBP from these DWORDs (no size guard).
        BinaryPrimitives.WriteUInt32LittleEndian(p.AsSpan(8), mana);
        BinaryPrimitives.WriteUInt32LittleEndian(p.AsSpan(12), bp);
        return p;
    }

    /// <summary>Scan outbound buffer for embedded C1 life/mana frames (M7d).</summary>
    public static bool TryApplyVitalsFromOutbound(
        ReadOnlySpan<byte> outbound,
        ref int currentHp,
        ref int maxHp,
        ref int currentMp,
        ref int maxMp,
        ref int currentShield,
        ref int maxShield)
    {
        var changed = false;
        for (var i = 0; i < outbound.Length; i++)
        {
            if (outbound[i] != 0xC1 || i + 4 > outbound.Length)
            {
                continue;
            }

            var size = outbound[i + 1];
            if (size < PacketLengthLifeLegacy || i + size > outbound.Length)
            {
                continue;
            }

            var head = outbound[i + 2];
            var type = outbound[i + 3];
            var v = (ushort)((outbound[i + 4] << 8) | outbound[i + 5]);
            var shieldWord = size >= PacketLengthLifeLegacy
                ? (ushort)((outbound[i + 7] << 8) | outbound[i + 8])
                : (ushort)0;
            var bpWord = head == HeadMana && size >= 8
                ? (ushort)((outbound[i + 6] << 8) | outbound[i + 7])
                : (ushort)0;

            if (head == HeadLife)
            {
                if (type == TypeCurrent && (currentHp != v || currentShield != shieldWord))
                {
                    currentHp = v;
                    currentShield = shieldWord;
                    changed = true;
                }
                else if (type == TypeMax && (maxHp != v || maxShield != shieldWord))
                {
                    maxHp = v;
                    maxShield = shieldWord;
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

                _ = bpWord;
            }

            i += size - 1;
        }

        return changed;
    }
}
