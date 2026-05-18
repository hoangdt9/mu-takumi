# M8 — Move map parity (`CMove::Move` + `0x8E`) — **dev only**

**Phạm vi:** `server-next` + `./scripts/smoke-m8.sh` / unit. **QA APK:** [`../../docs/qa/M8-move-map.md`](../../docs/qa/M8-move-map.md) · [`../../docs/QA-MILESTONE.md`](../../docs/QA-MILESTONE.md)

**Quy ước:** `[x]` merge + smoke/unit; `[~]` một phần; `[ ]` chưa; `[-]` N/A

**Verify:** `./scripts/smoke-m8.sh --no-recreate` · catalog: `./scripts/smoke-m8-move-catalog.sh`

### Code map (chính)

| Vùng | Type / file |
|------|-------------|
| Data | `MoveLoader`, `MoveMapCatalog`, `GateLoader`, `MapGateCatalog` |
| `0x8E` | `MoveMapHandler`, `MoveMapOutbound`, `MoveMapWire602`, `MoveMapKeyGenerator` |
| Rules | `MoveMapService`, `MoveMapEquipRules`, `CustomArenaScheduleFsm` |
| Warp | `MoveWarpJoinReload`, `MoveMapPostWarp`, `TeleportWire602` |
| Gate / skill | `MapGateService`, `SkillTeleportService`, `WorldGameplayHandlers` |
| UI block | `PlayerUiSession` |

### `MuServer` = data tĩnh (không chạy GS C++)

| Thành phần | Vai trò |
|------------|---------|
| **`server-next`** | `game-host` / `legacy-login` |
| **`MuServer/4.GameServer/Data`** | `Move.txt`, `Gate.txt`, … |
| **Mount** | `TAKUMI_GAMESERVER_DATA_HOST` → `/muserver-data` |

**Legacy:** `Move.cpp` · GS cũ **không** `8E 01`/key — tắt: `TAKUMI_MOVE_MAP_SKIP_KEY_CHECK=1`

---

## P0 — Data & Docker

| # | Hạng mục | | Ghi chú |
|---|----------|:-:|---------|
| P0.1 | `Move/Move.txt` | [x] | `MoveMapCatalog` |
| P0.2 | `Move/Gate.txt` | [x] | `MapGateCatalog` |
| P0.3 | `TAKUMI_GAMESERVER_DATA_PATH` | [x] | compose / `.env.lan.example` |
| P0.4 | `TAKUMI_MOVE_PATH` override | [x] | |
| P0.5 | ETL Postgres (tùy chọn) | [x] | `import-world-static-data.sh` |
| P0.6 | Startup log N moves | [x] | `[m8] MoveMapCatalog` |
| P0.7 | Smoke + unit | [x] | `smoke-m8.sh` |

---

## P1 — Wire `0x8E`

| # | Packet | | Ghi chú |
|---|--------|:-:|---------|
| P1.1 | `8E 01` seed S→C | [x] | `MoveMapOutbound` |
| P1.2 | `8E 02` C→S | [x] | key + index |
| P1.3 | `8E 03` result | [x] | `MoveMapWire602` |
| P1.4 | Block key | [x] | `SKIP_KEY_CHECK=1` |
| P1.5 | Re-seed sau warp | [x] | |
| P1.6 | `MAPMOVE_FAILED_*` | [x] | `0x00`–`0x0B` |

---

## P2 — `Move.txt` fields

| # | Trường | Load | Enforce |
|---|--------|:----:|:-------:|
| P2.1–P2.6 | Index, Money, Level, Reset, AccountLevel, Gate | [x] | [x] |

---

## P3 — `CMove::Move` rules

| # | Rule | | |
|---|------|:-:|---|
| P3.1 | Unknown index | [x] | |
| P3.2 | Level | [x] | |
| P3.3 | Reset | [x] | |
| P3.4 | AccountLevel | [x] | |
| P3.5 | Zen | [x] | |
| P3.6 | PK ≥ 5 | [x] | `TAKUMI_PK_LIMIT_FREE=1` |
| P3.7 | UI / teleport / dead / pshop | [x] | `PlayerUiSession` |
| P3.8 | Atlans equip | [x] | |
| P3.9 | Icarus / Kanturu3 | [x] | |
| P3.10 | Gens battle | [x] | |
| P3.11 | Custom arena | [x] | `SKIP_SCHEDULE=0` |
| P3.12 | Zen wire | [x] | |
| P3.13 | PShop clear | [x] | `C2 3F 00` |

---

## P4 — Teleport / join

| # | | |
|---|:-:|---|
| P4.1 | `0x1C` | [x] |
| P4.2 | `F3 03` + `F3 10` | [x] |
| P4.3 | Monster scope | [x] |
| P4.4 | Presence | [x] |
| P4.5 | `SaveRoster` | [x] |

---

## P6 — Ngoài `CMove::Move`

| # | Hạng mục | | Ghi chú |
|---|----------|:-:|---------|
| P6.1 | Gate `0x1C` proximity | [x] | ±5 TX/TY; `GATE_SKIP_PROXIMITY=1` |
| P6.2 | Skill teleport `gate==0` | [x] | `SkillTeleportService`; `SKILL_TELEPORT_SKIP=1` |
| P6.3 | `CustomNpcMove` | [x] | `CustomNpcMoveHandler` + `CustomNpcMove.txt` |
| P6.4 | Multi-GS | [-] | |

**M8 dev (scope file này): xong** (gồm P6.3). Chia sẻ handler với M9: `docs/M9-M8-NPC-GAMEPLAY-OWNERSHIP.md`.

---

## Env

```bash
export TAKUMI_GAMESERVER_DATA_HOST="../MuServer/4.GameServer/Data"
export TAKUMI_GAMESERVER_DATA_PATH="/muserver-data"
./scripts/smoke-m8.sh --no-recreate

# TAKUMI_MOVE_MAP_SKIP_KEY_CHECK=1
# TAKUMI_PK_LIMIT_FREE=1
# TAKUMI_CUSTOM_ARENA_SKIP_SCHEDULE=0
# TAKUMI_GATE_SKIP_PROXIMITY=1
# TAKUMI_SKILL_TELEPORT_SKIP=1
# TAKUMI_SKILL_TELEPORT_MANA=30
# TAKUMI_CUSTOM_NPC_MOVE_PATH=.../CustomNpcMove.txt
# TAKUMI_COMBAT_PVP_ENABLED=1  (default on; set 0 to disable)
# TAKUMI_COMBAT_PARTY_EXP_SHARE=1
```

**Liên kết:** `M8-M10-WORLD-RUNTIME-CHECKLIST.md` · `M1-PROTOCOL-PARITY-MAP.md` § `0x8E`
