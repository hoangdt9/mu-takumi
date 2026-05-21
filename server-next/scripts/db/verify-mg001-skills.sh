#!/usr/bin/env bash
# Verify test/mg001: 30 MG combat skills, compact slots 1..30 (017 + CharacterSkillCatalog).
# Usage: ./scripts/db/verify-mg001-skills.sh [postgres_uri]
set -euo pipefail
# shellcheck source=../_lib/paths.sh disable=SC1091
source "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../_lib/paths.sh"
takumi_script_paths

URI="${1:-${TAKUMI_PG_URI:-postgresql://takumi:takumi@127.0.0.1:54444/takumi_runtime}}"
ACCOUNT="${TAKUMI_VERIFY_ACCOUNT:-test}"
CHAR="${TAKUMI_VERIFY_CHAR:-mg001}"

EXPECTED_TYPES=(
  1 2 3 4 5 7 8 9 10 11 12 13 14 17 18 19 20 21 22 23 39 41 47 55 56 57 73 76 236 237
)

echo "== verify MG combat QA: ${ACCOUNT}/${CHAR} =="
echo "  uri: ${URI}"
echo "  expected: ${#EXPECTED_TYPES[@]} combat skills, slots 1..${#EXPECTED_TYPES[@]}"

VERIFY_LINES=()
while IFS= read -r line; do
  VERIFY_LINES+=("$line")
done < <(python3 - "$URI" "$ACCOUNT" "$CHAR" <<'PY'
import subprocess, sys
uri, account, char = sys.argv[1:4]
expected = [
    1, 2, 3, 4, 5, 7, 8, 9, 10, 11, 12, 13, 14, 17, 18, 19, 20, 21, 22, 23,
    39, 41, 47, 55, 56, 57, 73, 76, 236, 237,
]
sql = (
    "SELECT skill_slot, skill_type FROM character_skill "
    f"WHERE account_login='{account}' AND character_name='{char}';"
)
out = subprocess.check_output(["psql", uri, "-t", "-A", "-F", "|", "-c", sql], text=True)
rows = {}
for line in out.splitlines():
    if not line.strip():
        continue
    slot_s, typ_s = line.split("|", 1)
    rows[int(typ_s)] = int(slot_s)
missing = [t for t in expected if t not in rows]
extra = [t for t in rows if t not in expected]
slot_bad = [(t, rows[t], i + 1) for i, t in enumerate(expected) if t in rows and rows[t] != i + 1]
print(f"HAVE {len(rows)}")
if missing:
    print("MISSING " + " ".join(map(str, missing)))
if extra:
    print("EXTRA " + " ".join(map(str, extra)))
for t, got, want in slot_bad:
    print(f"SLOTBAD {t} {got} {want}")
if not missing and not extra and not slot_bad:
    print("OK")
PY
)

missing=()
extra=()
slot_bad=0
have_count=0
for line in "${VERIFY_LINES[@]}"; do
  [[ -z "$line" ]] && continue
  if [[ "$line" == HAVE* ]]; then
    have_count=${line#HAVE }
  elif [[ "$line" == MISSING* ]]; then
    read -r -a missing <<< "${line#MISSING }"
  elif [[ "$line" == EXTRA* ]]; then
    read -r -a extra <<< "${line#EXTRA }"
  elif [[ "$line" == SLOTBAD* ]]; then
    slot_bad=1
    set -- $line
    echo "  bad slot: type=$2 has slot=$3 want=$4"
  elif [[ "$line" == OK ]]; then
    :
  fi
done

echo "  have: ${have_count} skills"
if ((${#missing[@]} > 0)); then
  echo "  MISSING types (${#missing[@]}): ${missing[*]}"
fi
if ((${#extra[@]} > 0)); then
  echo "  EXTRA types (${#extra[@]}): ${extra[*]}"
fi

if ((${#missing[@]} > 0)) || ((${#extra[@]} > 0)) || ((slot_bad != 0)); then
  echo "  fix: ./scripts/db/reset-mg001-skills.sh \"${URI}\""
  exit 1
fi

echo "  OK — ${#EXPECTED_TYPES[@]} combat skills, compact slots."
exit 0
