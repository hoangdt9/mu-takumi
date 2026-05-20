#!/usr/bin/env bash
# Reset one account roster to server-next/takumi-roster/<account>.json.default (or empty list).
# Clears Postgres mirror rows so login reloads from JSON.
#
# Usage:
#   ./scripts/db/reset-roster-account.sh test
#   ./scripts/db/reset-roster-account.sh test 'postgresql://takumi:takumi@127.0.0.1:54444/takumi_runtime'
set -euo pipefail

# shellcheck source=../_lib/paths.sh disable=SC1091
source "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../_lib/paths.sh"
takumi_script_paths
ACCOUNT="${1:-test}"
PG_URL="${2:-postgresql://takumi:takumi@127.0.0.1:54444/takumi_runtime}"
ROSTER_DIR="${TAKUMI_ROSTER_DIR:-$ROOT/takumi-roster}"
TEMPLATE="$ROSTER_DIR/${ACCOUNT}.json.default"
TARGET="$ROSTER_DIR/${ACCOUNT}.json"

if [[ ! -f "$TEMPLATE" ]]; then
  echo "Missing template: $TEMPLATE" >&2
  echo "  Create ${ACCOUNT}.json.default or pass a custom template path as arg 3." >&2
  exit 1
fi

mkdir -p "$ROSTER_DIR"
cp "$TEMPLATE" "$TARGET"
echo "== Wrote $TARGET from template =="

if ! command -v psql >/dev/null 2>&1; then
  echo "psql not found — JSON only; restart server or login to refresh in-memory roster." >&2
  exit 0
fi

echo "== Clearing Postgres rows for account '$ACCOUNT' =="
psql "$PG_URL" -v ON_ERROR_STOP=1 <<SQL
DELETE FROM inventory_slot WHERE account_login = '${ACCOUNT}';
DELETE FROM warehouse_slot WHERE account_login = '${ACCOUNT}';
DELETE FROM character_domain WHERE account_login = '${ACCOUNT}';
DELETE FROM character_roster WHERE account_login = '${ACCOUNT}';
SQL

echo "== Done. Re-push JSON → DB (optional): =="
echo "  TAKUMI_MIGRATE_ROSTER_JSON_ONLY=1 dotnet run --project src/Takumi.Server.LegacyLoginHost/Takumi.Server.LegacyLoginHost.csproj -c Release --no-build"
echo "  Or login once — mirror upsert runs on disconnect when TAKUMI_ROSTER_DB_SYNC=1."
