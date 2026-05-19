#!/usr/bin/env bash
# Quick LAN QA: show Takumi-related containers and who listens on usual ports.
set -euo pipefail

echo "=== Docker: takumi-* (running) ==="
docker ps --filter "name=takumi" --format 'table {{.Names}}\t{{.Status}}\t{{.Ports}}' 2>/dev/null || echo "(docker not running)"

echo ""
echo "=== LISTEN (Mac host) ==="
for p in 44605 44606 54444 18080; do
  printf '%5s  ' "$p"
  if out=$(lsof -nP -iTCP:"$p" -sTCP:LISTEN 2>/dev/null); then
    echo "$out" | tail -n +2 | awk '{print $1, $2, $NF}' | head -3
  else
    echo "(nothing)"
  fi
done

echo ""
echo "Notes:"
echo "  - com.docke = Docker Desktop publishing a container port (normal)."
echo "  - 44605/44606 often map to container takumi-next-host — phone uses Mac LAN IP + these ports."
echo "  - To free 44606 for native dotnet LegacyLoginHost: docker stop takumi-next-host (you lose CS/login in that container until you start it again)."
