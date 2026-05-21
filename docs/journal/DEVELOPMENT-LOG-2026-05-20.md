# Development log — 2026-05-20

Nhật ký thay đổi **skill combat mobile (MG)** + hạ tầng QA server. Tiếp nối [SESSION-WORKLOG-2026-05-19.md](./journal/SESSION-WORKLOG-2026-05-19.md) (spawn, damage màu, scripts layout).

**Guide tổng hợp:** [MOBILE-SKILL-COMBAT-GUIDE.md](../android/MOBILE-SKILL-COMBAT-GUIDE.md) · **Checklist:** [SKILL-QA-CHECKLIST.md](../android/SKILL-QA-CHECKLIST.md)

---

## 2026-05-21 — Server hit volume (Twister corridor)

| Thay đổi | File |
|----------|------|
| Twister (8) forward corridor thay Chebyshev vuông | `SkillCombatDirection.cs`, `SkillCombatRange.cs`, `SkillCombatCatalog.cs` |
| Log `mode=0/1/2` trên `magic continue` | `MonsterCombatHandler.cs` |
| Unit tests 36 | `SkillCombatRangeTests`, `SkillCombatDirectionTests` |
| Doc checklist SSOT | `docs/android/SKILL-QA-CHECKLIST.md` |

**QA:** deploy `--host-build` → Lốc log `mode=2`, `hits` thấp; Evil Spirit vẫn `mode=0`.

---

## Client (Android / `Source/5.Main`)

| Thay đổi | File | Mục đích |
|----------|------|----------|
| Channel cast MG/DW | `ZzzInterface.cpp` | `IsDirectionChannelSkillType`, `CastDirectionChannelSkill` — Fire Slash, Power Slash, Flame, Hell, … |
| Gesture → channel | `TakumiAndroidInput.cpp` | Dùng shared channel list; long-press / double-tap |
| Linh hồn VFX speed | `ZzzCharacter.cpp`, `ZzzEffectJoint.cpp` | `GetMagicSpeedEffectRatio`, `GetEvilSpiritJoint*` |
| Lốc VFX speed | `ZzzEffect.cpp` | `MODEL_STORM` ∝ MagicSpeed |

**Chưa làm:** `SetAction` + `CreateEffect` per skill cho 55, 56, 236, … trên mobile (xem rollout plan S1b/S1c).

---

## Server (`server-next`)

| Thay đổi | File |
|----------|------|
| Parse C3 `0x1E` | `ClientHitPackets602.cs` |
| MG skill metadata + damage | `SkillCombatCatalog.cs`, `PlayerSkillCombatDamage602.cs`, `CharacterCombatPreview602.cs` |
| Combat handlers + logs | `MonsterCombatHandler.cs` |
| Build fix | `GamePortMinimalSession.cs`, `LegacyLoginHostRunner.cs` (`DecryptedRx`) |
| Tests | `MonsterCombatWire602Tests`, `SkillCombatCatalogMgTests` |

---

## Data / QA account

| Thay đổi | Path |
|----------|------|
| 44 skill MG trên `test/mg001` | `sql/patches/017_*`, `019_mg001_add_combat_rollout_skills.sql` |
| Default join MG | `CharacterSkillCatalog.cs` |
| Verify script | `scripts/db/verify-mg001-skills.sh` |

---

## Docs & scripts

| Thay đổi | Path |
|----------|------|
| Skill QA CSV (286 skill) | `docs/android/SKILL-QA-CHECKLIST.csv` |
| Mobile skill guide + **PC→mobile input §4** | `docs/android/MOBILE-SKILL-COMBAT-GUIDE.md` |
| Skill checklist done/chưa + test | `docs/android/SKILL-QA-CHECKLIST.md` |
| ANDROID-INPUT bảng gesture | `docs/android/ANDROID-INPUT.md` |
| QA MG (mới) | `docs/qa/M9-mg-skill-combat.md` |
| Xóa 27 wrapper trùng `scripts/*.sh` | Chỉ `scripts/{docker,db,android,host,smoke,spawn}/` |

---

## Lệnh deploy / test

```bash
# Server
cd server-next
./scripts/docker/docker-stack.sh --host-build --detach
./scripts/db/verify-mg001-skills.sh

# Client (sau đổi VFX/cast)
cd Source/android && ./gradlew :app:assembleRealDevicePreloadDefaultDebug

# Log
docker compose logs -f game-host 2>&1 | grep '\[m9\]'
adb logcat -s TakumiSkillAtk
```
