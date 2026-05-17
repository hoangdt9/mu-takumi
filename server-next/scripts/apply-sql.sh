#!/usr/bin/env bash
# Apply SQL to Postgres. psql expects a libpq URI, not an Npgsql key=value string.
# Usage from server-next/:
#   ./scripts/apply-sql.sh "postgresql://takumi:takumi@127.0.0.1:54444/takumi_runtime"
#   ./scripts/apply-sql.sh "postgresql://takumi:takumi@127.0.0.1:54444/takumi_runtime" sql/patches/013_test_account_mg001_seed.sql
# Default (no second arg): every sql/init/*.sql in lexical order.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
URI="${1:-}"
TARGET="${2:-}"
if [[ -z "$URI" ]]; then
  echo "usage: $0 'postgresql://USER:PASS@HOST:PORT/DATABASE' [sql/file.sql]" >&2
  echo "  env fallback: TAKUMI_PG_URI or postgresql://\${TAKUMI_PG_USER}:\${TAKUMI_PG_PASSWORD}@\${TAKUMI_PG_HOST}:\${TAKUMI_PG_PORT}/\${TAKUMI_PG_DATABASE}" >&2
  exit 1
fi

if [[ -n "$TARGET" ]]; then
  f="$TARGET"
  if [[ ! -f "$f" ]]; then
    f="$ROOT/$TARGET"
  fi
  if [[ ! -f "$f" ]]; then
    echo "[apply-sql] file not found: $TARGET" >&2
    exit 1
  fi
  echo "[apply-sql] $f"
  psql "$URI" -v ON_ERROR_STOP=1 -f "$f"
  echo "[apply-sql] done."
  exit 0
fi

shopt -s nullglob
for f in "$ROOT/sql/init"/*.sql; do
  echo "[apply-sql] $f"
  psql "$URI" -v ON_ERROR_STOP=1 -f "$f"
done
echo "[apply-sql] done."
