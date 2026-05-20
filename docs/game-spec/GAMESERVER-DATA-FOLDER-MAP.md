# Ánh xạ `MuServer/4.GameServer/Data/*` → backlog OpenMU

**Ngày:** 2026-04-30  
**Mục đích:** Đóng phần **discovery** checklist (FULL §8a + OPENMU Phase 4.1 “inventory chức năng”). Không thay [**`TAKUMI-MUSERVER-GAMEDATA-FILES.txt`**](../manifests/TAKUMI-MUSERVER-GAMEDATA-FILES.txt); file manifest vẫn là inventory từng file.

Giả định: **`4.GameServer/Data` ≡ `Sub 1/Data`** (đã chứng minh trong [`DATA-SUB1-DRIFT.md`](DATA-SUB1-DRIFT.md)).

## Thư mục con (theo listing root `Data/`)

| Path (tương đối `Data/`) | Vai trò gợi ý (Takumi) | OPENMU Phase / vùng code (ôn định) |
|--------------------------|------------------------|-------------------------------------|
| `CashShop/` | Gói & sản phẩm cash / WCoin | Persistence + plug-in Cash Shop / item mall |
| `Character/` | `DefaultClassInfo`, v.v. | Khởi tạo class / attribute — `GameConfiguration` hoặc DB seed |
| `Custom/` | Season custom (nhiều file `.txt`/`.ini`) | **PlugIns** Takumi-only; không kỳ vọng parity vanilla |
| `Event/` | `.dat` lịch sự kiện (BC, DS, siege, …) | Event plug-ins / `GameLogic` timers + maps |
| `EventItemBag/` | Túi rơi đánh số | Drop/item generators + plug-ins |
| `Hack/` | Policy speed/skill checks (text) | Thay rule server-side / anti-cheat policy OpenMU |
| `Item/` | Option, jewel, mix, stacking | Item definitions — config + parsers (so với định nghĩa OpenMU) |
| `Monster/` | Spawn / AI / level | Monster spawn tables + definitions |
| `Move/` | Gate, điều kiện chuyển map | Warp / gates — map logic |
| `Quest/`, `QuestWorld/` | Nhiệm vụ cổ điển + world quests | Quest system / persistence |
| `Ruud/` | Tiền tệ Ruud / shop Ruud | Tùy bản MU — có thể plug-in hoặc WONTFIX |
| `Shop/` | NPC shop | NPC shop definitions |
| `Skill/` | Skill tree / damage defs | Skill / master skill configuration |
| `Terrain/` | `.att`, map binary | Map terrain — world load pipeline |
| `Util/` | Helper `.txt`/`.dat` | Misc loaders — chia nhỏ theo chức năng khi port |

## File ở root `Data/` (không trong thư mục con)

| File | Gợi ý |
|------|--------|
| `Command.txt` | Mapping lệnh GM / chat — parity với Command plug-in OpenMU |
| `Effect.txt` | Effect ID / visuals — skill & item FX |
| `MapManager.txt` | Quản lý map / cờ — core map list |
| `MapServerInfo.dat` | Binding map ↔ server/group (binary); **đã thấy drift** giữa bản localhost trong [`GAMESERVER-VS-GAMESERVER-REAL.md`](GAMESERVER-VS-GAMESERVER-REAL.md) |
| `Message.txt` | Chuỗi server — localization / MU messages |
| `MiniMap.txt` | Meta minimap | 
| `ShopManager.txt` | Chỉ mục shop |
| `EventItemBagManager.txt` | Chỉ mục túi event |

## Việc tiếp theo

1. Checkbox từng thư mục trong FULL §8a vẫn giữ **`[ ]`** cho đến khi có **converter**, **nhập tay**, hoặc **ADR WONTFIX** trên fork.  
2. Khi Spike OpenMU: so từng loại file với **format** trong `OpenMU` (thường XML/embedded resources/các plug-in importer).  
3. Opcodes / gameplay: không suy luận từ folder này một mình — cần pcap → [`COMPATIBILITY-MATRIX.md`](../protocol/COMPATIBILITY-MATRIX.md).

**Liên kết:** [`TAKUMI-FULL-FILE-MIGRATION-CHECKLIST.md`](../TAKUMI-FULL-FILE-MIGRATION-CHECKLIST.md) §7–§8; [`TAKUMI-MIGRATION-OPENMU-CHECKLIST.md`](../TAKUMI-MIGRATION-OPENMU-CHECKLIST.md) Phase 4.1.
