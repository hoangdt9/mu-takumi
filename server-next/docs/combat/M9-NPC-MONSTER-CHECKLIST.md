# M9 — NPC & monster runtime (scope A) — **dev only**

Last updated: 2026-05-18

**QA APK:** [`../../docs/qa/M9-monster-combat.md`](../../docs/qa/M9-monster-combat.md) · MG skills: [`../../docs/qa/M9-mg-skill-combat.md`](../../docs/qa/M9-mg-skill-combat.md) · **Skill checklist:** [`../../docs/android/SKILL-COMBAT-CHECKLIST.md`](../../docs/android/SKILL-COMBAT-CHECKLIST.md) · shop/gate: [`../../docs/qa/M9-npc-shop.md`](../../docs/qa/M9-npc-shop.md)

**Chi tiết AI:** `M9-MONSTER-AI-PORT-CHECKLIST.md` · **Ownership:** `M9-M8-NPC-GAMEPLAY-OWNERSHIP.md`

---

## Scope A — dev status

- [x] `MonsterSetBase.txt` + `Monster.txt` loaders
- [x] Spawn per map, view range filter
- [x] `C2 0x13` sau `F3 03` + `F3 10`
- [x] Viewport on walk / instant move
- [x] Regen từ `RegenTime`
- [x] Combat stub `0x11` / `0x19` → die / destroy
- [x] Destroy on leave view
- [x] `MonsterCombatCalculator` + Defense column
- [x] M10a presence `0x15` / `0x18`
- [x] **M9b AI:** wander, chase, `0xD4`/`0x18`, mob→player dmg, periodic viewport, regen broadcast
- [x] NPC shop `0x30` → `0x31`
- [x] Gate `0x1C` + skill teleport `gate==0` (`MapGateService`, `SkillTeleportService`)
- [x] Shop commerce `0x32`–`0x34`
- [x] M8 ETL spawn Postgres; file fallback khi DB trống
- [x] **Combat PvP stub** — `PlayerCombatRules`, `TAKUMI_COMBAT_PVP_ENABLED`
- [x] **Party EXP share** — `TAKUMI_COMBAT_PARTY_EXP_SHARE=1`
- [ ] **Full quest NPC** (accept/reward persistence) — M11+

---

## Legacy / code

| Area | `Source/4.GameServer` |
|------|------------------------|
| Set base | `MonsterSetBase.cpp` |
| Viewport | `Viewport.cpp` `0x13` |
| Client | `WSclient.cpp` |

| Component | `server-next` |
|-----------|---------------|
| World | `Takumi.Server.Game/World/*` |
| Wire | `MonsterViewportWire602.cs` |
| Hook | `GamePortMinimalSession.cs` |

---

## Env (dev)

| Variable | Default |
|----------|---------|
| `TAKUMI_MONSTER_SET_BASE_PATH` | auto `MonsterSetBase.txt` |
| `TAKUMI_MONSTER_VIEW_RANGE` | 15 |
| `TAKUMI_MONSTER_VIEWPORT_MOVE_TILES` | 4 |
| `TAKUMI_COMBAT_STUB_DAMAGE` | 50 |
| `TAKUMI_MAP_PRESENCE_ENABLED` | 1 |

Thiếu set-base → Lorencia fallback spawn (dev only, không phải QA checklist).

**Verify dev:** mount data · log `[m9] sent C2 0x13` · `./scripts/smoke/smoke-m8.sh` (gate/move nếu test M8 cùng lúc)
