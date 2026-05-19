#!/usr/bin/env bash
# Wrapper → scripts/smoke/smoke-m8.sh (keeps ./scripts/<name> working from server-next/)
set -euo pipefail
exec "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/smoke/smoke-m8.sh" "$@"
