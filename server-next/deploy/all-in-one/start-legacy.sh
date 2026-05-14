#!/usr/bin/env bash
set -euo pipefail

for _ in $(seq 1 120); do
    if su postgres -s /bin/bash -c "/usr/local/bin/pg_isready -q"; then
        break
    fi
    sleep 1
done

POSTGRES_USER="${POSTGRES_USER:-takumi}"
POSTGRES_PASSWORD="${POSTGRES_PASSWORD:-takumi}"
POSTGRES_DB="${POSTGRES_DB:-takumi_runtime}"

escape_sql_literal() {
    printf '%s' "$1" | sed "s/'/''/g"
}

pw_esc="$(escape_sql_literal "$POSTGRES_PASSWORD")"

if ! su postgres -s /bin/bash -c "/usr/local/bin/psql -Atqc \"SELECT 1 FROM pg_roles WHERE rolname='${POSTGRES_USER}'\"" | grep -qx 1; then
    echo "[takumi-aio] creating role ${POSTGRES_USER}"
    su postgres -s /bin/bash -c "/usr/local/bin/psql -v ON_ERROR_STOP=1 -c \"CREATE ROLE \\\"${POSTGRES_USER}\\\" WITH LOGIN PASSWORD '${pw_esc}' CREATEDB;\""
fi

if ! su postgres -s /bin/bash -c "/usr/local/bin/psql -Atqc \"SELECT 1 FROM pg_database WHERE datname='${POSTGRES_DB}'\"" | grep -qx 1; then
    echo "[takumi-aio] creating database ${POSTGRES_DB}"
    su postgres -s /bin/bash -c "/usr/local/bin/psql -v ON_ERROR_STOP=1 -c \"CREATE DATABASE \\\"${POSTGRES_DB}\\\" OWNER \\\"${POSTGRES_USER}\\\";\""
fi

cd /opt/takumi/legacy-login
exec ./Takumi.Server.LegacyLoginHost
