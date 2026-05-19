#!/usr/bin/env bash
# Wrapper → scripts/db/export-roster-json-from-db.sh (keeps ./scripts/<name> working from server-next/)
set -euo pipefail
exec "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/db/export-roster-json-from-db.sh" "$@"
