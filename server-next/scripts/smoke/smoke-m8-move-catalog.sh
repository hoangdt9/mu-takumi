#!/usr/bin/env bash
# P0.7 — verify Move/Move.txt loaded (MoveMapCatalog N > 0) in game-host logs or on disk.
set -euo pipefail

# shellcheck source=../_lib/paths.sh disable=SC1091
source "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../_lib/paths.sh"
takumi_script_paths
cd "$ROOT"

if [[ -f .env ]]; then
  set -a
  # shellcheck disable=SC1091
  . "$ROOT/.env"
  set +a
fi

merge_profile() {
  local p="$1"
  local c=",${COMPOSE_PROFILES:-},"
  [[ "$c" == *",${p},"* ]] || export COMPOSE_PROFILES="${COMPOSE_PROFILES:+$COMPOSE_PROFILES,}$p"
}

merge_profile gamehost

count_from_logs() {
  docker compose logs game-host 2>/dev/null \
    | grep -E '\[m8\] MoveMapCatalog: [0-9]+ moves' \
    | tail -1 \
    | sed -n 's/.*MoveMapCatalog: \([0-9]*\) moves.*/\1/p'
}

N="$(count_from_logs)"
if [[ -z "$N" || "$N" -eq 0 ]]; then
  echo "== smoke-m8: no MoveMapCatalog line in game-host logs; checking Move.txt on host =="
  DATA_ROOT="${TAKUMI_GAMESERVER_DATA_HOST:-../MuServer/4.GameServer/Data}"
  MOVE_FILE="${TAKUMI_MOVE_PATH:-$DATA_ROOT/Move/Move.txt}"
  if [[ -f "$MOVE_FILE" ]]; then
    ROWS="$(grep -cE '^[[:space:]]*[0-9]+' "$MOVE_FILE" 2>/dev/null || echo 0)"
    echo "  Move.txt rows (approx): $ROWS at $MOVE_FILE"
    if [[ "$ROWS" -gt 0 ]]; then
      echo "  OK: Move.txt present (restart game-host to log catalog count)"
      exit 0
    fi
  fi
  echo "FAIL: MoveMapCatalog count is 0 or missing. Set TAKUMI_GAMESERVER_DATA_HOST / TAKUMI_MOVE_PATH and recreate game-host." >&2
  exit 1
fi

echo "OK: MoveMapCatalog loaded $N move(s) (game-host log)"
exit 0
