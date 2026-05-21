# server-next scripts

Operational scripts for Docker stack, Postgres, Android USB dev, host `dotnet run`, smoke tests, and monster spawn ETL.

**Run from `server-next/`** (repo root for compose/SQL paths). Each script lives in a category subfolder — no duplicate wrappers at `scripts/` root.

## Layout

| Folder | Purpose |
|--------|---------|
| [`docker/`](docker/) | `docker-stack.sh`, LAN+gamehost stack, `game-host` container boot |
| [`db/`](db/) | `apply-sql.sh`, roster/inventory migrate, world ETL import, account QA, `reset-mg001-skills.sh`, `verify-mg001-skills.sh` |
| [`android/`](android/) | `adb-reverse.sh`, `adb-reverse-takumi-dev.sh`, logcat watch |
| [`host/`](host/) | `dotnet run` Connect / Login / LegacyLogin on Mac (no Docker) |
| [`smoke/`](smoke/) | M8 smoke, C2 connect probe, port checks |
| [`spawn/`](spawn/) | MonsterSetBase sync (OpenMU + references), coverage reports |
| [`_lib/`](_lib/) | `paths.sh` — `ROOT` = server-next, `SCRIPTS_ROOT` = this directory |

## Common commands

```bash
cd server-next

# Docker (Postgres + legacy-login + datazip + optional game-host)
./scripts/docker/docker-stack.sh --detach
./scripts/docker/docker-stack.sh --host-build --recreate --detach

# LAN + game-host + smoke
./scripts/docker/docker-stack-lan-gamehost.sh

# SQL / seeds
./scripts/db/apply-sql.sh 'postgresql://takumi:takumi@127.0.0.1:54444/takumi_runtime'
TAKUMI_APPLY_DEV_SEEDS=1 ./scripts/docker/docker-stack.sh --detach
./scripts/db/reset-mg001-skills.sh
./scripts/db/verify-mg001-skills.sh

# Monster spawn (file + Postgres)
./scripts/spawn/sync-monster-spawn-stack.sh

# Android USB (when Wi‑Fi/AP isolation blocks LAN)
./scripts/android/adb-reverse-takumi-dev.sh
./scripts/android/watch-android-takumi-log.sh

# M8 verify
./scripts/smoke/smoke-m8.sh --no-recreate
```

## Python (spawn)

Run from `server-next/` or any cwd; modules import each other under `scripts/spawn/`:

```bash
python3 scripts/spawn/report-spawn-sources.py
python3 scripts/spawn/sync-all-spawns-from-openmu.py --help
```

## Docker compose

`game-host` invokes `scripts/docker/docker-game-host-boot.sh` (bind-mount `.:/app`).

## Docs

- [`docs/docker/DOCKER-BUILD-RUN.md`](../docs/docker/DOCKER-BUILD-RUN.md)
- [`docs/world/M8-MAP-MONSTER-SPAWN-TASKS.md`](../docs/world/M8-MAP-MONSTER-SPAWN-TASKS.md)
- [`../../docs/android/ANDROID-DEV-MAC.md`](../../docs/android/ANDROID-DEV-MAC.md)
