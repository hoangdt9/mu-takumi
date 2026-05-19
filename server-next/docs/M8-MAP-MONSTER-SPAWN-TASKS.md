# M8 × M9 — Đưa quái / NPC vào từng map (spawn parity)

Last updated: 2026-05-19

**Mục tiêu:** Mọi map đích từ **Move map (`0x8E`)** / **gate (`0x1C`)** phải có spawn từ `MonsterSetBase.txt` (hoặc Postgres `monster_spawn`), client thấy **`C2 0x13`** sau `F3 03` / warp.

**QA APK:** [`../../docs/qa/M8-move-map.md`](../../docs/qa/M8-move-map.md) · [`../../docs/qa/M9-monster-combat.md`](../../docs/qa/M9-monster-combat.md)

---

## Nguồn tham chiếu

| Repo / path | Spawn data? | Dùng cho |
|-------------|:-------------:|----------|
| **`takumi/MuServer/4.GameServer/Data`** | ✅ | SSOT file: `MonsterSetBase.txt`, `Monster.txt`, `Move/*.txt` |
| **`Github/OpenMU`** | ✅ (C#) | **Baseline S6:** `sync-all-spawns-from-openmu.py` — `VersionSeasonSix/Maps/*.cs` (+ 095d/075) |
| **`Source Pegasus 5.2/MuServer/Data`** | ✅ | **Denser field boxes:** Lorencia/Devias/Noria/Elbeland — `merge-spawns-from-references.py` |
| **`SRC ThangCuoi/Mu Server/4.Sub-1/Data`** | ✅ | Bổ sung map custom (Vulcanus, Kanturu, …) khi qty > Takumi |
| **`server-next/scripts/spawn/`** | [`scripts/README.md`](../scripts/README.md) | `sync-monster-spawn-stack.sh` = enable + OpenMU + merge + report + Postgres ETL |
| **MuMain-5.2** | ❌ client | `WSclient.cpp` — viewport `C2 0x13` / destroy `0x14` (wire QA only) |
| **muonline-xulek / muonline-bernat-main** | ❌ client | MonoGame client — không có `MonsterSetBase` |
| **muonline-bmd-viewer** | ❌ tools | BMD/terrain viewer — không spawn GS |

**Lệnh gộp (trên Mac, cần OpenMU + Pegasus + ThangCuoi checkout):**

```bash
cd server-next
./scripts/sync-monster-spawn-stack.sh   # hoặc từng bước:
# ./scripts/sync-all-spawns-from-openmu.py
# ./scripts/merge-spawns-from-references.py
# ./scripts/report-spawn-sources.py
docker compose --profile gamehost restart game-host
```

**Docker:** không set `TAKUMI_MONSTER_SET_BASE_PATH` host path trong `.env` — compose dùng `/muserver-data/Monster/*.txt`.

---

## Trạng thái runtime (đã có)

- [x] Load `MonsterSetBase.txt` + `Monster.txt`
- [x] ETL Postgres `monster_spawn` (`005_monster_spawn.sql`, `import-monster-spawn.sh`)
- [x] Spawn instance theo `map_id`, viewport `C2 0x13` join / walk / warp (`MoveWarpJoinReload`)
- [x] Startup log **`[m8-m9] monster spawn coverage`** (`MapMonsterSpawnCoverage`)

---

## Task P0 — Data trên mọi môi trường

| # | Task | | Ghi chú |
|---|------|:-:|---------|
| P0.1 | Mount `TAKUMI_GAMESERVER_DATA_HOST` → `/muserver-data` | [x] | `docker-compose.yml` |
| P0.2 | `TAKUMI_MONSTER_SET_BASE_PATH` / `MONSTER_INFO_PATH` | [x] | |
| P0.3 | Chạy `./scripts/import-monster-spawn.sh` (hoặc `import-world-static-data.sh`) | [x] | `./scripts/sync-monster-spawn-stack.sh`; bật `TAKUMI_MONSTER_SPAWN_DB=1` nếu dùng DB |
| P0.4 | Log startup: không còn “Lorencia fallback only” trên Docker prod | [ ] | Chỉ fallback khi thiếu file |
| P0.5 | `./scripts/report-monster-spawn-coverage.sh` pass | [x] | Unit + in ra map counts |

---

## Task P1 — Map đích Move / Gate có quái

| # | Task | | Ghi chú |
|---|------|:-:|---------|
| P1.1 | Mỗi `Move.txt` row → `Gate` → `map_id` có `field > 0` | [x] | Map 0–3, **1, 4, 7, 8**; gate map 79 NPC-only |
| P1.2 | Noria (3), Devias (2), Lorencia (0) smoke | [~] | Devias section 1 từ OpenMU075; QA: [`M8-MOVE-WARP-MONSTER-QA.md`](./M8-MOVE-WARP-MONSTER-QA.md) |
| P1.3 | Dungeon / Atlans / LT / Tarkan / Icarus / Aida | [x] | `enable-move-map-field-spawns.sh` — maps **1, 4, 7, 8, 10, 33** |
| P1.3b | Elbeland / Kanturu / Vulcanus / Karutan / Crywolf / Swamp / Refuge | [x] | `enable-move-map-field-spawns.sh` + `append-move-map-spawns-from-openmu.py` — maps **34, 37, 42, 51, 56, 63, 80, 81**; gate **335/344** trong `Gate.txt` |
| P1.4 | LorenMarket gate 333 → map 79 | [x] | NPC 545–547 (OpenMU: no field mobs) |
| P1.5 | Drift vs **OpenMU** SeasonSix | [~] | `./scripts/compare-spawn-openmu.sh` |

**Lưu ý Noria:** `MonsterSetBase` map 3 ~18 dòng (7 NPC + 8 spot section 1); viewport chỉ gửi ~4–8 entity trong range — bình thường nếu đứng trong town.

---

## Task P2 — Sau warp (M8)

| # | Task | | |
|---|------|:-:|---|
| P2.1 | `MoveWarpJoinReload` gửi `F3 03` + `F3 10` + `0x13` | [x] | |
| P2.2 | `tracker.ResetForMap` trên warp | [x] | |
| P2.3 | Gate `0x1C` cùng pipeline | [x] | |
| P2.4 | Death respawn map khác vẫn có NPC (`PlayerVitalsLoop`) | [x] | `MapRespawnCatalog` |

---

## Task P3 — Client (MuMain / takumi)

| # | Task | | |
|---|------|:-:|---|
| P3.1 | `Receive` monster list `0x52` vs viewport `0x13` | [x] | Khác opcode |
| P3.2 | Không nhầm guild stub / shop UI | [x] | Xem guild fix 2026-05-18 |
| P3.3 | Android: tap move không nuốt joystick | [x] | `android_main.cpp` hitbox tròn |

---

## Verify dev

```bash
cd server-next
export TAKUMI_GAMESERVER_DATA_HOST="../MuServer/4.GameServer/Data"
docker compose up -d game-host
docker compose logs game-host | grep -E '\[m9\]|\[m8-m9\]'

./scripts/report-monster-spawn-coverage.sh
./scripts/smoke-m8.sh --no-recreate
```

Kỳ vọng log:

```text
[m9] loaded MonsterSetBase … entries from /muserver-data/…
[m9] MapMonsterWorld ready: … instances on … maps
[m8-m9] monster spawn coverage: …
[m8-m9]   map   3: total=  18 npc=   7 field=  11
```

Warp Noria → `[m9] sent C2 0x13 monster viewport (join) count=… map=3`

### Data patch (2026-05-19)

**Bước 1** — uncomment section 1 có sẵn:

```bash
./scripts/enable-move-map-field-spawns.sh
```

**Bước 2** — thêm spot từ OpenMU SeasonSix (map chưa có section 1):

```bash
python3 ./scripts/append-move-map-spawns-from-openmu.py
```

| Map | Tên | Field qty (approx) |
|-----|-----|---------------------|
| 1 | Dungeon | 75+ |
| 4 | Lost Tower | 295+ |
| 7 | Atlans | 170+ |
| 8 | Tarkan | 133+ |
| 10 | Icarus | 103+ |
| 33 | Aida | 135+ |
| 34 | Crywolf | 865 (OpenMU) |
| 37 | Kanturu ruins | 307 |
| 42 | Balgass Refuge | 45 |
| 51 | Elbeland | 38 |
| 56 | Swamp of Calmness | 1410 (OpenMU) |
| 63 | Vulcanus | 192 |
| 80–81 | Karutan | 60 each |

**NPC-only by design:** map **30** (Valley of Loren siege), **79** (Loren Market) — không WARN trong `[m8-m9]` log.

Sau đổi data: `docker compose restart game-host` (hoặc `import-monster-spawn.sh` nếu `TAKUMI_MONSTER_SPAWN_DB=1`).

### OpenMU full sync (2026-05-19)

```bash
./scripts/sync-monster-spawn-stack.sh   # enable + sync-all-spawns-from-openmu + import Postgres
```

- `sync-all-spawns-from-openmu.py --min-ratio 1.0` — thay section-1 các map còn thiếu so với OpenMU (Kanturu, Raklion, Chaos Castle, …).
- Map town **0,2,3,8** vẫn từ `MonsterSetBase` gốc + Devias patch (chuỗi kế thừa OpenMU 075 chưa export tự động).
- Chuyển Mac: [`DEV-MAC-SERVER-MIGRATION.md`](./DEV-MAC-SERVER-MIGRATION.md).

---

## Liên kết

- [`M8-MOVE-MAP-PARITY-CHECKLIST.md`](./M8-MOVE-MAP-PARITY-CHECKLIST.md) · P4.3 Monster scope  
- [`M9-NPC-MONSTER-CHECKLIST.md`](./M9-NPC-MONSTER-CHECKLIST.md)  
- [`M8-M10-WORLD-RUNTIME-CHECKLIST.md`](./M8-M10-WORLD-RUNTIME-CHECKLIST.md)
