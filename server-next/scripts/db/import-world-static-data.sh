#!/usr/bin/env bash
# M8: ETL Gate.txt + ShopManager/Shop/*.txt + Custom/*.txt → Postgres.
# Requires sql/init/006_map_gate_npc_shop_custom.sql applied.
# Usage from server-next/:
#   export TAKUMI_PG_CONNECTION_STRING='Host=localhost;Port=54444;Username=takumi;Password=takumi;Database=takumi_runtime'
#   export TAKUMI_GAMESERVER_DATA_PATH=/path/to/MuServer/4.GameServer/Sub\ 1/Data
#   ./scripts/db/import-world-static-data.sh
set -euo pipefail
# shellcheck source=../_lib/paths.sh disable=SC1091
source "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../_lib/paths.sh"
takumi_script_paths
cd "$ROOT"

if [[ -z "${TAKUMI_PG_CONNECTION_STRING:-}" && -z "${TAKUMI_PG_HOST:-}" ]]; then
  echo "Set TAKUMI_PG_CONNECTION_STRING or TAKUMI_PG_HOST." >&2
  exit 1
fi

export TEST_PG_CONNECTION_STRING="${TEST_PG_CONNECTION_STRING:-$TAKUMI_PG_CONNECTION_STRING}"
dotnet test ./src/Takumi.Server.Tests/Takumi.Server.Tests.csproj -c Debug \
  --filter "FullyQualifiedName~WorldStaticDataPostgresEtlTests.Import_all_from_gameserver_data"

if [[ -n "${TAKUMI_MONSTER_SET_BASE_PATH:-}" ]]; then
  "$SCRIPTS_ROOT/db/import-monster-spawn.sh"
else
  export TAKUMI_MONSTER_SET_BASE_PATH="${TAKUMI_GAMESERVER_DATA_PATH:?}/Monster/MonsterSetBase.txt"
  "$SCRIPTS_ROOT/db/import-monster-spawn.sh"
fi

echo "[import-world-static-data] done (gates, shops, custom, monster_spawn)."
