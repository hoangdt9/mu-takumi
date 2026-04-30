#!/usr/bin/env bash
# Start SQL Server container only (docker compose profile db).
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT/docker"
if [[ ! -f .env ]]; then
	echo "Create $ROOT/docker/.env from .env.example (MSSQL_SA_PASSWORD)." >&2
	exit 1
fi
exec docker compose --profile db "$@"
