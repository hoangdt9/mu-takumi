# M8 — World static data (ETL + catalogs)

**Đã merge:** schema `005`/`006`, importers, `MapMonsterWorld` ưu tiên DB, `MapGateCatalog`, `NpcShopCatalog`.

**Wire gameplay:** gate teleport `C1 0x1C` (`MapGateTeleportHandler`), NPC shop `C1 0x30` + `C2 0x31` (`NpcShopHandler`).

## Phần A — ETL (host)

Làm **[02-sql-and-etl.md](./02-sql-and-etl.md)** trước.

- [ ] `monster_spawn` COUNT > 0 (hoặc chấp nhận fallback file M9).
- [ ] `map_gate` / `npc_shop` có row nếu đã import.

## Phần B — Runtime load (log)

Sau recreate `legacy-login`:

```bash
docker compose logs legacy-login 2>&1 | grep -E '\[m8\]|monster_spawn|MapGate|NpcShop|InitMonster|InitWorld'
```

- [ ] `TAKUMI_MONSTER_SPAWN_DB=1` → load spawn từ DB.
- [ ] `TAKUMI_WORLD_STATIC_DB=1` → gate/shop catalog init (không crash nếu bảng trống).

## Phần C — So sánh file vs DB (M9)

1. [ ] Tắt tạm `TAKUMI_MONSTER_SPAWN_DB` → restart → log `[m9] loaded MonsterSetBase … from <path>` (file).
2. [ ] Bật lại DB → số lượng spawn gần khớp ETL (cùng map Lorencia spot-check).

## Phần D — Gate Lorencia → Devias

1. [ ] Recreate stack: `./scripts/docker-stack.sh --host-build --recreate --detach`
2. [ ] Vào Lorencia, đứng gần **(123,233)** (cổng index **1** trong `Gate.txt`).
3. [ ] Dùng cổng / teleport client (`C1/C3 0x1C` gate=1).
4. [ ] **Host log:** `[m8] gate teleport gate=1 ok=True map=1 …`
5. [ ] Client: chuyển map Devias, tile hợp lệ.

```bash
docker compose logs -f game-host 2>&1 | grep '\[m8\]'
```

## Phần E — NPC shop

1. [ ] `NpcShopCatalog` load (log `[m8] NpcShopCatalog: N shops`).
2. [ ] Tap NPC shop (gửi `C1 0x30` với object key trong viewport).
3. [ ] **Host:** `[m8] npc shop talk … items=N`
4. [ ] Client: UI shop mở, có item (logcat `0x31 [ReceiveTradeInventory]`).

## Pass criteria

ETL + catalog load; gate QA Devias; shop mở được với ít nhất 1 NPC có `ShopManager.txt` mapping.
