# M9b — Monster AI & viewport port — **dev only**

Last updated: 2026-05-18

**QA APK:** [`../../docs/qa/M9-monster-combat.md`](../../docs/qa/M9-monster-combat.md) · **Scope A:** `M9-NPC-MONSTER-CHECKLIST.md`

**Legacy:** `Monster.cpp`, `Viewport.cpp`, `Map.cpp` · **Code:** `Takumi.Server.Game/World/*`

**Verify dev:** log `[m9-ai]`, `[m9-vp]`, `[m9] sent C2 0x13`

| Symbol | Meaning |
|--------|---------|
| [x] | Done |
| [~] | Partial |
| [ ] | Not started |

---

## P0 — Viewport & lifecycle

| # | Item | Status | Notes |
|---|------|:------:|-------|
| P0.1 | Spawn `0x13` join + walk | [x] | `MapMonsterScopeSender` |
| P0.2 | NPC-first sort | [x] | |
| P0.3 | Destroy `0x14` leave view | [x] | |
| P0.4 | Periodic rescan ~1s | [x] | `TAKUMI_MONSTER_VIEWPORT_PERIODIC_MS` |
| P0.5 | Regen broadcast | [x] | |
| P0.6 | Regen at spawn tile | [x] | Kalima: death tile (P3.5) |
| P0.7 | Skip spawn types 3/4 | [x] | `TAKUMI_MONSTER_INCLUDE_INVASION_SPAWN=1` |

---

## P1 — Monster AI

| # | Item | Status |
|---|------|:------:|
| P1.1 | AI tick loop | [x] |
| P1.2 | Wander | [x] |
| P1.3 | Walk `0xD4` | [x] |
| P1.4 | Attack `0x18` | [x] |
| P1.5 | Chase | [x] |
| P1.6 | Monster → player dmg | [x] |
| P1.7 | Player → monster + die | [x] |
| P1.8 | Miss / defense | [x] |
| P1.9 | Death broadcast | [x] |
| P1.10 | Hit aggro | [x] |

---

## P2 — ATT / path

| # | Item | Status |
|---|------|:------:|
| P2.1 | `Terrain.att` | [x] |
| P2.2 | NoMove / NoGround | [x] |
| P2.3 | Pathfinder step | [x] |
| P2.4 | Safe zone no aggro | [x] |
| P2.5 | Encrypted `EncTerrain*.att` | [x] |

---

## P3 — Advanced (còn lại)

| # | Item | Status |
|---|------|:------:|
| P3.1 | Monster magic `0xDB` / `0x19` | [x] |
| P3.2 | Element / resist | [x] | `MonsterCombatCalculator` + stat cols |
| P3.3 | Party exp / top damage | [x] | `TOP_DAMAGE_GRANT_EXP=0` off; `PARTY_EXP_SHARE=1` proportional |
| P3.4 | Invasion spawns 3/4 | [x] | default skip; `TAKUMI_MONSTER_INCLUDE_INVASION_SPAWN=1` |
| P3.5 | Kalima regen | [x] | maps 24–29, 36 @ death tile |
| P3.6 | PvP player damage | [x] | `PlayerCombatRules`, range/safe zone, `RollDamagePlayerToPlayer` |

---

## P4 — NPC / gate / shop (shared M8)

| # | Item | Status | Notes |
|---|------|:------:|-------|
| P4.1 | Shop `0x31` | [x] | |
| P4.2 | Gate `0x1C` + skill `gate==0` | [x] | `MapGateService`, `SkillTeleportService` |
| P4.3 | Buy/sell/repair | [x] | |
| P4.4 | Quest NPC stub | [x] | `C1 A0` mask + `A1` state; `TAKUMI_QUEST_NPC_DEFAULT_STATE` |

---

## Env (tham khảo)

| Variable | Default |
|----------|---------|
| `TAKUMI_MONSTER_AI_INTERVAL_MS` | 500 |
| `TAKUMI_MONSTER_VIEWPORT_PERIODIC_MS` | 1000 |
| `TAKUMI_MONSTER_VIEW_RANGE` | 15 |
| `TAKUMI_ATT_DATA_ROOT` | `/att-data` in Docker |

Full list: bảng env cũ trong git history hoặc `MapMonsterScopeSender` / `MonsterAiLoop` source.

---

## Dev tiếp (ưu tiên)

1. **M11+** — quest logic đầy đủ (accept/reward), warehouse/guild NPC.
2. PvP: equipment-based defense, duel map rules.

**Baseline P0–P3.6 + P4.1–P4.4: xong (dev stub).**
