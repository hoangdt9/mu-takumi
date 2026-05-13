#!/usr/bin/env bash
# Bring up Postgres + LegacyLoginHost (same as `docker compose up -d` in server-next).
# Optional: ./scripts/docker-up.sh --with-datazip  → also starts nginx for LAN data.zip on port 18080.
# Or set COMPOSE_PROFILES=datazip in .env (Compose reads it automatically).
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$ROOT"

if [[ ! -f .env ]]; then
  echo "== server-next: missing .env =="
  echo "  cp .env.lan.example .env"
  echo "  Then set TAKUMI_PUBLIC_HOST to this Mac's LAN IP (same Wi‑Fi as the phone)."
  exit 1
fi

compose_args=()
with_datazip=0
for arg in "$@"; do
  if [[ "$arg" == "--with-datazip" ]]; then
    with_datazip=1
  else
    compose_args+=("$arg")
  fi
done

if [[ "$with_datazip" -eq 1 ]]; then
  if [[ ",${COMPOSE_PROFILES:-}," != *",datazip,"* ]] && [[ "${COMPOSE_PROFILES:-}" != "datazip" ]]; then
    if [[ -n "${COMPOSE_PROFILES:-}" ]]; then
      export COMPOSE_PROFILES="${COMPOSE_PROFILES},datazip"
    else
      export COMPOSE_PROFILES="datazip"
    fi
  fi
fi

echo "== Takumi server-next (Docker) =="
echo "  cwd: $ROOT"
if [[ -n "${COMPOSE_PROFILES:-}" ]]; then
  echo "  COMPOSE_PROFILES=$COMPOSE_PROFILES"
fi
echo "  Starting: docker compose up -d …"

if [[ ${#compose_args[@]} -gt 0 ]]; then
  docker compose up -d "${compose_args[@]}"
else
  docker compose up -d
fi

echo ""
docker compose ps
echo ""
echo "  Connect / game: 44605 / 44606 (override with TAKUMI_CONNECT_PUBLISH / TAKUMI_LEGACY_LOGIN_PUBLISH)"
echo "  F4 06 list IDs:  TAKUMI_CS_CONNECT_BASE (default 20) × TAKUMI_CS_CONNECT_COUNT — must match ServerList.bmd groups (id/20)"
echo "  Postgres:        54444 (override with TAKUMI_POSTGRES_PUBLISH_PORT)"
if [[ -n "${COMPOSE_PROFILES:-}" ]] && [[ "${COMPOSE_PROFILES}" == *"datazip"* ]]; then
  echo "  data.zip HTTP:   ${DATA_ZIP_PUBLISH_PORT:-18080} → GET /data.zip (file: ../docker/data-zip/host/data.zip)"
fi
echo "  Logs:           docker compose logs -f legacy-login"
echo "  Stop:           docker compose down"
