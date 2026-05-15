# Workstream ownership (tránh conflict giữa dev / nhánh)

Last updated: 2026-05-16  
**Nhánh tích hợp hiện tại:** `mac-m4` (đã merge `origin/main` tại `8c1758b` — M8 ETL + M7 vitals wire)

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
| **M5** split Connect/Login hosts | `2c45956` (mac-m4) / main tương đương | `LoginHost`, `ConnectHost`, thin `Program.cs` |

---

## Đã xong trên `mac-m4` (M9 — **đã merge main**, giữ nguyên khi rebase)

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

## Đang làm / WIP trên `mac-m4` (M10 — **chưa push**, stash `[M10]`)

| Hạng mục | Trạng thái | File (chỉ M10 touch) |
|----------|------------|----------------------|
| Map presence registry | WIP | `Game/Networking/GameMapPresenceRegistry.cs` |
| Broadcast tile `C1 0x15` | WIP | `PlayerPositionWire602.cs` + hook walk/join |
| Broadcast action `C1 0x18` | WIP | `PlayerActionWire602.cs` + `MonsterCombatHandler` |
| Miss / skill damage % | WIP | `MonsterCombatCalculator`, env `TAKUMI_COMBAT_*` |

**Chưa ai làm (để trống cho PR tiếp):**

| Hạng mục | Gợi ý owner / nhánh | Tránh đụng |
|----------|---------------------|------------|
| Player viewport `C2 0x12` (model) | M10b | `MonsterViewportWire602` layout |
| PvP / player vs player damage | M10c | `MonsterCombatHandler` (chỉ mob) |
| AoE skill / nhiều target | M10c | `ClientHitPackets602` |
| Monster AI / pathing | M9b hoặc M11 | `MapMonsterInstance` state machine |
| NPC viewport + shop list + gate + buy/sell/repair stub | **mac-m4** (`docs/M9-M8-NPC-GAMEPLAY-OWNERSHIP.md`) | Đừng sửa `WorldGameplayHandlers` / `ShopCommerceHandler` trừ fix bug |
| ItemValue / persist inventory sau shop | M8 + M4b | `ShopCommerceHandler` |
| M4b SSOT Postgres-only roster | M4/M7 | `takumi-roster/*.json` merge logic |

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

## Sau khi merge `mac-m4` → `main`

1. Cập nhật bảng **“Đã xong trên main”** ở đây.  
2. Đóng mục M9 trong `M9-NPC-MONSTER-CHECKLIST.md` / `IMPLEMENTATION-CHECKLIST.md`.  
3. M10 WIP → PR riêng `[M10] map presence broadcast` trước khi ai đó bắt `C2 0x12`.
