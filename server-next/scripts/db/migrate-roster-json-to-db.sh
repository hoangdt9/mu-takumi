#!/usr/bin/env bash
# M7: push every character in takumi-roster/*.json → Postgres character_roster (+ character_domain).
# Usage (from server-next/):
#   ./scripts/db/migrate-roster-json-to-db.sh
#   ./scripts/db/migrate-roster-json-to-db.sh 'postgresql://takumi:takumi@127.0.0.1:54444/takumi_runtime'
set -euo pipefail
# shellcheck source=../_lib/paths.sh disable=SC1091
source "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../_lib/paths.sh"
takumi_script_paths
cd "$ROOT"

if [[ -f "$ROOT/.env" ]]; then
  set -a
  # shellcheck disable=SC1091
  source "$ROOT/.env"
  set +a
fi

export TAKUMI_ROSTER_DB_SYNC="${TAKUMI_ROSTER_DB_SYNC:-1}"
export TAKUMI_CHARACTER_DOMAIN_SYNC="${TAKUMI_CHARACTER_DOMAIN_SYNC:-1}"
export TAKUMI_MIGRATE_ROSTER_JSON_ONLY=1

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

echo "[migrate-roster] dir=${TAKUMI_ROSTER_DIR:-$ROOT/takumi-roster}"
dotnet run --project "$ROOT/src/Takumi.Server.LegacyLoginHost/Takumi.Server.LegacyLoginHost.csproj" -c Release --no-restore 2>/dev/null \
  || dotnet run --project "$ROOT/src/Takumi.Server.LegacyLoginHost/Takumi.Server.LegacyLoginHost.csproj" -c Release
