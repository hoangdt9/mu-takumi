"""Parse / format MonsterSetBase.txt section-1 field rows (shared by spawn scripts)."""
from __future__ import annotations

import re
from dataclasses import dataclass
from pathlib import Path


@dataclass(frozen=True)
class SpawnRow:
    monster: int
    map_id: int
    dis: int
    x: int
    y: int
    tx: int
    ty: int
    dir: int
    qty: int
    comment: str = ""

    @property
    def qty_total(self) -> int:
        return self.qty


def parse_section1(path: Path) -> list[SpawnRow]:
    section = -1
    rows: list[SpawnRow] = []
    for line in path.read_text(encoding="utf-8", errors="ignore").splitlines():
        raw = line
        comment = ""
        if "//" in line:
            parts = line.split("//", 1)
            line = parts[0]
            comment = parts[1].strip()
        s = line.strip()
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
        if len(p) < 6:
            continue
        try:
            monster = int(p[0])
            map_id = int(p[1])
            dis = int(p[2])
            x = int(p[3])
            y = int(p[4])
        except ValueError:
            continue
        tx, ty = x, y
        idx = 5
        if len(p) >= 8:
            try:
                tx = int(p[5])
                ty = int(p[6])
                idx = 7
            except ValueError:
                idx = 5
        dir_ = 255
        qty = 1
        if idx < len(p):
            try:
                dir_ = int(p[idx])
            except ValueError:
                dir_ = 255
        if idx + 1 < len(p):
            try:
                qty = int(p[idx + 1])
            except ValueError:
                qty = 1
        rows.append(SpawnRow(monster, map_id, dis, x, y, tx, ty, dir_, qty, comment))
    return rows


def field_qty_by_map(rows: list[SpawnRow]) -> dict[int, int]:
    qty: dict[int, int] = {}
    for r in rows:
        qty[r.map_id] = qty.get(r.map_id, 0) + r.qty
    return qty


def rows_for_map(rows: list[SpawnRow], map_id: int) -> list[SpawnRow]:
    return [r for r in rows if r.map_id == map_id]


def format_row(r: SpawnRow) -> str:
    d = r.dir if r.dir >= 0 else -1
    tail = f" //{r.comment}" if r.comment else " //"
    return (
        f"{r.monster:3d}            {r.map_id:2d}             {r.dis:02d}         "
        f"{r.x:3d}            {r.y:3d}            {r.tx:3d}          {r.ty:3d}          "
        f"{d:3d}             {r.qty:3d}            {tail}"
    )


def format_block(map_id: int, title: str, rows: list[SpawnRow]) -> str:
    lines = [
        "1",
        "//=================================",
        f"//SPOT - {title}",
        "//=================================",
        "//Monster      MapNumber      Range      BeginPosX      BeginPosY      EndPosX      EndPosY      Direction      Quantity      Comment",
    ]
    for r in rows:
        lines.append(format_row(r))
    lines.append("end")
    lines.append("")
    return "\n".join(lines)


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
            if len(p) >= 2:
                try:
                    if int(p[1]) in maps:
                        continue
                except ValueError:
                    pass
        out.append(line)
    return out


BEGIN_REF = "// [TAKUMI-REF-SUPPLEMENT-BEGIN] MuServer reference merge (Pegasus / ThangCuoi)"
END_REF = "// [TAKUMI-REF-SUPPLEMENT-END]"


def extract_marker_blocks(text: str, begin: str, end: str) -> dict[int, str]:
    blocks: dict[int, str] = {}
    if begin not in text:
        return blocks
    _, rest = text.split(begin, 1)
    body, _ = rest.split(end, 1)
    for part in re.split(r"(?=^1\r?\n//=+)", body, flags=re.M):
        part = part.strip()
        if not part.startswith("1"):
            continue
        map_id = None
        for line in part.splitlines():
            s = line.split("//")[0].strip()
            if not s or s in ("1", "end") or s.startswith("//"):
                continue
            p = s.split()
            if len(p) >= 2:
                try:
                    map_id = int(p[1])
                    break
                except ValueError:
                    pass
        if map_id is not None:
            blocks[map_id] = part + "\n"
    return blocks


def merge_marker_block(
    text: str,
    begin: str,
    end: str,
    new_blocks: dict[int, str],
    anchor: str = "// MONSTERS",
) -> str:
    preserved: dict[int, str] = {}
    post = ""
    if begin in text:
        pre, rest = text.split(begin, 1)
        body, post = rest.split(end, 1)
        preserved = extract_marker_blocks(body, begin, end)
        for mid in new_blocks:
            preserved.pop(mid, None)
    else:
        pre = text
        idx = pre.find(anchor)
        if idx >= 0:
            post = pre[idx:]
            pre = pre[:idx]

    merged = {**preserved, **new_blocks}
    chunks = [begin, ""]
    for mid in sorted(merged):
        chunks.append(merged[mid].rstrip())
        chunks.append("")
    chunks.append(end)
    chunks.append("")
    return pre.rstrip() + "\n\n" + "\n".join(chunks) + post.lstrip("\n")
