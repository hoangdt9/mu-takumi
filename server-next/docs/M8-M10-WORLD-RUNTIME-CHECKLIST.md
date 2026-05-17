# M8–M10 — World data, NPC runtime, movement visibility

Last updated: 2026-05-17

**Quy ước:** giống các checklist khác — `[x]` chỉ khi đã merge thật. File này gom **M8 / M9 / M10** vì chúng cùng chuỗi “thế giới sau login”.

**Phụ thuộc:** **M7** (`docs/M7-CHARACTER-PERSISTENCE-CHECKLIST.md`) cho stats persist; **M6** (`docs/M6-GAME-TCP-CHECKLIST.md`) cho listener game TCP; **M1** cho opcode map.  
**Nhật ký gần đây:** **`../../docs/DEVELOPMENT-LOG-2026-05-17.md`**.

---

## M8 — Dữ liệu tĩnh thế giới (ETL)

- [x] Schema **`sql/init/005_monster_spawn.sql`** (`monster_spawn` — parity `MONSTER_SET_BASE_INFO`).
- [x] ETL: **`MonsterSpawnDbImporter`** + **`scripts/import-monster-spawn.sh`** (cần `TEST_PG` / `TAKUMI_PG_*` + `TAKUMI_MONSTER_SET_BASE_PATH`).
- [x] Reader: **`PostgresMonsterSpawnRepository`** + **`TakumiPostgresMirror.InitMonsterSpawnIfEnabled`** (`TAKUMI_MONSTER_SPAWN_DB=1`).
- [x] Runtime: **`MapMonsterWorld`** ưu tiên DB khi bảng có dữ liệu; fallback file / Lorencia như M9.
- [x] **Gates:** `sql/init/006` → `map_gate` — loader **`GateLoader`** (`Move/Gate.txt`, parity **`CGate`**), ETL **`MapGateDbImporter`**, catalog **`MapGateCatalog`** (`TAKUMI_MAP_GATE_DB` / `TAKUMI_WORLD_STATIC_DB`).
- [x] **Shops:** `npc_shop` + `npc_shop_item` — **`ShopManagerLoader`** + **`ShopItemLoader`**, ETL **`NpcShopDbImporter`**, catalog **`NpcShopCatalog`** (`TAKUMI_NPC_SHOP_DB`).
- [x] **Custom:** `custom_world_config` (JSONB table/raw snapshot) — **`CustomWorldConfigDbImporter`** (`Custom/*.txt` + `.ini`/`.xml` raw); `TAKUMI_CUSTOM_WORLD_DB`.
- [x] Script gộp: **`scripts/import-world-static-data.sh`** (cần `TAKUMI_GAMESERVER_DATA_PATH` hoặc auto-detect MuServer Data).
- [x] Runtime gameplay: `C1 0x1C` teleport / `C2 0x31` shop list (`WorldGameplayHandlers`, `MapGateService`, `NpcShopWire602`).
- [x] **Shop wire (2026-05-17):** `F3 E9` item values (`ShopItemValueResolver`), optional **`F3 ED`** buy confirm; client `ShopItemValueCache`.
- [x] **Inventory placement:** `InventoryBagGrid` + BMD footprints; `0x24` + `F3 10` resync (`6330de9`).
- [x] **Move map (M):** `C1 0A 8E 02` → `MoveMapCatalog` + `MoveMapHandler` (zen/level gate, `8E 03` + `0x1C` + join reload).
- [~] **Move map parity:** chi tiết từng rule → **`docs/M8-MOVE-MAP-PARITY-CHECKLIST.md`** (`8E 01` checksum + key validate mới thêm).

---

## M9 — NPC & monster runtime

Chi tiết: **`docs/M9-NPC-MONSTER-CHECKLIST.md`**, **`docs/M9-MONSTER-AI-PORT-CHECKLIST.md`**.

- [x] Spawn theo `map_id` + tọa độ; `Monster.txt` stats; Postgres khi `TAKUMI_MONSTER_SPAWN_DB=1`.
- [x] Scope `C2 0x13` / destroy `0x14`; AI wander/chase; combat stub; gate/shop/commerce stub.
- [ ] Port còn lại từ `Source/4.GameServer`: encrypted ATT, skill/element, invasion spawns, quest NPC — xem P2.5–P4 trong AI port checklist.

---

## M10 — Movement & visibility

- [x] Walk / instant move → roster tile (**M4c**).
- [x] Broadcast `C1 0x15` / `0x18` peers cùng map (`GameMapPresenceRegistry`).
- [x] `C2 0x12` player viewport on join + walk view range + `0x14` on leave (`PlayerViewportWire602`, `PlayerViewportTracker`).
- [ ] Anti-flood broadcast; vitals mid-combat (M7).

---

## M11 — DataServer (xem trước)

- [ ] Quyết định Postgres-only vs bridge MSSQL legacy — **`docs/IMPLEMENTATION-CHECKLIST.md`** §11; API nội bộ cho `Takumi.Server.Game`.
