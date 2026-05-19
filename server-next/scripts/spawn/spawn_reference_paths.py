"""Default paths to MonsterSetBase reference trees (override via env)."""
from __future__ import annotations

import os
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]  # server-next
TAKUMI_ROOT = ROOT.parent


def _first_existing(*candidates: Path) -> Path | None:
    for c in candidates:
        if c.is_file():
            return c
    return None


def default_set_base() -> Path:
    env = os.environ.get("TAKUMI_MONSTER_SET_BASE_PATH", "").strip()
    if env:
        return Path(env)
    return TAKUMI_ROOT / "MuServer/4.GameServer/Data/Monster/MonsterSetBase.txt"


def default_openmu() -> Path | None:
    env = os.environ.get("OPENMU_MAPS_DIR", "").strip()
    if env:
        p = Path(env)
        return p if p.is_dir() else p.parent.parent.parent
    for c in (
        Path("/Users/hoangmac/Github/OpenMU"),
        TAKUMI_ROOT.parent.parent / "Github/OpenMU",
        TAKUMI_ROOT.parent / "OpenMU",
        ROOT / "../../OpenMU",
    ):
        if (c / "src/Persistence/Initialization").is_dir():
            return c
    return None


def default_pegasus_set_base() -> Path | None:
    env = os.environ.get("TAKUMI_REF_PEGASUS_SET_BASE", "").strip()
    if env:
        return Path(env)
    return _first_existing(
        Path("/Users/hoangmac/Project/MU/Source Pegasus 5.2/MuServer/Data/Monster/MonsterSetBase.txt"),
        TAKUMI_ROOT.parent / "Source Pegasus 5.2/MuServer/Data/Monster/MonsterSetBase.txt",
    )


def default_thangcuoi_set_base() -> Path | None:
    env = os.environ.get("TAKUMI_REF_THANGCUOI_SET_BASE", "").strip()
    if env:
        return Path(env)
    return _first_existing(
        Path("/Users/hoangmac/Project/MU/SRC ThangCuoi/Mu Server/4.Sub-1/Data/Monster/MonsterSetBase.txt"),
        TAKUMI_ROOT.parent / "SRC ThangCuoi/Mu Server/4.Sub-1/Data/Monster/MonsterSetBase.txt",
    )


def default_move_txt() -> Path:
    env = os.environ.get("TAKUMI_MOVE_PATH", "").strip()
    if env:
        return Path(env)
    return TAKUMI_ROOT / "MuServer/4.GameServer/Data/Move/Move.txt"


def default_gate_txt() -> Path:
    env = os.environ.get("TAKUMI_GATE_PATH", "").strip()
    if env:
        return Path(env)
    return TAKUMI_ROOT / "MuServer/4.GameServer/Data/Move/Gate.txt"
