#!/usr/bin/env bash
# Wrapper → scripts/docker/docker-game-host-boot.sh (keeps ./scripts/<name> working from server-next/)
set -euo pipefail
exec "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/docker/docker-game-host-boot.sh" "$@"
