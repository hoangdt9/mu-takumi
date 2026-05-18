# M8 × M9 — Đưa quái / NPC vào từng map (spawn parity)

Last updated: 2026-05-18

**Mục tiêu:** Mọi map đích từ **Move map (`0x8E`)** / **gate (`0x1C`)** phải có spawn từ `MonsterSetBase.txt` (hoặc Postgres `monster_spawn`), client thấy **`C2 0x13`** sau `F3 03` / warp.

**QA APK:** [`../../docs/qa/M8-move-map.md`](../../docs/qa/M8-move-map.md) · [`../../docs/qa/M9-monster-combat.md`](../../docs/qa/M9-monster-combat.md)

---

## Nguồn tham chiếu

| Repo / path | Dùng cho |
|-------------|----------|
| **`mu-takumi/MuServer/4.GameServer/Data`** | `Monster/MonsterSetBase.txt`, `Monster/Monster.txt`, `Move/Move.txt`, `Move/Gate.txt` |
| **`mu-takumi/server-next`** | `MapMonsterWorld`, `MonsterSetBaseLoader`, `MapMonsterScopeSender`, `MoveWarpJoinReload` |
| **OpenMU** (`OpenMU/src`) | `MonsterSpawnArea`, map definitions (XML/DB); admin map editor |
| **MuMain-5.2** | `WSclient.cpp` — `Receive*` viewport `0x13` / destroy `0x14` |
| **muonline-xulek** | So sánh `MonsterSetBase` / GS data nếu drift season |
| **Legacy** `Source/4.GameServer` | `MonsterSetBase.cpp`, `MonsterManager.cpp`, `Viewport.cpp` |

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
| P0.3 | Chạy `./scripts/import-monster-spawn.sh` (hoặc `import-world-static-data.sh`) | [ ] | Bật `TAKUMI_MONSTER_SPAWN_DB=1` nếu dùng DB |
| P0.4 | Log startup: không còn “Lorencia fallback only” trên Docker prod | [ ] | Chỉ fallback khi thiếu file |
| P0.5 | `./scripts/report-monster-spawn-coverage.sh` pass | [ ] | Unit + in ra map counts |

---

## Task P1 — Map đích Move / Gate có quái

| # | Task | | Ghi chú |
|---|------|:-:|---------|
| P1.1 | Mỗi `Move.txt` row → `Gate` → `map_id` có `field > 0` | [~] | WARN trong log nếu thiếu |
| P1.2 | Noria (3), Devias (2), Lorencia (0) smoke | [ ] | QA: warp + thấy mob ngoài safe zone |
| P1.3 | Dungeon / Atlans / LT / Tarkan theo level gate | [ ] | Map 8+ đã có hàng trong set-base |
| P1.4 | LorenMarket (48) / event maps | [ ] | Gate 333 → map 79? verify Gate.txt |
| P1.5 | Đồng bộ drift vs **OpenMU** spawn XML (tùy season) | [ ] | Không bắt buộc P0 |

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

---

## Liên kết

- [`M8-MOVE-MAP-PARITY-CHECKLIST.md`](./M8-MOVE-MAP-PARITY-CHECKLIST.md) · P4.3 Monster scope  
- [`M9-NPC-MONSTER-CHECKLIST.md`](./M9-NPC-MONSTER-CHECKLIST.md)  
- [`M8-M10-WORLD-RUNTIME-CHECKLIST.md`](./M8-M10-WORLD-RUNTIME-CHECKLIST.md)
