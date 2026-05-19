#!/usr/bin/env bash
# Full monster spawn sync: MonsterSetBase (file) + optional Postgres monster_spawn table.
# Run from server-next/ on a machine with MuServer data + OpenMU checkout sibling.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

export TAKUMI_MONSTER_SET_BASE_PATH="${TAKUMI_MONSTER_SET_BASE_PATH:-$ROOT/../MuServer/4.GameServer/Data/Monster/MonsterSetBase.txt}"
export TAKUMI_PG_CONNECTION_STRING="${TAKUMI_PG_CONNECTION_STRING:-postgresql://takumi:takumi@127.0.0.1:54444/takumi_runtime}"
OPENMU_DIR="${OPENMU_MAPS_DIR:-$ROOT/../../OpenMU}"

MIN_RATIO="${TAKUMI_OPENMU_SYNC_MIN_RATIO:-0.5}"
DRY="${TAKUMI_OPENMU_SYNC_DRY_RUN:-0}"

echo "[sync-monster-spawn-stack] MonsterSetBase=$TAKUMI_MONSTER_SET_BASE_PATH"
echo "[sync-monster-spawn-stack] OpenMU=$OPENMU_DIR"
echo "[sync-monster-spawn-stack] Postgres=$TAKUMI_PG_CONNECTION_STRING"

./scripts/enable-move-map-field-spawns.sh

SYNC_ARGS=("$TAKUMI_MONSTER_SET_BASE_PATH" "$OPENMU_DIR" "--min-ratio" "$MIN_RATIO")
if [[ "$DRY" == "1" ]]; then
  SYNC_ARGS=(--dry-run "${SYNC_ARGS[@]}")
fi
python3 ./scripts/sync-all-spawns-from-openmu.py "${SYNC_ARGS[@]}"

./scripts/report-monster-spawn-coverage.sh
./scripts/compare-spawn-openmu.sh || true

if [[ "${TAKUMI_SKIP_PG_IMPORT:-0}" != "1" ]]; then
  ./scripts/apply-sql.sh "$TAKUMI_PG_CONNECTION_STRING" sql/init/005_monster_spawn.sql 2>/dev/null || true
  export TEST_PG_CONNECTION_STRING="$TAKUMI_PG_CONNECTION_STRING"
  export TAKUMI_PG_CONNECTION_STRING
  ./scripts/import-monster-spawn.sh
  echo "[sync-monster-spawn-stack] Postgres monster_spawn imported (enable TAKUMI_MONSTER_SPAWN_DB=1 in .env)"
else
  echo "[sync-monster-spawn-stack] skipped Postgres import (TAKUMI_SKIP_PG_IMPORT=1)"
fi

echo "[sync-monster-spawn-stack] done. Restart: docker compose restart game-host legacy-login"
