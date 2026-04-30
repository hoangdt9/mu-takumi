# Game data drift — `MuServer/4.GameServer/Sub 1/Data` vs `Data`

**Ngày:** 2026-04-30  
**Lệnh:** `diff -rq "MuServer/4.GameServer/Data" "MuServer/4.GameServer/Sub 1/Data"`

**Kết quả:** Không có khác biệt — hai cây là **bản sao giống nhau** tại thời điểm quét (shard chính vẫn dùng payload data giống thư mục “mặc định”; khác nhau chủ yếu ở **`GameServer/Data/GameServerInfo - Common.ini`** và binary `GameServer.exe` / `GameServerCS.exe`).

### Gợi ý vận hành

- Khi chỉnh event/custom **chỉ cho một shard**, phải tách có chủ đích và ghi *drift* tại đây (đường dẫn file + lý do).
- OpenMU fork: có thể bắt đầu từ **một** bản `Data/` cho cả hai world nếu logic phân nhánh nằm ở cấu hình server (map group / castle instance).

**Liên kết:** `docs/TAKUMI-FULL-FILE-MIGRATION-CHECKLIST.md` §8a/§8c; [`GAMESERVER-VS-GAMESERVER-REAL.md`](GAMESERVER-VS-GAMESERVER-REAL.md).
