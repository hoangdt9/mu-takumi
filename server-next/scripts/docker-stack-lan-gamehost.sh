#!/usr/bin/env bash
# One Docker stack for Android LAN QA: Postgres + legacy-login (44605/44606) + datazip + game-host (55901).
# Phone and Mac must be on the same Wi‑Fi; .env TAKUMI_PUBLIC_HOST = this machine's LAN IP.
#
# Usage:
#   ./scripts/docker-stack-lan-gamehost.sh           # up + detach + smoke
#   ./scripts/docker-stack-lan-gamehost.sh --follow  # up then tail logs (like docker-stack.sh)
#   ./scripts/docker-stack-lan-gamehost.sh --no-sql    # skip apply-sql (volume already seeded)
#
# After stack is healthy, rebuild APK (no adb reverse):
#   cd ../../Source/android && ./gradlew assembleRealDevicePreloadDefaultRelease
#   adb install -r app/build/outputs/apk/realDevicePreloadDefault/release/*.apk
set -euo pipefail

compose_has_gamehost() {
  docker compose ps -q game-host 2>/dev/null | grep -q .
}

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$ROOT"

FOLLOW=0
SKIP_SQL=0
RECREATE=1

while [[ $# -gt 0 ]]; do
  case "$1" in
    --follow) FOLLOW=1 ;;
    --no-sql) SKIP_SQL=1 ;;
    --no-recreate) RECREATE=0 ;;
    -h|--help)
      sed -n '1,22p' "$0"
      exit 0
      ;;
    *)
      echo "Unknown option: $1" >&2
      exit 2
      ;;
  esac
  shift
done

if [[ ! -f .env ]]; then
  echo "Missing .env — copy from .env.lan.example and set TAKUMI_LAN_IP / TAKUMI_PUBLIC_HOST to this Mac's Wi‑Fi IP." >&2
  exit 1
fi

set -a
# shellcheck disable=SC1091
. "$ROOT/.env"
set +a

ensure_env_kv() {
  local key="$1"
  local val="$2"
  if grep -qE "^${key}=" .env 2>/dev/null; then
    return 0
  fi
  echo "${key}=${val}" >> .env
  echo "  appended ${key}=${val} to .env"
}

ensure_env_kv "TAKUMI_GAME_PORT" "55901"
ensure_env_kv "TAKUMI_GAME_PUBLISH" "55901"

if [[ -z "${TAKUMI_PUBLIC_HOST:-}" && -z "${TAKUMI_LAN_IP:-}" ]]; then
  echo "Set TAKUMI_PUBLIC_HOST (and TAKUMI_LAN_IP) in .env to this Mac's LAN IP (same subnet as the phone)." >&2
  exit 1
fi

LAN_HOST="${TAKUMI_LAN_IP:-${TAKUMI_PUBLIC_HOST}}"
CONNECT_PORT="${TAKUMI_CONNECT_PUBLISH:-${TAKUMI_CONNECT_PORT:-44605}}"
GAME_PUBLISH="${TAKUMI_GAME_PUBLISH:-${TAKUMI_GAME_PORT:-55901}}"

echo "== Takumi LAN stack (Postgres + legacy-login + datazip + game-host) =="
echo "  LAN host:     ${LAN_HOST}"
echo "  Connect:      ${CONNECT_PORT}   Game (F4 03): ${GAME_PUBLISH}"
echo "  data.zip:     ${TAKUMI_DATA_ZIP_URL:-http://${LAN_HOST}:18080/data.zip}"
echo ""

# Host dotnet listener conflicts with Docker publish on the same ports.
warn_port_conflict() {
  local port="$1"
  local label="$2"
  if ! lsof -nP -iTCP:"${port}" -sTCP:LISTEN >/dev/null 2>&1; then
    return 0
  fi
  echo "== WARN: something listens on ${port} (${label}) =="
  lsof -nP -iTCP:"${port}" -sTCP:LISTEN 2>/dev/null || true
  if lsof -nP -iTCP:"${port}" -sTCP:LISTEN 2>/dev/null | grep -qE 'dotnet|LegacyLogin'; then
    echo "  Stop ./scripts/run-legacy-login-host.sh before using Docker on the same ports."
  fi
  echo ""
}

warn_port_conflict "${CONNECT_PORT}" "connect"
warn_port_conflict "${TAKUMI_LEGACY_LOGIN_PUBLISH:-${TAKUMI_LOGIN_PORT:-44606}}" "login"
warn_port_conflict "${GAME_PUBLISH}" "game-host"

STACK_ARGS=(--detach --with-gamehost)
if [[ "$RECREATE" -eq 1 ]]; then
  STACK_ARGS+=(--recreate)
fi

"$SCRIPT_DIR/docker-stack.sh" "${STACK_ARGS[@]}"

if [[ "$SKIP_SQL" -eq 0 ]]; then
  PG_PORT="${TAKUMI_POSTGRES_PUBLISH_PORT:-54444}"
  echo ""
  echo "== apply-sql (account table on Postgres volume) =="
  deadline=$((SECONDS + 60))
  while [[ "$SECONDS" -lt "$deadline" ]]; do
    if docker compose exec -T postgres pg_isready -U takumi -d takumi_runtime >/dev/null 2>&1; then
      break
    fi
    sleep 2
  done
  if [[ -x "$SCRIPT_DIR/apply-sql.sh" ]]; then
    "$SCRIPT_DIR/apply-sql.sh" "postgresql://takumi:takumi@127.0.0.1:${PG_PORT}/takumi_runtime" || {
      echo "  apply-sql failed — run manually after stack is up." >&2
    }
  fi
fi

echo ""
echo "== Waiting for legacy-login build OK (max ~180s) =="
deadline=$((SECONDS + 180))
while [[ "$SECONDS" -lt "$deadline" ]]; do
  if docker compose logs legacy-login 2>&1 | tail -n 80 | grep -q 'build OK'; then
    echo "  legacy-login: build OK"
    break
  fi
  sleep 5
done

if compose_has_gamehost; then
  echo "== Waiting for game-host listening on *:${GAME_PUBLISH} (max ~240s) =="
  deadline=$((SECONDS + 240))
  while [[ "$SECONDS" -lt "$deadline" ]]; do
    if docker compose logs game-host 2>&1 | tail -n 40 | grep -q 'listening on'; then
      echo "  game-host: listening"
      break
    fi
    sleep 5
  done
fi

echo ""
if [[ -x "$SCRIPT_DIR/smoke-m8.sh" ]]; then
  echo "== M8 smoke (full) =="
  "$SCRIPT_DIR/smoke-m8.sh" --no-recreate || true
  echo ""
elif [[ -x "$SCRIPT_DIR/smoke-m8-move-catalog.sh" ]]; then
  echo "== M8 smoke: MoveMapCatalog =="
  "$SCRIPT_DIR/smoke-m8-move-catalog.sh" || true
  echo ""
fi

echo "== Port listeners (Mac) =="
"$SCRIPT_DIR/check-lan-connect-ports.sh"

echo ""
echo "== Smoke connect (127.0.0.1:${CONNECT_PORT}) =="
if "$SCRIPT_DIR/smoke-connect-from-host.sh" 127.0.0.1 "${CONNECT_PORT}"; then
  echo "  OK: C2 from localhost"
else
  echo "  FAIL: no C2 on localhost — check docker compose logs legacy-login" >&2
fi

if [[ -n "${LAN_HOST}" ]]; then
  echo ""
  echo "== Smoke connect (${LAN_HOST}:${CONNECT_PORT}) — phone uses this IP =="
  if "$SCRIPT_DIR/smoke-connect-from-host.sh" "${LAN_HOST}" "${CONNECT_PORT}"; then
    echo "  OK: C2 over LAN IP (Docker published port reachable)"
  else
    echo "  WARN: LAN IP smoke failed — Docker Desktop may block phone→Mac NAT."
    echo "        Try: USB ./scripts/adb-reverse-takumi-dev.sh + APK -PmuBootstrapAdbReverse=true"
    echo "        Or: docker compose stop legacy-login && ./scripts/run-legacy-login-host.sh"
  fi
fi

echo ""
echo "== Next: rebuild & install APK (MU_GAME_TCP_PORT from .env) =="
echo "  cd ../../Source/android"
echo "  ./gradlew assembleRealDevicePreloadDefaultRelease"
echo "  adb install -r app/build/outputs/apk/realDevicePreloadDefault/release/*.apk"
echo ""
echo "  Login test/admin — logcat: F4 03 → connect ${LAN_HOST}:${GAME_PUBLISH} → F1 00"
echo "  docker compose logs -f legacy-login game-host"

if [[ "$FOLLOW" -eq 1 ]]; then
  echo ""
  exec docker compose logs -f legacy-login postgres datazip game-host
fi
