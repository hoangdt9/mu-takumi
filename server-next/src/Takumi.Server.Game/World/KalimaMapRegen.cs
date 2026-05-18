namespace Takumi.Server.Game.World;

/// <summary>Kalima maps regen monsters at death tile (parity attribute maps 24–29, 36).</summary>
public static class KalimaMapRegen
{
    public static bool IsKalimaMap(byte mapId) =>
        mapId is 24 or 25 or 26 or 27 or 28 or 29 or 36;
}
