#!/usr/bin/env bash
# Compare test/mg001 character_skill vs MG combat rollout expected set.
# Usage: ./scripts/db/verify-mg001-skills.sh [postgres_uri]
set -euo pipefail
# shellcheck source=../_lib/paths.sh disable=SC1091
source "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../_lib/paths.sh"
takumi_script_paths

URI="${1:-${TAKUMI_PG_URI:-postgresql://takumi:takumi@127.0.0.1:54444/takumi_runtime}}"
ACCOUNT="${TAKUMI_VERIFY_ACCOUNT:-test}"
CHAR="${TAKUMI_VERIFY_CHAR:-mg001}"

# MG combat rollout + base kit (must match 017 + CharacterSkillCatalog.MagicGladiatorSkillTypes)
EXPECTED=(
  1 2 3 4 5 7 8 9 10 11 12 13 14 17 18 19 20 21 22 23
  41 47 48 49 50 51 52 55 56 57 61 62 63 64 65 73
  236 237 238 385 482 487 490 493
)

echo "== verify MG skills: ${ACCOUNT}/${CHAR} =="
echo "  uri: ${URI}"

HAVE=()
while IFS= read -r line; do
  [[ -n "$line" ]] && HAVE+=("$line")
done < <(
  psql "$URI" -t -A -c \
    "SELECT skill_type::text FROM character_skill
     WHERE account_login='${ACCOUNT}' AND character_name='${CHAR}'
     ORDER BY skill_type;"
)

missing=()
for id in "${EXPECTED[@]}"; do
  found=0
  for h in "${HAVE[@]}"; do
    if [[ "$h" == "$id" ]]; then
      found=1
      break
    fi
  done
  if [[ $found -eq 0 ]]; then
    missing+=("$id")
  fi
done

extra=()
for h in "${HAVE[@]}"; do
  [[ -z "$h" ]] && continue
  found=0
  for id in "${EXPECTED[@]}"; do
    if [[ "$h" == "$id" ]]; then
      found=1
      break
    fi
  done
  if [[ $found -eq 0 ]]; then
    extra+=("$h")
  fi
done

echo "  have: ${#HAVE[@]} skills"
if ((${#missing[@]} > 0)); then
  echo "  MISSING (${#missing[@]}): ${missing[*]}"
fi
if ((${#extra[@]} > 0)); then
  echo "  extra (ok for orb tests): ${extra[*]}"
fi

if ((${#missing[@]} > 0)); then
  echo "  fix: ./scripts/db/apply-sql.sh \"\$URI\" sql/patches/019_mg001_add_combat_rollout_skills.sql"
  exit 1
fi

echo "  OK — all ${#EXPECTED[@]} rollout skills present."
exit 0
