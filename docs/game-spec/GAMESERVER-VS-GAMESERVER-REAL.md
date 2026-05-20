# Layout drift — `MuServer/4.GameServer` vs `MuServer/4.GameServer_real`

**Ngày:** 2026-04-30  
**Mục đích:** Chốt §8c checklist — cây nào **đủ** cho migration snapshot và drift **có ý nghĩa** (bỏ qua `LOG/`, `*.pdb`, `*.exe`).

## Cấu trúc thư mục (tổng quan)

| | `4.GameServer` | `4.GameServer_real` |
|---|----------------|---------------------|
| `Data/` + `GameServer/` (cụm **Castle / GSCS** ở root) | Có | **Không** |
| `Sub 1/` (shard chính) | Có | Có |
| `GameServerCS.exe` | Trong **`GameServer/GameServerCS.exe`** | Ở **root** `4.GameServer_real/` |
| `Sub 1/GameServer/GameServer_real.exe` | Không | Có |

**Kết luận cấu trúc:** **`4.GameServer`** là layout **đầy đủ hai nhánh** (CS + Sub 1). **`4.GameServer_real`** là bản **rút gọn / snapshot** (thiếu cây CS đầy đủ ở root), phù hợp máy chạy local hoặc bản copy tay — **không** thay thế toàn bộ `4.GameServer` khi inventory manifest / OpenMU parity.

## `Sub 1/Data` — nội dung game

**Lệnh:** `diff -rq "MuServer/4.GameServer/Sub 1/Data" "MuServer/4.GameServer_real/Sub 1/Data"`

**Kết quả:** Chỉ khác **`MapServerInfo.dat`** (kích thước repo: ~960 vs ~1005 byte). Cần đối chiếu binary trên máy chủ thật nếu map group / IP trong file này ảnh hưởng client.

*(Hai cây `Data` và `Sub 1/Data` **trùng nhau** trong `4.GameServer` — đã ghi [`DATA-SUB1-DRIFT.md`](DATA-SUB1-DRIFT.md).)*

## Config / tuning (không phải “chỉ log”)

### `Sub 1/GameServer/Data/GameServerInfo - Common.ini`

Bản `_real` trỏ **Data / Join / Connect** về **`127.0.0.1`**; bản chuẩn dùng **`192.168.99.200`**. Cổng (`DataServerPort`, `JoinServerPort`, `ConnectServerPort`) trong diff mẫu **giữ nguyên** — chỉ đổi host.

→ Khi migrate OpenMU / Docker: coi đây là **profile mạng**, không phải fork logic game.

### `Sub 1/GameServer/GHRSReset.ini`

Khác tham số ví dụ: `GHRSMax` (50 vs 60), `DayCache` (30 vs 28). Đây là **drift chủ đích / theo season vận hành** — cần ghi rõ bản nào là production khi chỉnh fork.

## Runtime khác

- `msvcp140.dll` / `vcruntime140.dll` chỉ thấy trong `4.GameServer/Sub 1/GameServer` (bản chuẩn); `_real` có thể dựa VC++ cài hệ thống.

## `LOG/**`

`diff -rq` giữa hai cây chênh rất nhiều file log theo ngày — **không dùng** để quyết authority; chỉ debug vận hành.

**Liên kết:** [`docs/migration/TAKUMI-FULL-FILE-MIGRATION-CHECKLIST.md`](../TAKUMI-FULL-FILE-MIGRATION-CHECKLIST.md) §8, §8c; [`docs/protocol/TAKUMI-SERVER-NETWORK-BASELINE.md`](../protocol/TAKUMI-SERVER-NETWORK-BASELINE.md).
