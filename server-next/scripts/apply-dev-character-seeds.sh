#!/usr/bin/env bash
# Wrapper → scripts/db/apply-dev-character-seeds.sh (keeps ./scripts/<name> working from server-next/)
set -euo pipefail
exec "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/db/apply-dev-character-seeds.sh" "$@"
