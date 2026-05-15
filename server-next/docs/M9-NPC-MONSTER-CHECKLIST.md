# M9 — NPC & monster runtime (scope A)

Last updated: 2026-05-16

## Scope A (this iteration)

- [x] Load **`MonsterSetBase.txt`** (parity `CMonsterSetBase::Load` / `GetPosition`).
- [x] Load **`Monster.txt`** for minimal **Life/Level** (`MonsterStatCatalog`).
- [x] Spawn static rows per **map**; filter by **view range** around player join tile.
- [x] Send **`C2 0x13`** (`MonsterViewportWire602`) after **`F3 03` + `F3 10`** on login/game TCP.
- [x] **Incremental viewport on walk / instant move** (`MonsterViewportTracker`, `TrySendOnMoveAsync`).
- [x] **Regen timer** from `Monster.txt` `RegenTime` (`MapMonsterInstance.TryRegen` on viewport scan).
- [x] **Combat stub** — `C1 0x11` hit / `0x19` skill → damage, `MarkDead`, `C1 0x16` die, `C1 0x14` destroy.
- [x] **Destroy on leave view** — `SyncView` + `C1 0x14` when walk out of range (parity `DestroyViewportMonster1`).
- [x] **Damage vs Defense** — `MonsterCombatCalculator` + `Monster.txt` `Defense` column.
- [ ] **Full combat** (AoE, PvP) — **M10c** (xem **`WORKSTREAM-OWNERSHIP.md`**).
- [ ] **M10 partial (WIP trên `mac-m4`):** map presence `0x15`/`0x18`, miss, skill % — chưa merge `main`.
- [ ] **AI** / NPC shop rows — post-M9 (shop data **main** `8c1758b`; wire chưa).
- [x] **M8 ETL** spawn Postgres — trên **`main`** (`b33d890`); `MapMonsterWorld` fallback file khi DB trống.

## Legacy reference (`Source/4.GameServer`)

| Area | File |
|------|------|
| Set base | `MonsterSetBase.cpp` |
| Stats | `MonsterManager.cpp` → `Monster.txt` |
| Viewport TX | `Viewport.cpp` → `GCViewportMonsterSend` (head `0x13`) |
| Client RX | `WSclient.cpp` → `ReceiveCreateMonsterViewport` |

## `server-next` code

| Component | Path |
|-----------|------|
| Loaders / world | `Takumi.Server.Game/World/*` |
| Wire | `Takumi.Server.Protocol/MonsterViewportWire602.cs` |
| Hook (game port) | `GamePortMinimalSession.cs` |
| Hook (legacy login) | `LegacyLoginHost.Runner/LegacyLoginHostRunner.cs` |

## Env

| Variable | Default |
|----------|---------|
| `TAKUMI_MONSTER_SET_BASE_PATH` | search `Monster/MonsterSetBase.txt` under cwd / `MuServer/4.GameServer/Data` |
| `TAKUMI_MONSTER_INFO_PATH` | `Monster/Monster.txt` |
| `TAKUMI_MONSTER_VIEW_RANGE` | `15` (Manhattan tiles) |
| `TAKUMI_MONSTER_VIEWPORT_MAX` | `64` monsters per packet |
| `TAKUMI_MONSTER_VIEWPORT_MOVE_TILES` | `4` Manhattan tiles before rescan on walk |
| `TAKUMI_COMBAT_STUB_DAMAGE` | `50` damage per hit |
| `TAKUMI_COMBAT_MELEE_RANGE` | `3` tiles (Manhattan) |

If set-base file is missing, a small **Lorencia fallback** spawn set is used for QA.

## QA

1. Point env to real `MuServer/4.GameServer/Data/Monster/MonsterSetBase.txt`.
2. Login → select character → enter world.
3. Host log: `[m9] sent C2 0x13 monster viewport count=…`
4. Client logcat: `0x13 [ReceiveCreateMonsterViewport`
5. Tap attack near a mob (≤3 tiles): host `[m9] combat hit … died=True` then `C1 0x14 destroy + C1 0x16 die`.
6. Client: damage numbers, mob disappears; after regen delay, walk back into range → new `0x13` spawn.
7. Walk away from mob → host `[m9] sent C1 0x14 destroy viewport`; walk back → new `0x13`.
