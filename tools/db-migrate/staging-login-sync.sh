#!/usr/bin/env bash
# Mirror MEMB_INFO + Character into Postgres schema takumi_staging (Gate 2 dev).
# NEVER run against prod. Requires MSSQL + OpenMU Postgres connection env vars.
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$ROOT"
dotnet run --project "$ROOT/tools/db-migrate/dotnet/Takumi.Etl/Takumi.Etl.csproj" -c Release -- \
  staging-login-path --recreate --load "$@"
