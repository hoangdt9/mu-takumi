#!/usr/bin/env bash
# Wrapper → scripts/db/migrate-roster-json-to-db.sh (keeps ./scripts/<name> working from server-next/)
set -euo pipefail
exec "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/db/migrate-roster-json-to-db.sh" "$@"
