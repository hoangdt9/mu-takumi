# M8/M9 — NPC gameplay ownership (tránh duplicate)

Last updated: 2026-05-17 (shop tooltip buy/sell parity) · **Đã trên `main`** (F3 E9/ED, inventory 0x24 — xem **`../../docs/DEVELOPMENT-LOG-2026-05-17.md`**)

## Đã làm trên `mac-m4` — **đừng re-implement**

| # | Tính năng | Owner file | Wire / env |
|---|-----------|------------|------------|
| A | Monster/NPC spawn viewport `C2 0x13`, destroy `0x14` | `MapMonsterWorld`, `MonsterViewportTracker`, `MapMonsterScopeSender` | M9 checklist |
| B | Monster AI wander/chase/attack, ATT path | `MonsterAiLoop`, `MapMonsterInstance`, `MapAttWalkability`, `MapTilePathfinder` | `TAKUMI_MONSTER_*`, `TAKUMI_ATT_DATA_ROOT` |
| C | Player↔mob combat stub | `MonsterCombatHandler`, `MonsterCombatCalculator` | `TAKUMI_COMBAT_*` |
| D | **NPC talk → shop list** | `WorldGameplayHandlers.TryHandleNpcTalkAsync`, `NpcShopWire602` | `NpcShopCatalog`, `ShopManager.txt` |
| E | **Gate teleport** | `WorldGameplayHandlers`, `MapGateService`, `TeleportWire602` | `MapGateCatalog`, `TAKUMI_GATE_SKIP_PROXIMITY` |
| F | **Shop buy/sell/repair** | `ShopCommerceHandler`, `ShopItemValueResolver`, `PlayerShopSession`, `ShopCommerceWire602` | `TAKUMI_SHOP_*`, **`TAKUMI_SHOP_BUY_CONFIRM`** |
| F2 | **Item values on wire** | `ShopItemValueSender` → **`C2 F3 E9`**; client **`ShopItemValueCache`** | `ItemValue.txt` |
| G | Hook TCP sau join | `LegacyLoginHostRunner.cs` (`CharacterRosterEntry` bridge), `GamePortMinimalSession.cs` | block `// [M8 gameplay]` |

**Bạn thấy NPC trong town** nhờ **(A)** spawn + sort NPC-first, không phải shop wire alone.

## Chưa làm — để PR khác

| # | Tính năng | Gợi ý owner |
|---|-----------|-------------|
| H | ~~`0xDB` / `0x19` magic AoE~~ | Done (stub) |
| I | ~~Buy confirm `F3 ED`~~ | **Done** — `TAKUMI_SHOP_BUY_CONFIRM=1`; client `ReceiveBuyConfirm` + NPC shop msgbox. Coin-only shop: server debits `account.wcoin_*` / `goblin_point` (`012_account_wallet.sql`) |
| J | ~~`GCItemValueSend` / `ItemValue.txt`~~ | **Done** — server `F3 E9` + `ShopItemValueResolver` + client `ShopItemValueCache`. **2026-05-17:** tooltip **Giá mua** (shop grid, `Sell=false`) + **Giá bán** (túi, exc → `ItemValue(ip,0)`); potion `EstimatePotionBuy` server |
| K | ~~Persist `inventory_slot`~~ | **Done** (`InventorySlotPersist`); **2026-05-17:** `0x24`/`F3 10` move sync + BMD footprints (`6330de9`) |
| L | ~~Encrypted `EncTerrain*.att`~~ | Done (`ModulusCryptor` in `MapAttWalkability`) |
| M | NPC quest / warehouse / guild NPC | M11+ |
| N | ~~Player viewport walk `C2 0x12`~~ | Done (`PlayerViewportTracker` + M10b) |

## File chỉ touch khi sửa NPC gameplay

- `World/WorldGameplayHandlers.cs` — orchestration (talk, gate, delegate shop)
- `World/ShopCommerceHandler.cs`, `World/PlayerShopSession.cs`, `World/ShopItemPricing.cs`
- `Protocol/ShopCommerceWire602.cs`, `Protocol/ClientGameplayPackets602.cs`
- `LegacyLoginHostRunner.cs` / `GamePortMinimalSession.cs` — **chỉ thêm hook**, không refactor

## QA nhanh shop

1. Login Lorencia → click NPC potion → log `[m8] sent shop 0x31`
2. **Mua:** hover bình máu trong **Người bán** → **Giá mua** khớp Zen trừ; tap 2 → confirm (một OK)
3. **Bán:** hover đồ **exc** trong **Trang bị** → **Giá bán** khớp chat **Nhận … Zen** (~40M cho quần +9 exc đã QA)
4. Sell wire → zen tăng (`0x33` + gold sync); repair → `0x34`, durability full

**APK path (release):** `Source/android/app/build/outputs/apk/realDevicePreloadDefault/release/app-realDevice-preloadDefault-release.apk` (không có `/` giữa `realDevice` và `preloadDefault`).

Xem **`docs/M9-MONSTER-AI-PORT-CHECKLIST.md`** (P4) và **`docs/M8-M10-WORLD-RUNTIME-CHECKLIST.md`**.
