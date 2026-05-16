#!/usr/bin/env bash
# Who listens on connect/game ports (expect docker-proxy → legacy-login, or dotnet run).
# Multiple LISTEN rows for the same port = conflict — phone may TCP-connect but never get C2 F4 06.
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
if [[ -f "$ROOT/.env" ]]; then
  set -a
  # shellcheck disable=SC1091
  . "$ROOT/.env"
  set +a
fi

CONNECT="${TAKUMI_CONNECT_PUBLISH:-${TAKUMI_CONNECT_PORT:-44605}}"
GAME="${TAKUMI_LEGACY_LOGIN_PUBLISH:-${TAKUMI_LOGIN_PORT:-44606}}"
GAMEHOST="${TAKUMI_GAME_PUBLISH:-${TAKUMI_GAME_PORT:-0}}"

echo "== LISTEN on TCP ${CONNECT} (connect / F4 06) =="
lsof -nP -iTCP:"${CONNECT}" -sTCP:LISTEN 2>/dev/null || echo "  (no listener — stack down?)"
echo ""
echo "== LISTEN on TCP ${GAME} (legacy login port) =="
lsof -nP -iTCP:"${GAME}" -sTCP:LISTEN 2>/dev/null || echo "  (no listener)"
echo ""
if [[ -n "${GAMEHOST}" && "${GAMEHOST}" != "0" ]]; then
  echo "== LISTEN on TCP ${GAMEHOST} (game-host / F4 03 target) =="
  lsof -nP -iTCP:"${GAMEHOST}" -sTCP:LISTEN 2>/dev/null || echo "  (no listener — profile gamehost off or still building?)"
  echo ""
fi
echo "If 44605 is NOT docker-proxy/com.docker, Android will not get server list."
echo "M6 split stack: phone must reach ${GAMEHOST:-55901} after F4 03 (not only 44606)."
