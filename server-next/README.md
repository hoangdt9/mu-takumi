# Takumi Server Next

## What is in this folder

| Path | Purpose |
|------|---------|
| `docs/IMPLEMENTATION-CHECKLIST.md` | Feature and QA checklist for Connect / Login / character flow. |
| `.env.lan.example` | Copy to `.env` and set `TAKUMI_PUBLIC_HOST` to the LAN IP phones use. |
| `docker-compose.yml` | **PostgreSQL only** for local QA (`takumi-next-postgres` on port **54444** by default). |

## Source code status

- **Legacy login / character-list smoke host (MVP):** `src/Takumi.Server.LegacyLoginHost` — `dotnet build Takumi.Server.Next.slnx` then run the exe on the **game/login port** (e.g. **44606**). It sends `C1 F1 00` (join), accepts encrypted `F1 01` login (**must load the same `Data/Dec2.dat` as the Android client** via `TAKUMI_DEC2_PATH` or `Data/Dec2.dat` beside the exe — otherwise decryption fails and login never completes), then answers `C1 F1 01` and replies to `F3 00` with an empty list (also pushed once after success unless `TAKUMI_SKIP_AUTO_CHARLIST=1`). Default wire version bytes match `ServerVersion` **1.04.05** (`10405`); default serial matches `GameServerInfo - Common.ini` / Android `InitializeTakumiProtectState` fallback: **TbYehR2hFUPBKgZj**. Override with env vars in `Program.cs` if your `Main.info` differs.
- Other projects under `src/` may still be missing sources in some snapshots; restore from backup when needed.

Example Npgsql URL for a host app on the same machine as Docker Desktop:

`Host=127.0.0.1;Port=54444;Database=takumi_runtime;Username=takumi;Password=takumi`

(Adjust user/password/database to match `.env` / `docker-compose.yml` overrides.)

## Quick start (database + LAN)

1. `cp .env.lan.example .env` and set `TAKUMI_PUBLIC_HOST` to your Mac’s Wi‑Fi IP.
2. `docker compose up -d` — **Postgres only** (default host port **54444**).
3. Optional login smoke on **44606** (does not include Connect **44605**): see `docker-compose.legacy-login-host.yml` and `../docs/ANDROID-DEV-MAC.md`.
4. Full `Takumi.Server.Host`: run when `src/Takumi.Server.Host` sources exist; connection string → Postgres above.

Port/env sanity: `./scripts/check-takumi-ports.sh` from `server-next`.

See also `../docs/ANDROID-DEV-MAC.md` (Takumi Server Next + optional `datazip` profile under `../docker/`).
