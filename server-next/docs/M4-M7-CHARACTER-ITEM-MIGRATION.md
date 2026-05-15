# M4 / M7 — Character & item migration (`Source/` → `server-next`)

Last updated: 2026-05-16  
**Owner nhánh đề xuất:** `mac-m1` (M4/M7); **không** sửa `WorldGameplayHandlers` / `ShopCommerceHandler` trừ bug — xem **`docs/M9-M8-NPC-GAMEPLAY-OWNERSHIP.md`**.

**Checklist chi tiết:** `docs/M4-WORLD-POSITION-CHECKLIST.md`, `docs/M7-CHARACTER-PERSISTENCE-CHECKLIST.md`, `docs/M4-ROSTER-SSOT.md`, `docs/M4-TILE-AND-COORDINATES.md`.

---

## Quy tắc tránh làm trùng

| Việc | Ai làm | Đừng đụng |
|------|--------|-----------|
| Roster tile, JSON, `character_roster`, walk save | **M4** | Monster viewport, shop wire |
| HP/MP/zen join + outbound `0x26`/`0x27` | **M7** | `ItemManager` buy/sell logic |
| `F3 10` inventory blob 12-byte | **M4 + M7** (read path) | Shop list `C2 0x31` (M8) |
| NPC shop list + buy/sell/repair stub | **M8** (`main`) | `NpcShopHandler` cũ (đã xóa) |
| Monster spawn / combat | **M9** (`main`) | `inventory_slot` ETL |

Trước khi mở PR: `git fetch origin main` → rebase/merge; cập nhật bảng **Trạng thái** dưới đây.

---

## Ánh xạ legacy `Source/` → `server-next`

### A. Nhân vật & roster (M4)

| Legacy (`Source/`) | Hành vi | `server-next` (trạng thái) | Ghi chú |
|--------------------|---------|----------------------------|---------|
| `3.JoinServer` + `DSProtocol` character list | `GDCharacterListSend`, slot 34 B | `CharacterListWire602`, `F3 00` | **Done** — roster từ JSON + DB overlay |
| `4.GameServer/Protocol.cpp` `CGCharacterListRecv` | List request | `LegacyLoginHostRunner`, `GamePortMinimalSession` | **Done** |
| `User.cpp` / `ObjectManager` spawn tile | Map + XY + angle | `JoinMapSpawnWire`, `JoinMapServerWire602` | **Done** |
| Walk `0xD4` / `0x10`, instant `0x15` | Cập nhật tile | `ClientWalkPackets602` + roster in-memory | **Done** |
| Disconnect / periodic save | Flush DB | `SavePersistedRoster`, `TAKUMI_ROSTER_PERIODIC_SAVE_SECONDS` | **Done** |
| `2.DataServer` character tables | SSOT SQL | `takumi_runtime.character` + importer startup | **Partial** — entity có; **chưa** SSOT-only |
| — | Postgres mirror | `character_roster`, `CharacterRosterMirrorWriter` | **Done** |
| — | SSOT Postgres-only (bỏ JSON) | `CharacterRosterBootstrap`, `TAKUMI_ROSTER_DB_PRIMARY` | **Partial** — DB-first load + optional skip JSON export |

### B. Vitals & join stats (M7)

| Legacy | Hành vi | `server-next` | Ghi chú |
|--------|---------|---------------|---------|
| `GCLifeSend` / mana | `0x26` / `0x27` | `LifeManaWire602`, `RosterVitalsOutboundTracker`, `RosterVitalsCombat` | **Partial** — monster hit → `0x26` + `0x17` die + vitals DB upsert |
| Join map stats | `PRECEIVE_JOIN_MAP_SERVER` | `JoinMapStatWire`, `JoinMapVitalsSeed` | **Done** khi `max_hp`/`max_mp` > 0 |
| DB HP/MP/zen | Character save | `004_character_roster_vitals.sql`, `CharacterRosterRow` | **Done** |
| Full combat vitals sync | Damage → life | — | **OPEN** — M7d |

### C. Item & inventory (M4 read + M7/M8 write path)

| Legacy (`ItemManager.*`) | Opcode / wire | `server-next` | Ghi chú |
|--------------------------|---------------|---------------|---------|
| `ItemByteConvert` | 12-byte item blob | `ItemWire602.WriteSeason6Item` | **Done** (shop/NPC list) |
| `GCItemListSend` | `F3 10` | `InventoryListWire602`, `JoinInventoryPacket602` | **Done** — đọc `inventory_slot` |
| `CGItemGetRecv` | `0x22` pick up | `ItemWorldHandler` | **Partial** — ground store + `C2 0x20`/`0x21` |
| `CGItemDropRecv` | `0x23` drop | `ItemWorldHandler` | **Partial** — bag drop + viewport create |
| `CGItemMoveRecv` | `0x24` move | `ItemWorldHandler`, `InventoryEquipRules` | **Partial** — inv+wear (storage 0); trade/warehouse **OPEN** |
| `CGItemUseRecv` | `0x26` use | — | **OPEN** |
| `CGItemBuyRecv` | `0x32` | `ShopCommerceHandler` | **Partial** — zen + wire; **persist** `inventory_slot` qua `InventorySlotMirrorWriter` |
| `CGItemSellRecv` | `0x33` | `ShopCommerceHandler` | **Partial** — delete slot DB mirror |
| `CGItemRepairRecv` | `0x34` | `ShopCommerceHandler` | **Partial** — durability + upsert mirror |
| `CGMoveItemProc` | move proc | — | **OPEN** |
| DB item load/save | DataServer | `inventory_slot` + `InventorySlotMirrorWriter` | **Done** — upsert/delete/replace; cần `TAKUMI_ROSTER_DB_SYNC=1` |

### D. Join / login (tham chiếu, không duplicate M4)

| Legacy | `server-next` |
|--------|---------------|
| `1.ConnectServer` F4 06/03 | `Takumi.Server.Connect`, `LegacyLoginHost` connect sidecar |
| `3.JoinServer` ticket | `session_ticket`, `TAKUMI_GAME_PORT`, M5 checklist |

---

## Thứ tự implement đề xuất (M4/M7)

1. **M4b SSOT design** — doc + API một đường ghi (`character` ↔ `character_roster` ↔ JSON); không code song song hai writer.
2. ~~**M7d combat vitals (stub)**~~ — monster→player: `0x26`, `0x17` die, `ScheduleVitalsUpsert`; heal/revive **OPEN**.
3. ~~**Inventory persist (M4+M7)**~~ — **Done:** `InventorySlotMirrorWriter`, `PostgresInventorySlotRepository` write, flush disconnect.
4. ~~**Item world ops**~~ — **Partial:** `0x22`–`0x24` (`ItemWorldHandler`); trade/warehouse/equip rules **OPEN**.
5. **Staging ETL** — `inventory_staging` → 12-byte (`IMPLEMENTATION-CHECKLIST` §Data & Migration).

---

## File `server-next` theo module (quick reference)

| Module | File chính |
|--------|------------|
| M4 roster | `Persistence/CharacterRoster*.cs`, `LegacyLoginHostRunner.cs`, `GamePortMinimalSession.cs` |
| M7 vitals | `Protocol/JoinMapServerWire602.cs`, `LifeManaWire602.cs`, `RosterVitalsLifecycle.cs` |
| M4 inventory read | `JoinInventoryPacket602.cs`, `InventoryListWire602.cs`, `PostgresInventorySlotRepository.cs` |
| M8 shop (đừng duplicate) | `World/ShopCommerceHandler.cs`, `Protocol/ShopCommerceWire602.cs` |

---

## Env liên quan

```bash
# M4
TAKUMI_ROSTER_DB_SYNC=1
TAKUMI_ROSTER_DIR=./takumi-roster
TAKUMI_ROSTER_PERIODIC_SAVE_SECONDS=60
TAKUMI_ROSTER_DB_MERGE_MODE=overlay   # hoặc json

# M7
TAKUMI_SEND_LIFE_MANA_AFTER_JOIN=1

# Postgres (Docker)
TAKUMI_PG_CONNECTION_STRING=Host=postgres;Port=5432;...
```

---

## Cập nhật khi hoàn thành PR

1. Tick `[x]` trong `M4-WORLD-POSITION-CHECKLIST.md` / `M7-CHARACTER-PERSISTENCE-CHECKLIST.md`.
2. Sửa cột **Trạng thái** trong bảng trên (Done / Partial / OPEN).
3. Thêm một dòng vào **`docs/WORKSTREAM-OWNERSHIP.md`** § “Đã xong trên main”.
4. **Không** tick M8/M9 thay cho M4/M7.
