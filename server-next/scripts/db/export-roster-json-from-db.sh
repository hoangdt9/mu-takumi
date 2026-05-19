#!/usr/bin/env bash
# Export character_roster + inventory_slot for one account → takumi-roster/{account}.json + takumi-inventory/{account}.json
# Use when SQL seed was applied but login host still shows empty (JSON fallback path).
set -euo pipefail
# shellcheck source=../_lib/paths.sh disable=SC1091
source "$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/../_lib/paths.sh"
takumi_script_paths
URI="${TAKUMI_PG_CONNECTION_STRING:-postgresql://takumi:takumi@127.0.0.1:54444/takumi_runtime}"
ACCOUNT="${1:-test1}"

mkdir -p "$ROOT/takumi-roster" "$ROOT/takumi-inventory"

python3 - "$URI" "$ACCOUNT" "$ROOT" <<'PY'
import json, subprocess, sys
uri, account, root = sys.argv[1:4]

def psql(sql):
    r = subprocess.run(
        ["psql", uri, "-t", "-A", "-F", "\t", "-c", sql],
        capture_output=True,
        text=True,
        check=True,
    )
    return [ln for ln in r.stdout.strip().splitlines() if ln.strip()]

chars = []
for row in psql(
    f"""SELECT character_name, server_class, level, experience, map_id, pos_x, pos_y, angle,
        current_hp, max_hp, current_mp, max_mp, zen,
        strength, dexterity, vitality, energy, leadership, level_up_point
        FROM character_roster WHERE account_login = '{account}' ORDER BY character_name"""
):
    p = row.split("\t")
    chars.append({
        "name": p[0],
        "serverClass": int(p[1]),
        "level": int(p[2]),
        "experience": int(p[3]),
        "mapId": int(p[4]),
        "posX": int(p[5]),
        "posY": int(p[6]),
        "angle": int(p[7]),
        "currentHp": int(p[8]),
        "maxHp": int(p[9]),
        "currentMp": int(p[10]),
        "maxMp": int(p[11]),
        "zen": int(p[12]),
        "strength": int(p[13]),
        "dexterity": int(p[14]),
        "vitality": int(p[15]),
        "energy": int(p[16]),
        "leadership": int(p[17]),
        "levelUpPoint": int(p[18]),
    })

roster_path = f"{root}/takumi-roster/{account}.json"
with open(roster_path, "w", encoding="utf-8") as f:
    json.dump({"characters": chars}, f, indent=2)
    f.write("\n")

inv_chars = []
for name in [c["name"] for c in chars]:
    slots = []
    for row in psql(
        f"""SELECT slot_idx, encode(item, 'hex')
            FROM inventory_slot
            WHERE account_login = '{account}' AND character_name = '{name}'
            ORDER BY slot_idx"""
    ):
        slot, hx = row.split("\t")
        slots.append({"slot": int(slot), "itemHex": hx})
    inv_chars.append({"name": name, "slots": slots})

inv_path = f"{root}/takumi-inventory/{account}.json"
with open(inv_path, "w", encoding="utf-8") as f:
    json.dump({"characters": inv_chars}, f, indent=2)
    f.write("\n")

print(f"[export-roster-json] {roster_path} ({len(chars)} character(s))")
print(f"[export-roster-json] {inv_path}")
PY
