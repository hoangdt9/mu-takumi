namespace Takumi.Server.Protocol;

/// <summary>Client melee hit / targeted skill (Takumi Android <c>SendHitRequest</c> / <c>SendTargetedSkill*</c>).</summary>
public static class ClientHitPackets602
{
    public const byte HitHeadCode = 0x11;
    public const byte SkillHeadCode = 0x19;

    /// <summary><c>C1 07 11 targetId[BE] attackAnim dir</c>.</summary>
    public static bool TryFindHitRequest(
        ReadOnlySpan<byte> packet,
        out int frameOffset,
        out ushort targetId,
        out byte attackAnimation,
        out byte lookingDirection)
    {
        frameOffset = -1;
        targetId = 0;
        attackAnimation = lookingDirection = 0;
        for (var i = 0; i <= packet.Length - 7; i++)
        {
            if (packet[i] != 0xC1 || packet[i + 1] != 0x07 || packet[i + 2] != HitHeadCode)
            {
                continue;
            }

            frameOffset = i;
            targetId = ReadUInt16Be(packet, i + 3);
            attackAnimation = packet[i + 5];
            lookingDirection = packet[i + 6];
            return true;
        }

        return false;
    }

    /// <summary>Targeted skill: <c>C1/C3 … 19 … targetId[BE]</c> (last two payload bytes).</summary>
    public static bool TryFindTargetedSkill(
        ReadOnlySpan<byte> packet,
        out int frameOffset,
        out ushort targetId)
    {
        frameOffset = -1;
        targetId = 0;
        for (var i = 0; i <= packet.Length - 6; i++)
        {
            var kind = packet[i];
            if (kind is not (0xC1 or 0xC3))
            {
                continue;
            }

            var len = packet[i + 1];
            if (len < 6 || i + len > packet.Length || packet[i + 2] != SkillHeadCode)
            {
                continue;
            }

            var targetOff = i + len - 2;
            frameOffset = i;
            targetId = ReadUInt16Be(packet, targetOff);
            return true;
        }

        return false;
    }

    /// <summary><c>C1 … 0xDB skill[BE] x y serial count (key[BE] skillSerial)*</c>.</summary>
    public static bool TryFindMagicAttack(
        ReadOnlySpan<byte> packet,
        out int frameOffset,
        out ushort skillId,
        out byte x,
        out byte y,
        out List<ushort> targetIds)
    {
        frameOffset = -1;
        skillId = 0;
        x = y = 0;
        targetIds = [];
        for (var i = 0; i <= packet.Length - 8; i++)
        {
            if (packet[i] != 0xC1 || packet[i + 2] != 0xDB)
            {
                continue;
            }

            var len = packet[i + 1];
            if (len < 8 || i + len > packet.Length)
            {
                continue;
            }

            skillId = ReadUInt16Be(packet, i + 3);
            x = packet[i + 5];
            y = packet[i + 6];
            var count = packet[i + 8];
            var need = 9 + count * 3;
            if (len < need)
            {
                continue;
            }

            frameOffset = i;
            for (var t = 0; t < count; t++)
            {
                var off = i + 9 + t * 3;
                targetIds.Add(ReadUInt16Be(packet, off));
            }

            return true;
        }

        return false;
    }

    static ushort ReadUInt16Be(ReadOnlySpan<byte> packet, int offset) =>
        (ushort)((packet[offset] << 8) | packet[offset + 1]);
}
