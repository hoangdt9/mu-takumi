namespace Takumi.Server.Protocol;

/// <summary>
/// Client walk / instant-move wire used by Takumi Android (<c>SendWalkRequest</c> / <c>SendInstantMoveRequest</c>).
/// End tile decoding matches OpenMU CharacterWalkBaseHandlerPlugIn walk decode + path deltas.
/// Decoded X/Y are **tile indices** (stored as <see cref="byte"/> in roster / join spawn); see <c>docs/M4-TILE-AND-COORDINATES.md</c>.
/// </summary>
public static class ClientWalkPackets602
{
    /// <summary>Instant move: <c>C1 05 15 tx ty</c>.</summary>
    public static bool TryFindInstantMove(ReadOnlySpan<byte> packet, out int frameOffset, out byte targetX, out byte targetY)
    {
        frameOffset = -1;
        targetX = targetY = 0;
        for (var i = 0; i <= packet.Length - 5; i++)
        {
            if (packet[i] != 0xC1 || packet[i + 1] != 0x05 || packet[i + 2] != 0x15)
            {
                continue;
            }

            frameOffset = i;
            targetX = packet[i + 3];
            targetY = packet[i + 4];
            return true;
        }

        return false;
    }

    /// <summary>
    /// Walk: <c>C1 [len] 0xD4|0x10(075) srcX srcY (stepLo4|rotHi4) [dir nibbles packed]</c>.
    /// When <paramref name="changedPosition"/> is false (zero steps), <paramref name="endX"/>/<paramref name="endY"/> are still the source tile.
    /// </summary>
    public static bool TryFindWalkEndTile(
        ReadOnlySpan<byte> packet,
        out int frameOffset,
        out byte endX,
        out byte endY,
        out byte angle1To8,
        out bool changedPosition)
    {
        frameOffset = -1;
        endX = endY = 0;
        angle1To8 = 1;
        changedPosition = false;
        for (var i = 0; i <= packet.Length - 6; i++)
        {
            if (packet[i] != 0xC1)
            {
                continue;
            }

            var len = packet[i + 1];
            if (len < 6 || i + len > packet.Length)
            {
                continue;
            }

            var code = packet[i + 2];
            if (code is not (0xD4 or 0x10))
            {
                continue;
            }

            var srcX = packet[i + 3];
            var srcY = packet[i + 4];
            var meta = packet[i + 5];
            var stepCount = meta & 0x0F;
            var rotNib = (byte)((meta >> 4) & 0x0F);
            angle1To8 = (byte)Math.Clamp(rotNib + 1, 1, 8);

            var x = (int)srcX;
            var y = (int)srcY;
            if (stepCount > 0)
            {
                var dirByteCount = len - 6;
                var needed = (stepCount + 1) / 2;
                if (dirByteCount < needed)
                {
                    continue;
                }

                for (var s = 0; s < stepCount; s++)
                {
                    var b = packet[i + 6 + (s >> 1)];
                    var nib = (s & 1) == 0 ? (b >> 4) & 0x0F : b & 0x0F;
                    StepTile(ref x, ref y, nib);
                }

                changedPosition = true;
            }

            endX = (byte)x;
            endY = (byte)y;
            frameOffset = i;
            return true;
        }

        return false;
    }

    static void StepTile(ref int x, ref int y, int dirNibble)
    {
        // OpenMU Direction = dirNibble + 1; deltas from GameLogic DirectionExtensions.CalculateTargetPoint
        var d = dirNibble + 1;
        switch (d)
        {
            case 1: // West
                x--;
                y--;
                break;
            case 2: // SouthWest
                y--;
                break;
            case 3: // South
                x++;
                y--;
                break;
            case 4: // SouthEast
                x++;
                break;
            case 5: // East
                x++;
                y++;
                break;
            case 6: // NorthEast
                y++;
                break;
            case 7: // North
                x--;
                y++;
                break;
            case 8: // NorthWest
                x--;
                break;
        }

        x = Math.Clamp(x, 0, 255);
        y = Math.Clamp(y, 0, 255);
    }
}
