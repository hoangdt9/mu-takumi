#!/usr/bin/env bash
# Wrapper → scripts/smoke/check-lan-connect-ports.sh (keeps ./scripts/<name> working from server-next/)
set -euo pipefail
exec "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/smoke/check-lan-connect-ports.sh" "$@"
