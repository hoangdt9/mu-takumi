# M11 — Warehouse, trade, guild, skills (minimal hosts)

Last updated: 2026-05-16

## Scope

Parity stubs for GameServer features still missing from `LegacyLoginHost` / `GamePortMinimalSession`:

| Feature | Wire | Status |
|---------|------|--------|
| Warehouse | `0x81`/`0x83` money/state, `C2 0x31` item list, move flag `2` | **Partial** — DB `warehouse_slot` + `account.warehouse_zen`, NPC 240/383/384 open, inv↔wh moves, `0x81` deposit/withdraw |
| Coin shop | `F3 E9` priceType 1–3 | **Partial** — `account.wcoin_*` / `goblin_point` debit on buy; seed balances manually for QA |
| Trade | `0x36`–`0x3D` | **Partial** — request/accept between map-presence players, trade window moves |
| Guild | `0x50`–`0x67`, … | **Stub** — empty ack (no guild domain yet) |
| Skill list | `F3 11` | **Partial** — `character_skill` DB + class defaults on first join (`JoinSkillLifecycle`); learn/add skill wire **OPEN** |
| EF `takumi_runtime.character` | — | **Bridge** — `CharacterRuntimeStore` reads `character_roster` until full Host exists |

## Warehouse

- SQL: `sql/init/009_warehouse_slot.sql`
- Repo: `PostgresWarehouseSlotRepository`, `WarehouseSlotMirrorWriter`
- Session: `PlayerWarehouseSession` (account-wide, 240 slots)
- Open: NPC class `240` / `383` / `384` → `NpcTalkService.TryOpenWarehouseAsync` sends `0x31` item list from DB
- Moves: `ItemWorldHandler` flags `0` ↔ `2` and `2` ↔ `2`

## Trade

- `TradeWire602`, `TradeGameplayHandler`, `PlayerTradeSession`
- Requires `TAKUMI_MAP_PRESENCE_ENABLED=1` and two players on same map
- Trade window storage flag `1` (inventory ↔ trade slots)

## Runtime character (EF bridge)

`CharacterRuntimeStore.TryLoadCharacterAsync` loads from `character_roster` / `character_domain` when `TAKUMI_ROSTER_DB_SYNC=1`. Full `Takumi.Server.Host` + EF migrations remain a separate milestone.

## Env

```bash
TAKUMI_ROSTER_DB_SYNC=1
TAKUMI_MAP_PRESENCE_ENABLED=1   # trade between players
```

Apply SQL: `./scripts/db/apply-sql.sh 'postgresql://...'`
