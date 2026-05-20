# M10 — PvP + AoE combat QA (2 clients)

**Server:** `TAKUMI_MAP_PRESENCE_ENABLED=1` (default on), `TAKUMI_COMBAT_PVP_ENABLED` not `0`.

## Env

```bash
# game-host / docker .env
TAKUMI_MAP_PRESENCE_ENABLED=1
TAKUMI_COMBAT_PVP_ENABLED=1
TAKUMI_COMBAT_AOE_RANGE=3
TAKUMI_COMBAT_SKILL_DAMAGE_PCT=150
```

Apply wallet SQL if testing coin shop / warehouse zen:

```bash
./scripts/db/apply-sql.sh "postgresql://takumi:takumi@127.0.0.1:54444/takumi_runtime"
```

## AoE (`C1 0xDB`)

1. Two accounts on **same map** (e.g. Noria).
2. Class with multi-target skill; cast on a cluster of monsters.
3. **Host log:** `[m9] magic aoe xy=(x,y) targets=N map=M`
4. **Client:** damage numbers on mobs in AoE tile range (not only primary target).

## PvP (skill / melee on player)

1. Account A and B on same map, within skill range.
2. A attacks B (target B in skill list or melee on player sprite).
3. **Victim client:** HP/SD drop; `C1 0x11` damage on self.
4. **Attacker / bystander client:** damage on B’s viewport key (presence `C2 0x12` key).
5. **Host log:** `[m10c] pvp hit victim=…` and `broadcast PvP damage packet peers=…`

## Disable PvP (sanity)

Set `TAKUMI_COMBAT_PVP_ENABLED=0`, restart game-host — hits on players should not reduce HP; no `[m10c] pvp hit` lines.

## Pass criteria

| Case | Pass |
|------|------|
| AoE | Multiple mobs in range take damage from one cast |
| PvP victim | Victim sees damage + HP update |
| PvP observer | Second client on same map sees damage packet on target |
| PvP off | Env `0` blocks player damage |
