#!/usr/bin/env bash
# M8 smoke — verify game-host loaded move-map data + unit tests (P0.7 / checklist step 12).
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$ROOT"

NO_RECREATE=0
SKIP_TESTS=0
while [[ $# -gt 0 ]]; do
  case "$1" in
    --no-recreate) NO_RECREATE=1 ;;
    --skip-tests) SKIP_TESTS=1 ;;
    -h|--help)
      echo "Usage: $0 [--no-recreate] [--skip-tests]"
      exit 0
      ;;
    *)
      echo "Unknown option: $1" >&2
      exit 2
      ;;
  esac
  shift
done

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

fail() {
  echo "FAIL: $*" >&2
  exit 1
}

ok() {
  echo "OK: $*"
}

if [[ "$NO_RECREATE" -eq 0 ]]; then
  if ! command -v dotnet >/dev/null 2>&1; then
    fail "dotnet not in PATH (use --no-recreate if game-host already rebuilt)"
  fi
  echo "== smoke-m8: host build GameHost (Release) =="
  dotnet build "$ROOT/src/Takumi.Server.GameHost/Takumi.Server.GameHost.csproj" \
    -c Release -nologo -v minimal
  export TAKUMI_SKIP_CONTAINER_BUILD=1
  echo "== smoke-m8: recreate game-host =="
  docker compose up -d --force-recreate --no-deps game-host
  echo "== smoke-m8: waiting for catalog logs (max 180s) =="
  deadline=$((SECONDS + 180))
  while [[ "$SECONDS" -lt "$deadline" ]]; do
    if docker compose logs game-host 2>&1 | grep -q '\[m8\] MoveMapCatalog:'; then
      break
    fi
    sleep 3
  done
fi

if ! docker compose ps -q game-host 2>/dev/null | grep -q .; then
  fail "game-host container not running (COMPOSE_PROFILES=gamehost, TAKUMI_GAME_PORT set)"
fi

logs="$(docker compose logs game-host 2>&1)"

grep_log() {
  echo "$logs" | grep -E "$1" | tail -1 || true
}

require_log() {
  local pattern="$1"
  local label="$2"
  local line
  line="$(grep_log "$pattern")"
  if [[ -z "$line" ]]; then
    fail "missing log: $label (pattern: $pattern)"
  fi
  ok "$line"
}

echo "== smoke-m8: startup catalogs (game-host) =="
require_log '\[m8\] MapGateCatalog: [0-9]+ gates' 'MapGateCatalog'
require_log '\[m8\] MoveMapCatalog: [0-9]+ moves' 'MoveMapCatalog'
MOVE_N="$(echo "$logs" | sed -n 's/.*\[m8\] MoveMapCatalog: \([0-9]*\) moves.*/\1/p' | tail -1)"
[[ -n "$MOVE_N" && "$MOVE_N" -gt 0 ]] || fail "MoveMapCatalog count is 0"

require_log '\[m8\] CustomArenaCatalog:' 'CustomArenaCatalog'
require_log '\[m8\] MapManagerCatalog:' 'MapManagerCatalog'

DATA_MOUNT="${TAKUMI_GAMESERVER_DATA_PATH:-/muserver-data}"
echo "== smoke-m8: data mount inside container ($DATA_MOUNT) =="
docker compose exec -T game-host test -f "$DATA_MOUNT/Move/Move.txt" \
  || fail "Move/Move.txt missing at $DATA_MOUNT (check TAKUMI_GAMESERVER_DATA_HOST mount)"
docker compose exec -T game-host test -f "$DATA_MOUNT/Move/Gate.txt" \
  || fail "Move/Gate.txt missing at $DATA_MOUNT"
ok "Move.txt + Gate.txt present in container"

HOST_DATA="${TAKUMI_GAMESERVER_DATA_HOST:-../MuServer/4.GameServer/Data}"
HOST_DATA="$(cd "$ROOT" && cd "$HOST_DATA" 2>/dev/null && pwd || echo "$HOST_DATA")"
echo "== smoke-m8: host data path =="
echo "  TAKUMI_GAMESERVER_DATA_HOST → $HOST_DATA"
echo "  (MuServer tree = static game data for loaders; runtime is server-next in Docker)"

if [[ "$SKIP_TESTS" -eq 0 ]]; then
  echo "== smoke-m8: dotnet test (M8 unit) =="
  dotnet test "$ROOT/src/Takumi.Server.Tests/Takumi.Server.Tests.csproj" \
    -c Release \
    --filter "FullyQualifiedName~MoveMap|FullyQualifiedName~CustomArena|FullyQualifiedName~PlayerUi|FullyQualifiedName~PersonalShop" \
    -nologo -v minimal
fi

echo ""
echo "== smoke-m8: PASS (MoveMapCatalog=$MOVE_N moves) =="
