#!/bin/sh
# GameHost entry for docker-compose bind-mount: Linux container must not reuse host obj/bin
# (macOS/Windows NuGet paths → NETSDK1064). Do NOT rm shared src/*/obj — legacy-login uses it.
# Directory.Build.props + TAKUMI_DOCKER_GAMEHOST=1 redirect obj/bin to /tmp/takumi-gamehost/…
set -e
cd /app

HOST_DLL="src/Takumi.Server.GameHost/bin/Release/net10.0/Takumi.Server.GameHost.dll"
CONTAINER_DLL="/tmp/takumi-gamehost/bin/Takumi.Server.GameHost/Release/net10.0/Takumi.Server.GameHost.dll"

if [ "$TAKUMI_SKIP_CONTAINER_BUILD" = "1" ] || [ "$TAKUMI_SKIP_CONTAINER_BUILD" = "true" ]; then
  echo "[game-host] TAKUMI_SKIP_CONTAINER_BUILD=1 — using host-built IL (./scripts/docker/docker-stack.sh --host-build); dotnet exec DLL"
  GAMEHOST_DLL="$HOST_DLL"
  if [ ! -f "$GAMEHOST_DLL" ]; then
    echo "[game-host] ERROR: missing $GAMEHOST_DLL — run: dotnet build src/Takumi.Server.GameHost/Takumi.Server.GameHost.csproj -c Release" >&2
    exit 1
  fi
else
  echo "[game-host] bind-mount /app — isolated /tmp/takumi-gamehost obj+bin, restore+build (~1–3 min)…"
  rm -rf /tmp/takumi-gamehost 2>/dev/null || true
  dotnet restore src/Takumi.Server.GameHost/Takumi.Server.GameHost.csproj --force-evaluate -nologo -v q
  dotnet build src/Takumi.Server.GameHost/Takumi.Server.GameHost.csproj -c Release --no-restore -nologo -v q
  GAMEHOST_DLL="$CONTAINER_DLL"
  if [ ! -f "$GAMEHOST_DLL" ]; then
    echo "[game-host] FATAL: missing $GAMEHOST_DLL (Directory.Build.props BaseOutputPath)" >&2
    find /tmp/takumi-gamehost -name 'Takumi.Server.GameHost.dll' 2>/dev/null || true
    exit 1
  fi
fi

echo "[game-host] build OK — listening on *:${TAKUMI_GAME_PORT:-55901} (F4 03 target)…"
exec dotnet exec "$GAMEHOST_DLL"
