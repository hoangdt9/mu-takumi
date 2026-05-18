using Takumi.Server.Protocol;

namespace Takumi.Server.Game.World;

/// <summary>Parity MU: after death, respawn at the current map's town safe tile (not at corpse tile).</summary>
/// <remarks>
/// Each world map has its own town / gate anchor. Using Lorencia tile XY on other maps (e.g. Noria)
/// places the player in the field and feels like "revive on the spot".
/// Values follow <c>MuServer/Data/Move/Gate.txt</c> and OpenMU NPC town rings where applicable.
/// </remarks>
public static class MapRespawnCatalog
{
    public static JoinMapSpawnWire GetTownRespawn(byte mapId) =>
        mapId switch
        {
            0 => JoinMapSpawnWire.LorenciaDefault,
            // Dungeon: Lorencia↔Dungeon gates (Gate.txt #2–4), near entrance tile.
            1 => new JoinMapSpawnWire(1, 107, 247, 1),
            // Devias: classic town square (matches common GS defaults).
            2 => new JoinMapSpawnWire(2, 183, 32, 1),
            // Noria: vendor ring (OpenMU NPC 242 @ 173,125 — not Lorencia 135,122 on map 3).
            3 => new JoinMapSpawnWire(3, 173, 125, 1),
            // Lost Tower: ground floor hub (Gate.txt #32–33).
            4 => new JoinMapSpawnWire(4, 166, 164, 3),
            // Atlans: entrance safe band (Gate.txt #49 center ~21,17).
            7 => new JoinMapSpawnWire(7, 21, 17, 3),
            // Tarkan: main gate band (Gate.txt #57 center ~195,61).
            8 => new JoinMapSpawnWire(8, 195, 61, 1),
            // Icarus: lower platform gate (Gate.txt #64 center ~15,12).
            10 => new JoinMapSpawnWire(10, 15, 12, 1),
            _ => JoinMapSpawnWire.LorenciaDefault with { Map = mapId },
        };
}
