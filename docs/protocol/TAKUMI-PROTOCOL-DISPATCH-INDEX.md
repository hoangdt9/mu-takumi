# Chỉ mục điều phối packet — Takumi server (discovery cho OpenMU)

**Ngày:** 2026-04-30  
**Phạm vi:** Đầu vào **client/game** và luồng **Connect / Join(trên TCP nội bộ GS↔Join)** được rà **trực tiếp từ mã**. Đối chiếu thêm **`DSProtocol.cpp`** khi parity **GS↔DataServer**.

**Không:** Liệt kê đủ ~100+ `case` long-form trong **`Protocol.cpp`** (file >6k dòng) — parity từng flow vẫn bằng đọc `switch(head)` trong IDE/`grep`.

---

## 1. Kiểu frame MU (chuẩn dòng EX)

| Byte `lpMsg[0]` | Kiểu | **Head** (`head` vào dispatcher) | **Sub** (`Get*PacketSub`) |
|-----------------|------|----------------------------------|---------------------------|
| `0xC1`, `0xC3` | Biến cỡ 1-byte | `lpMsg[2]` | `lpMsg[3]` (khi `size≥4`) |
| `0xC2`, `0xC4` | Biến cỡ 2-byte | `lpMsg[3]` | `lpMsg[4]` (khi `size≥5`) |

Tham chiếu struct: **`PBMSG_HEAD`**, **`PBMSG_HEAD3`**, **`PWMSG_HEAD`** trong `Source/4.GameServer/GameServer/Protocol.h`.

---

## 2. Mã XOR / “encryption” theo locale build (`PROTOCOL_CODE*`)

Trong **`Protocol.h`**, các hằng **`PROTOCOL_CODE1` … `PROTOCOL_CODE4`** phụ thuộc **`GAMESERVER_LANGUAGE`**. Build checklist Takumi (**`JOINSERVER/GAMESERVER/DATASERVER _UPDATE=603`**) dùng **`GAMESERVER_LANGUAGE=1`** (thấy trong `Release_EX603`): khi **`#elif(GAMESERVER_LANGUAGE==1)`**:

| Macro | Hex |
|-------|-----|
| `PROTOCOL_CODE1` | `0xD4` |
| `PROTOCOL_CODE2` | `0x11` |
| `PROTOCOL_CODE3` | `0x15` |
| `PROTOCOL_CODE4` | `0xDB` |

Ở **`ProtocolCore`**, `PROTOCOL_CODE2` / **`PROTOCOL_CODE3`** được gán lần lượt cho **tấn công vật lý** (`CGAttackRecv`) và **di chuyển** (`CGPositionRecv`) — cần **khớp** với decrypt client khi spike OpenMU.

---

## 3. ConnectServer (`ConnectServerProtocol.cpp`)

| Entry | Head | Sub (`lpMsg[3]`) | Handler / ý nghĩa |
|-------|------|------------------|-------------------|
| Client → CS | **`0xF4`** | `0x03` | **`CCServerInfoRecv`** — một server trong list (theo ServerCode). |
| | | `0x06` | **`CCServerListRecv`** — full list blob. |

Mọi head khác → log unknown (security). Khởi tạo: **`CCServerInitSend`** header `head=0x00`.

---

## 4. JoinServer — **GameServer TCP nội bộ** (`JoinServerProtocolCore`)

Đây **không** phải gói từ client PC/Android; là **tin GS→Join**/Join xử lý (header `SET`/`SDHP_*` trong code).

| Head | Handler (ghi tắt) |
|------|-------------------|
| `0x00` | `GJServerInfoRecv` |
| `0x01` | `GJConnectAccountRecv` |
| `0x02` | `GJDisconnectAccountRecv` |
| `0x03` | `GJMapServerMoveRecv` |
| `0x04` | `GJMapServerMoveAuthRecv` |
| `0x05`, `0x06` | `GJAccountLevelRecv` / `…2` |
| `0x10`–`0x12` | map cancel / level save / lock save |
| `0x20` | server user info |
| `0x30` | external disconnect |
| `0x56` | **`GJRegistroMainRecv`** (custom regist path) |

`GetJoinServerPacketSub` chỉ hỗ trợ **`0xC1`/`0xC2`** tại `[3]` — cùng quy ước sub với MU frame.

---

## 5. GameServer — **`ProtocolCore`** (`Protocol.cpp`)

### 5.1 Nhánh tùy biến (trước `switch(head)`)

- **`head == 0xFB`** và **`lpMsg[0]==0xC1`**: submenu `switch(lpMsg[3])` (offline mode **`0x3D`**, change class **`0x08`**, …).

### 5.2 Bảng rút gọn **`switch(head)`** (đại diện — **thiếu** các head xa hơn `0xBC+` và nhánh **`0xF1`/`0xF3`/…`)

| Head | Tiêu biểu (handler) |
|------|---------------------|
| `0x00` | Chat |
| `0x02` | Whisper |
| `0x03` | Main check |
| `0x0E` | Live client |
| `PROTOCOL_CODE2` (`0x11`) | Attack |
| `PROTOCOL_CODE3` (`0x15`) | Position |
| `0x18`–`0x26` | Action, skill, teleport, item get/drop/move/use |
| `0x30`–`0x3D` | NPC talk, trade, cancel |
| `0x3F` | Personal shop (sub trong `lpMsg[3]` 01–09) |
| `0x40`–`0x43` | Party |
| `0x4A`/`0x4B` | Rage/dark skill (Season 6+) |
| `0x4C`–`0x4E` | Mining / event inventory / MuRummy / Muun |
| `0x50`–`0x66` | Guild, war |
| `0x81`–`0x87` | Warehouse, Chaos Box |
| `0x86`/`0x87` | Chaos mix / close |
| `0x8E` | Teleport move |
| `0x90`/`0x91` | Devil Square, event time |
| `0x9A` | Blood Castle enter |
| `0xA0`/`0xA2` | Quest |
| `0xA7`/`0xA9` | Pet |
| `0xAA`/`0xAB`/`0xAC` | Duel |
| `0xAE` | Helper |
| `0xAF` | Chaos Castle (sub) |
| `0xB0` | Skill teleport ally |
| `0xB1` | Map server move auth (`lpMsg[3]` sub `0x01`) |
| `0xB2` | Castle Siege (subs `0x02`… — state, reg, NPC, tax…) |
| `0xB3`–`0xB5` | NPC DB / CS guild lists |
| `0xB7`/`0xB9` | Siege weapon, mark owner |

Xem **`DSProtocol.cpp`**, **`JSProtocol.cpp`**, **`ESProtocol.cpp`**, **`NewsProtocol.cpp`** để parity **tin nội bộ**/data stream.

---

## 6. Liên quan checklist

- Ma trận + pcap: [`COMPATIBILITY-MATRIX.md`](COMPATIBILITY-MATRIX.md)  
- Baseline cổng: [`TAKUMI-SERVER-NETWORK-BASELINE.md`](TAKUMI-SERVER-NETWORK-BASELINE.md)  
- Full checklist file: [`TAKUMI-FULL-FILE-MIGRATION-CHECKLIST.md`](../TAKUMI-FULL-FILE-MIGRATION-CHECKLIST.md) §4–§7  
- Pha migration: [`TAKUMI-MIGRATION-OPENMU-CHECKLIST.md`](../TAKUMI-MIGRATION-OPENMU-CHECKLIST.md) Phase 0 / 3 / 4.1  

**Tiếp theo được đề xuất:** một pass **`rg 'case 0x' Protocol.cpp`** + script gom **`head`** top-level độ sâu 2 tab (qua AST tuỳ chọn); hoặc điền **`COMPATIBILITY-MATRIX`** từ pcap và map sang hàng **`ProtocolCore`.
