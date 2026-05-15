#!/usr/bin/env bash
# Open one TCP to the connect port and print how many bytes the server sends first (expect C2 F4 06, first byte 0xC2).
# Usage: ./scripts/smoke-connect-from-host.sh [host] [port]
#   ./scripts/smoke-connect-from-host.sh 127.0.0.1 44605
set -euo pipefail
HOST="${1:-127.0.0.1}"
PORT="${2:-44605}"
if ! command -v python3 >/dev/null 2>&1; then
  echo "Need python3 in PATH" >&2
  exit 1
fi
export SMOKE_HOST="$HOST"
export SMOKE_PORT="$PORT"
python3 <<'PY'
import os
import socket

h = os.environ["SMOKE_HOST"]
p = int(os.environ["SMOKE_PORT"])
s = socket.create_connection((h, p), timeout=8)
s.settimeout(8)
d = s.recv(8192)
s.close()
print("recv_len=", len(d))
if d:
    print("first16_hex=", d[:16].hex())
    print("first_byte=", hex(d[0]))
raise SystemExit(0 if d and d[0] == 0xC2 else 1)
PY
