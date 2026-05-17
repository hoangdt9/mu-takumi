using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>Destination-map equip rules from <c>CMove::Move</c> (Atlans / Icarus / Kanturu3).</summary>
public static class MoveMapEquipRules
{
    public const byte MapAtlans = 7;
    public const byte MapIcarus = 10;
    public const byte MapKanturu3 = 39;

    const int ItemUniria = (13 * 512) + 2;
    const int ItemDinorant = (13 * 512) + 3;
    const int ItemFenrir = (13 * 512) + 37;

    const byte WearWingsSlot = 7;
    const byte WearPetSlot = 8;

    public static bool BlocksWarpToMap(byte destinationMapId, Guid presenceSessionId, out bool wearingWrongEquip)
    {
        wearingWrongEquip = false;
        if (destinationMapId == MapAtlans)
        {
            wearingWrongEquip = BlocksAtlans(presenceSessionId);
            return wearingWrongEquip;
        }

        if (destinationMapId is MapIcarus or MapKanturu3)
        {
            return BlocksSkyMap(presenceSessionId);
        }

        return false;
    }

    static bool BlocksAtlans(Guid presenceSessionId)
    {
        if (!PlayerShopSession.TryGetSlot(presenceSessionId, WearPetSlot, out var pet)
            || ItemWire602.IsEmpty(pet))
        {
            return false;
        }

        var idx = ItemWire602.DecodeItemIndex(pet);
        return idx == ItemUniria || idx == ItemDinorant;
    }

    static bool BlocksSkyMap(Guid presenceSessionId)
    {
        var hasWings = PlayerShopSession.TryGetSlot(presenceSessionId, WearWingsSlot, out var wings)
                       && !ItemWire602.IsEmpty(wings);
        if (hasWings)
        {
            return false;
        }

        if (!PlayerShopSession.TryGetSlot(presenceSessionId, WearPetSlot, out var pet)
            || ItemWire602.IsEmpty(pet))
        {
            return true;
        }

        var idx = ItemWire602.DecodeItemIndex(pet);
        return idx != ItemDinorant && idx != ItemFenrir;
    }
}
