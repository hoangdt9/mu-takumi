#!/usr/bin/env bash
# Enable section-1 field spawns in MonsterSetBase for Move/warp maps (Dungeon, LT, Atlans, …).
# Idempotent: only uncomments data rows inside known SPOT blocks; leaves //// rows disabled.
set -euo pipefail
# shellcheck source=../_lib/paths.sh disable=SC1091
source "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../_lib/paths.sh"
takumi_script_paths
SET_BASE="${TAKUMI_MONSTER_SET_BASE_PATH:-$ROOT/../MuServer/4.GameServer/Data/Monster/MonsterSetBase.txt}"

if [[ ! -f "$SET_BASE" ]]; then
  echo "[enable-move-map-field-spawns] missing: $SET_BASE" >&2
  exit 1
fi

python3 - "$SET_BASE" <<'PY'
import re, sys
from pathlib import Path

path = Path(sys.argv[1])
lines = path.read_text(encoding='utf-8', errors='ignore').splitlines(keepends=True)
headers = (
    'SPOT - LOST TOWER',
    'SPOTS DUNGEON',
    'SPOT - ATLANS',
    'SPOT - TARKAN',
    '//ICARUS',
    'SPOT - AIDA',
    'SPOT - ELBELAND',
    'kANTURU',
    'SPOT - VULCANUS',
    'kArutan',
)

def is_spawn_data_comment(line: str) -> bool:
    s = line.lstrip()
    if s.startswith('////') or not s.startswith('//'):
        return False
    return bool(re.match(r'\d+\s+\d+', s[2:].lstrip()))

def uncomment(line: str) -> str:
    m = re.match(r'^(\s*)//(.*)$', line.rstrip('\n'))
    if not m:
        return line
    return m.group(1) + m.group(2) + ('\n' if line.endswith('\n') else '')

in_target = False
changed = 0
for i, line in enumerate(lines):
    if any(h in line for h in headers):
        in_target = True
        continue
    if in_target and line.strip().lower() == 'end':
        in_target = False
        continue
    if in_target and is_spawn_data_comment(line):
        new = uncomment(line)
        if new != line:
            lines[i] = new if new.endswith('\n') else new + '\n'
            changed += 1

path.write_text(''.join(lines), encoding='utf-8')
print(f'[enable-move-map-field-spawns] uncommented {changed} rows in {path}')
PY

echo "[enable-move-map-field-spawns] run: "$SCRIPTS_ROOT/spawn/report-monster-spawn-coverage.sh""
