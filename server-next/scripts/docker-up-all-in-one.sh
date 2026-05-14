#!/usr/bin/env bash
# One container: Postgres + LegacyLoginHost + GameHost (see deploy/all-in-one/README.md).
# Optional: ./scripts/docker-up-all-in-one.sh --with-datazip  → also starts nginx for LAN data.zip (port 18080).
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$ROOT"

if [[ ! -f .env ]]; then
  echo "== server-next (all-in-one): missing .env =="
  echo "  cp .env.lan.example .env"
  echo "  Then set TAKUMI_LAN_IP in .env to this machine's LAN IP."
  exit 1
fi

compose_args=()
with_datazip=0
for arg in ${1+"$@"}; do
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

echo "== Takumi server-next ALL-IN-ONE =="
echo "  cwd: $ROOT"
echo "  compose: docker-compose.all-in-one.yml"
if [[ -n "${COMPOSE_PROFILES:-}" ]]; then
  echo "  COMPOSE_PROFILES=$COMPOSE_PROFILES"
fi

if [[ ${#compose_args[@]} -gt 0 ]]; then
  docker compose -f docker-compose.all-in-one.yml up -d --build "${compose_args[@]}"
else
  docker compose -f docker-compose.all-in-one.yml up -d --build
fi

docker compose -f docker-compose.all-in-one.yml ps

if [[ ",${COMPOSE_PROFILES:-}," != *",datazip,"* ]] && [[ "${COMPOSE_PROFILES:-}" != "datazip" ]]; then
  echo ""
  echo "Note: profile datazip is OFF — nothing listens on host port 18080 (Android Preload data.zip)."
  echo "  Fix: ./scripts/docker-up-all-in-one.sh --with-datazip"
  echo "  or:  COMPOSE_PROFILES=datazip docker compose -f docker-compose.all-in-one.yml up -d --build"
  echo "  Put data.zip under takumi/docker/data-zip/host/ (see TAKUMI_DATA_ZIP_HOST_DIR in .env)."
fi

echo ""
echo "Logs: docker compose -f docker-compose.all-in-one.yml logs -f"
