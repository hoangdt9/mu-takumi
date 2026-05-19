#!/usr/bin/env python3
"""Compare MonsterSetBase section-1 density across Takumi + reference MuServer trees."""
from __future__ import annotations

import sys
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
sys.path.insert(0, str(SCRIPT_DIR))

from spawn_reference_paths import (  # noqa: E402
    default_pegasus_set_base,
    default_set_base,
    default_thangcuoi_set_base,
)
from spawn_setbase_io import field_qty_by_map, parse_section1  # noqa: E402

# Client-only repos (no GS MonsterSetBase) — listed for operators.
CLIENT_ONLY = (
    "MuMain-5.2",
    "muonline-xulek",
    "muonline-bernat-main",
    "muonline-bmd-viewer",
)


def main() -> int:
    takumi = default_set_base()
    sources: list[tuple[str, Path | None]] = [
        ("takumi", takumi),
        ("openmu_s6", None),
        ("pegasus52", default_pegasus_set_base()),
        ("thangcuoi", default_thangcuoi_set_base()),
    ]

    qty: dict[str, dict[int, int]] = {}
    for name, path in sources:
        if name == "openmu_s6":
            continue
        if path is None or not path.is_file():
            qty[name] = {}
            continue
        qty[name] = field_qty_by_map(parse_section1(path))

    all_maps = sorted(set().union(*[set(d) for d in qty.values()]))
    cols = [n for n, _ in sources if n != "openmu_s6"]

    print("[report-spawn-sources] MonsterSetBase section-1 quantity sum per map")
    print(f"  takumi:    {takumi}")
    peg = default_pegasus_set_base()
    th = default_thangcuoi_set_base()
    print(f"  pegasus:   {peg or '(missing)'}")
    print(f"  thangcuoi: {th or '(missing)'}")
    print(f"  openmu:    primary via sync-all-spawns-from-openmu.py (SeasonSix/095d/075)")
    print(f"  client-only (no spawn file): {', '.join(CLIENT_ONLY)}")
    print()
    hdr = f"{'map':>4}" + "".join(f"{c:>12}" for c in cols) + f"{'best':>12}  note"
    print(hdr)
    for m in all_maps:
        vals = {c: qty.get(c, {}).get(m, 0) for c in cols}
        best = max(vals.values()) if vals else 0
        cur = vals.get("takumi", 0)
        note = ""
        if cur == 0 and best > 0:
            note = "TAKUMI EMPTY"
        elif best > 0 and cur < best * 0.8:
            note = "TAKUMI LOW"
        elif cur >= best and best > 0:
            note = "ok"
        if best > 0 or cur > 0:
            line = f"{m:4d}" + "".join(f"{vals[c]:12d}" for c in cols) + f"{best:12d}  {note}"
            print(line)

    print()
    for c in cols:
        d = qty.get(c, {})
        print(
            f"  {c}: {len([m for m, v in d.items() if v > 0])} maps, "
            f"{sum(d.values())} total field qty"
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
