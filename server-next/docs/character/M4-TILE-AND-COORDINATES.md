# M4 — Tile coordinates vs “world float” (contract)

Last updated: 2026-05-16

## Contract for `server-next` M4

| Layer | Unit | Notes |
|-------|------|--------|
| Client walk / instant move (Season 6) | **Tile indices** `0…255` per axis | `ClientWalkPackets602` decodes **`C1 … 0xD4` / `0x10`** and **`C1 05 15`** to **`byte`** end tiles. |
| Roster JSON + in-memory roster | **`byte` `posX` / `posY`** | Same semantics as wire; persisted in `takumi-roster/<account>.json`. |
| Postgres `character_roster` | **`smallint` columns** storing **tile** values `0…255` | Cast to/from `byte` in `PostgresCharacterRosterRepository` / `CharacterRosterRow`. |
| Join map wire (`JoinMapSpawnWire`, `JoinMapServerWire602`) | **`byte` map + tile + angle** | Matches Takumi `PRECEIVE_JOIN_MAP_SERVER` / client expectations for minimal join. |

**There is no float world position** in the M4 path. MuMain historically mixes tile math with fixed-point in other subsystems; anything beyond **256×256 tile grid per axis** is **out of scope** until a dedicated **M7** physics / world-space milestone (would require new DB columns, wire review, and client parity).

## Out of scope (explicit)

- **Sub-tile / float `(x,y)`** for combat, pickups, or anticheat — not written to roster today.
- **Broadcast / scope** of other players’ tiles — **M6+–M9** (`GameServer`-style listener), not M4.

## References

- `src/Takumi.Server.Protocol/ClientWalkPackets602.cs`
- `src/Takumi.Server.Persistence/CharacterRosterRow.cs`
- `src/Takumi.Server.Protocol/JoinMapSpawnWire.cs`
- `character/M4-WORLD-POSITION-CHECKLIST.md`
