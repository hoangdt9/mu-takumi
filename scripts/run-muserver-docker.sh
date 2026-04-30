#!/usr/bin/env bash
# Chạy MuServer qua docker compose + Wine (takumi/docker).
# Muốn override thư mục: MU_SERVER_HOST_PATH=/path/to/MuServer ./scripts/run-muserver-docker.sh up --build -d
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
export MU_SERVER_HOST_PATH="${MU_SERVER_HOST_PATH:-$ROOT/MuServer}"
cd "$ROOT/docker"
# Profile wine = container MuServer (Wine). On Apple Silicon this often aborts — use profile db for SQL only.
exec env -u DOCKER_DEFAULT_PLATFORM docker compose --profile wine "$@"
