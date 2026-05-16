#!/usr/bin/env bash
# Reverse TCP ports from the Android device to this host (USB adb).
# Use when Wi‑Fi/AP isolation blocks phone → Mac LAN (Connect 44605 recv C2).
#
# After this script:
#   cd ../../Source/android && ./gradlew :app:assembleRealDevicePreloadDefaultRelease -PmuBootstrapAdbReverse=true
#   adb install -r app/build/outputs/apk/realDevicePreloadDefault/release/app-realDevice-preloadDefault-release.apk
#
# Ports match server-next/.env defaults (override env if you publish on other ports).
set -euo pipefail

CONNECT_PORT="${TAKUMI_CONNECT_PUBLISH:-${TAKUMI_CONNECT_PORT:-44605}}"
LOGIN_PORT="${TAKUMI_LEGACY_LOGIN_PUBLISH:-${TAKUMI_LOGIN_PORT:-44606}}"
GAME_PORT="${TAKUMI_GAME_PUBLISH:-${TAKUMI_GAME_PORT:-55901}}"
DATAZIP_PORT="${DATA_ZIP_PUBLISH_PORT:-18080}"

if ! command -v adb >/dev/null 2>&1; then
  echo "adb not in PATH" >&2
  exit 1
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
echo "Rebuild APK: cd Source/android && ./gradlew :app:assembleRealDevicePreloadDefaultRelease -PmuBootstrapAdbReverse=true"
