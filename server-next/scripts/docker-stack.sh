#!/usr/bin/env bash
# Takumi server-next — pull image → up stack → (mặc định) tail log Docker.
# Docs: docs/DOCKER-BUILD-RUN.md (khi nào recreate vs build image vs APK).
#
# Stack mặc định: Postgres + LegacyLoginHost (44605/44606) + data.zip (profile datazip, nginx 18080).
# M6: nếu .env có TAKUMI_GAME_PORT (số > 0) thì tự thêm profile **gamehost** — một lệnh đủ Connect + GameHost.
# Không chạy đồng thời ./scripts/run-legacy-login-host.sh (trùng cổng 44606).
#
# Usage:
#   ./scripts/docker-stack.sh
#   ./scripts/docker-stack.sh --detach
#   ./scripts/docker-stack.sh --no-datazip
#   ./scripts/docker-stack.sh --with-gamehost   # bật game-host kể cả khi chưa set TAKUMI_GAME_PORT
#   ./scripts/docker-stack.sh --no-gamehost     # tắt auto game-host dù .env có TAKUMI_GAME_PORT
#   ./scripts/docker-stack.sh --recreate
#   ./scripts/docker-stack.sh --host-build
#
# COMPOSE_PROFILES trong .env được tôn trọng; script chỉ bổ sung profile nếu thiếu.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$ROOT"

WITH_DATAZIP=1
WITH_GAMEHOST=0
NO_GAMEHOST=0
RECREATE=0
HOST_BUILD=0
DETACH=0

while [[ $# -gt 0 ]]; do
  case "$1" in
    --no-datazip)
      WITH_DATAZIP=0
      ;;
    --with-datazip) WITH_DATAZIP=1 ;;
    --with-gamehost) WITH_GAMEHOST=1 ;;
    --no-gamehost) NO_GAMEHOST=1 ;;
    --recreate) RECREATE=1 ;;
    --host-build) HOST_BUILD=1 ;;
    --detach) DETACH=1 ;;
    -h|--help)
      sed -n '1,50p' "$0"
      exit 0
      ;;
    *)
      echo "Unknown option: $1  (use --help)" >&2
      exit 2
      ;;
  esac
  shift
done

if [[ ! -f .env ]]; then
  echo "== server-next: thiếu .env =="
  echo "  cp .env.lan.example .env"
  echo "  Chỉnh TAKUMI_LAN_IP / TAKUMI_PUBLIC_HOST (cùng Wi‑Fi với điện thoại)."
  exit 1
fi

set -a
# shellcheck disable=SC1091
. "$ROOT/.env"
set +a

merge_profile() {
  local p="$1"
  local c=",${COMPOSE_PROFILES:-},"
  if [[ "$c" == *",${p},"* ]]; then
    return 0
  fi
  if [[ -n "${COMPOSE_PROFILES:-}" ]]; then
    export COMPOSE_PROFILES="${COMPOSE_PROFILES},${p}"
  else
    export COMPOSE_PROFILES="$p"
  fi
}

if [[ "$WITH_DATAZIP" -eq 1 ]]; then
  merge_profile datazip
fi

# GameHost (M6): explicit flag, hoặc .env đã cấu hình cổng game (Android F4 03).
if [[ "$NO_GAMEHOST" -eq 0 ]]; then
  if [[ "$WITH_GAMEHOST" -eq 1 ]]; then
    merge_profile gamehost
  elif [[ -n "${TAKUMI_GAME_PORT:-}" ]] && [[ "${TAKUMI_GAME_PORT}" =~ ^[0-9]+$ ]] && [[ "${TAKUMI_GAME_PORT}" -gt 0 ]]; then
    merge_profile gamehost
  fi
fi

compose_profiles_has_gamehost() {
  local raw="${COMPOSE_PROFILES:-}"
  [[ "$raw" == "gamehost" ]] && return 0
  [[ "$raw" == gamehost,* ]] && return 0
  [[ "$raw" == *,gamehost ]] && return 0
  [[ "$raw" == *,gamehost,* ]] && return 0
  return 1
}

echo "== Takumi server-next: stack Docker =="
echo "  cwd: $ROOT"
echo "  COMPOSE_PROFILES=${COMPOSE_PROFILES:-<none>}"
echo "  Lưu ý: không chạy ./scripts/run-legacy-login-host.sh đồng thời (trùng cổng 44606)."
echo ""

if [[ "$HOST_BUILD" -eq 1 ]]; then
  if ! command -v dotnet >/dev/null 2>&1; then
    echo "== Lỗi: --host-build nhưng không có lệnh dotnet trong PATH ==" >&2
    exit 1
  fi
  echo "== dotnet build (host, Release) — kiểm tra dependency trước khi lên container =="
  dotnet build "$ROOT/src/Takumi.Server.LegacyLoginHost/Takumi.Server.LegacyLoginHost.csproj" \
    -c Release -nologo -v minimal
  if compose_profiles_has_gamehost 2>/dev/null || [[ "$WITH_GAMEHOST" -eq 1 ]] || { [[ -n "${TAKUMI_GAME_PORT:-}" ]] && [[ "${TAKUMI_GAME_PORT}" =~ ^[0-9]+$ ]] && [[ "${TAKUMI_GAME_PORT}" -gt 0 ]]; }; then
    dotnet build "$ROOT/src/Takumi.Server.GameHost/Takumi.Server.GameHost.csproj" \
      -c Release -nologo -v minimal
  fi
  echo ""
fi

echo "== docker compose pull (postgres, sdk, nginx…) =="
docker compose pull

up_args=(up -d --pull always --remove-orphans)
if [[ "$RECREATE" -eq 1 ]]; then
  up_args+=(--force-recreate)
fi

echo "== docker compose ${up_args[*]} =="
docker compose "${up_args[@]}"

# Container cũ giữ PID — không chạy lại entrypoint (dotnet build) → C# mới (vd. F4 06 on-accept) không áp dụng.
# game-host cũng vậy: nếu chỉ recreate legacy-login, takumi-game-host có thể vẫn process cũ (hàng giờ) dù source đã đổi.
if [[ "$RECREATE" -eq 0 ]]; then
  echo "== docker compose up -d --force-recreate --no-deps legacy-login (rebuild/run từ bind-mount; ~30–120s) =="
  docker compose up -d --force-recreate --no-deps legacy-login
  if compose_profiles_has_gamehost; then
    echo "== docker compose up -d --force-recreate --no-deps game-host (M6; bind-mount rebuild) =="
    docker compose up -d --force-recreate --no-deps game-host
  fi
fi

echo ""
docker compose ps
echo ""
echo "  Connect / game:  ${TAKUMI_CONNECT_PUBLISH:-44605} / ${TAKUMI_LEGACY_LOGIN_PUBLISH:-44606}"
echo "  Postgres:        ${TAKUMI_POSTGRES_PUBLISH_PORT:-54444}"
if [[ "$WITH_DATAZIP" -eq 1 ]]; then
  dp="${DATA_ZIP_PUBLISH_PORT:-18080}"
  if [[ -n "${TAKUMI_DATA_ZIP_URL:-}" ]]; then
    echo "  data.zip:        ${TAKUMI_DATA_ZIP_URL}"
  elif [[ -n "${TAKUMI_PUBLIC_HOST:-}" ]]; then
    echo "  data.zip:        http://${TAKUMI_PUBLIC_HOST}:${dp}/data.zip"
  else
    echo "  data.zip:        http://<TAKUMI_PUBLIC_HOST>:${dp}/data.zip  (đặt TAKUMI_PUBLIC_HOST trong .env)"
  fi
fi
if compose_profiles_has_gamehost; then
  echo "  game-host:       ${TAKUMI_GAME_PUBLISH:-55901} (F4 03 phải khớp .env; vừa force-recreate nếu không dùng --recreate toàn stack)"
fi
echo "  LAN check:       ./scripts/check-lan-connect-ports.sh"
echo "  Smoke C2 local:  ./scripts/smoke-connect-from-host.sh 127.0.0.1 ${TAKUMI_CONNECT_PUBLISH:-44605}"
echo "  USB (AP isolation): ./scripts/adb-reverse-takumi-dev.sh  rồi build APK -PmuBootstrapAdbReverse=true"
echo "  Nếu APK không recv C2: dừng container rồi chạy host (bỏ NAT Docker): docker compose stop legacy-login && ./scripts/run-legacy-login-host.sh"
echo ""
if compose_profiles_has_gamehost; then
  echo "  M6 tip: đợi legacy-login log \"build OK\" rồi mới mở app — nếu không thấy dòng sub-server, xem legacy-login có \"sent … ServerList\" không; BMD/ids: TAKUMI_CS_CONNECT_* (docs/M3-CONNECT-BMD.md)."
  echo ""
fi

if [[ "$DETACH" -eq 1 ]]; then
  echo "== Detach (--detach): log =="
  echo "  docker compose logs -f legacy-login postgres datazip game-host"
  exit 0
fi

echo "== Logs (Ctrl+C chỉ thoát tail; container vẫn chạy) =="
LOG_SERVICES=(legacy-login postgres)
if docker compose ps -q datazip 2>/dev/null | grep -q .; then
  LOG_SERVICES+=(datazip)
fi
if docker compose ps -q game-host 2>/dev/null | grep -q .; then
  LOG_SERVICES+=(game-host)
fi
exec docker compose logs -f "${LOG_SERVICES[@]}"
