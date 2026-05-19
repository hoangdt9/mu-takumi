# Chuyển stack Takumi sang Mac khác

Hướng dẫn chạy `server-next` (Docker: Postgres + legacy-login + game-host) trên máy Mac mới mà không mất roster / spawn / cấu hình.

---

## 1. Clone & cấu trúc thư mục

```bash
git clone https://github.com/hoangdt9/mu-takumi.git
cd mu-takumi

# OpenMU cần cho sync spawn (script đọc map initializers)
git clone https://github.com/MUnique/OpenMU.git ../OpenMU
# hoặc: export OPENMU_MAPS_DIR=/path/to/OpenMU
```

Cây tối thiểu:

```text
parent/
  mu-takumi/          # repo này
  OpenMU/             # optional nhưng cần cho sync-all-spawns-from-openmu.py
  docker/data-zip/    # data.zip cho Android (nếu dùng profile datazip)
```

---

## 2. File cấu hình (phải tạo lại trên Mac mới)

| File | Việc làm |
|------|----------|
| `server-next/.env` | Copy từ `.env.lan.example`; đặt `TAKUMI_LAN_IP` / `TAKUMI_PUBLIC_HOST` = IP LAN **của Mac mới** |
| `server-next/keys/Dec2.dat` | Copy từ máy cũ hoặc client `Data/Dec2.dat` (SimpleModulus) |
| `MuServer/4.GameServer/Data` | Có trong git; sau `git pull` đã có `MonsterSetBase.txt` đã sync |

**Không commit:** `.env`, password thật, `takumi-roster/*.json` (gitignore).

---

## 3. Postgres — hai cách

### A) Volume Docker mới (mặc định, DB trống)

```bash
cd server-next
docker compose up -d postgres
# Lần đầu: sql/init/*.sql chạy tự động (roster, monster_spawn, …)
./scripts/apply-sql.sh "$TAKUMI_PG_CONNECTION_STRING" sql/patches/016_test1_dk400_blade_master_seed.sql  # QA nếu cần
```

### B) Mang volume / dump từ Mac cũ

```bash
# Mac cũ — backup
docker compose exec -T postgres pg_dump -U takumi takumi_runtime > takumi_runtime.dump

# Mac mới — restore (volume đã có DB hoặc drop/create trước)
cat takumi_runtime.dump | docker compose exec -T postgres psql -U takumi takumi_runtime
```

Port mặc định host: **54444** (`TAKUMI_POSTGRES_PUBLISH_PORT`).

---

## 4. Đồng bộ quái (file + Postgres)

**Spawn trong git** (`MonsterSetBase.txt`) — sau `git pull` thường **đủ**, chỉ cần restart container.

**Tái tạo / bổ sung từ OpenMU** (khi đổi season hoặc thiếu map):

```bash
cd server-next
export TAKUMI_PG_CONNECTION_STRING='postgresql://takumi:takumi@127.0.0.1:54444/takumi_runtime'
export TAKUMI_MONSTER_SET_BASE_PATH='../MuServer/4.GameServer/Data/Monster/MonsterSetBase.txt'

./scripts/sync-monster-spawn-stack.sh
```

Hoặc từng bước:

```bash
./scripts/enable-move-map-field-spawns.sh
python3 ./scripts/sync-all-spawns-from-openmu.py --min-ratio 0.5
./scripts/import-monster-spawn.sh
```

**Bật đọc spawn từ Postgres** (thay vì chỉ file):

Trong `server-next/.env`:

```bash
TAKUMI_MONSTER_SPAWN_DB=1
```

`game-host` / `legacy-login` sẽ load `monster_spawn` sau khi đã `import-monster-spawn.sh`.  
Đổi `MonsterSetBase.txt` → chạy lại import + restart.

---

## 5. Khởi động stack

```bash
cd server-next
cp -n .env.lan.example .env   # chỉnh IP
docker compose up -d          # postgres + legacy-login + game-host (+ datazip nếu profile bật)
docker compose restart game-host legacy-login

# Android QA cùng máy
./scripts/adb-reverse-takumi-dev.sh
```

Kiểm tra spawn:

```bash
./scripts/report-monster-spawn-coverage.sh
docker compose logs game-host | grep -E '\[m9\]|\[m8-m9\]|move-map destinations'
```

Kỳ vọng: `[m9] loaded MonsterSetBase …` và `move-map destinations: all resolved gates have field spawns`.

---

## 6. Checklist Mac mới (tóm tắt)

| # | Việc |
|---|------|
| 1 | `git pull` mu-takumi (và OpenMU nếu sync spawn) |
| 2 | `.env` với IP LAN Mac mới |
| 3 | `Dec2.dat` trong `server-next/keys/` |
| 4 | `docker compose up -d` |
| 5 | DB: init tự động **hoặc** restore dump |
| 6 | (Tuỳ chọn) `./scripts/sync-monster-spawn-stack.sh` + `TAKUMI_MONSTER_SPAWN_DB=1` |
| 7 | Seed QA: `apply-sql.sh` patch 016 test1 |
| 8 | `adb-reverse` + rebuild APK nếu test Android |

---

## 7. Lưu ý thường gặp

- **IP đổi** → sửa `TAKUMI_PUBLIC_HOST` / `TAKUMI_LAN_IP`; client F4 03 trỏ IP mới.
- **Postgres volume trống** → roster/inventory mất; dùng dump hoặc seed SQL/JSON.
- **`MonsterSetBase` chỉ trên git** — Mac mới không cần copy tay nếu đã pull commit mới nhất.
- **`monster_spawn` trong DB** — không tự sync git; cần `import-monster-spawn.sh` sau mỗi lần sửa file spawn (khi `TAKUMI_MONSTER_SPAWN_DB=1`).
- **OpenMU path** — nếu không clone OpenMU, set `OPENMU_MAPS_DIR` trỏ đúng checkout.
- **Loren Deep (30) / Loren Market (79)** — không có quái field (đúng OpenMU); không báo lỗi.

---

## Liên kết

- [`M8-MAP-MONSTER-SPAWN-TASKS.md`](./M8-MAP-MONSTER-SPAWN-TASKS.md)
- [`M4-ROSTER-SSOT.md`](./M4-ROSTER-SSOT.md)
- `server-next/.env.lan.example`
