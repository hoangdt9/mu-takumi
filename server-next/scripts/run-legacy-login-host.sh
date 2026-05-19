#!/usr/bin/env bash
# Wrapper → scripts/host/run-legacy-login-host.sh (keeps ./scripts/<name> working from server-next/)
set -euo pipefail
exec "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/host/run-legacy-login-host.sh" "$@"
