#!/usr/bin/env python3
"""Sync MonsterSetBase section-1 field spawns from OpenMU SeasonSix (+ 095d/075 bases).

Replaces per-map section-1 rows for maps below OpenMU quantity threshold, then writes
// [TAKUMI-OPENMU-S6-BEGIN] … END with OpenMU-derived spot blocks.

Usage:
  python3 ./scripts/sync-all-spawns-from-openmu.py [--dry-run] [--min-ratio 0.5]
  python3 ./scripts/sync-all-spawns-from-openmu.py --min-ratio 0   # fill only empty maps
"""
from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

BEGIN = "// [TAKUMI-OPENMU-S6-BEGIN] OpenMU SeasonSix field spawns"
END = "// [TAKUMI-OPENMU-S6-END]"
SPAWN_RE = re.compile(
    r"CreateMonsterSpawn\(\s*\d+\s*,\s*this\.NpcDictionary\[(\d+)\]\s*,\s*(\d+)\s*,\s*(\d+)",
)
MAP_NUM_RE = re.compile(r"internal const byte Number = (\d+);")
CLASS_RE = re.compile(r"internal class (\w+)")
INHERIT_RE = re.compile(r":\s*(?:Version\d+\w*\.Maps\.)?(\w+)\s*$", re.M)


def openmu_roots(openmu: Path) -> list[Path]:
    init = openmu / "src/Persistence/Initialization"
    return [
        init / "VersionSeasonSix/Maps",
        init / "Version095d/Maps",
        init / "Version075/Maps",
    ]


def load_class_files(roots: list[Path]) -> dict[str, Path]:
    out: dict[str, Path] = {}
    for root in roots:
        if not root.is_dir():
            continue
        for cs in root.glob("*.cs"):
            out[cs.stem] = cs
    return out


def read_text(path: Path) -> str:
    return path.read_text(encoding="utf-8", errors="ignore")


def parse_method_spawns(text: str, method: str) -> list[tuple[int, int, int]]:
    block = re.search(
        rf"protected override IEnumerable<MonsterSpawnArea> {method}\(\)\s*\{{(.*?)\n\s*\}}",
        text,
        re.S,
    )
    if not block:
        return []
    body = block.group(1)
    if "no monsters" in body.lower():
        return []
    return [(int(a), int(b), int(c)) for a, b, c in SPAWN_RE.findall(body)]


def resolve_field_spawns(class_name: str, files: dict[str, Path], seen: set[str]) -> list[tuple[int, int, int]]:
    if class_name in seen or class_name not in files:
        return []
    seen.add(class_name)
    text = read_text(files[class_name])
    rows: list[tuple[int, int, int]] = []
    inh = INHERIT_RE.search(text)
    if inh:
        rows.extend(resolve_field_spawns(inh.group(1), files, seen))
    rows.extend(parse_method_spawns(text, "CreateMonsterSpawns"))
    return rows


def map_entries(files: dict[str, Path]) -> dict[int, tuple[str, list[tuple[int, int, int]]]]:
    by_map: dict[int, tuple[str, list[tuple[int, int, int]]]] = {}
    for path in files.values():
        text = read_text(path)
        m = MAP_NUM_RE.search(text)
        c = CLASS_RE.search(text)
        if not m or not c:
            continue
        mid = int(m.group(1))
        rows = resolve_field_spawns(c.group(1), files, set())
        if mid not in by_map or len(rows) > len(by_map[mid][1]):
            by_map[mid] = (c.group(1), rows)
    return by_map


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


def strip_section1_for_maps(lines: list[str], maps: set[int]) -> list[str]:
    section = -1
    out: list[str] = []
    for line in lines:
        s = line.split("//")[0].strip()
        if s.lower() == "end":
            section = -1
            out.append(line)
            continue
        if re.fullmatch(r"\d+", s):
            section = int(s)
            out.append(line)
            continue
        if section == 1 and s:
            p = s.split()
            if len(p) >= 9 and int(p[1]) in maps:
                continue
        out.append(line)
    return out


def format_block(map_id: int, title: str, rows: list[tuple[int, int, int]]) -> str:
    lines = [
        "1",
        "//=================================",
        f"//SPOT - {title} (OpenMU S6 sync)",
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


def merge_block(set_base: Path, insert: str) -> None:
    text = set_base.read_text(encoding="utf-8", errors="ignore")
    if BEGIN in text:
        pre, rest = text.split(BEGIN, 1)
        _, post = rest.split(END, 1)
        text = pre.rstrip() + "\n\n" + insert + post.lstrip("\n")
    else:
        anchor = "// MONSTERS"
        idx = text.find(anchor)
        text = (
            (text[:idx].rstrip() + "\n\n" + insert + "\n" + text[idx:])
            if idx >= 0
            else text.rstrip() + "\n\n" + insert
        )
    set_base.write_text(text, encoding="utf-8")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument(
        "--min-ratio",
        type=float,
        default=0.5,
        help="Replace map section-1 when existing_qty < openmu_spots * ratio (0 = only empty maps)",
    )
    ap.add_argument("set_base", nargs="?", default=None)
    ap.add_argument("openmu", nargs="?", default=None)
    args = ap.parse_args()

    root = Path(__file__).resolve().parents[1]
    set_base = Path(args.set_base or root.parent / "MuServer/4.GameServer/Data/Monster/MonsterSetBase.txt")
    openmu = Path(args.openmu or root.parent.parent / "OpenMU")
    if not set_base.is_file():
        print(f"missing MonsterSetBase: {set_base}", file=sys.stderr)
        return 1
    if not openmu.is_dir():
        print(f"missing OpenMU repo: {openmu}", file=sys.stderr)
        return 1

    files = load_class_files(openmu_roots(openmu))
    openmu_maps = map_entries(files)
    existing = field_qty_by_map(set_base)

    replace_maps: set[int] = set()
    chunks = [BEGIN, ""]
    for mid in sorted(openmu_maps):
        title, rows = openmu_maps[mid]
        if not rows:
            continue
        openmu_qty = len(rows) * 5
        cur = existing.get(mid, 0)
        threshold = openmu_qty * args.min_ratio
        if cur >= threshold and args.min_ratio > 0:
            continue
        replace_maps.add(mid)
        chunks.append(format_block(mid, title, rows))
        print(
            f"[sync-openmu] map {mid:3} {title:28} spots={len(rows):4} "
            f"was_qty={cur:5} -> openmu_qty~{openmu_qty}"
        )

    chunks.append(END)
    chunks.append("")
    insert = "\n".join(chunks)

    if not replace_maps:
        print("[sync-openmu] nothing to change (all maps meet min-ratio vs OpenMU)")
        return 0

    if args.dry_run:
        print(f"[sync-openmu] dry-run: would replace section-1 for {len(replace_maps)} map(s)")
        return 0

    lines = strip_section1_for_maps(
        set_base.read_text(encoding="utf-8", errors="ignore").splitlines(keepends=True),
        replace_maps,
    )
    set_base.write_text("".join(lines), encoding="utf-8")
    merge_block(set_base, insert)
    print(f"[sync-openmu] updated {set_base} ({len(replace_maps)} maps)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
