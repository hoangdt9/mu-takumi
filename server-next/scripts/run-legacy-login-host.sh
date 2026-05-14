#!/usr/bin/env bash
# Auto-run Takumi LegacyLoginHost with sane LAN defaults: loads server-next/.env if present,
# then `dotnet watch` rebuilds on every Program.cs change so you only keep the phone client open.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$ROOT"

if [[ -f .env ]]; then
  set -a
  # shellcheck disable=SC1091
  source .env
  set +a
fi

export TAKUMI_VERBOSE="${TAKUMI_VERBOSE:-1}"
export DOTNET_ENVIRONMENT="${DOTNET_ENVIRONMENT:-Development}"

# Default Dec2 path (same client keys as APK); override in .env if your tree differs.
if [[ -z "${TAKUMI_DEC2_PATH:-}" ]]; then
  for _dec2 in "$ROOT/keys/Dec2.dat" "$ROOT/../ClientBuild/Data/Dec2.dat" "$ROOT/../ClientBuild_192.168.99.200/Data/Dec2.dat"; do
    if [[ -f "$_dec2" ]]; then
      export TAKUMI_DEC2_PATH="$_dec2"
      break
    fi
  done
fi

export TAKUMI_ACCOUNTS="${TAKUMI_ACCOUNTS:-test:test}"

echo "== Takumi LegacyLoginHost (watch) =="
echo "  cwd: $ROOT"
echo "  TAKUMI_VERBOSE=$TAKUMI_VERBOSE"
echo "  TAKUMI_LAN_IP=${TAKUMI_LAN_IP:-<unset — primary; phones use this>}"
echo "  TAKUMI_PUBLIC_HOST=${TAKUMI_PUBLIC_HOST:-<unset — optional override for F4 03>}"
echo "  TAKUMI_DEC2_PATH=${TAKUMI_DEC2_PATH:-<unset — login decrypt will fail>}"
echo "  TAKUMI_CS_CONNECT_IDS=${TAKUMI_CS_CONNECT_IDS:-<unset — Program.cs multi-group preset>}"
echo "  TAKUMI_CS_CONNECT_BASE=${TAKUMI_CS_CONNECT_BASE:-<unset>}"
echo "  TAKUMI_CS_CONNECT_COUNT=${TAKUMI_CS_CONNECT_COUNT:-<unset>}"
echo "  TAKUMI_REUSE_ADDR=${TAKUMI_REUSE_ADDR:-<unset — default off; set to 1 if bind fails after quick restart>}"
echo ""

exec dotnet watch run \
  --project "$ROOT/src/Takumi.Server.LegacyLoginHost/Takumi.Server.LegacyLoginHost.csproj" \
  -c Debug
