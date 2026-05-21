# Skill QA checklist (286 skill)

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
| Log combat | `docker compose logs -f game-host 2>&1 \| grep '\[m9\]'` |
| Map test | Atlans / Lorencia — đông quái, đủ khoảng channel |

**Channel log:** `mode=0` Chebyshev · `mode=1` forward arc · `mode=2` corridor (Twister)

---

## Ma trận skill theo class

### DW — Dark Wizard

*21 combat · 11 support · 68 master*

| ID | Tên skill | NV | Wire | Loại đánh | Range SV | Roll | Cat | F | R | H | W | S | A | V | QA |
|----|-----------|-----|------|-----------|----------|------|-----|---|---|---|---|---|---|---|---|
| 1 | Poison | DW/MG | 0x19 | targeted | 8 | wiz | y | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 2 | Meteorite | DW/MG/SU | 0x19 | targeted | 8 | wiz | y | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 3 | Lightning | DW/MG | 0x19 | targeted | 6 | wiz | y | [x] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 4 | Fire Ball | DW/MG/SU | 0x19 | targeted | 6 | wiz | y | [x] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 5 | Flame | DW/MG | 0x19 | targeted | 6 | wiz | y | [x] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 7 | Ice | DW/MG/SU | 0x19 | targeted | 8 | wiz | y | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 8 | Twister | DW/MG | 0x1E | corridor | 6 | wiz | y | [x] | [x] | [x] | [x] | [x] | [x] | [x] | [x] |
| 9 | Evil Spirit | DW/MG/SU | 0x1E | chebyshev | 6 | wiz | y | [x] | [x] | [x] | [x] | [x] | [x] | [x] | [x] |
| 10 | Hell Fire | DW/MG | 0x1E | forward_arc | 5 | wiz | y | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 11 | Power Wave | DW/MG/SU | — | — | 6 | pend | n | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 12 | Aqua Beam | DW/MG | 0x19 | targeted | 7 | wiz | y | [x] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 13 | Blast | DW/MG | 0x1E | forward_arc | 5 | wiz | y | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 14 | Inferno | DW/MG | 0x1E | chebyshev* | 6* | wiz | y | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 15 | Teleport Party | DW | 0x1E | chebyshev* | 6* | wiz | y | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 16 | Mana Shield | DW | 0x1E | chebyshev* | 6* | wiz | y | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 17 | Energy Ball | DW/MG | 0x1E | chebyshev* | 6* | wiz | y | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 38 | Decay | DW | 0x19 | targeted | 8 | wiz | y | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 39 | Ice Storm | DW/MG | 0x19 | targeted | 8 | wiz | y | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 40 | Nova | DW | 0x19 | targeted | 8 | wiz | y | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 45 | Mana Glaive | DW/SU | 0x1E | chebyshev* | 6 | wiz | y | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 76 | Plasma Storm | DW/DK/FE/MG/DL/SU/RF | — | — | 5 | pend | n | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |

<details><summary>Support / master (79 skill)</summary>


| ID | Tên | Role | Cat | QA |
|----|-----|------|-----|-----|

| 6 | Teleport | support | n | [ ] |
| 67 | Stern | support | n | [ ] |
| 68 | Remove Stern | support | n | [ ] |
| 69 | Greater Mana | support | n | [ ] |
| 70 | Invisibility | support | n | [ ] |
| 71 | Remove Invisibility | support | n | [ ] |
| 210 | Order of Protection | support | n | [ ] |
| 211 | Order of Restraint | support | n | [ ] |
| 212 | Order of Tracking | support | n | [ ] |
| 213 | Order of Weaken | support | n | [ ] |
| 233 | Magic Circle | support | n | [ ] |
| 300 | Add Item Durability Rate | master | n | [ ] |
| 301 | Add Defense Success Rate PvP | master | n | [ ] |
| 302 | Add Max SD | master | n | [ ] |
| 303 | Add MP Recovery Rate | master | n | [ ] |
| 304 | Add Poison Resistance | master | n | [ ] |
| 305 | Add Item Durability Rate | master | n | [ ] |
| 306 | Add SD Recovery Rate | master | n | [ ] |
| 307 | Add HP Recovery Rate | master | n | [ ] |
| 308 | Add Lightning Resistance | master | n | [ ] |
| 309 | Add Defense | master | n | [ ] |
| 310 | Add BP Recovery Rate | master | n | [ ] |
| 311 | Add Ice Resistance | master | n | [ ] |
| 312 | Add Item Durability Rate | master | n | [ ] |
| 313 | Add Defense Success Rate | master | n | [ ] |
| 315 | Add Armor Set Bonus | master | n | [ ] |
| 316 | Add Reflect Damage | master | n | [ ] |
| 317 | Add Energy | master | n | [ ] |
| 318 | Add Vitality | master | n | [ ] |
| 319 | Add Agility | master | n | [ ] |
| 320 | Add Strength | master | n | [ ] |
| 325 | Add Attack Success Rate | master | n | [ ] |
| 334 | Add Max HP | master | n | [ ] |
| 338 | Add Max MP | master | n | [ ] |
| 341 | Add Max BP | master | n | [ ] |
| 347 | Add Attack Success Rate PvP | master | n | [ ] |
| 357 | Add MP Consumption Rate | master | n | [ ] |
| 358 | Add Hunt SD | master | n | [ ] |
| 359 | Add Hunt HP | master | n | [ ] |
| 362 | Add Hunt MP | master | n | [ ] |
| … | +39 | | | [ ] |

</details>

### DK — Dark Knight

*15 combat · 9 support · 69 master*

| ID | Tên skill | NV | Wire | Loại đánh | Range SV | Roll | Cat | F | R | H | W | S | A | V | QA |
|----|-----------|-----|------|-----------|----------|------|-----|---|---|---|---|---|---|---|---|
| 18 | Defense | DK/MG/DL/SU | 0x1E | chebyshev* | 6* | wiz | y | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 19 | Falling Slash | DK/MG/DL/SU/RF | 0x11 | melee | 3 | tap | y | [x] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 20 | Lunge | DK/MG/DL/SU | 0x11 | melee | 2 | tap | y | [x] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 21 | Uppercut | DK/MG/DL/SU | 0x11 | melee | 2 | tap | y | [x] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 22 | Cyclone | DK/MG/DL/SU | 0x11 | melee | 2 | tap | y | [x] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 23 | Slash | DK/MG/DL/SU | 0x11 | melee | 2 | tap | y | [x] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 41 | Twisting Slash | DK/MG | 0x11 | melee | 3 | tap | y | [x] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 42 | Rageful Blow | DK | 0x11 | melee | 4 | tap | y | [x] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 43 | Death Stab | DK | 0x1E | chebyshev* | 2 | wiz | y | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 44 | Crescent Moon Slash | DK | 0x1E | chebyshev* | 4 | phys | y | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 47 | Impale | DK/MG | 0x1E | chebyshev* | 6 | phys | y | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 48 | Greater Life | DK | 0x1E | forward_arc | 5 | phys | y | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 49 | Fire Breath | DK | 0x1E | forward_arc | 3 | phys | y | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 76 | Plasma Storm | DW/DK/FE/MG/DL/SU/RF | — | — | 5 | pend | n | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 232 | Frozen Stab | DK | — | — | 6 | pend | n | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |

<details><summary>Support / master (78 skill)</summary>


| ID | Tên | Role | Cat | QA |
|----|-----|------|-----|-----|

| 67 | Stern | support | n | [ ] |
| 68 | Remove Stern | support | n | [ ] |
| 69 | Greater Mana | support | n | [ ] |
| 70 | Invisibility | support | n | [ ] |
| 71 | Remove Invisibility | support | n | [ ] |
| 210 | Order of Protection | support | n | [ ] |
| 211 | Order of Restraint | support | n | [ ] |
| 212 | Order of Tracking | support | n | [ ] |
| 213 | Order of Weaken | support | n | [ ] |
| 300 | Add Item Durability Rate | master | n | [ ] |
| 301 | Add Defense Success Rate PvP | master | n | [ ] |
| 302 | Add Max SD | master | n | [ ] |
| 303 | Add MP Recovery Rate | master | n | [ ] |
| 304 | Add Poison Resistance | master | n | [ ] |
| 305 | Add Item Durability Rate | master | n | [ ] |
| 306 | Add SD Recovery Rate | master | n | [ ] |
| 307 | Add HP Recovery Rate | master | n | [ ] |
| 308 | Add Lightning Resistance | master | n | [ ] |
| 309 | Add Defense | master | n | [ ] |
| 310 | Add BP Recovery Rate | master | n | [ ] |
| 311 | Add Ice Resistance | master | n | [ ] |
| 312 | Add Item Durability Rate | master | n | [ ] |
| 313 | Add Defense Success Rate | master | n | [ ] |
| 315 | Add Armor Set Bonus | master | n | [ ] |
| 316 | Add Reflect Damage | master | n | [ ] |
| 317 | Add Energy | master | n | [ ] |
| 318 | Add Vitality | master | n | [ ] |
| 319 | Add Agility | master | n | [ ] |
| 320 | Add Strength | master | n | [ ] |
| 322 | Add DK Wing Defense | master | n | [ ] |
| 324 | Add DK Wing Damage | master | n | [ ] |
| 325 | Add Attack Success Rate | master | n | [ ] |
| 326 | Add Cyclone Improved | master | n | [ ] |
| 327 | Add Slash Improved | master | n | [ ] |
| 328 | Add Falling Slash Improved | master | n | [ ] |
| 329 | Add Lunge Improved | master | n | [ ] |
| 330 | Add Twisting Slash Improved | master | n | [ ] |
| 331 | Add Rageful Blow Improved | master | n | [ ] |
| 332 | Add Twisting Slash Enhanced | master | n | [ ] |
| 333 | Add Rageful Blow Enhanced | master | n | [ ] |
| … | +38 | | | [ ] |

</details>

### FE — Fairy Elf

*5 combat · 22 support · 70 master*

| ID | Tên skill | NV | Wire | Loại đánh | Range SV | Roll | Cat | F | R | H | W | S | A | V | QA |
|----|-----------|-----|------|-----------|----------|------|-----|---|---|---|---|---|---|---|---|
| 46 | Starfall | FE | 0x1E | chebyshev* | 8 | wiz | y | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 51 | Ice Arrow | FE | 0x1E | forward_arc | 8 | phys | y | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 52 | Penetration | FE | 0x1E | forward_arc | 6 | phys | y | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 76 | Plasma Storm | DW/DK/FE/MG/DL/SU/RF | — | — | 5 | pend | n | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 235 | Five Shot | FE | — | — | 6 | pend | n | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |

<details><summary>Support / master (92 skill)</summary>


| ID | Tên | Role | Cat | QA |
|----|-----|------|-----|-----|

| 24 | Triple Shot | support | n | [ ] |
| 26 | Heal | support | n | [ ] |
| 27 | Greater Defense | support | n | [ ] |
| 28 | Greater Damage | support | n | [ ] |
| 30 | Summon Goblin | support | n | [ ] |
| 31 | Summon Stone Golem | support | n | [ ] |
| 32 | Summon Assassin | support | n | [ ] |
| 33 | Summon Elite Yeti | support | n | [ ] |
| 34 | Summon Dark Knight | support | n | [ ] |
| 35 | Summon Bali | support | n | [ ] |
| 36 | Summon Soldier | support | n | [ ] |
| 67 | Stern | support | n | [ ] |
| 68 | Remove Stern | support | n | [ ] |
| 69 | Greater Mana | support | n | [ ] |
| 70 | Invisibility | support | n | [ ] |
| 71 | Remove Invisibility | support | n | [ ] |
| 77 | Infinity Arrow | support | n | [ ] |
| 210 | Order of Protection | support | n | [ ] |
| 211 | Order of Restraint | support | n | [ ] |
| 212 | Order of Tracking | support | n | [ ] |
| 213 | Order of Weaken | support | n | [ ] |
| 234 | Shield Recover | support | n | [ ] |
| 300 | Add Item Durability Rate | master | n | [ ] |
| 301 | Add Defense Success Rate PvP | master | n | [ ] |
| 302 | Add Max SD | master | n | [ ] |
| 303 | Add MP Recovery Rate | master | n | [ ] |
| 304 | Add Poison Resistance | master | n | [ ] |
| 305 | Add Item Durability Rate | master | n | [ ] |
| 306 | Add SD Recovery Rate | master | n | [ ] |
| 307 | Add HP Recovery Rate | master | n | [ ] |
| 308 | Add Lightning Resistance | master | n | [ ] |
| 309 | Add Defense | master | n | [ ] |
| 310 | Add BP Recovery Rate | master | n | [ ] |
| 311 | Add Ice Resistance | master | n | [ ] |
| 312 | Add Item Durability Rate | master | n | [ ] |
| 313 | Add Defense Success Rate | master | n | [ ] |
| 315 | Add Armor Set Bonus | master | n | [ ] |
| 316 | Add Reflect Damage | master | n | [ ] |
| 317 | Add Energy | master | n | [ ] |
| 318 | Add Vitality | master | n | [ ] |
| … | +52 | | | [ ] |

</details>

### MG — Magic Gladiator


**QA mg001:** ✅ **4/30** combat done (9 Linh hồn · 8 Lốc · 55 Chém lửa · 237 Bão điện). Tiếp: 56, 236, 10, 13, 14, 3…

*30 combat · 9 support · 71 master*

| ID | Tên skill | NV | Wire | Loại đánh | Range SV | Roll | Cat | F | R | H | W | S | A | V | QA |
|----|-----------|-----|------|-----------|----------|------|-----|---|---|---|---|---|---|---|---|
| 1 | Poison | DW/MG | 0x19 | targeted | 8 | wiz | y | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 2 | Meteorite | DW/MG/SU | 0x19 | targeted | 8 | wiz | y | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 3 | Lightning | DW/MG | 0x19 | targeted | 6 | wiz | y | [x] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 4 | Fire Ball | DW/MG/SU | 0x19 | targeted | 6 | wiz | y | [x] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 5 | Flame | DW/MG | 0x19 | targeted | 6 | wiz | y | [x] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 7 | Ice | DW/MG/SU | 0x19 | targeted | 8 | wiz | y | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 8 | Twister | DW/MG | 0x1E | corridor | 6 | wiz | y | [x] | [x] | [x] | [x] | [x] | [x] | [x] | [x] |
| 9 | Evil Spirit | DW/MG/SU | 0x1E | chebyshev | 6 | wiz | y | [x] | [x] | [x] | [x] | [x] | [x] | [x] | [x] |
| 10 | Hell Fire | DW/MG | 0x1E | forward_arc | 5 | wiz | y | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 11 | Power Wave | DW/MG/SU | — | — | 6 | pend | n | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 12 | Aqua Beam | DW/MG | 0x19 | targeted | 7 | wiz | y | [x] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 13 | Blast | DW/MG | 0x1E | forward_arc | 5 | wiz | y | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 14 | Inferno | DW/MG | 0x1E | chebyshev* | 6* | wiz | y | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 17 | Energy Ball | DW/MG | 0x1E | chebyshev* | 6* | wiz | y | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 18 | Defense | DK/MG/DL/SU | 0x1E | chebyshev* | 6* | wiz | y | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 19 | Falling Slash | DK/MG/DL/SU/RF | 0x11 | melee | 3 | tap | y | [x] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 20 | Lunge | DK/MG/DL/SU | 0x11 | melee | 2 | tap | y | [x] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 21 | Uppercut | DK/MG/DL/SU | 0x11 | melee | 2 | tap | y | [x] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 22 | Cyclone | DK/MG/DL/SU | 0x11 | melee | 2 | tap | y | [x] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 23 | Slash | DK/MG/DL/SU | 0x11 | melee | 2 | tap | y | [x] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 39 | Ice Storm | DW/MG | 0x19 | targeted | 8 | wiz | y | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 41 | Twisting Slash | DK/MG | 0x11 | melee | 3 | tap | y | [x] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 47 | Impale | DK/MG | 0x1E | chebyshev* | 6 | phys | y | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 55 | Fire Slash | MG | 0x1E | forward_arc | 2 | phys | y | [x] | [x] | [x] | [x] | [x] | [~] | [~] | [x] |
| 56 | Power Slash | MG | 0x1E | forward_arc | 2 | phys | y | [x] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 57 | Spiral Slash | MG | 0x11 | melee | 4 | tap | y | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 73 | Mana Rays | MG | — | — | 6 | pend | n | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 76 | Plasma Storm | DW/DK/FE/MG/DL/SU/RF | — | — | 5 | pend | n | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 236 | Sword Slash | MG | 0x1E | forward_arc | 2 | phys | y | [x] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 237 | Lightning Storm | MG | 0x1E | chebyshev | 6 | wiz | y | [x] | [x] | [x] | [x] | [x] | [x] | [x] | [x] |

<details><summary>Support / master (80 skill)</summary>


| ID | Tên | Role | Cat | QA |
|----|-----|------|-----|-----|

| 67 | Stern | support | n | [ ] |
| 68 | Remove Stern | support | n | [ ] |
| 69 | Greater Mana | support | n | [ ] |
| 70 | Invisibility | support | n | [ ] |
| 71 | Remove Invisibility | support | n | [ ] |
| 210 | Order of Protection | support | n | [ ] |
| 211 | Order of Restraint | support | n | [ ] |
| 212 | Order of Tracking | support | n | [ ] |
| 213 | Order of Weaken | support | n | [ ] |
| 300 | Add Item Durability Rate | master | n | [ ] |
| 301 | Add Defense Success Rate PvP | master | n | [ ] |
| 302 | Add Max SD | master | n | [ ] |
| 303 | Add MP Recovery Rate | master | n | [ ] |
| 304 | Add Poison Resistance | master | n | [ ] |
| 305 | Add Item Durability Rate | master | n | [ ] |
| 306 | Add SD Recovery Rate | master | n | [ ] |
| 307 | Add HP Recovery Rate | master | n | [ ] |
| 308 | Add Lightning Resistance | master | n | [ ] |
| 309 | Add Defense | master | n | [ ] |
| 310 | Add BP Recovery Rate | master | n | [ ] |
| 311 | Add Ice Resistance | master | n | [ ] |
| 312 | Add Item Durability Rate | master | n | [ ] |
| 313 | Add Defense Success Rate | master | n | [ ] |
| 315 | Add Armor Set Bonus | master | n | [ ] |
| 316 | Add Reflect Damage | master | n | [ ] |
| 317 | Add Energy | master | n | [ ] |
| 318 | Add Vitality | master | n | [ ] |
| 319 | Add Agility | master | n | [ ] |
| 320 | Add Strength | master | n | [ ] |
| 325 | Add Attack Success Rate | master | n | [ ] |
| 334 | Add Max HP | master | n | [ ] |
| 338 | Add Max MP | master | n | [ ] |
| 341 | Add Max BP | master | n | [ ] |
| 344 | Add Blood Storm | master | n | [ ] |
| 346 | Add Blood Storm Improved | master | n | [ ] |
| 347 | Add Attack Success Rate PvP | master | n | [ ] |
| 348 | Add Two Hand Sword Damage | master | n | [ ] |
| 349 | Add One Hand Sword Damage | master | n | [ ] |
| 352 | Add Two Hand Sword Mastery | master | n | [ ] |
| 353 | Add One Hand Sword Mastery | master | n | [ ] |
| … | +40 | | | [ ] |

</details>

### DL — Dark Lord

*17 combat · 12 support · 38 master*

| ID | Tên skill | NV | Wire | Loại đánh | Range SV | Roll | Cat | F | R | H | W | S | A | V | QA |
|----|-----------|-----|------|-----------|----------|------|-----|---|---|---|---|---|---|---|---|
| 18 | Defense | DK/MG/DL/SU | 0x1E | chebyshev* | 6* | wiz | y | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 19 | Falling Slash | DK/MG/DL/SU/RF | 0x11 | melee | 3 | tap | y | [x] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 20 | Lunge | DK/MG/DL/SU | 0x11 | melee | 2 | tap | y | [x] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 21 | Uppercut | DK/MG/DL/SU | 0x11 | melee | 2 | tap | y | [x] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 22 | Cyclone | DK/MG/DL/SU | 0x11 | melee | 2 | tap | y | [x] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 23 | Slash | DK/MG/DL/SU | 0x11 | melee | 2 | tap | y | [x] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 60 | Force | DL | — | — | 5 | pend | n | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 61 | Fire Burst | DL | 0x1E | chebyshev* | 6* | wiz | y | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 62 | Earthquake | DL | 0x1E | chebyshev* | 6* | wiz | y | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 63 | Summon Party | DL | 0x1E | chebyshev* | 6* | wiz | y | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 64 | Greater Critical Damage | DL | 0x1E | chebyshev* | 6* | wiz | y | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 65 | Electric Spark | DL | 0x1E | chebyshev* | 6* | wiz | y | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 66 | Force Wave | DL | — | — | 4 | pend | n | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 74 | Fire Blast | DL | — | — | 6 | pend | n | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 76 | Plasma Storm | DW/DK/FE/MG/DL/SU/RF | — | — | 5 | pend | n | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 78 | Fire Scream | DL | — | — | 6 | pend | n | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 238 | Birds | DL | 0x1E | forward_arc | 3 | wiz | y | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |

<details><summary>Support / master (50 skill)</summary>


| ID | Tên | Role | Cat | QA |
|----|-----|------|-----|-----|

| 67 | Stern | support | n | [ ] |
| 68 | Remove Stern | support | n | [ ] |
| 69 | Greater Mana | support | n | [ ] |
| 70 | Invisibility | support | n | [ ] |
| 71 | Remove Invisibility | support | n | [ ] |
| 72 | Remove All Effect | support | n | [ ] |
| 75 | Brand | support | n | [ ] |
| 79 | Explosion | support | n | [ ] |
| 210 | Order of Protection | support | n | [ ] |
| 211 | Order of Restraint | support | n | [ ] |
| 212 | Order of Tracking | support | n | [ ] |
| 213 | Order of Weaken | support | n | [ ] |
| 300 | Add Item Durability Rate | master | n | [ ] |
| 301 | Add Defense Success Rate PvP | master | n | [ ] |
| 302 | Add Max SD | master | n | [ ] |
| 303 | Add MP Recovery Rate | master | n | [ ] |
| 304 | Add Poison Resistance | master | n | [ ] |
| 305 | Add Item Durability Rate | master | n | [ ] |
| 306 | Add SD Recovery Rate | master | n | [ ] |
| 307 | Add HP Recovery Rate | master | n | [ ] |
| 308 | Add Lightning Resistance | master | n | [ ] |
| 309 | Add Defense | master | n | [ ] |
| 310 | Add BP Recovery Rate | master | n | [ ] |
| 311 | Add Ice Resistance | master | n | [ ] |
| 312 | Add Item Durability Rate | master | n | [ ] |
| 313 | Add Defense Success Rate | master | n | [ ] |
| 315 | Add Armor Set Bonus | master | n | [ ] |
| 316 | Add Reflect Damage | master | n | [ ] |
| 317 | Add Energy | master | n | [ ] |
| 318 | Add Vitality | master | n | [ ] |
| 319 | Add Agility | master | n | [ ] |
| 320 | Add Strength | master | n | [ ] |
| 325 | Add Attack Success Rate | master | n | [ ] |
| 334 | Add Max HP | master | n | [ ] |
| 338 | Add Max MP | master | n | [ ] |
| 341 | Add Max BP | master | n | [ ] |
| 347 | Add Attack Success Rate PvP | master | n | [ ] |
| 357 | Add MP Consumption Rate | master | n | [ ] |
| 358 | Add Hunt SD | master | n | [ ] |
| 359 | Add Hunt HP | master | n | [ ] |
| … | +10 | | | [ ] |

</details>

### SU — Summoner

*20 combat · 14 support · 63 master*

| ID | Tên skill | NV | Wire | Loại đánh | Range SV | Roll | Cat | F | R | H | W | S | A | V | QA |
|----|-----------|-----|------|-----------|----------|------|-----|---|---|---|---|---|---|---|---|
| 2 | Meteorite | DW/MG/SU | 0x19 | targeted | 8 | wiz | y | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 4 | Fire Ball | DW/MG/SU | 0x19 | targeted | 6 | wiz | y | [x] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 7 | Ice | DW/MG/SU | 0x19 | targeted | 8 | wiz | y | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 9 | Evil Spirit | DW/MG/SU | 0x1E | chebyshev | 6 | wiz | y | [x] | [x] | [x] | [x] | [x] | [x] | [x] | [x] |
| 11 | Power Wave | DW/MG/SU | — | — | 6 | pend | n | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 18 | Defense | DK/MG/DL/SU | 0x1E | chebyshev* | 6* | wiz | y | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 19 | Falling Slash | DK/MG/DL/SU/RF | 0x11 | melee | 3 | tap | y | [x] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 20 | Lunge | DK/MG/DL/SU | 0x11 | melee | 2 | tap | y | [x] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 21 | Uppercut | DK/MG/DL/SU | 0x11 | melee | 2 | tap | y | [x] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 22 | Cyclone | DK/MG/DL/SU | 0x11 | melee | 2 | tap | y | [x] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 23 | Slash | DK/MG/DL/SU | 0x11 | melee | 2 | tap | y | [x] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 45 | Mana Glaive | DW/SU | 0x1E | chebyshev* | 6 | wiz | y | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 76 | Plasma Storm | DW/DK/FE/MG/DL/SU/RF | — | — | 5 | pend | n | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 214 | Drain Life | SU | — | — | 6 | pend | n | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 215 | Chain Lightning | SU | — | — | 6 | pend | n | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 216 | Electric Surge | SU | — | — | 6 | pend | n | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 223 | Sahamutt | SU | — | — | 6 | pend | n | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 224 | Neil | SU | — | — | 6 | pend | n | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 225 | Ghost Phantom | SU | — | — | 6 | pend | n | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 230 | Red Storm | SU | — | — | 5 | pend | n | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |

<details><summary>Support / master (77 skill)</summary>


| ID | Tên | Role | Cat | QA |
|----|-----|------|-----|-----|

| 67 | Stern | support | n | [ ] |
| 68 | Remove Stern | support | n | [ ] |
| 69 | Greater Mana | support | n | [ ] |
| 70 | Invisibility | support | n | [ ] |
| 71 | Remove Invisibility | support | n | [ ] |
| 210 | Order of Protection | support | n | [ ] |
| 211 | Order of Restraint | support | n | [ ] |
| 212 | Order of Tracking | support | n | [ ] |
| 213 | Order of Weaken | support | n | [ ] |
| 217 | Damage Reflect | support | n | [ ] |
| 218 | Sword Power | support | n | [ ] |
| 219 | Sleep | support | n | [ ] |
| 221 | Lesser Damage | support | n | [ ] |
| 222 | Lesser Defense | support | n | [ ] |
| 300 | Add Item Durability Rate | master | n | [ ] |
| 301 | Add Defense Success Rate PvP | master | n | [ ] |
| 302 | Add Max SD | master | n | [ ] |
| 303 | Add MP Recovery Rate | master | n | [ ] |
| 304 | Add Poison Resistance | master | n | [ ] |
| 305 | Add Item Durability Rate | master | n | [ ] |
| 306 | Add SD Recovery Rate | master | n | [ ] |
| 307 | Add HP Recovery Rate | master | n | [ ] |
| 308 | Add Lightning Resistance | master | n | [ ] |
| 309 | Add Defense | master | n | [ ] |
| 310 | Add BP Recovery Rate | master | n | [ ] |
| 311 | Add Ice Resistance | master | n | [ ] |
| 312 | Add Item Durability Rate | master | n | [ ] |
| 313 | Add Defense Success Rate | master | n | [ ] |
| 315 | Add Armor Set Bonus | master | n | [ ] |
| 316 | Add Reflect Damage | master | n | [ ] |
| 317 | Add Energy | master | n | [ ] |
| 318 | Add Vitality | master | n | [ ] |
| 319 | Add Agility | master | n | [ ] |
| 320 | Add Strength | master | n | [ ] |
| 325 | Add Attack Success Rate | master | n | [ ] |
| 334 | Add Max HP | master | n | [ ] |
| 338 | Add Max MP | master | n | [ ] |
| 341 | Add Max BP | master | n | [ ] |
| 347 | Add Attack Success Rate PvP | master | n | [ ] |
| 357 | Add MP Consumption Rate | master | n | [ ] |
| … | +37 | | | [ ] |

</details>

### RF — Rage Fighter

*3 combat · 19 support · 0 master*

| ID | Tên skill | NV | Wire | Loại đánh | Range SV | Roll | Cat | F | R | H | W | S | A | V | QA |
|----|-----------|-----|------|-----------|----------|------|-----|---|---|---|---|---|---|---|---|
| 19 | Falling Slash | DK/MG/DL/SU/RF | 0x11 | melee | 3 | tap | y | [x] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 76 | Plasma Storm | DW/DK/FE/MG/DL/SU/RF | — | — | 5 | pend | n | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |
| 269 | Charge | RF | — | — | 4 | pend | n | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] | [ ] |

<details><summary>Support / master (19 skill)</summary>


| ID | Tên | Role | Cat | QA |
|----|-----|------|-----|-----|

| 67 | Stern | support | n | [ ] |
| 68 | Remove Stern | support | n | [ ] |
| 69 | Greater Mana | support | n | [ ] |
| 70 | Invisibility | support | n | [ ] |
| 71 | Remove Invisibility | support | n | [ ] |
| 210 | Order of Protection | support | n | [ ] |
| 211 | Order of Restraint | support | n | [ ] |
| 212 | Order of Tracking | support | n | [ ] |
| 213 | Order of Weaken | support | n | [ ] |
| 260 | Large Ring Blower | support | n | [ ] |
| 261 | Upper Beast | support | n | [ ] |
| 262 | Chain Driver | support | n | [ ] |
| 263 | Dark Side | support | n | [ ] |
| 264 | Dragon Lore | support | n | [ ] |
| 265 | Dragon Slayer | support | n | [ ] |
| 266 | Greater Ignore Damage Rate | support | n | [ ] |
| 267 | Fitness | support | n | [ ] |
| 268 | Greater Defense Success Rate | support | n | [ ] |
| 270 | Phoenix Shot | support | n | [ ] |

</details>

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
