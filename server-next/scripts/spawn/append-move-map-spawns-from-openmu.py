#!/usr/bin/env python3
"""Append section-1 field spawns for Move/warp maps missing from MonsterSetBase (OpenMU S6)."""
from __future__ import annotations

import re
import sys
from pathlib import Path

BEGIN = "// [TAKUMI-M8-BEGIN] Move-warp field spawns (OpenMU SeasonSix)"
END = "// [TAKUMI-M8-END]"

# OpenMU map file -> (map_id, title)
OPENMU_MAPS = {
    "CrywolfFortress.cs": (34, "Crywolf Fortress"),
    "SwampOfCalmness.cs": (56, "Swamp of Calmness"),
    "BalgassRefuge.cs": (42, "Balgass Refuge"),
    "Karutan1.cs": (80, "Karutan 1"),
    "Karutan2.cs": (81, "Karutan 2"),
}

SPAWN_LINE = re.compile(
    r"CreateMonsterSpawn\(\s*\d+\s*,\s*this\.NpcDictionary\[(\d+)\]\s*,\s*(\d+)\s*,\s*(\d+)",
)


def parse_openmu_spawns(cs_path: Path) -> list[tuple[int, int, int]]:
    text = cs_path.read_text(encoding="utf-8", errors="ignore")
    block = re.search(
        r"protected override IEnumerable<MonsterSpawnArea> CreateMonsterSpawns\(\)\s*\{(.*?)\n\s*\}",
        text,
        re.S,
    )
    if not block:
        return []
    rows: list[tuple[int, int, int]] = []
    for m in SPAWN_LINE.finditer(block.group(1)):
        rows.append((int(m.group(1)), int(m.group(2)), int(m.group(3))))
    return rows


def field_qty_by_map(set_base: Path) -> dict[int, int]:
    section = -1
    qty: dict[int, int] = {}
    for line in set_base.read_text(encoding="utf-8", errors="ignore").splitlines():
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
        q = int(p[8])
        qty[m] = qty.get(m, 0) + q
    return qty


def format_block(map_id: int, title: str, rows: list[tuple[int, int, int]]) -> str:
    lines = [
        "1",
        "//=================================",
        f"//SPOT - {title.upper()} (OpenMU S6, takumi M8)",
        "//=================================",
        "//Monster      MapNumber      Range      BeginPosX      BeginPosY      EndPosX      EndPosY      Direction      Quantity      Comment",
    ]
    for monster, x, y in rows:
        lines.append(
            f"{monster:3d}            {map_id:2d}             05         {x:3d}            {y:3d}            {x:3d}          {y:3d}          -1             5            //"
        )
    lines.append("end")
    lines.append("")
    return "\n".join(lines)


def build_insert(openmu_dir: Path, set_base: Path) -> str:
    existing = field_qty_by_map(set_base)
    chunks = [BEGIN, ""]
    for fname, (map_id, title) in OPENMU_MAPS.items():
        if existing.get(map_id, 0) > 0:
            continue
        cs = openmu_dir / fname
        if not cs.is_file():
            print(f"[append-move-map-spawns] skip map {map_id}: missing {cs}", file=sys.stderr)
            continue
        rows = parse_openmu_spawns(cs)
        if not rows:
            print(f"[append-move-map-spawns] skip map {map_id}: no CreateMonsterSpawns in {fname}", file=sys.stderr)
            continue
        chunks.append(format_block(map_id, title, rows))
        print(f"[append-move-map-spawns] map {map_id} ({title}): {len(rows)} spots, qty={len(rows) * 5}")
    chunks.append(END)
    chunks.append("")
    return "\n".join(chunks)


def merge(set_base: Path, insert: str) -> None:
    text = set_base.read_text(encoding="utf-8", errors="ignore")
    if BEGIN in text:
        pre, rest = text.split(BEGIN, 1)
        _, post = rest.split(END, 1)
        new_text = pre.rstrip() + "\n\n" + insert + post.lstrip("\n")
    else:
        anchor = "// MONSTERS"
        idx = text.find(anchor)
        if idx < 0:
            new_text = text.rstrip() + "\n\n" + insert
        else:
            new_text = text[:idx].rstrip() + "\n\n" + insert + "\n" + text[idx:]
    set_base.write_text(new_text, encoding="utf-8")


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    set_base = Path(
        sys.argv[1]
        if len(sys.argv) > 1
        else root.parent / "MuServer/4.GameServer/Data/Monster/MonsterSetBase.txt"
    )
    openmu_dir = Path(
        sys.argv[2]
        if len(sys.argv) > 2
        else root.parent.parent / "OpenMU/src/Persistence/Initialization/VersionSeasonSix/Maps"
    )
    if not set_base.is_file():
        print(f"missing MonsterSetBase: {set_base}", file=sys.stderr)
        return 1
    if not openmu_dir.is_dir():
        print(f"missing OpenMU maps dir: {openmu_dir}", file=sys.stderr)
        return 1
    insert = build_insert(openmu_dir, set_base)
    if insert.strip() == f"{BEGIN}\n\n{END}".strip():
        print("[append-move-map-spawns] nothing to add (all target maps already have section-1 field rows)")
        return 0
    merge(set_base, insert)
    print(f"[append-move-map-spawns] updated {set_base}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
