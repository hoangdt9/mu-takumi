# M8/M9 — NPC gameplay ownership (tránh duplicate)

Last updated: 2026-05-16 · Branch: **`mac-m4`** (chưa merge main)

## Đã làm trên `mac-m4` — **đừng re-implement**

| # | Tính năng | Owner file | Wire / env |
|---|-----------|------------|------------|
| A | Monster/NPC spawn viewport `C2 0x13`, destroy `0x14` | `MapMonsterWorld`, `MonsterViewportTracker`, `MapMonsterScopeSender` | M9 checklist |
| B | Monster AI wander/chase/attack, ATT path | `MonsterAiLoop`, `MapMonsterInstance`, `MapAttWalkability`, `MapTilePathfinder` | `TAKUMI_MONSTER_*`, `TAKUMI_ATT_DATA_ROOT` |
| C | Player↔mob combat stub | `MonsterCombatHandler`, `MonsterCombatCalculator` | `TAKUMI_COMBAT_*` |
| D | **NPC talk → shop list** | `WorldGameplayHandlers.TryHandleNpcTalkAsync`, `NpcShopWire602` | `NpcShopCatalog`, `ShopManager.txt` |
| E | **Gate teleport** | `WorldGameplayHandlers`, `MapGateService`, `TeleportWire602` | `MapGateCatalog`, `TAKUMI_GATE_SKIP_PROXIMITY` |
| F | **Shop buy/sell/repair** | `ShopCommerceHandler`, `PlayerShopSession`, `ShopCommerceWire602` | `TAKUMI_SHOP_*` |
| G | Hook TCP sau join | `LegacyLoginHostRunner.cs` (`CharacterRosterEntry` bridge), `GamePortMinimalSession.cs` | block `// [M8 gameplay]` |

**Bạn thấy NPC trong town** nhờ **(A)** spawn + sort NPC-first, không phải shop wire alone.

## Chưa làm — để PR khác

| # | Tính năng | Gợi ý owner |
|---|-----------|-------------|
| H | `0xDB` / `0x19` magic skill wire đầy đủ | M9c / M10 |
| I | Buy confirm `F3 ED`, coin shop | M8 |
| J | `GCItemValueSend` / giá từng item client | M8 |
| K | Persist `inventory_slot` upsert sau mỗi buy/sell | M4b + M8 |
| L | Encrypted `EncTerrain*.att` | M9b |
| M | NPC quest / warehouse / guild NPC | M11+ |
| N | Player viewport `C2 0x12` | M10b |

## File chỉ touch khi sửa NPC gameplay

- `World/WorldGameplayHandlers.cs` — orchestration (talk, gate, delegate shop)
- `World/ShopCommerceHandler.cs`, `World/PlayerShopSession.cs`, `World/ShopItemPricing.cs`
- `Protocol/ShopCommerceWire602.cs`, `Protocol/ClientGameplayPackets602.cs`
- `LegacyLoginHostRunner.cs` / `GamePortMinimalSession.cs` — **chỉ thêm hook**, không refactor

## QA nhanh shop

1. Login Lorencia → click NPC potion → log `[m8] sent shop 0x31`
2. Buy slot 0 → log `[m8] shop buy` + item trong túi (slot ≥ 12)
3. Sell item → zen tăng (`0x33` + gold trong packet)
4. Repair → `0x34`, durability full

Xem **`docs/M9-MONSTER-AI-PORT-CHECKLIST.md`** (P4) và **`docs/M8-M10-WORLD-RUNTIME-CHECKLIST.md`**.
