#!/usr/bin/env bash
# M8: ETL Gate.txt + ShopManager/Shop/*.txt + Custom/*.txt → Postgres.
# Requires sql/init/006_map_gate_npc_shop_custom.sql applied.
# Usage from server-next/:
#   export TAKUMI_PG_CONNECTION_STRING='Host=localhost;Port=54444;Username=takumi;Password=takumi;Database=takumi_runtime'
#   export TAKUMI_GAMESERVER_DATA_PATH=/path/to/MuServer/4.GameServer/Sub\ 1/Data
#   ./scripts/import-world-static-data.sh
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

if [[ -z "${TAKUMI_PG_CONNECTION_STRING:-}" && -z "${TAKUMI_PG_HOST:-}" ]]; then
  echo "Set TAKUMI_PG_CONNECTION_STRING or TAKUMI_PG_HOST." >&2
  exit 1
fi

export TEST_PG_CONNECTION_STRING="${TEST_PG_CONNECTION_STRING:-$TAKUMI_PG_CONNECTION_STRING}"
dotnet test ./src/Takumi.Server.Tests/Takumi.Server.Tests.csproj -c Debug \
  --filter "FullyQualifiedName~WorldStaticDataPostgresEtlTests.Import_all_from_gameserver_data"
echo "[import-world-static-data] done."
