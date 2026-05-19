#!/usr/bin/env bash
# Wrapper → scripts/spawn/report-monster-spawn-coverage.sh (keeps ./scripts/<name> working from server-next/)
set -euo pipefail
exec "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/spawn/report-monster-spawn-coverage.sh" "$@"
