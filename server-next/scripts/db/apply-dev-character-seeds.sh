#!/usr/bin/env bash
# Apply dev SQL seeds for test account (mg001 wear + skills). QA APK optional.
# WARNING: 013_test_account_mg001_seed.sql overwrites test/mg001 stats, zen, and inventory_slot.
# docker-stack.sh runs this only when TAKUMI_APPLY_DEV_SEEDS=1 (not on every stack up).
set -euo pipefail
# shellcheck source=../_lib/paths.sh disable=SC1091
source "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../_lib/paths.sh"
takumi_script_paths
URI="${1:-${TAKUMI_PG_URI:-postgresql://takumi:takumi@127.0.0.1:54444/takumi_runtime}}"

apply() {
  local f="$1"
  echo "[dev-seed] $f"
  "$SCRIPTS_ROOT/db/apply-sql.sh" "$URI" "$f"
}

apply "sql/patches/015_character_key_configuration.sql"
apply "sql/patches/013_test_account_mg001_seed.sql"
apply "sql/patches/016_character_skill.sql"
apply "sql/patches/017_seed_mg001_character_skill.sql"

if command -v dotnet >/dev/null 2>&1 && [[ -f "$SCRIPTS_ROOT/db/migrate-inventory-json-to-db.sh" ]]; then
  echo "[dev-seed] inventory JSON → DB (test.json)"
  TAKUMI_PG_CONNECTION_STRING="$URI" "$SCRIPTS_ROOT/db/migrate-inventory-json-to-db.sh" || true
fi

echo "[dev-seed] done. Restart game-host; account=test mg001 has wear + skills in Postgres."
