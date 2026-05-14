#!/usr/bin/env bash
# Wrapper từ root repo (cwd có thể là Source/android): stack server-next + detach (legacy-login luôn được recreate).
set -euo pipefail
REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
exec "$REPO/server-next/scripts/docker-stack.sh" --detach "$@"
