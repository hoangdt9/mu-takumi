#!/usr/bin/env bash
# Wrapper → scripts/db/import-monster-spawn.sh (keeps ./scripts/<name> working from server-next/)
set -euo pipefail
exec "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/db/import-monster-spawn.sh" "$@"
