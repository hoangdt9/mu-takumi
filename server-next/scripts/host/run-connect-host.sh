#!/usr/bin/env bash
# M5: Connect Server only (Takumi.Server.ConnectHost — F4 06/03). Pair with ./scripts/run-login-host.sh
set -euo pipefail

# shellcheck source=../_lib/paths.sh disable=SC1091
source "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../_lib/paths.sh"
takumi_script_paths
cd "$ROOT"

if [[ -f "$ROOT/env.defaults" ]]; then
  set -a
  # shellcheck disable=SC1091
  source "$ROOT/env.defaults"
  set +a
fi

if [[ -f "$ROOT/.env" ]]; then
  set -a
  # shellcheck disable=SC1091
  source "$ROOT/.env"
  set +a
fi

export TAKUMI_VERBOSE="${TAKUMI_VERBOSE:-1}"

echo "== Takumi ConnectHost (M5 split) =="
echo "  TAKUMI_CONNECT_PORT=${TAKUMI_CONNECT_PORT:-44605}"
echo "  F4 03 game port → TAKUMI_GAME_PORT or TAKUMI_LOGIN_PORT"
echo ""

exec dotnet watch run \
  --project "$ROOT/src/Takumi.Server.ConnectHost/Takumi.Server.ConnectHost.csproj" \
  -c Debug
