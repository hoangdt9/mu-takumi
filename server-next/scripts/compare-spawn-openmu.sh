#!/usr/bin/env bash
# Rough drift report: MonsterSetBase field spots (section 1) vs OpenMU SeasonSix map initializers.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SET_BASE="${TAKUMI_MONSTER_SET_BASE_PATH:-$ROOT/../MuServer/4.GameServer/Data/Monster/MonsterSetBase.txt}"
OPENMU_MAPS="${OPENMU_MAPS_DIR:-$ROOT/../../OpenMU/src/Persistence/Initialization/VersionSeasonSix/Maps}"

if [[ ! -f "$SET_BASE" ]]; then
  echo "[compare-spawn-openmu] missing MonsterSetBase: $SET_BASE" >&2
  exit 1
fi

echo "[compare-spawn-openmu] MonsterSetBase=$SET_BASE"
echo "[compare-spawn-openmu] OpenMU maps=$OPENMU_MAPS"
echo ""

python3 - "$SET_BASE" "$OPENMU_MAPS" <<'PY'
import re, sys
from pathlib import Path

set_path, openmu_dir = Path(sys.argv[1]), Path(sys.argv[2])
section = -1
field_by_map = {}
with set_path.open(encoding="utf-8", errors="ignore") as f:
    for line in f:
        s = line.split("//")[0].strip()
        if not s:
            continue
        if s.lower() == "end":
            section = -1
            continue
        if re.fullmatch(r"\d+", s):
            section = int(s)
            continue
        if section != 1:
            continue
        p = s.split()
        if len(p) < 9:
            continue
        m = int(p[1])
        qty = int(p[8])
        field_by_map[m] = field_by_map.get(m, 0) + qty

openmu_qty = {}
if openmu_dir.is_dir():
    for cs in sorted(openmu_dir.glob("*.cs")):
        text = cs.read_text(encoding="utf-8", errors="ignore")
        m = re.search(r"internal const byte Number = (\d+);", text)
        if not m:
            continue
        map_id = int(m.group(1))
        # CreateMonsterSpawn(..., count) in CreateMonsterSpawns only
        block = re.search(
            r"CreateMonsterSpawns\(\).*?(?=protected override|CreateMonsters|\Z)",
            text,
            re.S,
        )
        if not block:
            continue
        qty = sum(int(x) for x in re.findall(r"CreateMonsterSpawn\([^)]+,\s*(\d+)\s*\)", block.group(0)))
        if qty:
            openmu_qty[map_id] = qty

keys = sorted(set(field_by_map) | set(openmu_qty))
print(f"{'map':>4}  {'setbase_s1_qty':>14}  {'openmu_s6_qty':>14}  note")
for k in keys:
    a, b = field_by_map.get(k, 0), openmu_qty.get(k, 0)
    note = ""
    if b and not a:
        note = "MISSING in set-base"
    elif a and not b:
        note = "extra/custom in set-base"
    elif a and b and abs(a - b) > max(5, b // 4):
        note = "drift"
    print(f"{k:4}  {a:14}  {b:14}  {note}")

print("")
print("Notes:")
print("- set-base section 1 quantity sum vs OpenMU CreateMonsterSpawns count (approximate).")
print("- Section 3 invasion rows excluded at runtime unless TAKUMI_MONSTER_INCLUDE_INVASION_SPAWN=1.")
print("- Loren Market (79): OpenMU NPC-only — set-base should have section 0 NPCs, not field.")
PY
