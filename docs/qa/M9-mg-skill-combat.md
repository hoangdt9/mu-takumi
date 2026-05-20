# QA — M9 MG skill combat trên Android (APK)

**Guide:** [../android/MOBILE-SKILL-COMBAT-GUIDE.md](../android/MOBILE-SKILL-COMBAT-GUIDE.md) · **Dev:** `SkillCombatCatalog.cs`, `MonsterCombatHandler.cs` · **Matrix:** [../android/SKILL-MATRIX.csv](../android/SKILL-MATRIX.csv)

**Account QA:** `test` / `mg001` (44 skill MG — `./scripts/db/verify-mg001-skills.sh`)

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
- [ ] `./scripts/db/verify-mg001-skills.sh` → OK 44 skill
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
- [ ] Server log: `[m9] magic continue skill=9` (không chỉ `len=12 head=0xC3` rồi im)
- [ ] Đổi đồ tăng MagicSpeed → linh hồn bay **nhanh hơn** rõ (cần APK client mới)

Ghi chú damage / log: ____

---

## MG channel khác (wire + damage)

| Skill | ID | Gesture | Server log mong đợi | HP quái giảm | VFX/anim đẹp |
|-------|-----|---------|---------------------|--------------|--------------|
| Lốc | 8 | channel | `skill=8` | [ ] | [ ] (speed OK, spawn TODO) |
| Fire Slash | 55 | channel | `skill=55` | [ ] | [ ] |
| Power Slash | 56 | channel | `skill=56` | [ ] | [ ] |
| Flame Strike | 236 | channel | `skill=236` | [ ] | [ ] |
| Gigantic Storm | 237 | channel | `skill=237` | [ ] | [ ] |

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
| Không cast | Hotbar có skill? `TakumiSkillAtk` logcat; tầm / `CheckTarget` |
| Không skill trên bar | `verify-mg001-skills.sh`; relog sau SQL; `game-host` restart |
| Chỉ Linh hồn đẹp, skill khác trơ | Đúng trạng thái hiện tại — xem guide §10 animation TODO |
