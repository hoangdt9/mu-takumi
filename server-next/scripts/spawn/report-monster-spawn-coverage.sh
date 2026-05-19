#!/usr/bin/env bash
# Report MonsterSetBase coverage vs Move.txt destinations (unit test + optional live log hint).
set -euo pipefail
# shellcheck source=../_lib/paths.sh disable=SC1091
source "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../_lib/paths.sh"
takumi_script_paths
cd "$ROOT"

export TAKUMI_MONSTER_SET_BASE_PATH="${TAKUMI_MONSTER_SET_BASE_PATH:-$ROOT/../MuServer/4.GameServer/Data/Monster/MonsterSetBase.txt}"
export TAKUMI_MOVE_PATH="${TAKUMI_MOVE_PATH:-$ROOT/../MuServer/4.GameServer/Data/Move/Move.txt}"
export TAKUMI_GATE_PATH="${TAKUMI_GATE_PATH:-$ROOT/../MuServer/4.GameServer/Data/Move/Gate.txt}"

echo "[report-monster-spawn-coverage] MonsterSetBase=${TAKUMI_MONSTER_SET_BASE_PATH}"
echo "[report-monster-spawn-coverage] Move=${TAKUMI_MOVE_PATH}"

dotnet test ./src/Takumi.Server.Tests/Takumi.Server.Tests.csproj -c Release \
  --filter "FullyQualifiedName~MapMonsterSpawnCoverageTests" \
  --verbosity minimal

echo "[report-monster-spawn-coverage] OK — see game-host logs for [m8-m9] lines after restart"
