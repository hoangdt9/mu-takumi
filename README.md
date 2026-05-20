# MU Takumi — server toolkit & OpenMU migration

Private/dev fork of the Takumi MU stack focused on:

- Migration path to **OpenMU** (.NET, Linux/macOS/Docker) — see `docs/migration/TAKUMI-MIGRATION-OPENMU-CHECKLIST.md`.
- Baseline manifests and protocol/network notes — `docs/`, `docs/manifests/`.

## Tài liệu migration (OpenMU)

- [`docs/migration/TAKUMI-MIGRATION-OPENMU-CHECKLIST.md`](docs/migration/TAKUMI-MIGRATION-OPENMU-CHECKLIST.md) — phases + gate.
- [`docs/migration/TAKUMI-FULL-FILE-MIGRATION-CHECKLIST.md`](docs/migration/TAKUMI-FULL-FILE-MIGRATION-CHECKLIST.md) — inventory file + manifest.
- [`docs/protocol/TAKUMI-SERVER-NETWORK-BASELINE.md`](docs/protocol/TAKUMI-SERVER-NETWORK-BASELINE.md) — cổng & shard.
- [`docs/protocol/COMPATIBILITY-MATRIX.md`](docs/protocol/COMPATIBILITY-MATRIX.md) — ma trận gói tin (điền sau pcap).
- [`docs/protocol/TAKUMI-PROTOCOL-DISPATCH-INDEX.md`](docs/protocol/TAKUMI-PROTOCOL-DISPATCH-INDEX.md) — head/sub dispatcher từ mã (Connect / Join-internal / Game).
- [`docs/game-spec/TAKUMI-SQL-BACKLOG.md`](docs/game-spec/TAKUMI-SQL-BACKLOG.md) — ODBC / proc / bảng + **SQL Back**.
- [`docs/game-spec/PHASE2-OPENMU-DATA-MODEL-MAP.md`](docs/game-spec/PHASE2-OPENMU-DATA-MODEL-MAP.md) — Phase 2: `SQLUp` ↔ entity OpenMU / ADR nháp ETL.
- [`docs/game-spec/PHASE2-MAPPING-TEMPLATE.csv`](docs/game-spec/PHASE2-MAPPING-TEMPLATE.csv) — seed CSV proc/bảng × OpenMU / plugin.
- [`tools/db-migrate/README.md`](tools/db-migrate/README.md) — **`takumi-mssql-inspect`** + **`takumi-pg-inspect`** (dump schema read-only) + quy ước ETL (ETL TODO).
- [`docs/game-spec/DATA-SUB1-DRIFT.md`](docs/game-spec/DATA-SUB1-DRIFT.md) — drift `Sub 1/Data` vs `Data`.
- [`docs/game-spec/GAMESERVER-VS-GAMESERVER-REAL.md`](docs/game-spec/GAMESERVER-VS-GAMESERVER-REAL.md) — `4.GameServer` vs `4.GameServer_real`.
- [`docs/game-spec/CONNECT-SERVER-REAL-DRIFT.md`](docs/game-spec/CONNECT-SERVER-REAL-DRIFT.md) — `1.ConnectServer` vs `_real`.
- [`docs/game-spec/GAMESERVER-DATA-FOLDER-MAP.md`](docs/game-spec/GAMESERVER-DATA-FOLDER-MAP.md) — bản đồ thư mục `Data/`.
- [`docs/migration/MANIFEST-TRACKER-TEMPLATE.md`](docs/migration/MANIFEST-TRACKER-TEMPLATE.md) — template tracker manifest §17.
- [`docs/migration/OPERATIONS-MIGRATION-NOTES.md`](docs/migration/OPERATIONS-MIGRATION-NOTES.md) — thứ tự batch Windows, Docker, scripts.

Repository: https://github.com/hoangdt9/mu-takumi  

## Sau khi clone

1. **ODBC configs (không commit secret):**

   ```bash
   cp MuServer/2.DataServer/DataServer.ini.example MuServer/2.DataServer/DataServer.ini
   cp MuServer/3.JoinServer/JoinServer.ini.example MuServer/3.JoinServer/JoinServer.ini
   ```

   Điền `CHANGE_ME` (SQL user/password, `GlobalPassword`). File thật bị `.gitignore`.

2. **Chạy server Windows:** cần bản **`.exe`/`.dll` build sẵn** (không có trong repo) — đặt vào các thư `MuServer/...` như máy dev.

3. **Docker (SQL/dev):** `docker/.env.example` → `docker/.env`; xem `docker/README.md`.

4. **Client PC prebuilt:** thư `ClientBuild_*` bị ignore (dung lượng). Giữ offline hoặc pipeline release riêng.

5. **Android:** không commit `app/build/` / `.gradle` — chỉnh `local.properties` (SDK path) máy dev.

## Cấu trúc đáng chú ý

| Path | Mô tả |
|------|--------|
| `Source/1.ConnectServer` … `4.GameServer` | Mã nguồn MSVC server |
| `Source/6.GetMainInfo` | Tool patch client |
| `Source/android` | Client Android |
| `MuServer/` | Layout deploy + Data (txt/dat) |
| `docs/` | Checklist migrate OpenMU + protocol baseline |

## Đẩy lên GitHub (maintainer)

Repo đích: [hoangdt9/mu-takumi](https://github.com/hoangdt9/mu-takumi). Trên máy có quyền ghi:

```bash
cd /path/to/takumi   # nhánh main, đã commit

# PAT + HTTPS hoặc SSH:
git remote set-url origin git@github.com:hoangdt9/mu-takumi.git
git push -u origin main
```

Không có credential sẽ lỗi `could not read Username` — chạy `gh auth login` hoặc dùng SSH key.

## Bảo mật

Đừng force-add `DataServer.ini` / `JoinServer.ini` chứa mật khẩu thật lên GitHub công khai. Nếu đã lộ, đổi pass SQL và `GlobalPassword`.
