#!/usr/bin/env bash
# Wrapper: run server-next docker compose from repo root (works when cwd is Source/android).
set -euo pipefail
REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
exec "$REPO/server-next/scripts/docker-recreate-legacy-login.sh" "$@"
