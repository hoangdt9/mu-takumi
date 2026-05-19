#!/usr/bin/env bash
# Wrapper → scripts/spawn/sync-monster-spawn-stack.sh (keeps ./scripts/<name> working from server-next/)
set -euo pipefail
exec "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/spawn/sync-monster-spawn-stack.sh" "$@"
