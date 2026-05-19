#!/usr/bin/env bash
# Wrapper → scripts/android/adb-reverse-takumi-dev.sh (keeps ./scripts/<name> working from server-next/)
set -euo pipefail
exec "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/android/adb-reverse-takumi-dev.sh" "$@"
