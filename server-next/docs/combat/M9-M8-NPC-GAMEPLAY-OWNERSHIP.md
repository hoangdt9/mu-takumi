# M9/M8 — NPC gameplay ownership — **dev only**

Last updated: 2026-05-18

**QA APK:** [`../../docs/qa/M9-npc-shop.md`](../../docs/qa/M9-npc-shop.md) · [`../../docs/QA-MILESTONE.md`](../../docs/QA-MILESTONE.md)

Orchestration: **`World/WorldGameplayHandlers.cs`**. **Đừng re-implement** các mục `[x]` dưới đây.

---

## Đã làm (owner)

| # | Tính năng | File | Wire / env |
|---|-----------|------|------------|
| A | Monster/NPC viewport `0x13` / `0x14` | `MapMonsterScopeSender`, `MonsterViewportTracker` | M9 |
| B | Monster AI wander/chase/attack | `MonsterAiLoop`, `MapAttWalkability` | `TAKUMI_MONSTER_*` |
| C | Player↔mob combat | `MonsterCombatHandler` | `TAKUMI_COMBAT_*` |
| D | NPC talk → shop `0x31` | `TryHandleNpcTalkAsync`, `NpcShopWire602` | `NpcShopCatalog` |
| E | Gate teleport `0x1C` | `MapGateService`, `TeleportWire602` | `TAKUMI_GATE_SKIP_PROXIMITY` |
| E2 | Skill teleport `gate==0` | `SkillTeleportService`, `TryHandleSkillTeleportAsync` | `TAKUMI_SKILL_TELEPORT_*` |
| E3 | Custom NPC warp | `CustomNpcMoveHandler`, `MoveWarpExecutor` | `TAKUMI_CUSTOM_NPC_MOVE_PATH` |
| F | Shop buy/sell/repair | `ShopCommerceHandler`, `ShopItemValueResolver` | `TAKUMI_SHOP_*`, `F3 E9`/`ED` |
| G | TCP hook sau join | `GamePortMinimalSession`, `LegacyLoginHostRunner` | `// [M8 gameplay]` |
| N | Player viewport walk `0x12` | `PlayerViewportTracker` | M10 |
| P | PvP melee / skill hit | `PlayerCombatRules`, `MonsterCombatHandler` | `TAKUMI_COMBAT_PVP_*` |
| Q | Quest NPC dialog stub | `NpcQuestCatalog`, `QuestWire602` | `TAKUMI_QUEST_NPC_DEFAULT_STATE` |

**Move-map UI (`0x8E`):** `MoveMapHandler` — **`world/M8-MOVE-MAP-PARITY-CHECKLIST.md`**.

---

## Chưa làm — PR khác

| # | Tính năng | Gợi ý |
|---|-----------|--------|
| M | Quest accept/reward + warehouse/guild đầy đủ | M11+ |

---

## File chỉ touch khi sửa gameplay

- `World/WorldGameplayHandlers.cs`
- `World/ShopCommerceHandler.cs`, `World/MapGateService.cs`, `World/SkillTeleportService.cs`
- `World/MoveMapHandler.cs` (move-map)
- `World/CustomNpcMoveHandler.cs`, `World/MoveWarpExecutor.cs`
- `Protocol/*Wire602.cs`, `Protocol/ClientGameplayPackets602.cs`

**Verify dev:** `[m8] sent shop 0x31` · `[m8] teleport` · `[m8] skill teleport` · `[m9] sent C2 0x13`

**Liên kết:** `M9-NPC-MONSTER-CHECKLIST.md` · `M9-MONSTER-AI-PORT-CHECKLIST.md` · `M8-M10-WORLD-RUNTIME-CHECKLIST.md`
