#!/usr/bin/env python3
"""Regenerate SKILL-QA-CHECKLIST.csv and SKILL-QA-CHECKLIST.md from Skill.txt."""

from __future__ import annotations

import argparse
import csv
import re
import sys
from collections import defaultdict
from pathlib import Path

REPO = Path(__file__).resolve().parents[3]
SKILL_TXT = REPO / "MuServer/4.GameServer/Data/Skill/Skill.txt"
OUT_CSV = REPO / "docs/android/SKILL-QA-CHECKLIST.csv"
OUT_MD_SECTIONS = REPO / "docs/android/_skill-qa-sections.md"
OUT_CHECKLIST = REPO / "docs/android/SKILL-QA-CHECKLIST.md"

CLASS_LABELS = ["DW", "DK", "FE", "MG", "DL", "SU", "RF"]
CLASS_NAMES = {
    "DW": "Dark Wizard",
    "DK": "Dark Knight",
    "FE": "Fairy Elf",
    "MG": "Magic Gladiator",
    "DL": "Dark Lord",
    "SU": "Summoner",
    "RF": "Rage Fighter",
}

AREA_CONTINUE = {8, 9, 55, 56, 236, 237, 238, 10, 13, 14, 378, 483, 482, 490, 493}
AREA_CONTINUE.update(range(43, 53))
AREA_CONTINUE.update(range(61, 66))
AREA_CONTINUE.update(range(14, 19))
AREA_CONTINUE.update({385, 487})

DIRECTIONAL = {55, 56, 236, 238, 490, 493, 482, 10, 13, 378, 483} | set(range(48, 53))
CORRIDOR = {8}
TARGETED = {1, 2, 3, 4, 5, 7, 12, 17, 38, 39, 40}
BURST = {13, 14, 38, 40, 382}
PHYS = {19, 20, 21, 22, 23, 41, 42, 44, 47, 57}
# Mirror SkillCombatCatalog.UsesPhysicalStatRoll (MG slashes + DK melee skill IDs on magic wire).
PHYS_STAT = set(PHYS) | {55, 56, 236, 482, 490, 493} | set(range(48, 53))

# Combat skills not yet wired on server (stub damage until wire + catalog).
PENDING_WIRE_COMBAT = {
    11,
    60,
    66,
    73,
    74,
    76,
    78,
    214,
    215,
    216,
    223,
    224,
    225,
    230,
    232,
    235,
    269,
}

RANGE_OVERRIDE = {
    8: 6,
    9: 6,
    55: 2,
    56: 2,
    236: 2,
    237: 6,
    238: 3,
    10: 5,
    13: 5,
    490: 2,
    493: 2,
    482: 2,
    3: 6,
    4: 6,
    5: 6,
    12: 7,
}
MELEE_RANGE = {19: 3, 20: 2, 21: 2, 22: 2, 23: 2, 41: 3, 42: 4, 44: 4, 47: 6, 57: 4}

# MG combat QA on mg001 (slot 1–30). User-confirmed done → full verify_* + QA.
MG_QA_DONE_IDS = {8, 9, 55, 237}
MG_QA_DONE_LABELS = {9: "Linh hồn", 8: "Lốc", 55: "Chém lửa", 237: "Bão điện"}
MG_QA_NEXT_IDS = [56, 236, 10, 13, 14, 3, 4, 5, 12, 47, 39, 41, 57, 19, 20, 21, 22, 23]

PRE_VERIFY = {
    9: {
        "verify_formula": "x",
        "verify_range": "x",
        "verify_hit_volume": "x",
        "verify_wire_cast": "x",
        "verify_vfx_speed": "x",
        "verify_char_anim": "x",
        "verify_world_vfx": "x",
        "verify_qa_ingame": "x",
        "notes": "Linh hồn ref; user confirmed 2026-05-21",
    },
    8: {
        "verify_formula": "x",
        "verify_range": "x",
        "verify_hit_volume": "x",
        "verify_wire_cast": "x",
        "verify_vfx_speed": "x",
        "verify_char_anim": "x",
        "verify_world_vfx": "x",
        "verify_qa_ingame": "x",
        "notes": "Lốc corridor mode=2; user confirmed 2026-05-21",
    },
    55: {
        "verify_formula": "x",
        "verify_range": "x",
        "verify_hit_volume": "x",
        "verify_wire_cast": "x",
        "verify_vfx_speed": "x",
        "verify_char_anim": "p",
        "verify_world_vfx": "p",
        "verify_qa_ingame": "x",
        "notes": "Chém lửa phys+skill; user confirmed 2026-05-21",
    },
    56: {"verify_formula": "x", "notes": "phys+skill Power Slash"},
    236: {"verify_formula": "x", "notes": "phys+skill Flame Strike"},
    237: {
        "verify_formula": "x",
        "verify_range": "x",
        "verify_hit_volume": "x",
        "verify_wire_cast": "x",
        "verify_vfx_speed": "x",
        "verify_char_anim": "x",
        "verify_world_vfx": "x",
        "verify_qa_ingame": "x",
        "notes": "Bão điện chebyshev r6 mode=0; channel; user confirmed 2026-05-21",
    },
    3: {"verify_formula": "x"},
    4: {"verify_formula": "x"},
    5: {"verify_formula": "x"},
    12: {"verify_formula": "x"},
    19: {"verify_formula": "x"},
    20: {"verify_formula": "x"},
    21: {"verify_formula": "x"},
    22: {"verify_formula": "x"},
    23: {"verify_formula": "x"},
    41: {"verify_formula": "x"},
    42: {"verify_formula": "x"},
}

CSV_HEADER = [
    "skill_id",
    "skill_name",
    "class",
    "role",
    "wire",
    "hit_volume",
    "server_range",
    "skill_txt_range",
    "skill_txt_radio",
    "damage_formula",
    "stat_roll",
    "catalog_in_code",
    "verify_formula",
    "verify_range",
    "verify_hit_volume",
    "verify_wire_cast",
    "verify_vfx_speed",
    "verify_char_anim",
    "verify_world_vfx",
    "verify_qa_ingame",
    "notes",
]

QA_INTRO = """# Skill QA checklist (286 skill)

**Cập nhật:** 2026-05-21  
**Phạm vi:** QA in-game — cast wire, damage, hit volume, animation, VFX (`server-next` + Android).  
**Nguồn:** `MuServer/4.GameServer/Data/Skill/Skill.txt`  
**Regenerate:** `python3 docs/android/scripts/generate-skill-qa-checklist.py`

**Tài liệu dev (không bắt buộc QA):** [MOBILE-SKILL-COMBAT-GUIDE.md](./MOBILE-SKILL-COMBAT-GUIDE.md) · [qa/M9-mg-skill-combat.md](../qa/M9-mg-skill-combat.md)

**Export Excel:** [SKILL-QA-CHECKLIST.csv](./SKILL-QA-CHECKLIST.csv) — filter `role=combat`, sort `class`, tick cột `verify_*`. Cột **`stat_roll`** = công thức server hiện tại.

---

## Công thức damage (2026-05-21) — đã code, chưa QA hết

**Code:** `PlayerSkillCombatDamage602.cs` · `SkillCombatCatalog.cs` · `MonsterCombatCalculator.ApplySkillDamageToMonster`

| `stat_roll` (CSV) | Áp dụng khi | Roll server |
|-------------------|-------------|-------------|
| **wiz** | Wire `0x19` / `0x1E` / `0xDB`, không thuộc MG slash | `MagicMin/Max + Skill.txt base` → trừ def → `SkillFinalMultiplier` |
| **phys** | MG Fire/Power/Flame (55–56, 236, 48–52, 482, 490, 493) | `PhysiMin/Max + skill base` (Webzen `GetAttackDamage`) |
| **tap** | Wire `0x11` (Cyclone, Falling Slash, …) | `Physi` **không** cộng base theo skill ID (`skillId=0`) — skill phải **> tap** khi test **wiz/phys** |
| **pend** | Combat nhưng **chưa wire** (17 skill) | Stub `level×8+10` — **chưa** công thức mới |
| **n/a** | Support / master | Không test damage combat |

**Coverage combat (63 skill):** **46** skill `wiz`/`phys` · **8** skill `tap` (`0x11`) · **17** `pend` · còn lại support/master (223 skill).

**Pass cột F (formula):** rebuild `game-host` → đánh mob → log `[m9]` có `statDmg>0` (không `-1`) → số HP **skill > tap trái** (MG: Fire Slash 55 vs tap). MG slash: `statOverride=1`, damage type phys.

**Chưa làm (dev backlog):** wire + stat roll cho 17 skill `pend` (Plasma Storm, Five Shot, SU chain, RF Charge, …); gắn `skillId` vào `0x11` để melee skill dùng `GetSkillBaseDamage`.

---

## Cách test (mỗi skill combat)

| Cột | Ý nghĩa | Pass |
|-----|---------|------|
| **F** | Damage formula — khớp `stat_roll`, log `statDmg` | HP giảm hợp lý; skill > tap khi `wiz`/`phys` |
| **R** | Range khớp Skill.txt | Đánh/trúng đúng khoảng cách |
| **H** | Hit volume (hình vùng đánh) | Mob đúng vùng; channel log `mode=0/1/2` |
| **W** | Wire cast (`0x11` / `0x19` / `0x1E` / `0xDB`) | Client gửi đúng, server nhận |
| **S** | VFX speed (MagicSpeed / AttackSpeed) | Hiệu ứng không quá chậm/nhanh |
| **A** | Animation nhân vật | Pose/skill action đúng (không chỉ pose cast chung) |
| **V** | World VFX | Joint/effect spawn đúng skill |
| **QA** | End-to-end | Chỉ tick khi **F+R+H+W** + quan sát in-game OK |

**Ký hiệu bảng:** `[x]` done · `[~]` một phần · `[ ]` chưa · **Cat** `y` = wire trong catalog · **Roll** = `wiz` / `phys` / `tap` / `pend`

**Loại đánh (H):** `melee` · `targeted` · `chebyshev` (vòng quanh player) · `forward_arc` · `corridor` (Lốc) · `burst` · `chebyshev*` = cần review geometry

**Wire:** `0x11` melee · `0x19` targeted · `0x1E` channel · `0xDB` burst

---

## Chuẩn bị

| Việc | Lệnh / ghi chú |
|------|----------------|
| Deploy game-host | `cd server-next && ./scripts/docker/docker-stack.sh --host-build --recreate --detach` |
| Account MG đầy đủ skill | `test` / `mg001` — `./scripts/db/verify-mg001-skills.sh` |
| Log combat | `docker compose logs -f game-host 2>&1 \\| grep '\\[m9\\]'` |
| Map test | Atlans / Lorencia — đông quái, đủ khoảng channel |

**Channel log:** `mode=0` Chebyshev · `mode=1` forward arc · `mode=2` corridor (Twister)

---

## Ma trận skill theo class

"""

QA_TAIL = """
---

## Backlog — 17 combat chưa wire (`stat_roll=pend`)

Cần implement wire + catalog trước khi tick **F**: 11 Power Wave · 60/66/74/78 DL · 73 Mana Rays · 76 Plasma Storm · 214–216/223–225/230 SU · 232 Frozen Stab · 235 Five Shot · 269 Charge RF.

---

## MG QA tiến độ (`test` / `mg001`)

**Seed:** 30 combat · slot 1–30 · `./scripts/db/reset-mg001-skills.sh`

| Trạng thái | ID | Tên |
|------------|-----|-----|
| ✅ Done (user 2026-05-21) | 9 | Linh hồn (Evil Spirit) |
| ✅ Done | 8 | Lốc xoáy (Twister) |
| ✅ Done | 55 | Chém lửa (Fire Slash) |
| ✅ Done | 237 | Bão điện (Lightning Storm) |
| ⬜ Tiếp theo | 56 | Power Slash |
| ⬜ | 236 | Flame Strike |
| ⬜ | 10, 13, 14 | Hell Fire / Blast / Inferno |
| ⬜ | 3–5, 12 | Lightning / Fire Ball / Flame / Aqua Beam |
| ⬜ | 47 | Impale |
| ⬜ | 19–23, 41, 57 | melee `0x11` |

**Pass cột QA:** tick khi **F+R+H+W** + in-game OK (không chỉ build).

---

## MG — test nhanh (account mg001)

| ID | Tên | Hit volume | QA | Ghi chú |
|----|-----|------------|-----|---------|
| 9 | Linh hồn | `chebyshev` `mode=0` | ✅ | Done — reference |
| 8 | Lốc xoáy | `corridor` `mode=2` | ✅ | Done — hàng trên trục |
| 55 | Chém lửa | `forward_arc` | ✅ | Done — phys+skill |
| 237 | Bão điện | `chebyshev` `mode=0` | ✅ | Done — omnidirectional r6 |
| 56 | Power Slash | `forward_arc` | ⬜ | Tiếp theo |
| 236 | Flame Strike | `forward_arc` | ⬜ | |
| 10, 13 | Hell Fire / Blast | `forward_arc` | ⬜ | |
| 14 | Inferno | `chebyshev*` | ⬜ | Review geometry |

| ID | Wire | QA |
|----|------|-----|
| 3–5, 12 | `0x19` | ⬜ |
| 19–23, 41, 57 | `0x11` | ⬜ |

---

## Cập nhật tiến độ

1. Tick `[x]` trong bảng hoặc điền `x` / `p` vào cột `verify_*` trong CSV.
2. Hoặc thêm skill vào `PRE_VERIFY` trong `scripts/generate-skill-qa-checklist.py`, rồi regenerate.
3. Chạy: `python3 docs/android/scripts/generate-skill-qa-checklist.py`

*Không tick QA chỉ vì build pass — cần log `[m9]` + quan sát in-game.*
"""


def parse_row(line: str):
    m = re.match(r'^(\d+)\s+("([^"]+)"|[^\t]+)', line)
    if not m:
        return None
    sid = int(m.group(1))
    name = m.group(3) if m.group(3) else m.group(2).strip().strip('"')
    parts = line.split("\t")
    nums = [(i, p.strip()) for i, p in enumerate(parts) if i > 0 and re.fullmatch(r"\d+", p.strip())]
    dmg_val = nums[0][1] if nums else "0"
    rng = nums[3][1] if len(nums) > 3 else "0"
    radio = nums[4][1] if len(nums) > 4 else "0"
    flags = [int(v) for _, v in nums if int(v) <= 4][-7:]
    classes = [CLASS_LABELS[i] for i, v in enumerate(flags) if v > 0] if len(flags) == 7 else []
    return sid, name, dmg_val, rng, radio, "/".join(classes)


def wire(sid: int) -> str:
    if sid in AREA_CONTINUE:
        return "0x1E"
    if sid in TARGETED:
        return "0x19"
    if sid in BURST:
        return "0xDB"
    if sid in PHYS:
        return "0x11"
    return "—"


def hit_volume(sid: int) -> str:
    if sid in CORRIDOR:
        return "corridor"
    if sid in DIRECTIONAL:
        return "forward_arc"
    if sid in AREA_CONTINUE and sid not in CORRIDOR and sid not in DIRECTIONAL:
        return "chebyshev" if sid in (9, 237, 385, 487) else "chebyshev*"
    if sid in TARGETED:
        return "targeted"
    if sid in BURST:
        return "burst"
    if sid in PHYS:
        return "melee"
    return "—"


def stat_roll_kind(sid: int, wv: str, rol: str) -> str:
    if rol != "combat":
        return "n/a"
    if wv == "—" or sid in PENDING_WIRE_COMBAT:
        return "pend"
    if sid in PHYS_STAT and wv in ("0x1E", "0x19", "0xDB"):
        return "phys"
    if wv in ("0x1E", "0x19", "0xDB"):
        return "wiz"
    if wv == "0x11":
        return "tap"
    return "pend"


def server_range(sid: int, rng: str, radio: str) -> str:
    if sid in RANGE_OVERRIDE:
        return str(RANGE_OVERRIDE[sid])
    if sid in CORRIDOR or sid in DIRECTIONAL or sid in AREA_CONTINUE:
        if 61 <= sid <= 65 or 14 <= sid <= 18:
            return "6*"
        return radio if radio != "0" else (rng if rng != "0" else "3")
    if sid in TARGETED:
        return "8"
    if sid in BURST:
        return "6"
    if sid in PHYS:
        return str(MELEE_RANGE.get(sid, 3))
    return rng if rng != "0" else radio


def role(sid: int, dmg_val: str, wv: str) -> str:
    if sid >= 300:
        return "master"
    if wv in ("0x1E", "0x19", "0xDB", "0x11"):
        return "combat"
    if dmg_val == "0":
        return "support"
    return "combat"


def cb(v: str) -> str:
    if v == "x":
        return "[x]"
    if v == "p":
        return "[~]"
    return "[ ]"


def load_rows():
    lines = SKILL_TXT.read_text(encoding="utf-8", errors="replace").splitlines()
    rows = []
    for line in lines:
        r = parse_row(line)
        if not r:
            continue
        sid, name, dmg, rng, radio, cls = r
        if not cls or sid >= 500:
            continue
        wv = wire(sid)
        rows.append((sid, name, cls, role(sid, dmg, wv), wv, hit_volume(sid), server_range(sid, rng, radio), rng, radio))
    rows.sort(key=lambda r: (r[2], r[0]))
    return rows


def write_csv(rows, out_csv: Path):
    with out_csv.open("w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow(CSV_HEADER)
        for sid, name, cls, rol, wv, hv, sr, rng, radio in rows:
            roll = stat_roll_kind(sid, wv, rol)
            df = roll if roll != "n/a" else "—"
            cat = "y" if wv != "—" else "n"
            notes = {8: "mode=2 corridor", 9: "ref Evil Spirit"}.get(sid, "")
            if hv == "chebyshev*":
                notes = (notes + " review geom").strip()
            p = PRE_VERIFY.get(sid, {})
            if p.get("notes"):
                notes = (notes + " " + p["notes"]).strip() if notes else p["notes"]
            w.writerow(
                [
                    sid,
                    name,
                    cls,
                    rol,
                    wv,
                    hv,
                    sr,
                    rng,
                    radio,
                    df,
                    roll,
                    cat,
                    p.get("verify_formula", ""),
                    p.get("verify_range", ""),
                    p.get("verify_hit_volume", ""),
                    p.get("verify_wire_cast", ""),
                    p.get("verify_vfx_speed", ""),
                    p.get("verify_char_anim", ""),
                    p.get("verify_world_vfx", ""),
                    p.get("verify_qa_ingame", ""),
                    notes,
                ]
            )


def write_md_sections(reader_rows, out_md: Path):
    by: dict[str, list] = defaultdict(list)
    for r in reader_rows:
        for c in r["class"].split("/"):
            by[c].append(r)

    order = ["DW", "DK", "FE", "MG", "DL", "SU", "RF"]
    md_lines: list[str] = []
    for c in order:
        cr = sorted({r["skill_id"]: r for r in by[c]}.values(), key=lambda x: int(x["skill_id"]))
        md_lines.append(f"### {c} — {CLASS_NAMES[c]}\n")
        if c == "MG":
            done_n = len(MG_QA_DONE_IDS)
            done_order = [9, 8, 55, 237]
            done_txt = " · ".join(
                f"{sid} {MG_QA_DONE_LABELS[sid]}"
                for sid in done_order
                if sid in MG_QA_DONE_IDS
            )
            md_lines.append(
                f"\n**QA mg001:** ✅ **{done_n}/30** combat done "
                f"({done_txt}). "
                f"Tiếp: {', '.join(str(i) for i in MG_QA_NEXT_IDS[:6])}…\n"
            )
        combat = [x for x in cr if x["role"] == "combat"]
        support_n = sum(1 for x in cr if x["role"] == "support")
        master_n = sum(1 for x in cr if x["role"] == "master")
        md_lines.append(f"*{len(combat)} combat · {support_n} support · {master_n} master*\n")
        md_lines.append(
            "| ID | Tên skill | NV | Wire | Loại đánh | Range SV | Roll | Cat | F | R | H | W | S | A | V | QA |"
        )
        md_lines.append(
            "|----|-----------|-----|------|-----------|----------|------|-----|---|---|---|---|---|---|---|---|"
        )
        for r in combat:
            md_lines.append(
                f"| {r['skill_id']} | {r['skill_name']} | {r['class']} | {r['wire']} | {r['hit_volume']} | "
                f"{r['server_range']} | {r['stat_roll']} | {r['catalog_in_code']} | {cb(r['verify_formula'])} | {cb(r['verify_range'])} | "
                f"{cb(r['verify_hit_volume'])} | {cb(r['verify_wire_cast'])} | {cb(r['verify_vfx_speed'])} | "
                f"{cb(r['verify_char_anim'])} | {cb(r['verify_world_vfx'])} | {cb(r['verify_qa_ingame'])} |"
            )
        md_lines.append("")
        sup = [x for x in cr if x["role"] != "combat"]
        if sup:
            md_lines.append(f"<details><summary>Support / master ({len(sup)} skill)</summary>\n\n")
            md_lines.append("| ID | Tên | Role | Cat | QA |\n|----|-----|------|-----|-----|\n")
            for r in sorted(sup, key=lambda x: int(x["skill_id"]))[:40]:
                md_lines.append(
                    f"| {r['skill_id']} | {r['skill_name']} | {r['role']} | {r['catalog_in_code']} | [ ] |"
                )
            if len(sup) > 40:
                md_lines.append(f"| … | +{len(sup) - 40} | | | [ ] |")
            md_lines.append("\n</details>\n")

    out_md.write_text("\n".join(md_lines), encoding="utf-8")


def merge_checklist(sections_path: Path, out_checklist: Path) -> None:
    parts = [QA_INTRO, sections_path.read_text(encoding="utf-8"), QA_TAIL]
    out_checklist.write_text("".join(parts), encoding="utf-8")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--csv", type=Path, default=OUT_CSV)
    parser.add_argument("--sections", type=Path, default=OUT_MD_SECTIONS)
    parser.add_argument("--checklist", type=Path, default=OUT_CHECKLIST)
    parser.add_argument("--no-merge-checklist", action="store_true")
    args = parser.parse_args()

    if not SKILL_TXT.is_file():
        print(f"Missing {SKILL_TXT}", file=sys.stderr)
        sys.exit(1)

    rows = load_rows()
    write_csv(rows, args.csv)
    reader = list(csv.DictReader(args.csv.open(encoding="utf-8")))
    write_md_sections(reader, args.sections)

    combat = sum(1 for r in reader if r["role"] == "combat")
    cat = sum(1 for r in reader if r["catalog_in_code"] == "y")
    from collections import Counter

    rolls = Counter(r["stat_roll"] for r in reader if r["role"] == "combat")
    print(f"Wrote {args.csv} ({len(reader)} rows, {combat} combat, {cat} in catalog)")
    print(f"  stat_roll combat: {dict(rolls)}")
    print(f"Wrote {args.sections}")
    if not args.no_merge_checklist:
        merge_checklist(args.sections, args.checklist)
        print(f"Wrote {args.checklist}")


if __name__ == "__main__":
    main()
