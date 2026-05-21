# `server-next` docs — dev only

Parity, wire, Docker. **QA APK:** [`../../docs/QA-MILESTONE.md`](../../docs/QA-MILESTONE.md) · [`../../docs/qa/`](../../docs/qa/README.md)

## Cấu trúc

| Thư mục | Nội dung |
|---------|----------|
| [`milestones/`](milestones/) | Tiến độ tổng (`IMPLEMENTATION-CHECKLIST`, ownership) |
| [`character/`](character/) | M4–M7, M14 — roster, join, game TCP, persistence |
| [`world/`](world/) | M8 — move-map, spawn, world runtime |
| [`combat/`](combat/) | M9 — monster/NPC combat, AI port, gameplay ownership |
| [`items/`](items/) | M12 — item system |
| [`social/`](social/) | M11 — warehouse, trade, guild, skills stub |
| [`protocol/`](protocol/) | M1, M3, login wire |
| [`docker/`](docker/) | Stack Docker, dev Mac migration |
| [`client/`](client/) | MonoGame client roadmap |
| [`qa-gates/`](qa-gates/) | Gate QA dev (redirect → `docs/qa/` cho APK) |

## Checklist nhanh

| Doc | Mục đích |
|-----|----------|
| [milestones/IMPLEMENTATION-CHECKLIST.md](./milestones/IMPLEMENTATION-CHECKLIST.md) | Tổng tiến độ |
| [world/M8-MOVE-MAP-PARITY-CHECKLIST.md](./world/M8-MOVE-MAP-PARITY-CHECKLIST.md) | Move map `0x8E` + gate/skill TP |
| [world/M8-MAP-MONSTER-SPAWN-TASKS.md](./world/M8-MAP-MONSTER-SPAWN-TASKS.md) | Spawn quái/NPC theo map |
| [combat/M9-NPC-MONSTER-CHECKLIST.md](./combat/M9-NPC-MONSTER-CHECKLIST.md) | Monster scope A |
| [../../docs/android/SKILL-COMBAT-CHECKLIST.md](../../docs/android/SKILL-COMBAT-CHECKLIST.md) | Skill hit volume + MG QA done/chưa |
| [combat/M9-MONSTER-AI-PORT-CHECKLIST.md](./combat/M9-MONSTER-AI-PORT-CHECKLIST.md) | AI / viewport port |
| [combat/M9-M8-NPC-GAMEPLAY-OWNERSHIP.md](./combat/M9-M8-NPC-GAMEPLAY-OWNERSHIP.md) | File ownership |
| [character/M6-GAME-TCP-CHECKLIST.md](./character/M6-GAME-TCP-CHECKLIST.md) | Game TCP |
| [docker/DOCKER-BUILD-RUN.md](./docker/DOCKER-BUILD-RUN.md) | Stack Docker |

`scripts/spawn/enable-move-map-field-spawns.sh` — bật section 1 quái map trong MonsterSetBase.

**Verify:** `./scripts/smoke/smoke-m8.sh --no-recreate` · `dotnet test` (filter trong smoke scripts)

**QA gates (không thêm checklist APK ở đây):** [qa-gates/M8-MOVE-MAP-QA-CHECKLIST.md](./qa-gates/M8-MOVE-MAP-QA-CHECKLIST.md) · [qa-gates/S2-GATE-QA.md](./qa-gates/S2-GATE-QA.md) → APK: [`../../docs/qa/`](../../docs/qa/)
