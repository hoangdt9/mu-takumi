#!/usr/bin/env bash
# Who listens on connect/game ports (expect docker-proxy → legacy-login, or dotnet run).
# Multiple LISTEN rows for the same port = conflict — phone may TCP-connect but never get C2 F4 06.
set -euo pipefail
CONNECT="${TAKUMI_CONNECT_PUBLISH:-44605}"
GAME="${TAKUMI_LEGACY_LOGIN_PUBLISH:-44606}"
echo "== LISTEN on TCP ${CONNECT} (connect) =="
lsof -nP -iTCP:"${CONNECT}" -sTCP:LISTEN 2>/dev/null || echo "  (no listener — stack down?)"
echo ""
echo "== LISTEN on TCP ${GAME} (game/login) =="
lsof -nP -iTCP:"${GAME}" -sTCP:LISTEN 2>/dev/null || echo "  (no listener)"
echo ""
echo "If 44605 is NOT docker-proxy/com.docker or your LegacyLoginHost, Android will not get server list."
