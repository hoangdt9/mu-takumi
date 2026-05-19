#!/usr/bin/env bash
# Wrapper → scripts/spawn/compare-spawn-openmu.sh (keeps ./scripts/<name> working from server-next/)
set -euo pipefail
exec "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/spawn/compare-spawn-openmu.sh" "$@"
