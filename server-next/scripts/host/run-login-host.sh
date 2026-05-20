#!/usr/bin/env bash
# M5: login/game TCP only (Takumi.Server.LoginHost). Run Connect separately: ./scripts/host/run-connect-host.sh
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
export DOTNET_ENVIRONMENT="${DOTNET_ENVIRONMENT:-Development}"

echo "== Takumi LoginHost (M5 split — no Connect port) =="
echo "  TAKUMI_LOGIN_PORT=${TAKUMI_LOGIN_PORT:-44606}"
echo "  Pair with: ./scripts/host/run-connect-host.sh (TAKUMI_CONNECT_PORT)"
echo ""

exec dotnet watch run \
  --project "$ROOT/src/Takumi.Server.LoginHost/Takumi.Server.LoginHost.csproj" \
  -c Debug
