#!/usr/bin/env bash
# Delete all test/mg001 skills and re-seed MG combat QA kit (30 skills, compact slots 1..30).
# Usage: ./scripts/db/reset-mg001-skills.sh [postgres_uri]
set -euo pipefail
# shellcheck source=../_lib/paths.sh disable=SC1091
source "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../_lib/paths.sh"
takumi_script_paths

URI="${1:-${TAKUMI_PG_URI:-postgresql://takumi:takumi@127.0.0.1:54444/takumi_runtime}}"

echo "== reset MG skills: test/mg001 =="
echo "  uri: ${URI}"

"$SCRIPTS_ROOT/db/apply-sql.sh" "$URI" sql/patches/017_seed_mg001_character_skill.sql

"$SCRIPTS_ROOT/db/verify-mg001-skills.sh" "$URI"

echo "== done. Relog mg001 (or restart game-host) to refresh F3 magic list. =="
