#!/usr/bin/env bash
# Apply sql/init/*.sql to an existing Postgres (when Docker volume already ran initdb once).
# psql expects a libpq URI, not an Npgsql key=value string.
# Usage from server-next/:
#   ./scripts/apply-sql.sh "postgresql://takumi:takumi@127.0.0.1:54444/takumi_runtime"
# Inside legacy-login container (peer auth to service postgres):
#   docker compose exec -T legacy-login sh -c 'psql "postgresql://takumi:takumi@postgres:5432/takumi_runtime" -v ON_ERROR_STOP=1 -f /app/sql/init/001_character_roster.sql'
#   (Loop runs every sql/init/*.sql in lexical order — includes 002_inventory_slot.sql when present.)
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
URI="${1:-}"
if [[ -z "$URI" ]]; then
  echo "usage: $0 'postgresql://USER:PASS@HOST:PORT/DATABASE'" >&2
  exit 1
fi

shopt -s nullglob
for f in "$ROOT/sql/init"/*.sql; do
  echo "[apply-sql] $f"
  psql "$URI" -v ON_ERROR_STOP=1 -f "$f"
done
echo "[apply-sql] done."
