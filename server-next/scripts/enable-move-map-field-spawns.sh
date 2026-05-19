#!/usr/bin/env bash
# Wrapper → scripts/spawn/enable-move-map-field-spawns.sh (keeps ./scripts/<name> working from server-next/)
set -euo pipefail
exec "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/spawn/enable-move-map-field-spawns.sh" "$@"
