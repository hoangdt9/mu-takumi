# M6 — Dedicated game TCP (`Takumi.Server.Game` + `Takumi.Server.GameHost`)

Last updated: 2026-05-14 (M4c periodic roster save; merge mode env; explicit DB delete; `CharacterRosterMirrorWriter`; `[event=decrypted_rx]` + `TAKUMI_STRUCTURED_LOG`)

## Goal

Mirror **`Source/4.GameServer`** after client TCP connect: **`GCConnectClientSend`** → plain **`C1 F1 00`**, then the same **SimpleModulus + XOR32** pipeline as **`LegacyLoginHost`**.

When **`TAKUMI_GAME_PORT`** differs from **`TAKUMI_LOGIN_PORT`**, the client follows ConnectServer **`C1 F4 03`** to this port and must be able to **log in again**, see the **character list**, and **enter the world** (`F3 03`) using the same **`takumi-roster/<account>.json`** files as the login host.

### Troubleshooting: disconnect / “mất kết nối” right after choosing a sub-server

- Android log: **`connect failed … port=55901 errno=111 (Connection refused)`** means nothing is **listening** on that host:port.
- **Cause:** `.env` sets **`TAKUMI_GAME_PORT`** to a dedicated port, but only **`legacy-login`** is running (it still listens on **`TAKUMI_LOGIN_PORT`**, e.g. 44606). F4 03 advertised 55901 → client TCP to 55901 → refused.
- **Fix (pick one):**
  1. **Single TCP (simplest):** remove **`TAKUMI_GAME_PORT`** / **`TAKUMI_GAME_PUBLISH`** from `.env` (or leave them unset). F4 03 then defaults to **`TAKUMI_LOGIN_PORT`**.
  2. **Split stack:** keep **`TAKUMI_GAME_PORT=55901`** and start **`GameHost`** — e.g. `docker compose --profile gamehost up -d` or `./scripts/docker-stack.sh --detach --with-gamehost` — so something accepts TCP on the advertised port.

## Modes (`GameListenHost`)

| Mode | When | Behaviour |
|------|------|-----------|
| **minimal-login** | `TAKUMI_ACCOUNTS` resolves to a non-empty map **and** `TAKUMI_SERVER_SERIAL` is 16 ASCII bytes | `F1 01` login, auto `F3 00` list (unless `TAKUMI_SKIP_AUTO_CHARLIST=1`), **`F3 01` / `F3 02`** create/delete, `F3 03`/`F3 15` join + **`F3 10`**, stub move-map `8E 02` + `F3 03` + **`F3 10`**, walk/instant-move roster updates, keepalive **`C1 03 71`**, `F1 02` logout ack |
| **bootstrap-only** | Accounts missing or serial invalid | Join + decrypt RX log only (`TAKUMI_VERBOSE=1`) |

`RepoEnvLoader` loads **`env.defaults`** then **`.env`**. If `.env` omits accounts/serial, **`env.defaults`** still supplies them when you run from `server-next/` (same as LegacyLoginHost).

## Exit criteria (this iteration)

- [x] `Takumi.Server.GameHost` binds **`TAKUMI_GAME_PORT`**.
- [x] **minimal-login** path: roster JSON via **`GameRosterDisk`** (same layout as LegacyLoginHost).
- [x] Docker **`--profile gamehost`** service **`game-host`**.
- [x] Host **`.env`** example: `TAKUMI_GAME_PORT` + `TAKUMI_GAME_PUBLISH` aligned with F4 03.
- [x] **minimal-login** parity vs single TCP: **`GamePortKeepAliveRunner`** (env `TAKUMI_GAME_KEEPALIVE_SECONDS`) + **`F3 01` / `F3 02`** create/delete character (shared **`GamePacketFinders`** / **`GameRosterMutations`**).
- [x] **M6+ (partial):** dòng log **`[event=decrypted_rx]`** trên stderr khi `TAKUMI_VERBOSE` hoặc **`TAKUMI_STRUCTURED_LOG=1`**; **`GamePortMinimalSession`** upsert **`character_roster`** qua **`CharacterRosterMirrorWriter`** khi `TAKUMI_ROSTER_DB_SYNC` (cùng `SaveRoster` như Legacy).
- [ ] Cross-process **session ticket** on client wire (signed); optional Postgres **write** path chỉ từ GameHost ngoài roster mirror hiện tại. **Done (server-only):** `session_ticket` consume khi **`TAKUMI_GAME_REQUIRE_LOGIN_HANDOFF`**.
- [x] `F3 10` after join / move-map: **`JoinInventoryPacket602`** (same as LegacyLoginHost) — empty when DB sync off; loads **`inventory_slot`** when **`TAKUMI_ROSTER_DB_SYNC`** + table present (`sql/init/002_inventory_slot.sql`).

## `.env` (host QA)

**Default (only `legacy-login`):** do **not** set `TAKUMI_GAME_PORT`; omit it so F4 03 uses `TAKUMI_LOGIN_PORT`.

**Split port + Docker `gamehost`:** set both so F4 03 and the published container port match.

```bash
TAKUMI_PUBLIC_HOST=192.168.x.x
TAKUMI_CONNECT_PORT=44605
TAKUMI_LOGIN_PORT=44606
# Split M6 only — requires GameHost / `docker compose --profile gamehost`:
# TAKUMI_GAME_PORT=55901
# TAKUMI_GAME_PUBLISH=55901
```

Keep **`TAKUMI_DEC2_PATH`** (or `./keys/Dec2.dat` mount in Docker) identical to the Android client’s **`Data/Dec2.dat`**.

## Run (two terminals, host)

Terminal A — login + connect (when **`TAKUMI_GAME_PORT`** is set, F4 03 points at that port — then Terminal B must run):

```bash
cd server-next
./scripts/run-legacy-login-host.sh
# or: dotnet run --project src/Takumi.Server.LegacyLoginHost/...
```

Terminal B — game TCP:

```bash
cd server-next
dotnet run --project src/Takumi.Server.GameHost/Takumi.Server.GameHost.csproj -c Release
```

Smoke (listen): after start, `lsof -nP -iTCP:55901 -sTCP:LISTEN` should show **`Takumi.Server.GameHost`**.

## Run (Docker)

```bash
# .env must set TAKUMI_GAME_PORT=55901 (and legacy-login reads it for F4 03)
docker compose --profile gamehost up -d
# or: ./scripts/docker-stack.sh --detach --with-gamehost
```

## Env reference

| Variable | Purpose |
|----------|---------|
| `TAKUMI_GAME_PORT` | **Omit** for single-TCP (F4 03 = login port). Set for **split** stack: must match GameHost listen port = F4 03 target. |
| `TAKUMI_GAME_PUBLISH` | Host port mapped into **`game-host`** (default **55901**). |
| `TAKUMI_ACCOUNTS` | Enables **minimal-login** when non-empty (with serial). |
| `TAKUMI_SERVER_SERIAL` | 16-byte ASCII; must match client. |
| `TAKUMI_JOIN_VERSION` | Same as login host. |
| `TAKUMI_GAME_JOIN_WIRE_INDEX` | Optional `ushort` for join packet index bytes (default **0**). |
| `TAKUMI_ROSTER_DIR` | Override roster JSON directory (default `./takumi-roster`). |
| `TAKUMI_STRUCTURED_LOG` | `1` / `true` = luôn ghi `[event=decrypted_rx]` trên stderr (minimal-login), kể khi không `TAKUMI_VERBOSE`. |
| `TAKUMI_ROSTER_PERIODIC_SAVE_SECONDS` | Khoảng thời gian (5–3600) flush roster JSON + Postgres khi có walk/instant move (M4c). |
| `TAKUMI_ROSTER_DB_MERGE_MODE` | `json` = sau login **không** overlay `character_roster` lên roster; mặc định / `overlay` = overlay như cũ. |
| `TAKUMI_SESSION_HANDOFF_DB` | `1` = ghi pending **`session_ticket`** sau F1 01 trên **LegacyLoginHost** (cần `003_session_ticket.sql` + `TAKUMI_PG_*`). |
| `TAKUMI_GAME_REQUIRE_LOGIN_HANDOFF` | Trên **GameHost**: `1` = F1 01 chỉ OK sau khi **consume** một `session_ticket` (đăng nhập legacy trước). |
| `TAKUMI_GAME_HANDOFF_MATCH_IP` | Mặc định so khớp IP lưu lúc login; `0` = chỉ khớp account (yếu hơn). |
