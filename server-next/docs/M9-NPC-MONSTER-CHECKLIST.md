# M9 — NPC & monster runtime (scope A)

Last updated: 2026-05-15

## Scope A (this iteration)

- [x] Load **`MonsterSetBase.txt`** (parity `CMonsterSetBase::Load` / `GetPosition`).
- [x] Load **`Monster.txt`** for minimal **Life/Level** (`MonsterStatCatalog`).
- [x] Spawn static rows per **map**; filter by **view range** around player join tile.
- [x] Send **`C2 0x13`** (`MonsterViewportWire602`) after **`F3 03` + `F3 10`** on login/game TCP.
- [ ] **Regen** / AI / combat — M9 later.
- [ ] **M8 ETL** to Postgres spawn table — optional; file path env for now.

## Legacy reference (`Source/4.GameServer`)

| Area | File |
|------|------|
| Set base | `MonsterSetBase.cpp` |
| Stats | `MonsterManager.cpp` → `Monster.txt` |
| Viewport TX | `Viewport.cpp` → `GCViewportMonsterSend` (head `0x13`) |
| Client RX | `WSclient.cpp` → `ReceiveCreateMonsterViewport` |

## `server-next` code

| Component | Path |
|-----------|------|
| Loaders / world | `Takumi.Server.Game/World/*` |
| Wire | `Takumi.Server.Protocol/MonsterViewportWire602.cs` |
| Hook (game port) | `GamePortMinimalSession.cs` |
| Hook (legacy login) | `LegacyLoginHost.Runner/LegacyLoginHostRunner.cs` |

## Env

| Variable | Default |
|----------|---------|
| `TAKUMI_MONSTER_SET_BASE_PATH` | search `Monster/MonsterSetBase.txt` under cwd / `MuServer/4.GameServer/Data` |
| `TAKUMI_MONSTER_INFO_PATH` | `Monster/Monster.txt` |
| `TAKUMI_MONSTER_VIEW_RANGE` | `15` (Manhattan tiles) |
| `TAKUMI_MONSTER_VIEWPORT_MAX` | `64` monsters per packet |

If set-base file is missing, a small **Lorencia fallback** spawn set is used for QA.

## QA

1. Point env to real `MuServer/4.GameServer/Data/Monster/MonsterSetBase.txt`.
2. Login → select character → enter world.
3. Host log: `[m9] sent C2 0x13 monster viewport count=…`
4. Client logcat: `0x13 [ReceiveCreateMonsterViewport`
