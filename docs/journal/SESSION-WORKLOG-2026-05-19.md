# Session worklog — 2026-05-19

Tóm tắt công việc QA Android + `server-next` (Lorencia spawn, damage màu, model quái, scripts). Cập nhật sau phiên chat / deploy.

**Tiếp theo (20/05):** skill combat mobile MG — [MOBILE-SKILL-COMBAT-GUIDE.md](./MOBILE-SKILL-COMBAT-GUIDE.md) · [DEVELOPMENT-LOG-2026-05-20.md](./journal/DEVELOPMENT-LOG-2026-05-20.md).

## Đã hoàn thiện (trong repo)

### 1. `server-next/scripts/` — tổ chức lại thư mục

| Trạng thái | Nội dung |
|-----------|----------|
| Done | Chia `docker/`, `db/`, `android/`, `host/`, `smoke/`, `spawn/`, `_lib/paths.sh` |
| Done | Wrapper tương thích ngược: `./scripts/docker/docker-stack.sh` → `docker/docker-stack.sh` |
| Done | `scripts/README.md`, cập nhật `M8-MAP-MONSTER-SPAWN-TASKS.md`, `docker-compose.yml` boot path |
| Done | Python spawn: `spawn/sync-all-spawns-from-openmu.py`, `merge-spawns-from-references.py`, … |

### 2. Combat — màu damage (Excellent / Critical / …)

| Trạng thái | Nội dung |
|-----------|----------|
| Done | **Root cause:** `MonsterDamageWire602` bit 15 (`stuckFlag`) = damage đỏ “player bị đánh” trên client (`ReceiveAttackDamage`). |
| Done | Player → monster: `stuckFlag: false`; monster/PvP → player: `stuckFlag: true`. |
| Done | `MonsterCombatCalculator.ApplyClientDamageTypeMultiplier()` (+20% excellent, +30% critical, ×2 double, +50% combo). |
| Done | Tests: `MonsterCombatWire602Tests`, `MonsterCombatCalculatorTests` (20/20 pass khi chạy filter). |
| **Deploy** | `cd server-next && ./scripts/docker/docker-stack.sh --host-build --recreate --detach` |

**Bằng chứng server (log):** `dmgType=0x02`, `0x03`, `0x80` trên hit; không còn mọi hit `0x00` + stuck bit.

### 3. Lorencia / field spawn — góc lag (server)

| Trạng thái | Nội dung |
|-----------|----------|
| Done | `MonsterSpawnResolver`: kiểm tra `CanWalk` khi có ATT; fallback từ **tâm** hộp spawn, không góc `(180,90)`; `spreadKey` rải instance trong hộp Pegasus `(180,90)–(226,244)`. |
| Done | `MapMonsterWorld.RebuildInstances()` truyền `fieldSpreadKey` cho field mob. |
| Done | Test `MonsterSpawnResolverLorenciaTests` (cần `TAKUMI_ATT_DATA_ROOT` trên máy dev). |
| **Deploy** | Cùng lệnh recreate `game-host` ở trên. |

### 4. Data patches — model Rồng Con / Nhện (client)

| Trạng thái | Nội dung |
|-----------|----------|
| Done | `assets/data-patches/Monster/Monster03.bmd` (~88 KB, class 2), `Monster04.bmd` (~166 KB, class 3) từ MuMain-5.2. |
| Done | `scripts/sync-data-patches-android.sh` — `adb push` khi `DEV_SKIP_DATA_ZIP`. |
| Done | `assets/data-patches/README.md`, `docs/migration/DATA-ZIP-MERGE-PLAN.md` (bảng size + verify). |
| **Thiết bị** | `./scripts/sync-data-patches-android.sh` → thoát app → `adb shell ls -la .../Monster/Monster03.bmd Monster04.bmd` |

### 5. World leaf patches (character select / map)

| Trạng thái | Nội dung |
|-----------|----------|
| Done | `World1`, `World38`, `World58`, `World74`, `World75` — OZT/OZJ thiếu trong zip S20 (xem `DATA-ZIP-MERGE-PLAN.md`). |

---

## Còn dở / cần QA sau deploy

### A. Mob Kanturu (353/354) ở góc ~(212,38) khi UI ghi Lorencia

| Mức | Ghi chú |
|-----|---------|
| Open | Logcat: batch đầu `type=353` tại `(212,38)` — spawn **map 37** Kanturu, không phải Lorencia. |
| Nguyên nhân khả dĩ | `mg001` lưu `map_id=37` hoặc XY góc Kanturu trong Postgres; viewport server đúng map DB, client/terrain có thể lệch. |
| Việc làm | Reset QA: `apply-sql.sh … sql/patches/013_test_account_mg001_seed.sql` (map 0, 130,125); thoát game trước khi apply. |
| Việc làm | Log server join: `sent join map (F3 03) map=… xy=(…)`; so với `ReceiveJoinMapServer` trên Android. |

### B. Model nhện / rồng con vẫn sai trên máy

| Mức | Ghi chú |
|-----|---------|
| Open | Chỉ fix sau khi patch BMD **thực sự** trên device (skip zip không tự merge). |
| Verify | `Monster03.bmd` ~87994, `Monster04.bmd` ~166065 bytes trên phone. |

### C. Crash Android khi disconnect

| Mức | Ghi chú |
|-----|---------|
| Open | `FORTIFY: pthread_mutex_lock called on a destroyed mutex` sau `Connection closed` (pid 7687). |
| Việc làm | Repro + audit thread shutdown (`AndroidNetwork`, render vs network). **Chưa commit fix client.** |

### D. Spawn / data pipeline (dài hạn)

| Mức | Ghi chú |
|-----|---------|
| Open | OpenMU block Lorencia trống trong `MonsterSetBase.txt` — bổ sung qua `sync-monster-spawn-stack.sh`. |
| Open | `TAKUMI_MONSTER_SPAWN_DB=1` (Postgres) đang tắt trong `.env` — spawn từ file. |
| Open | `MonsterSetBase.txt` local diff chưa review — không gộp vào commit này nếu chưa sync có chủ đích. |

### E. Client UI / Crywolf / warehouse (WIP local)

| Mức | Ghi chú |
|-----|---------|
| WIP | Nhiều file `Source/5.Main/source/NewUI*`, `CBInterface*`, `GMCrywolf*` — **chưa đưa vào commit session này** (ngoài phạm vi spawn/damage/data). |

---

## Lệnh deploy / QA nhanh

```bash
# Server
cd server-next
./scripts/docker/docker-stack.sh --host-build --recreate --detach

# Android patches (DEV_SKIP_DATA_ZIP)
cd ..   # repo root takumi
./scripts/sync-data-patches-android.sh

# Reset vị trí mg001 (tùy chọn)
cd server-next
./scripts/db/apply-sql.sh "$TAKUMI_PG_CONNECTION_STRING" sql/patches/013_test_account_mg001_seed.sql

# Log
./scripts/android/watch-android-takumi-log.sh
```

---

## File then chốt (commit session)

- `server-next/scripts/**` (reorg + spawn)
- `server-next/src/.../MonsterSpawnResolver.cs`, `MapMonsterWorld.cs`, combat/damage wire
- `server-next/src/Takumi.Server.Tests/*Combat*`, `*Spawn*`, `*Att*`
- `assets/data-patches/**`, `scripts/sync-data-patches-android.sh`
- `docs/migration/DATA-ZIP-MERGE-PLAN.md`, `docs/journal/SESSION-WORKLOG-2026-05-19.md`

**Không gộp (trừ khi commit riêng):** thay đổi client UI lớn, `MonsterSetBase.txt` chưa review, `__pycache__`.
