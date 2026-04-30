#!/usr/bin/env bash
# Restore MuOnline.bak into the Docker SQL Server (run after sqlserver is healthy).
# Usage from takumi/docker:
#   ./sql/restore-muonline.sh
#
# Requires: docker compose --profile db up -d sqlserver
# Adjust logical file names if RESTORE fails — list with:
#   docker exec takumi-mssql /opt/mssql-tools18/bin/sqlcmd -S localhost -U sa -P "$MSSQL_SA_PASSWORD" -C -Q "RESTORE FILELISTONLY FROM DISK='/backup/MuOnline.bak'"
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
DOCKER_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
BAK="${MUONLINE_BAK:-$DOCKER_DIR/../MuServer/7.DataBase/MuOnline.bak}"
CONTAINER="${MSSQL_CONTAINER:-takumi-mssql}"

if [[ ! -f "$BAK" ]]; then
	echo "Missing backup: $BAK" >&2
	exit 1
fi

load_env() {
	if [[ -f "$DOCKER_DIR/.env" ]]; then
		set -a
		# shellcheck disable=SC1090
		source "$DOCKER_DIR/.env"
		set +a
	fi
}
load_env
PASS="${MSSQL_SA_PASSWORD:?Set MSSQL_SA_PASSWORD in docker/.env}"

echo "Copying $(basename "$BAK") into $CONTAINER..."
docker cp "$BAK" "$CONTAINER:/backup/MuOnline.bak"

echo "Restoring (default logical names MuOnline / MuOnline_log — fix script if yours differ)..."
docker exec "$CONTAINER" /opt/mssql-tools18/bin/sqlcmd -S localhost -U sa -P "$PASS" -C -Q "
IF DB_ID('MuOnline') IS NOT NULL
  ALTER DATABASE MuOnline SET SINGLE_USER WITH ROLLBACK IMMEDIATE;
RESTORE DATABASE MuOnline FROM DISK='/backup/MuOnline.bak' WITH REPLACE,
  MOVE 'MuOnline' TO '/var/opt/mssql/data/MuOnline.mdf',
  MOVE 'MuOnline_Log' TO '/var/opt/mssql/data/MuOnline_log.ldf';
ALTER DATABASE MuOnline SET MULTI_USER;
SELECT name FROM sys.databases WHERE name='MuOnline';
"

echo "Done. Point ODBC (Windows) to host:1433 or sqlserver:1433 from another container."
