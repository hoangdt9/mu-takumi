#!/usr/bin/env bash
# Recreate only the LegacyLoginHost container (rebuild entrypoint: dotnet build + run).
# Use this when you ran `docker compose …` from the wrong directory and saw:
#   no configuration file provided: not found
# This script always cds to server-next (where docker-compose.yml lives).
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$ROOT"
exec docker compose up -d --force-recreate legacy-login "$@"
