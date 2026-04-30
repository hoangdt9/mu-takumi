# Ghi chú vận hành Takumi ↔ OpenMU target

Tham chiếu khi viết **`compose`/systemd/OpenMU fork** thay batch Windows.

## Thứ tự start (Windows hiện tại)

File: `MuServer/Start_192.168.99.200.bat`

1. `Stop_MuServer.bat` (best-effort)
2. `1.ConnectServer/ConnectServer.exe`
3. `2.DataServer/DataServer.exe` (ODBC → `MuOnline`)
4. `3.JoinServer/JoinServer.exe` (ODBC → `MuOnlineJoin`)
5. (optional) `5.Antihack/XShield.exe`
6. `4.GameServer/GameServer/GameServerCS.exe` (shard CS / `GAMESERVER_TYPE=1`)
7. `4.GameServer/Sub 1/GameServer/GameServer.exe` (shard chính / `GAMESERVER_TYPE=0`)

**Cổng & serial:** `docs/protocol/TAKUMI-SERVER-NETWORK-BASELINE.md`.

## Repo Docker (Takumi)

- `docker/docker-compose.yml` — profile **`db`** (SQL Server dev), **`wine`** (sandbox server Win32 — **không** mục tiêu production trên Apple Silicon).
- `docker/sql/restore-muonline.sh` — restore `.bak` cho **golden compare** với OpenMU roadmap Postgres (không thay thế migration schema).

## Script local

- `scripts/run-mssql-docker.sh` — profile `db`
- `scripts/run-muserver-docker.sh` — profile `wine` (legacy)
- `scripts/open-mu-vm.sh` — VM dev (VMware), **không** track trong git lớn

## Hướng OpenMU

- Thay 2→7 bằng process **.NET** (Connect / Login / Game servers) + **PostgreSQL**; giữ **thứ tự nghiệp vụ**: auth → world list → game TCP.
- Antihack: quyết chính sách (bỏ / thay server-side) — xem checklist §9.

**Liên kết:** `docs/TAKUMI-MIGRATION-OPENMU-CHECKLIST.md`, `docs/SERVER-PORT-PLAN.md`.
