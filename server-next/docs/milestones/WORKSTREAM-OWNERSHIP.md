# Workstream ownership (tránh conflict giữa dev / nhánh)

Last updated: 2026-05-17  
**Nhật ký:** **`../../docs/journal/DEVELOPMENT-LOG-2026-05-17.md`**, **`../../docs/journal/DEVELOPMENT-LOG-2026-05-16.md`**.  
**Nhánh tích hợp hiện tại:** `main` — M4/M7 item + M8 ETL + M9 gameplay + shop F3 E9/ED + level-up VFX

**M4 / M7 (nhân vật + item):** migration map + ownership — **`character/M4-M7-CHARACTER-ITEM-MIGRATION.md`** (owner đề xuất: **`mac-m1`**).

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

## Đã xong trên `main` / `mac-m4` (M9 — viewport / combat / AI / gameplay stub)

| Hạng mục | Trạng thái | File chính (owner M9) |
|----------|------------|------------------------|
| MonsterSetBase + Monster.txt load | Done | `World/MonsterSetBaseLoader.cs`, `MonsterStatCatalog.cs`, `MapMonsterWorld.cs` (file load **fallback** khi DB trống) |
| Viewport spawn `C2 0x13` | Done | `MonsterViewportWire602.cs`, `MapMonsterScopeSender.cs`, `MonsterViewportTracker.cs` |
| Walk / instant move viewport | Done | Hook trong `LegacyLoginHostRunner.cs`, `GamePortMinimalSession.cs` |
| Regen từ Monster.txt | Done | `MapMonsterInstance.TryRegen` |
| Combat stub `0x11` / `0x19` → damage / die / destroy | Done | `MonsterCombatHandler.cs`, `ClientHitPackets602.cs`, wire `MonsterDamageWire602` |
| Destroy khi rời view `0x14` | Done | `MonsterViewportTracker.SyncView` |
| Defense trong damage | Done | `MonsterCombatCalculator.cs` |
| Gate / NPC shop / buy-sell-repair stub | Done | `MapGateService`, `WorldGameplayHandlers`, `ShopCommerceHandler` — xem `combat/M9-M8-NPC-GAMEPLAY-OWNERSHIP.md` |

**Không sửa** logic spawn DB trên M9 branch trừ khi fix bug — spawn Postgres thuộc **M8 owner** (`MapMonsterWorld` init path).

---

## Đã xong (M10 — map presence + viewport)

| Hạng mục | Trạng thái | File (chỉ M10 touch) |
|----------|------------|----------------------|
| Map presence registry | Done | `Game/Networking/GameMapPresenceRegistry.cs` |
| Broadcast tile `C1 0x15` + rate-limit | Done | `PlayerPositionWire602.cs`, `TAKUMI_PRESENCE_MAX_BROADCASTS_PER_SECOND` |
| Broadcast action `C1 0x18` | Done | `PlayerActionWire602.cs` + `MonsterCombatHandler` |
| Player viewport `C2 0x12` on walk | Done | `PlayerViewportWire602`, `PlayerViewportTracker` |
| PvP / AoE stub | Done | `MonsterCombatHandler`, `ClientHitPackets602` |

## Đang làm / WIP — **M4 + M7** (owner: `mac-m1`)

| Hạng mục | Trạng thái | File / ghi chú |
|----------|------------|----------------|
| Postgres-only roster SSOT | **OPEN** | `character/M4-ROSTER-SSOT.md`, `character` domain — **đừng** song song 2 writer |
| `inventory_slot` **write** sau shop buy/sell | **Done** | `InventorySlotMirrorWriter`, `PostgresInventorySlotRepository` — **`TAKUMI_ROSTER_DB_SYNC=1`** |
| Item pick/drop/move `0x22`–`0x24` | **Partial** | `ItemWorldHandler`, `InventoryBagGrid` — inv sync `6330de9`; trade/warehouse **OPEN** |
| Shop F3 E9 / F3 ED | **Done** | `ShopItemValueResolver`, client `ShopItemValueCache` — `../../docs/journal/DEVELOPMENT-LOG-2026-05-17.md` |
| Combat-driven `GCLifeSend` / vitals mid-fight | **OPEN** | M7d — `RosterVitalsOutboundTracker` mở rộng |
| `inventory_staging` ETL → 12-byte | **OPEN** | `IMPLEMENTATION-CHECKLIST` §Data & Migration |

**Chưa ai làm (để trống cho PR tiếp):**

| Hạng mục | Gợi ý owner | Tránh đụng |
|----------|-------------|------------|
| Warehouse DB load (class 240) | M8 + M4b | `NpcTalkService`, `WarehouseWire602` |
| Monster AI pathing nâng cao | M9b | `MapTilePathfinder` |
| Element/exp/invasion đầy đủ | M9c | `MonsterStatCatalog`, invasion spawns |

---

## Ma trận file → module (quick reference)

| File | Module | Ghi chú conflict |
|------|--------|------------------|
| `LegacyLoginHostRunner.cs` | M4 walk, M7 vitals, **M9**, **M10** | Merge theo block; giữ `monsterViewportTracker` + `WriteOutboundAsync` |
| `GamePortMinimalSession.cs` | M6, M7, **M9**, **M10** | Giống Legacy; thêm `protect` + `TrackVitalsOutbound` |
| `MapMonsterWorld.cs` | **M8** (main), M9 read-only | Chỉ M8 thêm ETL/DB; M9 dùng API public |
| `World/Monster*.cs` (trừ MapMonsterWorld init DB) | **M9** | An toàn cho M9 PR |
| `World/WorldGameplayHandlers.cs`, `ShopCommerceHandler.cs` | **M9/M8** | Đừng sửa trừ fix bug — xem `M9-M8-NPC-GAMEPLAY-OWNERSHIP.md` |
| `Game/Networking/GameMapPresenceRegistry.cs` | **M10** | File mới — ít conflict |
| `Protocol/Player*Wire602.cs` | **M10** | File mới |
| `sql/init/*.sql` | **M7/M8** | Một PR một migration |
| `milestones/IMPLEMENTATION-CHECKLIST.md` | Meta | Cập nhật checkbox, không rewrite lộ trình |

---

## Env theo module

| Module | Biến |
|--------|------|
| M8 DB spawn | `TAKUMI_MONSTER_SPAWN_DB`, ETL paths (xem README / M8 checklist) |
| M9 file fallback | `TAKUMI_MONSTER_SET_BASE_PATH`, `TAKUMI_MONSTER_INFO_PATH`, `TAKUMI_MONSTER_VIEW_*`, `TAKUMI_COMBAT_*` |
| M9 gameplay | `TAKUMI_GATE_SKIP_PROXIMITY`, `TAKUMI_SHOP_*` (xem `env.defaults`) |
| M10 presence | `TAKUMI_MAP_PRESENCE_ENABLED` (`0` = tắt), `TAKUMI_PLAYER_VIEWPORT_WIRE` (`0` = chỉ `0x15`), `TAKUMI_COMBAT_MISS_RATE_PCT`, `TAKUMI_COMBAT_SKILL_DAMAGE_PCT` |

---

## Tiếp theo

1. Giữ bảng **“Đã xong trên main”** cập nhật khi merge PR.  
2. **M4/M7:** làm theo thứ tự trong **`character/M4-M7-CHARACTER-ITEM-MIGRATION.md`** — ưu tiên SSOT design + item world ops.  
3. Sau khi `mac-m4` → `main` ổn định: feature branch mới rebase `origin/main`.
