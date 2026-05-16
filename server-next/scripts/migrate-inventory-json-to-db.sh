#!/usr/bin/env bash
# M7g: push takumi-inventory/*.json → Postgres inventory_slot.
# Usage (from server-next/):
#   ./scripts/migrate-inventory-json-to-db.sh
#   ./scripts/migrate-inventory-json-to-db.sh 'postgresql://takumi:takumi@127.0.0.1:54444/takumi_runtime'
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
export TAKUMI_MIGRATE_INVENTORY_JSON_ONLY=1

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

echo "[migrate-inventory] dir=${TAKUMI_INVENTORY_DIR:-$ROOT/takumi-inventory}"
dotnet run --project "$ROOT/src/Takumi.Server.LegacyLoginHost/Takumi.Server.LegacyLoginHost.csproj" -c Release --no-restore 2>/dev/null \
  || dotnet run --project "$ROOT/src/Takumi.Server.LegacyLoginHost/Takumi.Server.LegacyLoginHost.csproj" -c Release
