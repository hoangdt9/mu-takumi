#!/usr/bin/env bash
# Reverse TCP ports from the Android device to this host (USB adb).
# Use when Wi‑Fi/AP isolation blocks phone → Mac LAN, or to avoid rebuilding APK after each server-next change.
#
# Workflow (one-time APK with localhost bootstrap):
#   ./scripts/adb-reverse.sh
#   cd ../../Source/android && ./gradlew :app:assembleRealDevicePreloadDefaultDebug \
#     -PmuRequiredAbis=armeabi-v7a,arm64-v8a -PmuBootstrapAdbReverse=true
#   adb install -r app/build/outputs/apk/realDevicePreloadDefault/debug/app-realDevice-preloadDefault-debug.apk
#
# Daily QA (server/docker changes only):
#   ./scripts/docker-stack.sh --host-build --recreate --detach
#   ./scripts/adb-reverse.sh
#   (no APK rebuild if already installed with -PmuBootstrapAdbReverse=true)
#
# F4 03 game sub-server must advertise 127.0.0.1 (not LAN IP) while on USB reverse:
#   TAKUMI_PUBLIC_HOST=127.0.0.1 and TAKUMI_LAN_IP=127.0.0.1 in server-next/.env, then recreate stack.
#
# Usage:
#   ./scripts/adb-reverse.sh
#   ./scripts/adb-reverse.sh --remove
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$ROOT"

REMOVE=0
while [[ $# -gt 0 ]]; do
  case "$1" in
    --remove)
      REMOVE=1
      ;;
    -h|--help)
      sed -n '1,35p' "$0"
      exit 0
      ;;
    *)
      echo "Unknown option: $1 (use --help)" >&2
      exit 2
      ;;
  esac
  shift
done

if [[ -f "$ROOT/.env" ]]; then
  set -a
  # shellcheck disable=SC1091
  . "$ROOT/.env"
  set +a
fi

CONNECT_PORT="${TAKUMI_CONNECT_PUBLISH:-${TAKUMI_CONNECT_PORT:-44605}}"
LOGIN_PORT="${TAKUMI_LEGACY_LOGIN_PUBLISH:-${TAKUMI_LOGIN_PORT:-44606}}"
GAME_PORT="${TAKUMI_GAME_PUBLISH:-${TAKUMI_GAME_PORT:-0}}"
DATAZIP_PORT="${DATA_ZIP_PUBLISH_PORT:-18080}"

if ! command -v adb >/dev/null 2>&1; then
  echo "adb not in PATH" >&2
  exit 1
fi

if ! adb devices 2>/dev/null | grep -qE '\tdevice$'; then
  echo "No device in 'adb devices' (USB debugging + authorize)." >&2
  exit 1
fi

if [[ "$REMOVE" -eq 1 ]]; then
  adb reverse --remove-all 2>/dev/null || true
  echo "OK: adb reverse --remove-all"
  exit 0
fi

adb reverse "tcp:${CONNECT_PORT}" "tcp:${CONNECT_PORT}"
adb reverse "tcp:${LOGIN_PORT}" "tcp:${LOGIN_PORT}"
adb reverse "tcp:${DATAZIP_PORT}" "tcp:${DATAZIP_PORT}"

PORTS_OK="tcp:${CONNECT_PORT}, tcp:${LOGIN_PORT}, tcp:${DATAZIP_PORT}"
if [[ -n "$GAME_PORT" ]] && [[ "$GAME_PORT" =~ ^[0-9]+$ ]] && [[ "$GAME_PORT" -gt 0 ]]; then
  adb reverse "tcp:${GAME_PORT}" "tcp:${GAME_PORT}"
  PORTS_OK="${PORTS_OK}, tcp:${GAME_PORT}"
fi

echo "OK: adb reverse ${PORTS_OK} → this host."
echo ""
echo "APK (once): cd Source/android && ./gradlew :app:assembleRealDevicePreloadDefaultDebug \\"
echo "  -PmuRequiredAbis=armeabi-v7a,arm64-v8a -PmuBootstrapAdbReverse=true"
echo ""
echo "Server list / F4 03: set TAKUMI_PUBLIC_HOST=127.0.0.1 in server-next/.env when using USB reverse."
echo "Logcat: ./scripts/watch-android-takumi-log.sh"
echo ""
adb reverse --list 2>/dev/null || true
