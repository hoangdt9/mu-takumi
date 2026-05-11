# Takumi Server Next

## What is in this folder

| Path | Purpose |
|------|---------|
| `docs/IMPLEMENTATION-CHECKLIST.md` | Feature and QA checklist for Connect / Login / character flow. |
| `.env.lan.example` | Copy to `.env` and set `TAKUMI_PUBLIC_HOST` to the LAN IP phones use. |
| `docker-compose.yml` | **PostgreSQL only** for local QA (`takumi-next-postgres` on port **54444** by default). |

## Source code status

This repository snapshot may contain **`src/*/bin` and `src/*/obj` artifacts only**, with **no `.csproj`, `.sln`, or `.cs` sources**. You cannot `dotnet build` the host until those files are **restored from backup** or re-added to the tree. After restore, run the host on the Mac (or in your own Dockerfile) and point it at Postgres started by `docker compose` here.

Example Npgsql URL for a host app on the same machine as Docker Desktop:

`Host=127.0.0.1;Port=54444;Database=takumi_runtime;Username=takumi;Password=takumi`

(Adjust user/password/database to match `.env` / `docker-compose.yml` overrides.)

## Quick start (database + LAN)

1. `cp .env.lan.example .env` and set `TAKUMI_PUBLIC_HOST` to your Mac’s Wi‑Fi IP.
2. `docker compose up -d`
3. When the .NET solution exists again: run `Takumi.Server.Host` with connection string pointing at `127.0.0.1:54444` (or published port).

See also `../docs/ANDROID-DEV-MAC.md` (Takumi Server Next + optional `datazip` profile under `../docker/`).
