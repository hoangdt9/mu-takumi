#!/usr/bin/env bash
# Clear all inventory_slot + warehouse_slot rows for one account (clean bag QA).
#
# Usage:
#   ./scripts/clear-inventory-account.sh test
#   ./scripts/clear-inventory-account.sh test 'postgresql://takumi:takumi@127.0.0.1:54444/takumi_runtime'
#   ./scripts/clear-inventory-account.sh test '' rf001   # only one character
set -euo pipefail

ACCOUNT="${1:-test}"
PG_URL="${2:-postgresql://takumi:takumi@127.0.0.1:54444/takumi_runtime}"
CHAR_FILTER="${3:-}"

if [[ -n "$CHAR_FILTER" ]]; then
  CHAR_WHERE="AND character_name = '${CHAR_FILTER}'"
  echo "== Clearing inventory for account '$ACCOUNT' character '$CHAR_FILTER' =="
else
  CHAR_WHERE=""
  echo "== Clearing inventory for account '$ACCOUNT' (all characters) =="
fi

run_psql() {
  if command -v docker >/dev/null 2>&1 && docker compose ps postgres 2>/dev/null | grep -q 'Up'; then
    docker compose exec -T postgres psql -U takumi -d takumi_runtime -v ON_ERROR_STOP=1 "$@"
  elif command -v psql >/dev/null 2>&1; then
    psql "$PG_URL" -v ON_ERROR_STOP=1 "$@"
  else
    echo "Need docker compose (postgres up) or psql." >&2
    exit 1
  fi
}

run_psql <<SQL
BEGIN;
DELETE FROM inventory_slot WHERE account_login = '${ACCOUNT}' ${CHAR_WHERE};
DELETE FROM warehouse_slot WHERE account_login = '${ACCOUNT}';
COMMIT;
SELECT character_name, COUNT(*) AS remaining_slots
FROM inventory_slot
WHERE account_login = '${ACCOUNT}'
GROUP BY character_name
ORDER BY character_name;
SQL

echo "== Done. Restart game-host or re-login so in-memory session reloads from DB. =="
