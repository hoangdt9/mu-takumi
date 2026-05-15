# M9b — Monster AI & viewport port (legacy `4.GameServer`)

Last updated: 2026-05-16

**Legacy refs:** `Monster.cpp` (`gObjMonsterProcess`, `gObjMonsterRegen`), `MonsterAIUtil.cpp` (`SendMonsterMoveMsg`), `Viewport.cpp` (`GCViewportMonsterSend`, `DestroyViewportMonster1`), `User.cpp` (`gObjViewportListProtocolCreate` ~1s), `Map.cpp` (`PathFinding2/4`).

**Code:** `Takumi.Server.Game/World/*`, `Takumi.Server.Game/Networking/MonsterViewerRegistry.cs`, `Takumi.Server.Protocol/*Wire602.cs`.

---

## Status legend

| Symbol | Meaning |
|--------|---------|
| [x] | Done in `server-next` |
| [~] | Partial / stub |
| [ ] | Not started |

---

## P0 — Viewport & lifecycle (player-visible)

| # | Item | Legacy | Status | `server-next` / env |
|---|------|--------|--------|---------------------|
| P0.1 | Spawn `C2 0x13` after join + on walk threshold | `GCViewportMonsterSend` | [x] | `MapMonsterScopeSender`, `TAKUMI_MONSTER_VIEWPORT_MOVE_TILES` |
| P0.2 | NPC-first viewport + distance sort | viewport create | [x] | `GetViewportEntities`, `MAX_NPC` / `MAX_MOB` |
| P0.3 | Destroy `C1 0x14` when leaving view | `DestroyViewportMonster1` | [x] | `MonsterViewportTracker.SyncView` |
| P0.4 | **Periodic viewport rescan ~1s** | `gObjViewportListProtocolCreate` | [x] | `MonsterViewportPeriodicLoop`, `TAKUMI_MONSTER_VIEWPORT_PERIODIC_MS` (default 1000) |
| P0.5 | **Regen: destroy + respawn for all viewers** | `gObjClearViewport` + `gObjViewportListCreate` | [x] | `MonsterViewportBroadcast.RegenMonsterAsync` on `TryRegen` |
| P0.6 | Regen at spawn tile / set-base position | `gObjMonsterRegen` | [x] | `MapMonsterInstance.ResetToSpawn` |
| P0.7 | Skip spawn types 3/4 at init | `CMonsterManager::SetMonsterData` | [x] | `MapMonsterWorld.RebuildInstances` |

---

## P1 — Monster AI (movement & combat)

| # | Item | Legacy | Status | Notes |
|---|------|--------|--------|-------|
| P1.1 | AI tick loop (~100–500ms) | `ObjectManager` monster timer | [x] | `MonsterAiLoop`, `TAKUMI_MONSTER_AI_INTERVAL_MS` |
| P1.2 | Random wander in `MoveRange` / `Dis` | `MonsterMoveCheck` | [x] | `TryRollWander` |
| P1.3 | Walk broadcast `C1 0xD4` | `PMSG_MOVE_SEND` | [x] | `MonsterWalkWire602` |
| P1.4 | Attack anim `C1 0x18` action 120 | `GCActionSend` | [x] | `PlayerActionWire602` |
| P1.5 | **Aggro + chase toward player** | `TargetNumber`, move to target | [x] | `TryChaseStep`, `AggroTargetKey`, `TAKUMI_MONSTER_AI_CHASE_RANGE` |
| P1.6 | **Monster → player damage** | `gAttack.Attack` | [~] | `0x11` + `0x26` stub via `ApplyMonsterHitToPlayerAsync` (`TAKUMI_MONSTER_TO_PLAYER_DAMAGE`) |
| P1.7 | Player → monster damage + die | `GCDamageSend` / die | [x] | `MonsterCombatHandler` |
| P1.8 | Miss / defense from `Monster.txt` | calc | [x] | `MonsterCombatCalculator` |
| P1.9 | Broadcast monster death to viewers | viewport destroy | [x] | `MonsterViewportBroadcast.BroadcastDestroyAsync` |
| P1.10 | Hit aggro on player attack | `LastAttackerID` | [x] | `MonsterCombatHandler` → `SetAggro` |

---

## P2 — Pathfinding & terrain (ATT)

| # | Item | Legacy | Status | Notes |
|---|------|--------|--------|-------|
| P2.1 | Load `Terrain.att` per map | `MapManager` / `*.att` | [~] | `MapAttWalkability`, `TAKUMI_ATT_DATA_ROOT` (plain ATT; EncTerrain TBD) |
| P2.2 | Block `NoMove` / `NoGround` tiles | `TWFlags` | [x] | `CanWalk(map,x,y)` used in wander/chase |
| P2.3 | PathFinding2/4 multi-step path | `gMap[].PathFinding*` | [ ] | greedy 1-step first; full A* later |
| P2.4 | Safe zone: no mob aggro | attribute | [ ] | needs safe tiles from ATT |
| P2.5 | Encrypted `EncTerrain*.att` | client maps | [ ] | decrypt parity with client |

---

## P3 — Advanced AI & skills

| # | Item | Legacy | Status |
|---|------|--------|--------|
| P3.1 | `MonsterSkillManager` / magic attack | `gObjMonsterMagicAttack` | [ ] |
| P3.2 | Element / resist | `Monster.txt` cols | [ ] |
| P3.3 | Party exp / top damage user | `gObjMonsterGetTopHitDamageUser` | [ ] |
| P3.4 | Invasion / event spawns (type 3/4) | `SetMonsterData` | [ ] |
| P3.5 | Kalima / special regen | `gObjMonsterRegen` branches | [ ] |

---

## P4 — NPC & shops (M9c)

| # | Item | Legacy | Status |
|---|------|--------|--------|
| P4.1 | NPC shop list `0x31` | `NpcTalk` | [ ] |
| P4.2 | Gate teleport `0x1C` | `MoveGate` | [ ] (M8 catalog loaded) |
| P4.3 | Bot buffer / quest NPC scripts | various | [ ] |

---

## Environment (quick reference)

| Variable | Default | Feature |
|----------|---------|---------|
| `TAKUMI_MONSTER_AI_ENABLED` | on | Wander + attack loop |
| `TAKUMI_MONSTER_AI_INTERVAL_MS` | 500 | AI tick |
| `TAKUMI_MONSTER_VIEWPORT_PERIODIC_MS` | 1000 | Legacy 1s viewport; `0` = off |
| `TAKUMI_MONSTER_VIEW_RANGE` | 15 | Manhattan |
| `TAKUMI_MONSTER_VIEWPORT_MAX_NPC` | 32 | |
| `TAKUMI_MONSTER_VIEWPORT_MAX_MOB` | 48 | |
| `TAKUMI_MONSTER_AI_WANDER_PCT` | 28 | |
| `TAKUMI_MONSTER_AI_ATTACK_PCT` | 12 | |
| `TAKUMI_MONSTER_AI_CHASE_RANGE` | 12 | Aggro leash |
| `TAKUMI_MONSTER_AI_ATTACK_RANGE` | 3 | Melee |
| `TAKUMI_MONSTER_TO_PLAYER_DAMAGE` | 15 | Stub dmg per hit |
| `TAKUMI_ATT_DATA_ROOT` | (search) | `Data/<World>/Terrain.att` |
| `TAKUMI_MAP_PRESENCE_ENABLED` | 1 | Player object key for damage |

---

## Suggested implementation order

1. **P0.4 + P0.5** — periodic viewport + regen broadcast (fix “few mobs / no respawn visible”).
2. **P1.5 + P1.6** — chase + player HP drop (game feel).
3. **P1.9 + P1.10** — death broadcast + aggro on hit.
4. **P2.1–P2.3** — ATT walk + better paths.
5. **P3+ / P4** — skills, events, shops.

---

## QA

1. `docker compose` recreate `legacy-login`; log `[m9-ai]` + `[m9-vp]`.
2. Join Lorencia town → many NPCs (`0x13` count ≥ 10).
3. Walk to field → mobs spawn; mobs walk (`0xD4`) and swing (`0x18`).
4. Stand near mob → HP drops (`0x11` on self + `0x26`).
5. Kill mob → `0x14` + `0x16`; wait regen → all clients see destroy + respawn.
6. Idle 5s without moving → periodic rescan still updates viewport (`[m9-vp] periodic`).

See also **`docs/M9-NPC-MONSTER-CHECKLIST.md`** (scope A baseline).
