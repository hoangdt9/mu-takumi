#!/usr/bin/env bash
# Full monster spawn sync: MonsterSetBase (file) + optional Postgres monster_spawn table.
# Run from server-next/ on a machine with MuServer data + OpenMU checkout sibling.
set -euo pipefail
# shellcheck source=../_lib/paths.sh disable=SC1091
source "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../_lib/paths.sh"
takumi_script_paths
cd "$ROOT"

export TAKUMI_MONSTER_SET_BASE_PATH="${TAKUMI_MONSTER_SET_BASE_PATH:-$ROOT/../MuServer/4.GameServer/Data/Monster/MonsterSetBase.txt}"
export TAKUMI_PG_CONNECTION_STRING="${TAKUMI_PG_CONNECTION_STRING:-postgresql://takumi:takumi@127.0.0.1:54444/takumi_runtime}"
if [[ -n "${OPENMU_MAPS_DIR:-}" ]]; then
  OPENMU_DIR="$OPENMU_MAPS_DIR"
elif [[ -d "$ROOT/../../../Github/OpenMU/src/Persistence/Initialization" ]]; then
  OPENMU_DIR="$ROOT/../../../Github/OpenMU"
elif [[ -d "$ROOT/../../OpenMU/src/Persistence/Initialization" ]]; then
  OPENMU_DIR="$ROOT/../../OpenMU"
else
  OPENMU_DIR="$ROOT/../../OpenMU"
fi

MIN_RATIO="${TAKUMI_OPENMU_SYNC_MIN_RATIO:-0.5}"
DRY="${TAKUMI_OPENMU_SYNC_DRY_RUN:-0}"

echo "[sync-monster-spawn-stack] MonsterSetBase=$TAKUMI_MONSTER_SET_BASE_PATH"
echo "[sync-monster-spawn-stack] OpenMU=$OPENMU_DIR"
echo "[sync-monster-spawn-stack] Postgres=$TAKUMI_PG_CONNECTION_STRING"

"$SCRIPTS_ROOT/spawn/enable-move-map-field-spawns.sh"

echo "[sync-monster-spawn-stack] reference report (before sync):"
python3 "$SCRIPTS_ROOT/spawn/report-spawn-sources.py" || true

SYNC_ARGS=("$TAKUMI_MONSTER_SET_BASE_PATH" "$OPENMU_DIR" "--min-ratio" "$MIN_RATIO")
if [[ "$DRY" == "1" ]]; then
  SYNC_ARGS=(--dry-run "${SYNC_ARGS[@]}")
fi
python3 "$SCRIPTS_ROOT/spawn/sync-all-spawns-from-openmu.py" "${SYNC_ARGS[@]}"

echo "[sync-monster-spawn-stack] merge denser Pegasus / ThangCuoi spots where Takumi is lower:"
python3 "$SCRIPTS_ROOT/spawn/merge-spawns-from-references.py"

"$SCRIPTS_ROOT/spawn/report-spawn-sources.py" || true
"$SCRIPTS_ROOT/spawn/report-monster-spawn-coverage.sh"
"$SCRIPTS_ROOT/spawn/compare-spawn-openmu.sh" || true

if [[ "${TAKUMI_SKIP_PG_IMPORT:-0}" != "1" ]]; then
  "$SCRIPTS_ROOT/db/apply-sql.sh" "$TAKUMI_PG_CONNECTION_STRING" sql/init/005_monster_spawn.sql 2>/dev/null || true
  export TEST_PG_CONNECTION_STRING="$TAKUMI_PG_CONNECTION_STRING"
  export TAKUMI_PG_CONNECTION_STRING
  "$SCRIPTS_ROOT/db/import-monster-spawn.sh"
  echo "[sync-monster-spawn-stack] Postgres monster_spawn imported (enable TAKUMI_MONSTER_SPAWN_DB=1 in .env)"
else
  echo "[sync-monster-spawn-stack] skipped Postgres import (TAKUMI_SKIP_PG_IMPORT=1)"
fi

echo "[sync-monster-spawn-stack] done. Restart: docker compose restart game-host legacy-login"
