# M7 — Character persistence lifecycle (HP / MP / zen / map / tile)

Last updated: 2026-05-18

**Quy ước:** chỉ tick `[x]` khi đã merge + smoke/unit (nếu có). QA APK: **`../../docs/QA-MILESTONE.md`**.  
**Nhật ký:** DEVELOPMENT-LOG 2026-05-16 → 18.

**Phụ thuộc:** **`character/M4-TILE-AND-COORDINATES.md`**, **`character/M4-ROSTER-SSOT.md`**, **`character/M6-GAME-TCP-CHECKLIST.md`**.  
**Port từ `Source/` (character + item):** **`character/M4-M7-CHARACTER-ITEM-MIGRATION.md`**. **Không chặn M5** (join/ticket).

**Tham chiếu client / legacy:** `Source/4.GameServer` save/load nhân vật; `WSclient.h` join stats (`PRECEIVE_JOIN_MAP_SERVER`); M1 map opcode sau vào thế giới.

---

## M7a — Schema & migration

- [x] `sql/init/004_character_roster_vitals.sql` — thêm `current_hp`, `max_hp`, `current_mp`, `max_mp`, `zen` trên `public.character_roster` (`ALTER … IF NOT EXISTS`).
- [x] `sql/init/011_character_experience.sql` — `experience BIGINT` trên `character_roster` + `character_domain`.
- [x] `sql/patches/015_character_key_configuration.sql` — `key_configuration BYTEA` trên **`character_roster`** (10 skill hotkey + QWER + flags, 30 byte — parity client `SaveOptions` / `C1 F3 30`).
- [x] Ghi chú trong `README.md` / `apply-sql.sh` header nếu thứ tự file thay đổi (hiện `001` → `004` theo tên).
- [x] (Tuỳ chọn) Cột `shield` / `skill_mana` nếu join wire cần parity đầy đủ hơn — đối chiếu `JoinMapServerWire602` offsets. **Đã có:** `current_shield` / `max_shield` trên `character_roster` + `character_domain` (`008_character_roster_shield.sql`), join `F3 03` + vitals upsert + JSON roster; `skill_mana` join vẫn mirror mana khi chưa có cột riêng.

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
- [x] Mục “vitals overlay” trong **`character/M4-ROSTER-SSOT.md`**.

---

## M7d — Vòng đời phiên (save triggers)

- [x] Lưu vitals khi **disconnect** (cùng `SavePersistedRoster` / `SaveRoster` — đã ghi field vitals; cần seed trước, xem dưới).
- [x] **Seed sau join / move-map:** `JoinMapVitalsSeed` + `RosterVitalsLifecycle` — copy HP/MP/zen từ **`F3 03`** vào roster khi `max_hp == 0`; `rosterDirty` cho **`TAKUMI_ROSTER_PERIODIC_SAVE_SECONDS`**.
- [x] **Cả hai host:** `LegacyLoginHost` + `GamePortMinimalSession`.
- [x] **`LifeManaWire602`** + **`RosterVitalsOutboundTracker`** — quét outbound `C1 0x26`/`0x27` (type `0xFF`/`0xFE`) cập nhật roster + `rosterDirty` (**bao gồm shield** trong word thứ hai của `0x26`); gửi sync sau join khi `TAKUMI_SEND_LIFE_MANA_AFTER_JOIN` (mặc định bật) — thứ tự legacy: max life+SD rồi current.
- [x] **EXP / level persist:** `011_character_experience.sql`, `ExperienceProgression602`, `RosterExperienceCombat` — grant on monster kill (`0x16`), `UpsertProgressAsync` + `ScheduleProgressUpsert`, join `F3 03` offset 8; **`CharacterRosterEntryMapping`** full DB load (stats + exp).
- [x] **Progress mirror:** `RosterProgressMirror` + `ScheduleProgressUpsert` cập nhật cả `character_roster` và `character_domain` (level/EXP/stats/vitals) sau kill EXP và `F3 06` stat point.
- [x] **Stat allocation UI:** client `CAddStatPointMsgBoxLayout` gửi `C1 F3 06` (`SendRequestAddPoint`) thay chat `/addstr`; Android tap-to-focus trên `CNewUITextInputMsgBox` + không dismiss IME khi message box đang mở; server `CharacterStatPointHandler` + `LevelUpPointWire602` word `Max` cho Vit/Ene.
- [x] **Batch `F3 06` (server):** `CharacterStatPointHandler` — parse nhiều `C1 F3 06` trong một decrypted buffer → một `LevelUpPointWire602` success + một mirror upsert.
- [x] **Stat pump (Android):** `TakumiScheduleLevelUpPoints` / `TakumiPumpLevelUpPoints` — tối đa 4 gói/frame; tránh flood disconnect (`4e84ff3`).
- [x] **Combat HUD (`C1 F3 E1`):** `NewCharacterCalcWire602` (172 byte) + `CharacterCombatPreview602` — gửi sau join trên `LegacyLoginHostRunner` + `GamePortMinimalSession`; tests `NewCharacterCalcWire602Tests.cs`.
- [x] **Monster → player defense:** `MonsterViewerRegistry` dùng `CharacterCombatPreview602.ResolvePlayerDefense` khi sheet có base stats (không stub `level*3` thuần).

---

## M7g — Bulk migrate JSON → Postgres (tất cả nhân vật)

- [x] **`CharacterRosterJsonMigrator`** — quét `takumi-roster/*.json`, mỗi file = một account, **mọi** entry trong `characters[]` → `character_roster` (+ `character_domain` khi sync bật).
- [x] Script **`./scripts/db/migrate-roster-json-to-db.sh`** (`TAKUMI_MIGRATE_ROSTER_JSON_ONLY=1`).
- [x] Startup tùy chọn: **`TAKUMI_MIGRATE_ROSTER_JSON=1`** trước khi host listen (Legacy + GameHost).
- [x] **`inventory_slot`** bulk từ JSON — `InventorySlotJsonMigrator` quét `takumi-inventory/*.json` (mỗi file = account, `characters[].slots[]` với `itemHex` hoặc `itemBase64` 12 byte); script **`./scripts/db/migrate-inventory-json-to-db.sh`**; startup **`TAKUMI_MIGRATE_INVENTORY_JSON=1`**.

---

## M7e — Kiểm thử

- [x] Unit test: JSON vitals — **`GameRosterVitalsJsonTests`**; seed join — **`JoinMapVitalsSeedTests`**.
- [x] `TEST_PG_CONNECTION_STRING`: vitals round-trip — **`CharacterRosterPostgresVitalsTests`** (cần `004_character_roster_vitals.sql` đã apply).
- [x] **`LifeManaWire602Tests`** — build/parse 0x26/0x27.
- [x] **`ExperienceProgression602Tests`** — level-up from kill EXP + join wire EXP offset.
- [x] **`NewCharacterCalcWire602Tests`** — golden length/head `F3 E1`.

---

## M7f — Item inventory (liên kết M4 + M8)

- [x] **`F3 10`** đọc `inventory_slot` (12-byte) — `JoinInventoryPacket602` (xem M4 checklist §M5+).
- [x] **Ghi** `inventory_slot` sau buy/sell/repair — `InventorySlotMirrorWriter` + `PostgresInventorySlotRepository` (upsert/delete/replace); disconnect flush `PlayerShopSession.FlushInventoryMirrorOnDisconnect`. Cần **`TAKUMI_ROSTER_DB_SYNC=1`**.
- [x] Port `ItemManager` pick/drop/move (`0x22`–`0x24`) — `ItemWorldHandler` + **`InventoryBagGrid`** / BMD footprints; server gửi **`0x24` trước `F3 10`** trên inv→inv; client plain **`C4 F3 10`** + footprint clear (`6330de9`). Trade/warehouse flags **OPEN**.
- [x] **Level-up VFX (client, 2026-05-17):** FLARE spiral + white disc — không liên quan wire M7; xem **`../../docs/journal/DEVELOPMENT-LOG-2026-05-17.md`**.
- [x] **M7d (partial):** potion use `CGItemUseRecv` (`C1`/`C3` `0x26`) — `ItemWorldHandler` + `InventoryConsumableRules` (HP/MP/SD potions); `0x28` delete / `0x2A` durability; **SD persist:** `current_shield`/`max_shield` DB + roster JSON + `GCLifeSend` shield word; monster/PvP hit giảm SD trước HP (parity `Attack.cpp`). Trade/warehouse/scroll **OPEN**.

---

## M7h — wear seed dev (`mg001`)

- [x] **`sql/patches/013_test_account_mg001_seed.sql`** + **`takumi-inventory/test.json`** mirror.
- [x] Script **`scripts/apply-dev-character-seeds.sh`** — apply `015` + `013` + `016`/`017` + optional inventory JSON migrate.
- [ ] Xác nhận wear trên APK → QA milestone (không chặn dev).

---

## M7i — Skill hotkey / option blob (`C1 F3 30`)

**Không lưu trong `character_domain`.** Bảng đó mirror stats/EXP/vitals; **skill bar + Q/W/E/R** nằm ở **`public.character_roster.key_configuration`** (cột `bytea`, 30 byte).

| Bước | Thành phần |
|------|------------|
| Client gửi | `SaveOptions()` → `SendRequestHotKey` → **`C1 F3 30`** + 30 byte (`Source/5.Main/source/ZzzOpenData.cpp`) |
| Server nhận | `CharacterOptionHandler` (`WorldGameplayHandlers`) trên **game-host** (55901) |
| Ghi DB ngay | `CharacterRosterMirrorWriter.ScheduleKeyConfigurationUpsert` → `UPDATE character_roster SET key_configuration = …` |
| Ghi JSON (dev) | `GameRosterDisk.SaveEntries` → `KeyConfigurationBase64` trong `takumi-roster/<account>.json` |
| Vào game lại | Sau join: server gửi **`C1 F3 30`** (`OptionDataWire602.BuildApply`) → client `ReceiveOption` |

**Apply SQL (volume đã chạy):**

```bash
cd server-next
./scripts/db/apply-sql.sh "postgresql://takumi:takumi@127.0.0.1:54444/takumi_runtime" sql/patches/015_character_key_configuration.sql
```

**DBeaver / psql kiểm tra:**

```sql
SELECT character_name, account_login,
       length(key_configuration) AS bytes,
       encode(key_configuration, 'hex') AS cfg_hex
FROM character_roster
WHERE account_login = 'test'   -- đổi account đang chơi
ORDER BY character_name;
```

- `bytes` **NULL** = chưa từng lưu (hoặc nhân vật tạo trước khi có cột).
- Sau khi gán skill (vd. Độc → ô **0**) và thoát map: `bytes` = **30**, hex bắt đầu bằng skill hotkey (mỗi slot 2 byte skill type, `ffff` = trống).

**Client (Android legacy HUD):**

- Ô skill chính = hotkey slot **`0`** (ô có số **0** trên bar 6–9–0).
- Gán skill → `SetHotKey` → `SaveOptions()` (trừ lúc `ReceiveOption` đang hydrate).
- Chi tiết wire + QA: **`../../docs/game-spec/SKILL-HOTKEY-PERSISTENCE.md`**.

- [x] Schema `015` + repo `UpdateKeyConfigurationAsync` + load/join `BuildApply`.
- [x] Handler `CharacterOptionHandler` + upsert ngay khi nhận `F3 30` từ client.
- [x] Client: `SetHotKey` auto-`SaveOptions`, Android gán ô 0 qua `ApplySelectedSkillIndex`.
- [ ] **QA APK:** login → gán Độc vào ô **0** → relog → vẫn ô **0** + `key_configuration` có dữ liệu trong DB.

---

## M7k — Character create/delete error codes

- [x] `CharacterRosterErrorCodes` + `CharacterRosterOps` — `F3 01`/`F3 02` reject with client-visible result bytes.
- [x] Doc: **`docs/CHARACTER-ROSTER-ERROR-CODES.md`**.

---

## M7j — Learned skill list (`C1 F3 11`)

**Phụ thuộc M7i:** hotkey blob lưu **skill type**; client `ReceiveOption` chỉ khớp khi `F3 11` đã nạp `CharacterAttribute->Skill[]`.

| Bước | Thành phần |
|------|------------|
| Schema | `sql/patches/016_character_skill.sql` — `character_skill(skill_slot, skill_type, skill_level)` |
| Join | `JoinSkillLifecycle` — load DB → nếu trống seed `CharacterSkillCatalog` theo class → ghi DB → `MagicListWire602` |
| QA seed | `sql/patches/017_seed_mg001_character_skill.sql` |
| Hosts | `GamePortMinimalSession`, `LegacyLoginHostRunner` (sau `F3 10`, trước `F3 30`) |

**Apply SQL:**

```bash
cd server-next
./scripts/db/apply-sql.sh "postgresql://takumi:takumi@127.0.0.1:54444/takumi_runtime" sql/patches/016_character_skill.sql
./scripts/db/apply-sql.sh "postgresql://takumi:takumi@127.0.0.1:54444/takumi_runtime" sql/patches/017_seed_mg001_character_skill.sql
```

- [x] Table `character_skill` + `PostgresCharacterSkillRepository`.
- [x] Class default kits (Elf có skill **1** = Độc) + join seed khi DB trống.
- [x] Wire join qua `JoinSkillLifecycle` (cần `TAKUMI_ROSTER_DB_SYNC=1`).
- [ ] **QA APK:** Elf gán Độc ô **0** → relog → skill còn + hotkey còn (`key_configuration` + `character_skill`).
- [ ] Skill learn / master tree / `F3 11` delta updates — **OPEN** (M11).

---

## Liên kết milestone khác

| Milestone | Checklist |
|-----------|-----------|
| M4 — roster / tile / mirror | **`character/M4-WORLD-POSITION-CHECKLIST.md`** |
| Port map `Source/` | **`character/M4-M7-CHARACTER-ITEM-MIGRATION.md`** |
| M8 — ETL + shop commerce stub | **`world/M8-M10-WORLD-RUNTIME-CHECKLIST.md`** §M8 |
| M9 — NPC / monster | cùng file §M9 |
| M10 — Movement + broadcast | cùng file §M10 |
| Skill hotkey / `F3 30` | **`../../docs/game-spec/SKILL-HOTKEY-PERSISTENCE.md`** |
