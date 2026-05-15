#!/usr/bin/env bash
# M7: Postgres-first character data migrate (no JSON required for inventory).
# - inventory_staging → inventory_slot (TAKUMI_IMPORT_INVENTORY_STAGING=1)
# - optional one-time roster backfill from takumi-roster/*.json (TAKUMI_MIGRATE_ROSTER_JSON=1)
#
# Usage:
#   ./scripts/migrate-character-data-to-db.sh
#   TAKUMI_MIGRATE_ROSTER_JSON=1 ./scripts/migrate-character-data-to-db.sh
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

if [[ -f "$ROOT/.env" ]]; then
  set -a
  # shellcheck disable=SC1091
  source "$ROOT/.env"
  set +a
fi

export TAKUMI_ROSTER_DB_SYNC="${TAKUMI_ROSTER_DB_SYNC:-1}"
export TAKUMI_CHARACTER_DOMAIN_SYNC="${TAKUMI_CHARACTER_DOMAIN_SYNC:-1}"
export TAKUMI_IMPORT_INVENTORY_STAGING="${TAKUMI_IMPORT_INVENTORY_STAGING:-1}"
export TAKUMI_MIGRATE_CHARACTER_DATA_ONLY=1
# Roster JSON backfill is opt-in (not required when roster already in Postgres):
export TAKUMI_MIGRATE_ROSTER_JSON="${TAKUMI_MIGRATE_ROSTER_JSON:-0}"

if [[ -n "${1:-}" ]]; then
  export TAKUMI_PG_CONNECTION_STRING="$1"
fi

if [[ -z "${TAKUMI_PG_CONNECTION_STRING:-}" && -z "${TAKUMI_PG_HOST:-}" ]]; then
  export TAKUMI_PG_HOST="${TAKUMI_PG_HOST:-127.0.0.1}"
  export TAKUMI_PG_PORT="${TAKUMI_PG_PORT:-54444}"
  export TAKUMI_PG_USER="${TAKUMI_PG_USER:-takumi}"
  export TAKUMI_PG_PASSWORD="${TAKUMI_PG_PASSWORD:-takumi}"
  export TAKUMI_PG_DATABASE="${TAKUMI_PG_DATABASE:-takumi_runtime}"
fi

echo "[migrate] inventory_staging=${TAKUMI_IMPORT_INVENTORY_STAGING} roster_json=${TAKUMI_MIGRATE_ROSTER_JSON}"
dotnet run --project "$ROOT/src/Takumi.Server.LegacyLoginHost/Takumi.Server.LegacyLoginHost.csproj" -c Release
