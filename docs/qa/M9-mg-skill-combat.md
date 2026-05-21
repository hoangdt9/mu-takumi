# QA — M9 MG skill combat trên Android (APK)

**Guide:** [../android/MOBILE-SKILL-COMBAT-GUIDE.md](../android/MOBILE-SKILL-COMBAT-GUIDE.md) · **Checklist SSOT:** [../android/SKILL-QA-CHECKLIST.md](../android/SKILL-QA-CHECKLIST.md) · [../android/SKILL-QA-CHECKLIST.csv](../android/SKILL-QA-CHECKLIST.csv) · **Dev:** `SkillCombatCatalog.cs`, `SkillCombatRange.cs`, `MonsterCombatHandler.cs`

**Account QA:** `test` / `mg001` (30 combat MG — `./scripts/db/reset-mg001-skills.sh`)

**Quy ước:** `[ ]` · `[x]` · `[!]` · `[-]`

---

## Gesture (PC → mobile)

| Muốn test | Cách làm trên APK |
|-----------|-------------------|
| Cast channel (9, 55, …) | Gán skill ô 0 → **long-press** hoặc **double-tap** lên quái |
| Đi / đánh thường | **Tap ngắn** đất/quái hoặc nút **ATTACK** (không cast skill) |
| Học skill sách | Túi mở → **long-press** trên sách |

Chi tiết: [MOBILE-SKILL-COMBAT-GUIDE.md](../android/MOBILE-SKILL-COMBAT-GUIDE.md) §4.

---

## Chuẩn bị

- [ ] `cd server-next && ./scripts/docker/docker-stack.sh --host-build --detach`
- [ ] `./scripts/db/reset-mg001-skills.sh` → verify OK 30 combat, slot 1..30
- [ ] APK build **sau** mọi đổi `ZzzCharacter.cpp` / `ZzzInterface.cpp` / `TakumiAndroidInput.cpp`
- [ ] (USB) `./scripts/android/adb-reverse-takumi-dev.sh` nếu không ping LAN
- [ ] Terminal: `docker compose logs -f game-host 2>&1 | grep '\[m9\]'`
- [ ] (Tuỳ chọn) `adb logcat -s TakumiSkillAtk`

---

## Join & skill bar

- [ ] Login `test` → chọn `mg001` → vào map
- [ ] Skill list không trống (F3 11) — có Linh hồn, Lốc, Fire Slash, …
- [ ] Gán skill 9 vào ô chính (picker / tap ô 0)
- [ ] Thoát app → vào lại → skill vẫn ở ô 0 ([SKILL-HOTKEY-PERSISTENCE.md](../game-spec/SKILL-HOTKEY-PERSISTENCE.md))

---

## Linh hồn (9) — reference skill

- [ ] **Long-press** hoặc **double-tap** lên quái (trong tầm) → thấy linh hồn bay
- [ ] Quái **mất HP** (không chỉ VFX)
- [ ] Server log: `[m9] magic continue skill=9` **`mode=0`** (không chỉ `len=12 head=0xC3` rồi im)
- [ ] Đổi đồ tăng MagicSpeed → linh hồn bay **nhanh hơn** rõ (cần APK client mới)

Ghi chú damage / log: ____

---

## MG channel khác (wire + damage + hit volume)

Log `0x1E`: `mode=0` vòng Chebyshev (Evil Spirit) · `mode=1` cung phía trước (slash) · `mode=2` hành lang hẹp (Lốc).

| Skill | ID | Gesture | Server log mong đợi | Hit volume | HP quái giảm | VFX/anim |
|-------|-----|---------|---------------------|------------|--------------|----------|
| Lốc | 8 | channel | `skill=8` **`mode=2`** `hits=1-4` | [ ] chỉ trên đường lốc, không 10+ mob hai bên | [ ] | [ ] |
| Fire Slash | 55 | channel | `skill=55` **`mode=1`** | [ ] chỉ phía trước ~2 ô | [ ] | [ ] |
| Power Slash | 56 | channel | `skill=56` **`mode=1`** | [ ] | [ ] | [ ] |
| Flame Strike | 236 | channel | `skill=236` **`mode=1`** | [ ] | [ ] | [ ] |
| Gigantic Storm | 237 | channel | `skill=237` **`mode=1`** | [ ] | [ ] | [ ] |

---

## MG targeted / burst (tap)

| Skill | ID | Wire | HP quái | Ghi chú |
|-------|-----|------|---------|---------|
| Sét | 3 | `0x19` | [ ] | |
| Hỏa cầu | 4 | `0x19` | [ ] | |
| Độc | 12 | `0x19` | [ ] | |
| Inferno | 13 | `0xDB` | [ ] | |

---

## Regression

- [ ] Nút **ATTACK** chỉ đánh thường (`[m9] combat hit`), không thay channel skill
- [ ] Pick up item / talk NPC vẫn hoạt động (gesture không “kẹt” skill)
- [ ] Melee + di chuyển joystick bình thưường

---

## Khi FAIL

| Triệu chứng | Kiểm tra |
|-------------|----------|
| Có VFX, không damage | Server rebuild? Log có `magic continue`? Trước đây: C3 `0x1E` không parse |
| Lốc quét rộng 2 bên | Log `mode=0` hoặc `hits=10+` → deploy bản corridor; xem [SKILL-QA-CHECKLIST.md](../android/SKILL-QA-CHECKLIST.md) (MG test nhanh) |
| Không cast | Hotbar có skill? `TakumiSkillAtk` logcat; tầm / `CheckTarget` |
| Không skill trên bar | `verify-mg001-skills.sh`; relog sau SQL; `game-host` restart |
| Chỉ Linh hồn đẹp, skill khác trơ | Đúng trạng thái hiện tại — xem guide §10 animation TODO |
