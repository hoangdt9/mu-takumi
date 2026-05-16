#!/usr/bin/env bash
# One-shot: promote takumi_legacy (+ takumi_staging) → public.*, migrate EF takumi_runtime.account, drop schema takumi_runtime.
# Requires sql/init/010_account.sql applied.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

export TAKUMI_LEGACY_PROMOTE="${TAKUMI_LEGACY_PROMOTE:-1}"
export TAKUMI_DROP_EF_RUNTIME_SCHEMA="${TAKUMI_DROP_EF_RUNTIME_SCHEMA:-1}"
export TAKUMI_ROSTER_DB_SYNC="${TAKUMI_ROSTER_DB_SYNC:-1}"
export TAKUMI_ACCOUNT_DB="${TAKUMI_ACCOUNT_DB:-1}"
export TAKUMI_LEGACY_PROMOTE_ONLY=1

PG_URL="${1:-postgresql://takumi:takumi@127.0.0.1:54444/takumi_runtime}"
# Always win over env.defaults / .env (bash `source` can truncate unquoted semicolon connection strings).
export TAKUMI_PG_CONNECTION_STRING="$PG_URL"

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
export TAKUMI_PG_CONNECTION_STRING="$PG_URL"

echo "[promote-legacy] applying sql/init…"
"$ROOT/scripts/apply-sql.sh" "$PG_URL"

echo "[promote-legacy] running promoter (no TCP listen)…"
dotnet build "$ROOT/src/Takumi.Server.GameHost/Takumi.Server.GameHost.csproj" -c Release -nologo -v q
dotnet run --project "$ROOT/src/Takumi.Server.GameHost/Takumi.Server.GameHost.csproj" -c Release --no-build
