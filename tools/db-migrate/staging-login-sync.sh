#!/usr/bin/env bash
# Mirror MEMB_INFO + Character into Postgres schema takumi_staging (Gate 2 dev).
# NEVER run against prod.
# Copies db-migrate.env.sample → tools/db-migrate/.env for connection strings if you have not exported them.
# Optional: bash auto-sources tools/db-migrate/.env.
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$ROOT"
ENV_FILE="$ROOT/tools/db-migrate/.env"
if [[ -f "$ENV_FILE" ]]; then
  set -a
  # shellcheck disable=1091
  source "$ENV_FILE"
  set +a
fi
dotnet run --project "$ROOT/tools/db-migrate/dotnet/Takumi.Etl/Takumi.Etl.csproj" -c Release -- \
  staging-login-path --recreate --load "$@"
