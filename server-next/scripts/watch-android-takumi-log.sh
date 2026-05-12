#!/usr/bin/env bash
# Tail Takumi-related Android logs in a second terminal while LegacyLoginHost runs in the first.
# Usage: ./scripts/watch-android-takumi-log.sh
# Env: TAKUMI_LOGCAT_CLEAR=0 to skip "adb logcat -c" (keep buffer history).
set -euo pipefail

if ! command -v adb >/dev/null 2>&1; then
  echo "adb not found. Install Android platform-tools or add it to PATH." >&2
  exit 1
fi

if ! adb devices 2>/dev/null | grep -qE '\tdevice$'; then
  echo "No device/emulator in 'adb devices'. Connect USB + authorize debugging or start an emulator." >&2
  exit 1
fi

CLEAR_FIRST="${TAKUMI_LOGCAT_CLEAR:-1}"
if [[ "$CLEAR_FIRST" == "1" ]]; then
  adb logcat -c
fi

echo "== Takumi Android logcat (threadtime) — MuPreload / MuMain / TakumiErrorReport / AndroidRuntime:E — Ctrl+C to stop =="
exec adb logcat -v threadtime \
  MuPreload:I MuMain:I TakumiErrorReport:I AndroidRuntime:E libc:F '*:S'
