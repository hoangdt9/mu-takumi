# Takumi Server Next

## What is in this folder

| Path | Purpose |
|------|---------|
| `docs/IMPLEMENTATION-CHECKLIST.md` | Feature and QA checklist for Connect / Login / character flow. |
| `.env.lan.example` | Copy to `.env` and set `TAKUMI_PUBLIC_HOST` to the LAN IP phones use. |
| `docker-compose.yml` | **Postgres + LegacyLoginHost** in Docker (**54444**, **44605**, **44606**). Optional profile **`datazip`**: nginx serves `../docker/data-zip/host/data.zip` on host port **18080** (see `.env.lan.example`). |
| `scripts/docker-up.sh` | `docker compose up -d` + `ps`. Pass **`--with-datazip`** to enable the `datazip` profile. |
| `scripts/docker-recreate-legacy-login.sh` | `docker compose up -d --force-recreate legacy-login` from **this** folder (avoids *no configuration file provided* when your shell cwd was `Source/android`). From repo root `takumi/`: `bash scripts/docker-recreate-legacy-login.sh`. From `takumi/Source/android`: `bash ../../scripts/docker-recreate-legacy-login.sh`. |

## Source code status

- **Legacy login / character-list smoke host (MVP):** `src/Takumi.Server.LegacyLoginHost` — `dotnet build Takumi.Server.Next.slnx` then run the exe on the **game/login port** (e.g. **44606**). It sends `C1 F1 00` (join), accepts encrypted `F1 01` login (**must load the same `Data/Dec2.dat` as the Android client** via `TAKUMI_DEC2_PATH` or `Data/Dec2.dat` beside the exe — otherwise decryption fails and login never completes), then answers `C1 F1 01` and replies to `F3 00` with an empty list (also pushed once after success unless `TAKUMI_SKIP_AUTO_CHARLIST=1`). Default wire version bytes match `ServerVersion` **1.04.05** (`10405`); default serial matches `GameServerInfo - Common.ini` / Android `InitializeTakumiProtectState` fallback: **TbYehR2hFUPBKgZj**. Override with env vars in `Program.cs` if your `Main.info` differs.
- Other projects under `src/` may still be missing sources in some snapshots; restore from backup when needed.

Example Npgsql URL for a host app on the same machine as Docker Desktop:

`Host=127.0.0.1;Port=54444;Database=takumi_runtime;Username=takumi;Password=takumi`

(Adjust user/password/database to match `.env` / `docker-compose.yml` overrides.)

## Quick start (database + LAN server in Docker)

1. `cp .env.lan.example .env` and set `TAKUMI_PUBLIC_HOST` to your Mac’s Wi‑Fi IP (phones must reach it).
2. **`docker compose up -d`** or **`./scripts/docker-up.sh`** — starts **Postgres** (default **54444**) and **LegacyLoginHost** (**44605** Connect + **44606** login). Dec2 is always **`/keys/Dec2.dat`** inside the container (read-only mount **`./keys`** → `/keys`, i.e. commit **`keys/Dec2.dat`** next to `docker-compose.yml`). Override **`TAKUMI_DEC2_HOST_DIR`** if your `Dec2.dat` lives elsewhere; **`TAKUMI_DEC2_PATH` in `.env` is only for host `dotnet run`**, not substituted into Docker (host paths would break SimpleModulus). Connect list **F4 06** (sub-server lines): if **`TAKUMI_CS_CONNECT_IDS`** / **`TAKUMI_CS_CONNECT_BASE`** are unset, the host sends **32 safe wire ids** (`0..14`, `20..34`, `40..41`) so each BMD group uses at most **15** connect slots — the Takumi client’s `ServerListManager` only has **`SLM_MAX_SERVER_COUNT` (15)** `NonPVP` bytes per group but indexes with `(connectIndex % 20) + 1`, so denser presets (e.g. `0..19` in one group) can **SIGSEGV** on Android. Override with **`TAKUMI_CS_CONNECT_IDS`** or **`TAKUMI_CS_CONNECT_BASE` + `TAKUMI_CS_CONNECT_COUNT`** when your `ServerList.bmd` uses other `connectIndex/20` groups (keep **≤15** distinct `%20` remainders per group, or fix the client).
3. **Optional LAN `data.zip` download** (different host port, default **18080**): `docker compose --profile datazip up -d` or `./scripts/docker-up.sh --with-datazip`. Serves `../docker/data-zip/host/data.zip` as `http://<LAN-IP>:18080/data.zip` (override `DATA_ZIP_PUBLISH_PORT`). Same nginx config as `takumi/docker` — do not run both `datazip` stacks on the same publish port.
4. Logs: `docker compose logs -f legacy-login`. Stop: `docker compose down`.
5. Full `Takumi.Server.Host`: run when `src/Takumi.Server.Host` sources exist; connection string → Postgres above.

Port/env sanity: `./scripts/check-takumi-ports.sh` from `server-next`.

## One-command LAN smoke host (auto rebuild)

From `server-next/`:

```bash
./scripts/run-legacy-login-host.sh
```

This loads `.env` if present, sets `TAKUMI_VERBOSE=1`, defaults `TAKUMI_DEC2_PATH` when **`keys/Dec2.dat`** exists (or the first path in **`TAKUMI_DEC2_FALLBACK_PATHS`** from `.env`), then runs **`dotnet watch`** on `Takumi.Server.LegacyLoginHost` so you edit `Program.cs`, save, and the listener restarts — you only interact with the Android client.

**Second terminal — phone logs:**

```bash
./scripts/watch-android-takumi-log.sh
```

Clears logcat by default; use `TAKUMI_LOGCAT_CLEAR=0 ./scripts/watch-android-takumi-log.sh` to keep history. Requires `adb` and a connected device.

See also `../docs/ANDROID-DEV-MAC.md` (Takumi Server Next + optional `datazip` profile under `../docker/`).
