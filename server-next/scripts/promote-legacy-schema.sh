#!/usr/bin/env bash
# Wrapper → scripts/db/promote-legacy-schema.sh (keeps ./scripts/<name> working from server-next/)
set -euo pipefail
exec "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/db/promote-legacy-schema.sh" "$@"
