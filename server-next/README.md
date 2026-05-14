# Takumi Server Next

## What is in this folder

| Path | Purpose |
|------|---------|
| `env.defaults` | **Committed** stack defaults (ports, join version, serial, test accounts). Loaded by `LegacyLoginHost` at startup; override in `.env`. |
| `docs/IMPLEMENTATION-CHECKLIST.md` | Feature and QA checklist for Connect / Login / character flow. |
| `docs/M1-PROTOCOL-PARITY-MAP.md` | **M1** inventory: client `TranslateProtocol` / `ProtocolCoreEx` vs `LegacyLoginHost` coverage. |
| `docs/LOGIN-WIRE-FORMAT.md` | Encrypted login wire notes + link to M1 map. |
| `docs/M4-WORLD-POSITION-CHECKLIST.md` | **M4** world spawn + JSON ↔ Postgres roster (tick từng mục khi xong). |
| `docs/M5-JOIN-HANDOFF-CHECKLIST.md` | **M5** F4 03 advertised game port + in-memory session ticket (prep for M6). |
| `docs/M6-GAME-TCP-CHECKLIST.md` | **M6** dedicated game TCP scaffold (`GameListenHost`, `GameHost`, compose profile `gamehost`). |
| `src/Takumi.Server.Game/` | **M6** shared bootstrap: env loader, Dec2 keys, `GameListenHost`. |
| `src/Takumi.Server.GameHost/` | **M6** executable listening on **`TAKUMI_GAME_PORT`**. |
| `src/Takumi.Server.Protocol/` | **M2** shared Season 6 wire builders (character list, join map, login/connect packets). |
| `src/Takumi.Server.Join/` | **M5** session ticket store (in-memory; validate from future game TCP). |
| `src/Takumi.Server.Persistence/` | **M4b** Npgsql: `character_roster` + **`inventory_slot`** (`PostgresCharacterRosterRepository`, **`PostgresInventorySlotRepository`**, **`TakumiPostgresMirror`**, **`JoinInventoryPacket602`**) when **`TAKUMI_ROSTER_DB_SYNC=1`**. |
| `sql/init/` | Postgres **first-init** scripts (`001_character_roster.sql`, **`002_inventory_slot.sql`**). **Existing DB volume:** re-apply with **`./scripts/apply-sql.sh`** or `psql` (see **Postgres migrations** below). |
| `scripts/apply-sql.sh` | Runs every `sql/init/*.sql` against a **libpq** URI (`postgresql://user:pass@host:port/db`). |
| `src/Takumi.Server.Tests/` | **M2** golden-byte xUnit tests for `Takumi.Server.Protocol`. |
| `.env.lan.example` | Copy to `.env` and set `TAKUMI_PUBLIC_HOST` / URLs (replace `YOUR_LAN_IP`). |
| `docker-compose.yml` | **Postgres + LegacyLoginHost** in Docker (**54444**, **44605**, **44606**). Optional **`datazip`**: nginx **18080**. Optional **`gamehost`**: M6 `game-host` (see **M6** doc). |
| `scripts/docker-up.sh` | `docker compose up -d` + `ps`. **`--with-datazip`** / **`--with-gamehost`** enable compose profiles. |
| `scripts/docker-recreate-legacy-login.sh` | `docker compose up -d --force-recreate legacy-login` from **this** folder (avoids *no configuration file provided* when your shell cwd was `Source/android`). From repo root `takumi/`: `bash scripts/docker-recreate-legacy-login.sh`. From `takumi/Source/android`: `bash ../../scripts/docker-recreate-legacy-login.sh`. |

## Source code status

- **Legacy login / character-list smoke host (MVP):** `src/Takumi.Server.LegacyLoginHost` — `dotnet build Takumi.Server.Next.slnx` then run the exe on the **game/login port** (e.g. **44606**). It sends `C1 F1 00` (join), accepts encrypted `F1 01` login (**must load the same `Data/Dec2.dat` as the Android client** via `TAKUMI_DEC2_PATH` or `Data/Dec2.dat` beside the exe — otherwise decryption fails and login never completes), then answers `C1 F1 01` and replies to `F3 00` with an empty list (also pushed once after success unless `TAKUMI_SKIP_AUTO_CHARLIST=1`). Default wire version bytes match `ServerVersion` **1.04.05** (`10405`); default serial matches `GameServerInfo - Common.ini` / Android `InitializeTakumiProtectState` fallback: **TbYehR2hFUPBKgZj**. Override with env vars in `Program.cs` if your `Main.info` differs. Character roster + world spawn are persisted under **`takumi-roster/<account>.json`** (override `TAKUMI_ROSTER_DIR`). **M4c (same TCP):** server updates **`posX`/`posY`/`angle`** from client **walk** (`0xD4` / `0x10`) and **instant move** (`0x15`) while in-world; **flush** on disconnect — host logs should show a later **`F3 03`** join at saved tiles (e.g. xy changing from default Lorencia to walked coordinates). **Optional:** `TAKUMI_ROSTER_DB_SYNC=1` + `TAKUMI_PG_*` mirrors roster JSON to **`character_roster`** and enables reading **`inventory_slot`** for post-join **`F3 10`** (see **`docs/M4-WORLD-POSITION-CHECKLIST.md`**; apply **`002_inventory_slot.sql`** on existing DB volumes).

Example Npgsql URL for a host app on the same machine as Docker Desktop:

`Host=127.0.0.1;Port=54444;Database=takumi_runtime;Username=takumi;Password=takumi`

(Adjust user/password/database to match `.env` / `docker-compose.yml` overrides.)

**M5 (join handoff):** optional **`TAKUMI_GAME_PORT`** overrides the port embedded in Connect Server **`C1 F4 03`** (defaults to **`TAKUMI_LOGIN_PORT`**). Session tickets are issued in-memory after login (`Takumi.Server.Join`); see **`docs/M5-JOIN-HANDOFF-CHECKLIST.md`**.

**M6 (game TCP):** optional split port **`TAKUMI_GAME_PORT`** + run **`Takumi.Server.GameHost`** (or Docker **`--profile gamehost`** / **`./scripts/docker-up.sh --with-gamehost`**). With **`TAKUMI_ACCOUNTS`** + **`TAKUMI_SERVER_SERIAL`** (from **`env.defaults`** or `.env`), GameHost runs **minimal-login** (same roster JSON as LegacyLoginHost). See **`docs/M6-GAME-TCP-CHECKLIST.md`**.

## Quick start (database + LAN server in Docker)

1. `cp .env.lan.example .env` and replace **`YOUR_LAN_IP`** with your Mac/PC LAN IP (phones must reach it). Ports and serial come from **`env.defaults`** unless you override them in `.env`.
2. **`docker compose up -d`** or **`./scripts/docker-up.sh`** — starts **Postgres** (default **54444**) and **LegacyLoginHost** (**44605** Connect + **44606** login). Dec2 is always **`/keys/Dec2.dat`** inside the container (read-only mount **`./keys`** → `/keys`, i.e. commit **`keys/Dec2.dat`** next to `docker-compose.yml`). Override **`TAKUMI_DEC2_HOST_DIR`** if your `Dec2.dat` lives elsewhere; **`TAKUMI_DEC2_PATH` in `.env` is only for host `dotnet run`**, not substituted into Docker (host paths would break SimpleModulus). Connect list **F4 06** (sub-server lines): if **`TAKUMI_CS_CONNECT_IDS`** / **`TAKUMI_CS_CONNECT_BASE`** are unset, the host sends **32 safe wire ids** (`0..14`, `20..34`, `40..41`) so each BMD group uses at most **15** connect slots — the Takumi client’s `ServerListManager` only has **`SLM_MAX_SERVER_COUNT` (15)** `NonPVP` bytes per group but indexes with `(connectIndex % 20) + 1`, so denser presets (e.g. `0..19` in one group) can **SIGSEGV** on Android. Override with **`TAKUMI_CS_CONNECT_IDS`** or **`TAKUMI_CS_CONNECT_BASE` + `TAKUMI_CS_CONNECT_COUNT`** when your `ServerList.bmd` uses other `connectIndex/20` groups (keep **≤15** distinct `%20` remainders per group, or fix the client).
3. **Optional LAN `data.zip` download** (different host port, default **18080**): `docker compose --profile datazip up -d` or `./scripts/docker-up.sh --with-datazip`. Serves `../docker/data-zip/host/data.zip` as `http://<LAN-IP>:18080/data.zip` (override `DATA_ZIP_PUBLISH_PORT`). Same nginx config as `takumi/docker` — do not run both `datazip` stacks on the same publish port.
4. Logs: `docker compose logs -f legacy-login`. Stop: `docker compose down`.
5. Full `Takumi.Server.Host`: run when `src/Takumi.Server.Host` sources exist; connection string → Postgres above.

Port/env sanity: `./scripts/check-takumi-ports.sh` from `server-next`.

## Postgres: apply `sql/init` when the Docker volume already exists

Compose mounts `./sql/init` into **`/docker-entrypoint-initdb.d`** — scripts run **only on first database init**. If the volume was created before a new file appeared in `sql/init/`, apply manually (recommended: run **`./scripts/apply-sql.sh`** so **`001_*.sql`** then **`002_*.sql`** both apply).

**Host** (default publish **54444**):

```bash
cd server-next
./scripts/apply-sql.sh "postgresql://takumi:takumi@127.0.0.1:54444/takumi_runtime"
```

**Single file** (when you only need one delta; fresh DBs usually get both via `apply-sql.sh` / initdb):

```bash
# Roster mirror (M4b)
psql "postgresql://takumi:takumi@127.0.0.1:54444/takumi_runtime" -v ON_ERROR_STOP=1 -f sql/init/001_character_roster.sql
# Inventory rows for F3 10 (12-byte item blobs)
psql "postgresql://takumi:takumi@127.0.0.1:54444/takumi_runtime" -v ON_ERROR_STOP=1 -f sql/init/002_inventory_slot.sql
```

**Compose `postgres` service** (example for `002`; use `/docker-entrypoint-initdb.d/<file>.sql` or `apply-sql.sh` from the host):

```bash
docker compose exec -T postgres psql -U takumi -d takumi_runtime -v ON_ERROR_STOP=1 -f /docker-entrypoint-initdb.d/002_inventory_slot.sql
```

## One-command LAN smoke host (auto rebuild)

From `server-next/`:

```bash
./scripts/run-legacy-login-host.sh
```

This loads `env.defaults` (committed ports/serial/accounts), then `server-next/.env` if present, sets `TAKUMI_VERBOSE=1`, defaults `TAKUMI_DEC2_PATH` when **`keys/Dec2.dat`** exists (or the first path in **`TAKUMI_DEC2_FALLBACK_PATHS`** from `.env`), then runs **`dotnet watch`** on `Takumi.Server.LegacyLoginHost` so you edit `Program.cs`, save, and the listener restarts — you only interact with the Android client.

**Second terminal — phone logs:**

```bash
./scripts/watch-android-takumi-log.sh
```

Clears logcat by default; use `TAKUMI_LOGCAT_CLEAR=0 ./scripts/watch-android-takumi-log.sh` to keep history. Requires `adb` and a connected device.

See also `../docs/ANDROID-DEV-MAC.md` (Takumi Server Next + optional `datazip` profile under `../docker/`).
