#!/usr/bin/env bash
# Wrapper → scripts/db/import-world-static-data.sh (keeps ./scripts/<name> working from server-next/)
set -euo pipefail
exec "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/db/import-world-static-data.sh" "$@"
