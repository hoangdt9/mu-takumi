# M8–M10 — World data, NPC runtime, movement visibility

**Quy ước:** giống các checklist khác — `[x]` chỉ khi đã merge thật. File này gom **M8 / M9 / M10** vì chúng cùng chuỗi “thế giới sau login”.

**Phụ thuộc:** **M7** (`docs/M7-CHARACTER-PERSISTENCE-CHECKLIST.md`) cho stats persist; **M6** (`docs/M6-GAME-TCP-CHECKLIST.md`) cho listener game TCP; **M1** cho opcode map.

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
- [x] Runtime gameplay: `C1 0x1C` teleport (`MapGateTeleportHandler`) / NPC shop `0x30` + `C2 0x31` (`NpcShopHandler`).

---

## M9 — NPC & monster runtime

- [ ] Spawn theo `map_id` + tọa độ; bảng `monster_id` → stats tối thiểu (HP).
- [ ] Gói **scope** tới client (opcode theo **M1**); chỉ gửi cho session đã join hợp lệ.

---

## M10 — Movement & visibility

- [x] Walk / instant move trên TCP minimal → cập nhật **tile** roster (**M4c** — `LegacyLoginHost`, `GamePortMinimalSession`).
- [x] **Broadcast** xung quanh player (`GameMapPresenceRegistry` `0x15`/`0x18`).
- [x] Anti-flood broadcast: `TAKUMI_PRESENCE_MAX_BROADCASTS_PER_SECOND`.
- [ ] Đồng bộ sâu vitals mid-combat (M7).

---

## M11 — DataServer (xem trước)

- [ ] Quyết định Postgres-only vs bridge MSSQL legacy — **`docs/IMPLEMENTATION-CHECKLIST.md`** §11; API nội bộ cho `Takumi.Server.Game`.
