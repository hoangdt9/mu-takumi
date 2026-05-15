# M7 — Character persistence lifecycle (HP / MP / zen / map / tile)

**Quy ước:** chỉ tick `[x]` khi đã có trong git và có thể chứng minh (test hoặc QA ghi rõ). Cập nhật file này khi merge.

**Phụ thuộc:** **`docs/M4-TILE-AND-COORDINATES.md`**, **`docs/M4-ROSTER-SSOT.md`**, **`docs/M6-GAME-TCP-CHECKLIST.md`**.  
**Port từ `Source/` (character + item):** **`docs/M4-M7-CHARACTER-ITEM-MIGRATION.md`**. **Không chặn M5** (join/ticket).

**Tham chiếu client / legacy:** `Source/4.GameServer` save/load nhân vật; `WSclient.h` join stats (`PRECEIVE_JOIN_MAP_SERVER`); M1 map opcode sau vào thế giới.

---

## M7a — Schema & migration

- [x] `sql/init/004_character_roster_vitals.sql` — thêm `current_hp`, `max_hp`, `current_mp`, `max_mp`, `zen` trên `public.character_roster` (`ALTER … IF NOT EXISTS`).
- [x] Ghi chú trong `README.md` / `apply-sql.sh` header nếu thứ tự file thay đổi (hiện `001` → `004` theo tên).
- [ ] (Tuỳ chọn) Cột `shield` / `skill_mana` nếu join wire cần parity đầy đủ hơn — đối chiếu `JoinMapServerWire602` offsets.

---

## M7b — Domain model (C#)

- [x] Mở rộng **`CharacterRosterRow`** + **`PostgresCharacterRosterRepository`** (`Load` / `Replace`) để đọc/ghi vitals (0 = “chưa set”).
- [x] Đồng bộ **`GameRosterEntry`** (JSON `takumi-roster`) + **`CharacterRosterEntry`** / **`RosterPersistChar`** trong **`LegacyLoginHost`** (camelCase JSON).
- [x] Migration path file JSON cũ: thiếu field → default 0; không làm vỡ `ApplyLegacySpawnIfUnset`.
- [x] **`CharacterRosterRowMapping.ToRow`** + merge overlay vitals trên login (`LegacyLoginHost`, `GamePortMinimalSession`).

---

## M7c — Join wire (F3 03) từ dữ liệu đã lưu

- [x] **`JoinMapStatWire.FromRoster`** + **`CharacterRosterVitals`**: khi `max_hp` / `max_mp` &gt; 0 dùng giá trị đã lưu; ngược lại stub theo class/level; `zen` &gt; 0 ghi vào offset gold.
- [x] DB overlay sau login copy vitals cùng map/xy (`ApplyDbOverlay`); JSON thiếu field vẫn 0.
- [x] Mục “vitals overlay” trong **`docs/M4-ROSTER-SSOT.md`**.

---

## M7d — Vòng đời phiên (save triggers)

- [x] Lưu vitals khi **disconnect** (cùng `SavePersistedRoster` / `SaveRoster` — đã ghi field vitals; cần seed trước, xem dưới).
- [x] **Seed sau join / move-map:** `JoinMapVitalsSeed` + `RosterVitalsLifecycle` — copy HP/MP/zen từ **`F3 03`** vào roster khi `max_hp == 0`; `rosterDirty` cho **`TAKUMI_ROSTER_PERIODIC_SAVE_SECONDS`**.
- [x] **Cả hai host:** `LegacyLoginHost` + `GamePortMinimalSession`.
- [x] **Giữa phiên (partial):** `LifeManaWire602` + `RosterVitalsOutboundTracker` — quét outbound `C1 0x26`/`0x27` (type `0xFF`/`0xFE`) cập nhật roster + `rosterDirty`; gửi sync sau join khi `TAKUMI_SEND_LIFE_MANA_AFTER_JOIN` (mặc định bật). **Open:** combat gửi life/mana thường xuyên hơn (full `GCLifeSend` parity).

---

## M7g — Bulk migrate JSON → Postgres (tất cả nhân vật)

- [x] **`CharacterRosterJsonMigrator`** — quét `takumi-roster/*.json`, mỗi file = một account, **mọi** entry trong `characters[]` → `character_roster` (+ `character_domain` khi sync bật).
- [x] Script **`./scripts/migrate-roster-json-to-db.sh`** (`TAKUMI_MIGRATE_ROSTER_JSON_ONLY=1`).
- [x] Startup tùy chọn: **`TAKUMI_MIGRATE_ROSTER_JSON=1`** trước khi host listen (Legacy + GameHost).
- [x] **`inventory_slot`** bulk từ **`inventory_staging`** (`008_inventory_staging.sql`, `InventoryStagingImporter`) — Postgres SSOT, không qua roster JSON.
- [x] **`CharacterRosterDiscovery`** — liệt kê nhân vật từ `character_roster` / `character_domain` / `inventory_slot` (không cần JSON).

---

## M7e — Kiểm thử

- [x] Unit test: JSON vitals — **`GameRosterVitalsJsonTests`**; seed join — **`JoinMapVitalsSeedTests`**.
- [x] `TEST_PG_CONNECTION_STRING`: vitals round-trip — **`CharacterRosterPostgresVitalsTests`** (cần `004_character_roster_vitals.sql` đã apply).
- [x] **`LifeManaWire602Tests`** — build/parse 0x26/0x27.

---

## M7f — Item inventory (liên kết M4 + M8)

- [x] **`F3 10`** đọc `inventory_slot` (12-byte) — `JoinInventoryPacket602` (xem M4 checklist §M5+).
- [x] **Ghi** `inventory_slot` sau buy/sell/repair — `InventorySlotMirrorWriter` + `PostgresInventorySlotRepository` (upsert/delete/replace); disconnect flush `PlayerShopSession.FlushInventoryMirrorOnDisconnect`. Cần **`TAKUMI_ROSTER_DB_SYNC=1`**.
- [x] Port `ItemManager` pick/drop/move (`0x22`–`0x24`) từ `Source/4.GameServer` — `ItemWorldHandler` (bag + wear slot 0–11, storage flag 0); trade/warehouse flags **OPEN**.
- [x] **M7d (partial):** potion use `CGItemUseRecv` (`C1`/`C3` `0x26`) — `ItemWorldHandler` + `InventoryConsumableRules` (HP/MP/SD potions); `0x28` delete / `0x2A` durability; SD session trên `GameRosterEntry` (chưa persist DB). Trade/warehouse/scroll **OPEN**.

---

## Liên kết milestone khác

| Milestone | Checklist |
|-----------|-----------|
| M4 — roster / tile / mirror | **`docs/M4-WORLD-POSITION-CHECKLIST.md`** |
| Port map `Source/` | **`docs/M4-M7-CHARACTER-ITEM-MIGRATION.md`** |
| M8 — ETL + shop commerce stub | **`docs/M8-M10-WORLD-RUNTIME-CHECKLIST.md`** §M8 |
| M9 — NPC / monster | cùng file §M9 |
| M10 — Movement + broadcast | cùng file §M10 |
