using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>Parity MU: after death, respawn at town safe zone (not at corpse tile).</summary>
public static class MapRespawnCatalog
{
    public static JoinMapSpawnWire GetTownRespawn(byte mapId) =>
        mapId switch
        {
            0 => JoinMapSpawnWire.LorenciaDefault,
            _ => JoinMapSpawnWire.LorenciaDefault with { Map = mapId },
        };
}
