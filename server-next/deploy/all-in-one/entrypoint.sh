#!/usr/bin/env bash
set -euo pipefail

export PGDATA="${PGDATA:-/var/lib/postgresql/data}"

mkdir -p "$PGDATA"
chown -R postgres:postgres "$PGDATA"

if [[ ! -f "$PGDATA/PG_VERSION" ]]; then
    echo "[takumi-aio] initdb -> $PGDATA"
    su postgres -s /bin/bash -c "/usr/local/bin/initdb -D \"$PGDATA\" --encoding=UTF8 --locale=C.UTF-8 --auth-local=peer --auth-host=scram-sha-256"
    {
        echo ""
        echo "# takumi all-in-one"
        echo "listen_addresses = '*'"
        echo "max_connections = 100"
    } >>"$PGDATA/postgresql.conf"
    {
        echo ""
        echo "# takumi all-in-one (host port publish -> 5432)"
        echo "host all all 0.0.0.0/0 scram-sha-256"
    } >>"$PGDATA/pg_hba.conf"
fi

dec2_path="${TAKUMI_DEC2_PATH:-/keys/Dec2.dat}"
# Default 1 = same as older images: start even without Dec2 (login decrypt will fail until you mount keys).
# Set TAKUMI_ALLOW_MISSING_DEC2=0 in .env for fail-fast (CI / strict LAN QA).
allow_missing="${TAKUMI_ALLOW_MISSING_DEC2:-1}"
if [[ ! -f "$dec2_path" ]]; then
    if [[ "$allow_missing" == "1" ]]; then
        echo "[takumi-aio] WARNING: Dec2 not found at: $dec2_path — starting anyway (matches legacy all-in-one behaviour)."
        echo "  Login/game need the same Data/Dec2.dat as the client; copy to host mount (see ClientBuild/Data/README.txt)."
        echo "  Strict mode: set TAKUMI_ALLOW_MISSING_DEC2=0 in server-next/.env."
    else
        echo "[takumi-aio] CRITICAL: Dec2 not found at: $dec2_path (TAKUMI_ALLOW_MISSING_DEC2=0)."
        echo "  Put Dec2.dat on the host, set TAKUMI_DEC2_HOST_DIR in .env to that folder, recreate the container."
        echo "  See: ClientBuild/Data/README.txt"
        exit 1
    fi
fi

echo "[takumi-aio] starting supervisord (postgres + legacy-login + game-host)…"
exec supervisord -c /etc/supervisor/supervisord.conf
