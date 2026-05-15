using System.Buffers.Binary;

namespace Takumi.Server.Protocol;

/// <summary>Client gameplay TX parsers (gate teleport, NPC talk, finish loading).</summary>
public static class ClientGameplayPackets602
{
    public static bool TryFindTeleportRequest(ReadOnlySpan<byte> packet, out int frameOffset, out ushort gate, out byte x, out byte y)
    {
        frameOffset = -1;
        gate = 0;
        x = 0;
        y = 0;
        for (var i = 0; i <= packet.Length - 7; i++)
        {
            if (packet[i] is not (0xC1 or 0xC3))
            {
                continue;
            }

            var headOff = packet[i] == 0xC1 ? i + 2 : i + 2;
            if (headOff >= packet.Length || packet[headOff] != 0x1C)
            {
                continue;
            }

            var payload = headOff + 1;
            if (payload + 4 > packet.Length)
            {
                continue;
            }

            frameOffset = i;
            gate = BinaryPrimitives.ReadUInt16LittleEndian(packet.Slice(payload));
            x = packet[payload + 2];
            y = packet[payload + 3];
            return true;
        }

        return false;
    }

    public static bool TryFindNpcTalkRequest(ReadOnlySpan<byte> packet, out int frameOffset, out int objectKey)
    {
        frameOffset = -1;
        objectKey = -1;
        for (var i = 0; i <= packet.Length - 5; i++)
        {
            if (packet[i] is not (0xC1 or 0xC3))
            {
                continue;
            }

            var headOff = i + 2;
            if (headOff >= packet.Length || packet[headOff] != 0x30)
            {
                continue;
            }

            var payload = headOff + 1;
            if (payload + 2 > packet.Length)
            {
                continue;
            }

            frameOffset = i;
            objectKey = (packet[payload] << 8) | packet[payload + 1];
            return true;
        }

        return false;
    }

    public static bool TryFindFinishLoadingRequest(ReadOnlySpan<byte> packet, out int frameOffset)
    {
        frameOffset = -1;
        for (var i = 0; i <= packet.Length - 4; i++)
        {
            if (packet[i] is not (0xC1 or 0xC3))
            {
                continue;
            }

            var headOff = i + 2;
            if (headOff + 1 >= packet.Length)
            {
                continue;
            }

            if (packet[headOff] == 0xF3 && packet[headOff + 1] == 0x12)
            {
                frameOffset = i;
                return true;
            }
        }

        return false;
    }

    public static bool TryFindBuyRequest(ReadOnlySpan<byte> packet, out int frameOffset, out byte shopSlot)
    {
        frameOffset = -1;
        shopSlot = 0;
        for (var i = 0; i <= packet.Length - 4; i++)
        {
            if (packet[i] is not (0xC1 or 0xC3))
            {
                continue;
            }

            var headOff = i + 2;
            if (headOff >= packet.Length || packet[headOff] != 0x32)
            {
                continue;
            }

            frameOffset = i;
            shopSlot = packet[headOff + 1];
            return true;
        }

        return false;
    }

    public static bool TryFindSellRequest(ReadOnlySpan<byte> packet, out int frameOffset, out byte inventorySlot)
    {
        frameOffset = -1;
        inventorySlot = 0;
        for (var i = 0; i <= packet.Length - 4; i++)
        {
            if (packet[i] is not (0xC1 or 0xC3))
            {
                continue;
            }

            var headOff = i + 2;
            if (headOff >= packet.Length || packet[headOff] != 0x33)
            {
                continue;
            }

            frameOffset = i;
            inventorySlot = packet[headOff + 1];
            return true;
        }

        return false;
    }

    public static bool TryFindRepairRequest(ReadOnlySpan<byte> packet, out int frameOffset, out byte slot, out byte repairType)
    {
        frameOffset = -1;
        slot = 0;
        repairType = 0;
        for (var i = 0; i <= packet.Length - 5; i++)
        {
            if (packet[i] is not (0xC1 or 0xC3))
            {
                continue;
            }

            var headOff = i + 2;
            if (headOff + 1 >= packet.Length || packet[headOff] != 0x34)
            {
                continue;
            }

            frameOffset = i;
            slot = packet[headOff + 1];
            repairType = packet[headOff + 2];
            return true;
        }

        return false;
    }

    public static bool TryFindBuyConfirmRequest(ReadOnlySpan<byte> packet, out int frameOffset, out byte shopSlot)
    {
        frameOffset = -1;
        shopSlot = 0;
        for (var i = 0; i <= packet.Length - 5; i++)
        {
            if (packet[i] is not (0xC1 or 0xC3))
            {
                continue;
            }

            var headOff = i + 2;
            if (headOff + 2 >= packet.Length)
            {
                continue;
            }

            if (packet[headOff] == ShopBuyConfirmWire602.Head && packet[headOff + 1] == ShopBuyConfirmWire602.Sub)
            {
                frameOffset = i;
                shopSlot = packet[headOff + 2];
                return true;
            }
        }

        return false;
    }

    public const int ItemMoveFrameLength = 19;

    /// <summary>Main inventory + equipment (<c>fromStorage</c>/<c>toStorage</c> = 0 on Android).</summary>
    public static bool IsSupportedItemStorage(byte storageFlag) => storageFlag == 0;

    public static bool TryFindItemPickRequest(ReadOnlySpan<byte> packet, out int frameOffset, out ushort mapItemIndex)
    {
        frameOffset = -1;
        mapItemIndex = 0;
        for (var i = 0; i <= packet.Length - 5; i++)
        {
            if (packet[i] is not (0xC1 or 0xC3))
            {
                continue;
            }

            var headOff = i + 2;
            if (headOff + 2 >= packet.Length || packet[headOff] != 0x22)
            {
                continue;
            }

            frameOffset = i;
            mapItemIndex = (ushort)((packet[headOff + 1] << 8) | packet[headOff + 2]);
            return true;
        }

        return false;
    }

    public static bool TryFindItemDropRequest(ReadOnlySpan<byte> packet, out int frameOffset, out byte x, out byte y, out byte slot)
    {
        frameOffset = -1;
        x = 0;
        y = 0;
        slot = 0;
        for (var i = 0; i <= packet.Length - 6; i++)
        {
            if (packet[i] is not (0xC1 or 0xC3))
            {
                continue;
            }

            var headOff = i + 2;
            if (headOff + 3 >= packet.Length || packet[headOff] != 0x23)
            {
                continue;
            }

            frameOffset = i;
            x = packet[headOff + 1];
            y = packet[headOff + 2];
            slot = packet[headOff + 3];
            return true;
        }

        return false;
    }

    public static bool TryFindItemMoveRequest(
        ReadOnlySpan<byte> packet,
        out int frameOffset,
        out byte sourceFlag,
        out byte sourceSlot,
        out byte targetFlag,
        out byte targetSlot)
    {
        frameOffset = -1;
        sourceFlag = 0;
        sourceSlot = 0;
        targetFlag = 0;
        targetSlot = 0;
        for (var i = 0; i <= packet.Length - ItemMoveFrameLength; i++)
        {
            if (packet[i] != 0xC1)
            {
                continue;
            }

            var len = packet[i + 1];
            if (len < ItemMoveFrameLength || i + len > packet.Length)
            {
                continue;
            }

            var headOff = i + 2;
            if (packet[headOff] != 0x24)
            {
                continue;
            }

            frameOffset = i;
            sourceFlag = packet[headOff + 1];
            sourceSlot = packet[headOff + 2];
            targetFlag = packet[headOff + 15];
            targetSlot = packet[headOff + 16];
            return true;
        }

        return false;
    }

    public static bool TryFindShopExitRequest(ReadOnlySpan<byte> packet, out int frameOffset)
    {
        frameOffset = -1;
        for (var i = 0; i <= packet.Length - 3; i++)
        {
            if (packet[i] == 0xC1 && packet[i + 1] == 0x03 && packet[i + 2] == 0x31)
            {
                frameOffset = i;
                return true;
            }
        }

        return false;
    }
}
