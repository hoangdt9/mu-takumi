# M8 — QA warp Move map + monster viewport

Last updated: 2026-05-19

**Mục tiêu:** Mỗi đích **Move map (`0x8E`)** có NPC/quái hợp lệ; log `[m9]` / `[m8-m9]` khớp kỳ vọng.

## Chuẩn bị

```bash
cd server-next
export TAKUMI_GAMESERVER_DATA_HOST="../MuServer/4.GameServer/Data"
docker compose up -d --build game-host
docker compose logs game-host | grep -E '\[m9\]|\[m8-m9\]'
./scripts/spawn/report-monster-spawn-coverage.sh
```

Kỳ vọng startup:

- `[m9] MapMonsterWorld ready: … instances on … maps`
- `[m8-m9] monster spawn coverage: …`
- Không `WARN move index … -> map …: no spawns` cho map đã có data (trừ map chỉ NPC theo thiết kế — xem bảng dưới).

## Bảng smoke (APK / client thật)

| Move / map | Map ID | Kỳ vọng client | Log server |
|------------|--------|----------------|------------|
| Lorencia | 0 | Quái ngoài town (Bull Fighter, …) | `C2 0x13` join `map=0` count>0 |
| Devias | 2 | Yeti / Ice Queen ngoài safe zone | `map=2` field>0; không chỉ NPC town |
| Noria | 3 | Goblin / Forest Monster spots | `map=3` (đã verify) |
| Dungeon | 1 | Quái trong dungeon (gate 20/30/40) | `map=1` section 1 enabled (~75 mobs) |
| Atlans | 7 | Bahamut / Vepar / Hydra… | `map=7` ~170 |
| Lost Tower | 4 | Death Knight / Gorgon… | `map=4` ~295 |
| Tarkan | 8 | Bloody wolf / Zaikan… | `map=8` ~133 |
| Icarus | 10 | Drakan / Alquamos (gate/wing) | `map=10` ~103 |
| Aida | 33 | Forest Orc (warp tùy season) | `map=33` ~135 |
| Loren Market | 79 | **Chỉ NPC** (OpenMU: no monsters) | `C2 0x13` NPC join; có thể `no new monsters` khi đi xa NPC |

**Cách test:** Mở Move map → warp từng dòng đủ level → đi ra khỏi spawn gate ~15–30 ô → xác nhận mob/NPC + đánh thử 1 con (Noria/Devias).

```bash
docker compose logs -f game-host | grep '\[m9\]'
```

## Pass criteria

- [ ] Warp không disconnect / không kẹt loading
- [ ] Ít nhất một lần `sent C2 0x13 monster viewport (join)` với `map=` đúng
- [ ] Devias: thấy field mob (section 1), không chỉ NPC town
- [ ] Loren Market: thấy 3 NPC (545–547), không bắt buộc field mob
- [ ] `./scripts/spawn/report-monster-spawn-coverage.sh` pass

## Drift OpenMU / season khác

```bash
./scripts/spawn/compare-spawn-openmu.sh
```

So sánh số spot section 1 theo map với **OpenMU SeasonSix** (`OpenMU/src/Persistence/Initialization/VersionSeasonSix/Maps`). Repo **muonline-xulek** (nếu có): diff `MonsterSetBase.txt` thủ công — season custom thường lệch map 51+, invasion section 3.

## Liên kết

- [`M8-MAP-MONSTER-SPAWN-TASKS.md`](./world/M8-MAP-MONSTER-SPAWN-TASKS.md)
- [`M8-MOVE-MAP-PARITY-CHECKLIST.md`](./world/M8-MOVE-MAP-PARITY-CHECKLIST.md)
