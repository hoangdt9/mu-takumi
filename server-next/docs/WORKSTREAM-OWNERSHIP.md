# Workstream ownership (tránh conflict giữa dev / nhánh)

Last updated: 2026-05-16  
**Nhánh tích hợp hiện tại:** `main` @ `0fee99a` (M8/M9 gameplay + M7 vitals + M10 presence + M6 game-host data mount)

**M4 / M7 (nhân vật + item):** migration map + ownership — **`docs/M4-M7-CHARACTER-ITEM-MIGRATION.md`** (owner đề xuất: **`mac-m1`**).

## Quy ước làm việc

1. **Trước khi mở PR / push feature:** `git fetch origin main` rồi **`git merge origin/main`** (hoặc `git rebase origin/main` nếu history gọn) vào nhánh feature — **không** làm lại M8/M7 đã có trên `main`.
2. **Một hạng mục = một owner** (hoặc một PR). Nếu cần sửa file “owned”, ping owner hoặc rebase lên commit mới nhất của họ.
3. **Hook chung** (dễ conflict): chỉ `LegacyLoginHostRunner.cs` và `GamePortMinimalSession.cs` — thay đổi theo **block** có comment `// [M9]` / `// [M10]`; tránh refactor toàn file.

---

## Đã xong trên `main` (đừng re-implement trên nhánh feature)

| Hạng mục | Commit gần nhất (main) | File / ghi chú |
|----------|-------------------------|----------------|
| **M8** spawn ETL Postgres | `b33d890` | `MapMonsterWorld.TryLoadSetBaseFromPostgres`, `sql/init/*monster*`, ETL scripts |
| **M8** gates / NPC shop / Custom | `8c1758b` | Persistence ETL + world data tables (xem `M8-M10-WORLD-RUNTIME-CHECKLIST.md`) |
| **M7** vitals mirror + join seed | `68b31c1` | `RosterVitalsLifecycle`, `JoinMapVitalsSeed`, `004_character_roster_vitals.sql` |
| **M7** Life/Mana wire + outbound track | `68b31c1` | `WriteOutboundAsync` / `TrackVitalsOutbound` trong Legacy + Game port |
| **M4** roster JSON + walk + mirror | (nhiều commit) | `takumi-roster/`, `CharacterRosterMirrorWriter`, `ClientWalkPackets602` |
| **M4** `F3 10` read từ DB | `002_inventory_slot.sql` | `JoinInventoryPacket602`, `PostgresInventorySlotRepository` |
| **M10** presence rate-limit | `6d5a810` | `GameMapPresenceRegistry`, `TAKUMI_PRESENCE_MAX_BROADCASTS_PER_SECOND` |
| **M6** game-host MuServer mount | `b66efab` | `docker-compose.yml` `game-host` + `MapMonsterWorld` boot |
| **M5** split Connect/Login hosts | `2c45956` | `LoginHost`, `ConnectHost`, profile `splitstack` |

---

## Đã xong trên `main` (M9 — viewport / combat / AI)

| Hạng mục | Trạng thái | File chính (owner M9) |
|----------|------------|------------------------|
| MonsterSetBase + Monster.txt load | Done | `World/MonsterSetBaseLoader.cs`, `MonsterStatCatalog.cs`, `MapMonsterWorld.cs` (file load **fallback** khi DB trống) |
| Viewport spawn `C2 0x13` | Done | `MonsterViewportWire602.cs`, `MapMonsterScopeSender.cs`, `MonsterViewportTracker.cs` |
| Walk / instant move viewport | Done | Hook trong `LegacyLoginHostRunner.cs`, `GamePortMinimalSession.cs` |
| Regen từ Monster.txt | Done | `MapMonsterInstance.TryRegen` |
| Combat stub `0x11` / `0x19` → damage / die / destroy | Done | `MonsterCombatHandler.cs`, `ClientHitPackets602.cs`, wire `MonsterDamageWire602` |
| Destroy khi rời view `0x14` | Done | `MonsterViewportTracker.SyncView` |
| Defense trong damage | Done | `MonsterCombatCalculator.cs` |

**Không sửa** logic spawn DB trên M9 branch trừ khi fix bug — spawn Postgres thuộc **M8 owner** (`MapMonsterWorld` init path).

---

## Đang làm / WIP — **M4 + M7** (owner: `mac-m1`)

| Hạng mục | Trạng thái | File / ghi chú |
|----------|------------|----------------|
| Postgres-only roster SSOT | **OPEN** | `docs/M4-ROSTER-SSOT.md`, `character` domain — **đừng** song song 2 writer |
| `inventory_slot` **write** sau shop buy/sell | **Done** | `InventorySlotMirrorWriter`, `PostgresInventorySlotRepository` — **`TAKUMI_ROSTER_DB_SYNC=1`** |
| Item pick/drop/move `0x22`–`0x24` | **OPEN** | Port từ `ItemManager.cpp` — PR riêng |
| Combat-driven `GCLifeSend` / vitals mid-fight | **OPEN** | M7d — `RosterVitalsOutboundTracker` mở rộng |
| `inventory_staging` ETL → 12-byte | **OPEN** | `IMPLEMENTATION-CHECKLIST` §Data & Migration |

**Chưa ai làm (để trống cho PR tiếp):**

| Hạng mục | Gợi ý owner | Tránh đụng |
|----------|-------------|------------|
| Player viewport `C2 0x12` | M10b | `MonsterViewportWire602` |
| PvP / AoE | M10c | `MonsterCombatHandler` |
| NPC shop/gate/commerce | **main** (done) | `WorldGameplayHandlers` — **`docs/M9-M8-NPC-GAMEPLAY-OWNERSHIP.md`** |
| Monster AI pathing nâng cao | M9b | `MapTilePathfinder` (đã có trên main) |

---

## Ma trận file → module (quick reference)

| File | Module | Ghi chú conflict |
|------|--------|------------------|
| `LegacyLoginHostRunner.cs` | M4 walk, M7 vitals, **M9**, **M10** | Merge theo block; giữ `monsterViewportTracker` + `WriteOutboundAsync` |
| `GamePortMinimalSession.cs` | M6, M7, **M9**, **M10** | Giống Legacy; thêm `protect` + `TrackVitalsOutbound` |
| `MapMonsterWorld.cs` | **M8** (main), M9 read-only | Chỉ M8 thêm ETL/DB; M9 dùng API public |
| `World/Monster*.cs` (trừ MapMonsterWorld init DB) | **M9** | An toàn cho M9 PR |
| `Game/Networking/GameMapPresenceRegistry.cs` | **M10** | File mới — ít conflict |
| `Protocol/Player*Wire602.cs` | **M10** | File mới |
| `sql/init/*.sql` | **M7/M8** | Một PR một migration |
| `docs/IMPLEMENTATION-CHECKLIST.md` | Meta | Cập nhật checkbox, không rewrite lộ trình |

---

## Env theo module

| Module | Biến |
|--------|------|
| M8 DB spawn | `TAKUMI_MONSTER_SPAWN_DB`, ETL paths (xem README / M8 checklist) |
| M9 file fallback | `TAKUMI_MONSTER_SET_BASE_PATH`, `TAKUMI_MONSTER_INFO_PATH`, `TAKUMI_MONSTER_VIEW_*`, `TAKUMI_COMBAT_*` |
| M10 presence | `TAKUMI_MAP_PRESENCE_ENABLED` (`0` = tắt), `TAKUMI_COMBAT_MISS_RATE_PCT`, `TAKUMI_COMBAT_SKILL_DAMAGE_PCT` |

---

## Tiếp theo trên `main`

1. Giữ bảng **“Đã xong trên main”** cập nhật khi merge PR.  
2. **M4/M7:** làm theo thứ tự trong **`docs/M4-M7-CHARACTER-ITEM-MIGRATION.md`** — ưu tiên `inventory_slot` write + SSOT design trước item world ops.  
3. M10: player viewport `C2 0x12`, PvP, AoE — PR riêng; presence broadcast **đã** trên main.
