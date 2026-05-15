#!/usr/bin/env bash
# M8: ETL MonsterSetBase.txt → Postgres monster_spawn (requires 005_monster_spawn.sql applied).
# Usage from server-next/:
#   export TAKUMI_PG_CONNECTION_STRING='postgresql://takumi:takumi@127.0.0.1:54444/takumi_runtime'
#   export TAKUMI_MONSTER_SET_BASE_PATH=/path/to/MuServer/4.GameServer/Data/Monster/MonsterSetBase.txt
#   ./scripts/import-monster-spawn.sh
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

if [[ -z "${TAKUMI_PG_CONNECTION_STRING:-}" && -z "${TAKUMI_PG_HOST:-}" ]]; then
  echo "Set TAKUMI_PG_CONNECTION_STRING or TAKUMI_PG_HOST (+ user/password/database)." >&2
  exit 1
fi

if [[ -z "${TAKUMI_MONSTER_SET_BASE_PATH:-}" ]]; then
  echo "Set TAKUMI_MONSTER_SET_BASE_PATH to MonsterSetBase.txt." >&2
  exit 1
fi

export TEST_PG_CONNECTION_STRING="${TEST_PG_CONNECTION_STRING:-$TAKUMI_PG_CONNECTION_STRING}"
dotnet test ./src/Takumi.Server.Tests/Takumi.Server.Tests.csproj -c Debug \
  --filter "FullyQualifiedName~MonsterSpawnPostgresEtlTests.Import_file_etl" \
  --no-restore 2>/dev/null || dotnet test ./src/Takumi.Server.Tests/Takumi.Server.Tests.csproj -c Debug \
  --filter "FullyQualifiedName~MonsterSpawnPostgresEtlTests.Import_file_etl"
echo "[import-monster-spawn] done."
