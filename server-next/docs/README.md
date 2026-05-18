# `server-next` docs — **dev only**

Parity, wire, Docker. **QA APK:** [`../../docs/QA-MILESTONE.md`](../../docs/QA-MILESTONE.md) · [`../../docs/qa/`](../../docs/qa/README.md)

| Doc | Mục đích |
|-----|----------|
| [IMPLEMENTATION-CHECKLIST.md](./IMPLEMENTATION-CHECKLIST.md) | Tổng tiến độ |
| [M8-MOVE-MAP-PARITY-CHECKLIST.md](./M8-MOVE-MAP-PARITY-CHECKLIST.md) | Move map `0x8E` + gate/skill TP |
| [M8-MAP-MONSTER-SPAWN-TASKS.md](./M8-MAP-MONSTER-SPAWN-TASKS.md) | Spawn quái/NPC theo map + Move đích |
| [M8-MOVE-WARP-MONSTER-QA.md](./M8-MOVE-WARP-MONSTER-QA.md) | QA warp Move + viewport `[m9]` |
| [M9-NPC-MONSTER-CHECKLIST.md](./M9-NPC-MONSTER-CHECKLIST.md) | Monster scope A |
| [M9-MONSTER-AI-PORT-CHECKLIST.md](./M9-MONSTER-AI-PORT-CHECKLIST.md) | AI / viewport port |
| [M9-M8-NPC-GAMEPLAY-OWNERSHIP.md](./M9-M8-NPC-GAMEPLAY-OWNERSHIP.md) | File ownership (đừng duplicate) |
| [M8-M10-WORLD-RUNTIME-CHECKLIST.md](./M8-M10-WORLD-RUNTIME-CHECKLIST.md) | M8–M10 gom |
| [M6-GAME-TCP-CHECKLIST.md](./M6-GAME-TCP-CHECKLIST.md) | Game TCP |
| [DOCKER-BUILD-RUN.md](./DOCKER-BUILD-RUN.md) | Stack |

**Verify:** `./scripts/smoke-m8.sh --no-recreate` · `dotnet test` (filter trong smoke scripts)

**Redirect QA (đừng thêm checklist APK ở đây):** [M8-MOVE-MAP-QA-CHECKLIST.md](./M8-MOVE-MAP-QA-CHECKLIST.md) · [S2-GATE-QA.md](./S2-GATE-QA.md)
