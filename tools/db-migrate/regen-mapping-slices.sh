#!/usr/bin/env bash
# Regenerate auto-generated Phase 2 CSV slices when legacy MSSQL / OpenMU Postgres are up.
# Requires: dotnet SDK from repo root. Connection strings optional — missing side is skipped.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
MSQL_PROJ="$ROOT/tools/db-migrate/dotnet/Takumi.MssqlInspect/Takumi.MssqlInspect.csproj"
PG_PROJ="$ROOT/tools/db-migrate/dotnet/Takumi.PgInspect/Takumi.PgInspect.csproj"
DOC="$ROOT/docs/takumi-game-spec"
SCHEMA_DIR="$ROOT/tools/db-migrate/schemas"

mkdir -p "$SCHEMA_DIR"
cd "$ROOT"

dotnet build "$ROOT/tools/db-migrate/dotnet/Takumi.DbTools.slnx" -c Release --nologo -v q

if [[ -n "${TAKUMI_MSSQL_CONNECTION:-}" ]]; then
  echo "Writing MSSQL mapping rows → PHASE2-MAPPING-MSSQL-DBO-AUTO.csv"
  dotnet run --project "$MSQL_PROJ" -c Release --no-build -- --mapping-rows > "$DOC/PHASE2-MAPPING-MSSQL-DBO-AUTO.csv"
  echo "Writing MSSQL row counts → $SCHEMA_DIR/mssql-dbo-row-counts.csv"
  dotnet run --project "$MSQL_PROJ" -c Release --no-build -- --row-counts > "$SCHEMA_DIR/mssql-dbo-row-counts.csv"
else
  echo "[MSSQL] SKIP — set TAKUMI_MSSQL_CONNECTION to regen dbo slices"
fi

if [[ -n "${TAKUMI_PG_CONNECTION:-}" ]]; then
  echo "Writing OpenMU mapping rows → PHASE2-MAPPING-OPENMU-EF-TABLES-FULL.csv"
  dotnet run --project "$PG_PROJ" -c Release --no-build -- --mapping-openmu-all > "$DOC/PHASE2-MAPPING-OPENMU-EF-TABLES-FULL.csv"
  echo "Writing OpenMU row counts → $SCHEMA_DIR/openmu-all-row-counts.csv"
  dotnet run --project "$PG_PROJ" -c Release --no-build -- --row-counts-openmu-all > "$SCHEMA_DIR/openmu-all-row-counts.csv"
else
  echo "[PG]    SKIP — set TAKUMI_PG_CONNECTION to regen OpenMU slices"
fi

echo "Done. Merge dbo + OpenMU CSV into PHASE2-MAPPING-TEMPLATE.csv manually (see tools/db-migrate/README.md)."
