# Baseline cổng & shard — Takumi `MuServer` (cho map OpenMU / Docker)

Tài liệu này đóng mục **§0 checklist migration**: ghi nhận **IP/hostname**, **TCP/UDP cổng dịch vụ**, và **hai process GameServer** (CS shard vs shard chính) tại thời điểm rà **2026-04-30**. Khi đổi máy chủ/OpenMU staging, chỉnh lại bảng dưới và giữ **`ServerList`/client IP** nhất quán.

## Thứ tự process (tham chiếu)

`MuServer/Start_192.168.99.200.bat`: Connect → Data → Join → (tùy chọn) XShield → `GameServerCS.exe` → `Sub 1\GameServer.exe`.

## ConnectServer

| Nguồn | Tham số | Giá trị snapshot |
|-------|---------|------------------|
| `MuServer/1.ConnectServer/ConnectServer.ini` | `ConnectServerPortTCP` | `63000` |
| Cùng file | `ConnectServerPortUDP` | `63001` |
| `GameServerInfo - Common.ini` (cả hai shard) | `ConnectServerPort` (nội bộ GS↔CS) | `63001` |

**Ghi chú:** Client MU thường dùng **UDP** cho danh sách server; JoinServer.ini tham chiếu `ConnectServerPort = 63001`. Cần **spike pcap** để xác nhận client Takumi dùng TCP 63000 hay UDP 63001 (hoặc cả hai).

## DataServer

| Tham số | Giá trị |
|---------|---------|
| `DataServerPort` | `63002` |
| ODBC DSN | `MuOnline` (xem `DataServer.ini` — **không** commit mật khẩu ra doc) |

## JoinServer

| Tham số | Giá trị |
|---------|---------|
| `JoinServerPort` | `63003` |
| `ConnectServerAddress` / `ConnectServerPort` (Join → Connect) | `127.0.0.1` / `63001` (trong `JoinServer.ini` snapshot) |
| `MD5Encryption` | `0` |
| `GlobalPassword` | Có trong `JoinServer.ini` — xoay khi chuyển production |

## GameServer — hai shard

| Process / EXE (theo batch) | Thư mục runtime | `ServerName` | `ServerCode` | `ServerPort` (game TCP) | `ServerVersion` | `ServerSerial` |
|----------------------------|----------------|-------------|--------------|-------------------------|-----------------|----------------|
| `GameServerCS.exe` | `MuServer/4.GameServer/GameServer/` | `KEN-CS` | `19` | `55920` | `1.04.05` | `TbYehR2hFUPBKgZj` |
| `GameServer.exe` | `MuServer/4.GameServer/Sub 1/GameServer/` | `KEN-1` | `0` | `55901` | `1.04.05` | (cùng serial) |

Cả hai dùng cùng block **Connection Settings** trong `GameServerInfo - Common.ini`:

- `DataServerAddress` / `DataServerPort`: `192.168.99.200` / `63002`
- `JoinServerAddress` / `JoinServerPort`: `192.168.99.200` / `63003`
- `ConnectServerAddress` / `ConnectServerPort`: `192.168.99.200` / `63001`

`GAMESERVER_TYPE` trong `.vcxproj`: build **SUB1** ≡ `GameServer.exe` (**type 0**, “GS”), build **`…\4.GameServer\GameServer`** với suffix **CS** ≡ `GameServerCS.exe` (**type 1**, “GSCS”) — khớp `stdafx.h` (`GAMESERVER_VERSION` `"GS"` / `"GSCS"`).

## OpenMU

Map sang listener **Connect / Login / Game** của fork PostgreSQL/OpenMU và ghi vào đây khi có bản staging (điền cột parity).

| Hạng mục | Takumi (snapshot) | OpenMU (staging) | Parity |
|----------|-------------------|-----------------|--------|
| Public connect / server list | UDP `63001` (client), TCP `63000` (.ini) | TBD | TODO |
| Data | `63002` | N/A — thay abstraction DB | TODO |
| Join / login path | `63003` | TBD | TODO |
| Game shard 1 | `55901` | TBD | TODO |
| Game CS shard | `55920` | TBD | TODO |

## Files nguồn chính

- `MuServer/4.GameServer/GameServer/Data/GameServerInfo - Common.ini` (CS shard)
- `MuServer/4.GameServer/Sub 1/GameServer/Data/GameServerInfo - Common.ini` (main shard)
- `MuServer/1.ConnectServer/ConnectServer.ini`
- **`1.ConnectServer` vs `_real`:** ini đồng nhất trong repo — [`docs/takumi-game-spec/CONNECT-SERVER-REAL-DRIFT.md`](takumi-game-spec/CONNECT-SERVER-REAL-DRIFT.md)
- `MuServer/2.DataServer/DataServer.ini`
- `MuServer/3.JoinServer/JoinServer.ini`
