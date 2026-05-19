#!/usr/bin/env python3
"""Upgrade Takumi MonsterSetBase section-1 from denser MuServer references (Pegasus / ThangCuoi).

OpenMU SeasonSix remains the baseline (sync-all-spawns-from-openmu.py). This script fills maps
where a legacy MuServer tree has higher field quantity — typical for Lorencia/Devias/Noria boxes.

Usage:
  python3 "$SCRIPTS_ROOT/spawn/merge-spawns-from-references.py" [--dry-run] [--min-boost 1.05]
"""
from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
sys.path.insert(0, str(SCRIPT_DIR))

from spawn_reference_paths import (  # noqa: E402
    default_gate_txt,
    default_move_txt,
    default_pegasus_set_base,
    default_set_base,
    default_thangcuoi_set_base,
)
from spawn_setbase_io import (  # noqa: E402
    BEGIN_REF,
    END_REF,
    field_qty_by_map,
    format_block,
    merge_marker_block,
    parse_section1,
    rows_for_map,
    strip_section1_for_maps,
)

# No static field mobs via Move / by design (events, siege, devil square lobby).
SKIP_MAPS = frozenset({9, 30, 64, 79})


def move_destination_maps(move_path: Path, gate_path: Path) -> set[int]:
    gates: dict[int, int] = {}
    if gate_path.is_file():
        for line in gate_path.read_text(encoding="utf-8", errors="ignore").splitlines():
            s = line.split("//")[0].strip()
            if not s:
                continue
            p = s.split()
            try:
                gates[int(p[0])] = int(p[1])
            except (ValueError, IndexError):
                continue
    out: set[int] = set()
    if not move_path.is_file():
        return out
    for line in move_path.read_text(encoding="utf-8", errors="ignore").splitlines():
        s = line.split("//")[0].strip()
        if not s:
            continue
        p = s.split()
        try:
            gate = int(p[-1])
            out.add(gates[gate])
        except (ValueError, IndexError):
            continue
    return out


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument(
        "--min-boost",
        type=float,
        default=1.05,
        help="Replace map when best_ref_qty >= takumi_qty * min_boost (default 1.05)",
    )
    ap.add_argument(
        "--move-only",
        action="store_true",
        help="Only upgrade maps that appear as Move.txt gate destinations",
    )
    args = ap.parse_args()

    set_base = default_set_base()
    if not set_base.is_file():
        print(f"missing MonsterSetBase: {set_base}", file=sys.stderr)
        return 1

    refs: list[tuple[str, Path]] = []
    for label, path in (
        ("pegasus52", default_pegasus_set_base()),
        ("thangcuoi", default_thangcuoi_set_base()),
    ):
        if path is None:
            print(f"[merge-ref] skip {label}: file not found", file=sys.stderr)
            continue
        refs.append((label, path))

    if not refs:
        print("[merge-ref] no reference MonsterSetBase files found", file=sys.stderr)
        return 1

    takumi_rows = parse_section1(set_base)
    takumi_qty = field_qty_by_map(takumi_rows)
    ref_rows: dict[str, list] = {label: parse_section1(p) for label, p in refs}
    ref_qty: dict[str, dict[int, int]] = {
        label: field_qty_by_map(rows) for label, rows in ref_rows.items()
    }

    move_maps = move_destination_maps(default_move_txt(), default_gate_txt())
    candidate_maps = move_maps if args.move_only else set(takumi_qty) | move_maps
    candidate_maps -= SKIP_MAPS

    replace: dict[int, tuple[str, int, int]] = {}
    for map_id in sorted(candidate_maps):
        cur = takumi_qty.get(map_id, 0)
        best_label = ""
        best_q = cur
        for label, qmap in ref_qty.items():
            q = qmap.get(map_id, 0)
            if q > best_q:
                best_q = q
                best_label = label
        if not best_label:
            continue
        if cur > 0 and best_q < cur * args.min_boost:
            continue
        replace[map_id] = (best_label, cur, best_q)

    if not replace:
        print("[merge-ref] nothing to merge (Takumi already >= references on candidate maps)")
        return 0

    blocks: dict[int, str] = {}
    for map_id, (label, was, now) in replace.items():
        src_path = next(p for lbl, p in refs if lbl == label)
        rows = rows_for_map(ref_rows[label], map_id)
        title = f"Map {map_id} ({label} ref, was_qty={was} -> {now})"
        blocks[map_id] = format_block(map_id, title, rows)
        print(f"[merge-ref] map {map_id:3}: {label} qty {was} -> {now}")

    if args.dry_run:
        print(f"[merge-ref] dry-run: would update {len(replace)} map(s) in {set_base}")
        return 0

    text = set_base.read_text(encoding="utf-8", errors="ignore")
    lines = strip_section1_for_maps(text.splitlines(keepends=True), set(replace))
    text = "".join(lines)
    text = merge_marker_block(text, BEGIN_REF, END_REF, blocks)
    set_base.write_text(text, encoding="utf-8")
    print(f"[merge-ref] updated {set_base} ({len(replace)} maps)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
