# M8 — World static data (ETL + catalogs)

**Đã merge:** schema `005`/`006`, importers, `MapMonsterWorld` ưu tiên DB, `MapGateCatalog`, `NpcShopCatalog`.

**Chưa wire gameplay:** teleport `C1 0x1C`, shop list `C2 0x31` (catalog sẵn, handler chưa).

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

## Phần D — Client (gián tiếp)

- [ ] Vào map có spawn trong `MonsterSetBase` → số quái khớp khu vực (M9 viewport).
- [ ] Cửa teleport / NPC shop: **chưa expect** hoạt động — chỉ verify server không lỗi khi catalog load.

## Pass criteria

ETL + log xác nhận DB path; gameplay shop/teleport vẫn **out of scope** cho M8 close.
