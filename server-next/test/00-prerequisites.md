# 00 — Prerequisites (trước khi test M7–M10)

## Checklist

- [ ] **1. Wi‑Fi:** Điện thoại và Mac cùng mạng LAN (ví dụ `192.168.1.x`).
- [ ] **2. APK:** Đã build & cài `realDevicePreloadDefaultDebug` (ABI `armeabi-v7a` + `arm64-v8a`).
  - **LAN:** BuildConfig khớp `.env` — `MU_BOOTSTRAP_SERVER=<LAN>:44605`, `DATA_ZIP_URL_LAN=http://<LAN>:18080/data.zip`.
  - **USB adb reverse (không rebuild APK mỗi lần đổi server):** build **một lần** với `-PmuBootstrapAdbReverse=true`, mỗi session chạy `./scripts/android/adb-reverse.sh`, `.env` dùng `TAKUMI_PUBLIC_HOST=127.0.0.1` + recreate stack.
- [ ] **3. `server-next/.env`:** Copy từ `.env.lan.example` nếu chưa có; `TAKUMI_PUBLIC_HOST` = IP Mac.
- [ ] **4. `keys/Dec2.dat`:** Có trong `server-next/keys/` (mount vào container `/keys/Dec2.dat`).
- [ ] **5. Tắt stack khác** tránh trùng cổng:
  - OpenMU / `takumi-openmu-*` (44505, …)
  - Không chạy `./scripts/host/run-legacy-login-host.sh` trên host khi Docker đã bind `44605`/`44606`.
- [ ] **6. (Tuỳ chọn M8 DB)** Có file MuServer trên máy:
  - `…/4.GameServer/Data/Monster/MonsterSetBase.txt`
  - `…/4.GameServer/Data/Monster/Monster.txt`
  - `…/4.GameServer/Sub 1/Data/Move/Gate.txt` (gates ETL)
- [ ] **7. adb:** `adb devices` thấy thiết bị khi test logcat.

## Bật feature flags trong `.env` (khuyến nghị cho QA đầy đủ)

Thêm hoặc bỏ comment (host `dotnet`/container đọc qua `RepoEnvLoader`):

```bash
TAKUMI_VERBOSE=1
TAKUMI_STRUCTURED_LOG=1

# M4b + M7 vitals/inventory mirror
TAKUMI_ROSTER_DB_SYNC=1
TAKUMI_ROSTER_DB_PRIMARY=1
TAKUMI_CHARACTER_DOMAIN_SYNC=1
TAKUMI_PG_CONNECTION_STRING=Host=postgres;Port=5432;Username=takumi;Password=takumi;Database=takumi_runtime

# M7 — đẩy hết nhân vật trong takumi-roster/*.json lên DB (một lần hoặc mỗi lần start)
# ./scripts/db/migrate-roster-json-to-db.sh
# TAKUMI_MIGRATE_ROSTER_JSON=1

# M7d — gửi 0x26/0x27 sau join
TAKUMI_SEND_LIFE_MANA_AFTER_JOIN=1

# M8 — spawn/gates/shops từ Postgres (sau ETL)
TAKUMI_MONSTER_SPAWN_DB=1
TAKUMI_WORLD_STATIC_DB=1
# Đường dẫn trên HOST (import script); trong container dùng auto-detect hoặc mount thêm
TAKUMI_MONSTER_SET_BASE_PATH=/path/on/host/.../Monster/MonsterSetBase.txt
TAKUMI_GAMESERVER_DATA_PATH=/path/on/host/.../4.GameServer/Sub 1/Data

# M6 split (APK F4 03 → 55901) — đã có trong .env mẫu của bạn
TAKUMI_GAME_PORT=55901
TAKUMI_GAME_PUBLISH=55901
```

**Lưu ý Docker:** `legacy-login` bind-mount cả repo → `RepoEnvLoader` đọc `/app/.env`. Biến `TAKUMI_PG_CONNECTION_STRING` trong container nên dùng `Host=postgres`, không `127.0.0.1`.

## Pass criteria

Tất cả mục trên tick → chuyển **[01-docker-stack.md](./01-docker-stack.md)**.
