# Skill combat — checklist sửa & test (server + mobile)

**Cập nhật:** 2026-05-21  
**Phạm vi:** `server-next` hit volume + Android cast/VFX.  
**Liên quan:** [MOBILE-SKILL-COMBAT-GUIDE.md](./MOBILE-SKILL-COMBAT-GUIDE.md) · [SKILL-COMBAT-ROLLOUT-PLAN.md](./SKILL-COMBAT-ROLLOUT-PLAN.md) · [qa/M9-mg-skill-combat.md](../qa/M9-mg-skill-combat.md) · [journal/DEVELOPMENT-LOG-2026-05-20.md](../journal/DEVELOPMENT-LOG-2026-05-20.md)

**Quy ước trạng thái**

| Ký hiệu | Ý nghĩa |
|---------|---------|
| ✅ | Đã làm + có test/unit hoặc QA xác nhận |
| 🟡 | Code xong, **cần QA APK** sau deploy |
| ⬜ | Chưa làm / chưa verify |
| ❌ | Biết sai — cần sửa tiếp |

**Log server `0x1E`:** `mode=0` omnidirectional (Chebyshev) · `mode=1` forward arc · `mode=2` forward corridor (Twister)

---

## 1. Hạng mục cần sửa (ưu tiên)

### 1.1 Server — vùng ảnh hưởng (hit volume) `SkillCombatRange` / `SkillCombatCatalog`

| # | Hạng mục | Mô tả | Trạng thái |
|---|----------|-------|------------|
| H1 | Parse **C3** `0x1E` | Client Android gửi encrypted continue | ✅ |
| H2 | Tâm AoE channel = **vị trí player** | Evil Spirit không dùng `skillX/Y` packet làm tâm | ✅ |
| H3 | Evil Spirit (9) — **Chebyshev 6** quanh player | OpenMU: không frustum | ✅ |
| H4 | Twister (8) — **forward corridor** width 1.5, range 6 | Không còn vuông 13×13; log `mode=2` | 🟡 |
| H5 | Fire Slash (55) / Power Slash (56) — **forward arc** range 2 | `mode=1`, không quét 360° | 🟡 |
| H6 | Flame Strike (236) / Gigantic (237) — arc range 2–3 | `mode=1` | 🟡 |
| H7 | Hell Fire (10) / Blast (13) — arc range 5 | `mode=1` | ⬜ |
| H8 | Inferno (14) / skill 15–18 — geometry đúng | Hiện gộp Chebyshev 6 — cần đối chiếu OpenMU (target area / ground) | ⬜ |
| H9 | Skill 61–65 trong catalog | **Không** phải Twister — tách range/geometry (Fire Burst, Earthquake, …) | ⬜ |
| H10 | Evil Spirit master **385, 487** | Kế thừa mode 0 từ skill 9 | 🟡 |
| H11 | Magic burst **0xDB** — tâm / hình | Inferno burst từ player tile | 🟡 |
| H12 | Targeted **0x19** — range Chebyshev | Sét, Hỏa cầu, Độc | 🟡 |
| H13 | Melee **0x11** — gắn stat sheet | Không chỉ stub damage cố định | ⬜ |

**Code:** `server-next/src/Takumi.Server.Protocol/SkillCombat*.cs` · `MonsterCombatHandler.cs`  
**Test:** `SkillCombatRangeTests`, `SkillCombatDirectionTests` (36 tests)

### 1.2 Client — cast + animation + VFX (MG rollout)

| # | Hạng mục | Trạng thái |
|---|----------|------------|
| C1 | Gesture long-press / double-tap → channel | ✅ |
| C2 | Linh hồn (9) — joint + MagicSpeed | ✅ |
| C3 | Lốc (8) — `MODEL_STORM` speed | 🟡 spawn channel ổn định ⬜ |
| C4 | Fire Slash / Power Slash — `SetAction` + gathering/force | ⬜ |
| C5 | Flame Strike / Gigantic — model effect | ⬜ |
| C6 | Targeted bolt VFX (3–5, 12) | ⬜ |

Chi tiết sprint: [SKILL-COMBAT-ROLLOUT-PLAN.md](./SKILL-COMBAT-ROLLOUT-PLAN.md) §9–12.

### 1.3 QA & deploy

| # | Việc | Lệnh |
|---|------|------|
| Q1 | Host build + recreate game-host | `cd server-next && ./scripts/docker/docker-stack.sh --host-build --recreate --detach` |
| Q2 | Verify 44 skill mg001 | `./scripts/db/verify-mg001-skills.sh` |
| Q3 | Log combat | `docker compose logs -f game-host 2>&1 \| grep '\[m9\]'` |
| Q4 | APK chỉ khi đổi client | `Source/android` gradle assemble |

---

## 2. Checklist skill MG / DW (QA `test` / `mg001`)

Account: **44 skill** trên `character_skill`. Test tại Atlans / Lorencia đông quái.

### 2.1 Channel `0x1E` — server hit volume + damage

| ID | Tên | Wire | Server damage | Hit volume (`mode`) | QA in-game | Ghi chú |
|----|-----|------|---------------|---------------------|------------|---------|
| 9 | Linh hồn | `0x1E` | ✅ | ✅ `mode=0` Chebyshev 6 | 🟡 | Reference; quái trong vòng 6 ô quanh **player** |
| 8 | Lốc xoáy | `0x1E` | ✅ | 🟡 `mode=2` corridor | ⬜ | Chỉ quái **trên đường lốc**, không 10–15 mob hai bên; log `hits=1–3` |
| 55 | Fire Slash | `0x1E` | ✅ | 🟡 `mode=1` arc r=2 | ⬜ | Chỉ phía trước, không quét ngang xa |
| 56 | Power Slash | `0x1E` | ✅ | 🟡 `mode=1` arc r=2 | ⬜ | |
| 48–52 | Power Slash+ | `0x1E` | ✅ | 🟡 inherit 56 | ⬜ | |
| 236 | Flame Strike | `0x1E` | ✅ | 🟡 `mode=1` r=2 | ⬜ | |
| 237 | Gigantic Storm | `0x1E` | ✅ | 🟡 `mode=1` r=3 | ⬜ | |
| 238 | Chaotic | `0x1E` | ✅ | 🟡 `mode=1`? | ⬜ | |
| 10 | Hell Fire | `0x1E` | ✅ | 🟡 `mode=1` r=5 | ⬜ | |
| 13 | Blast | `0x1E` | ✅ | 🟡 `mode=1` r=5 | ⬜ | |
| 14 | Inferno | `0x1E`/`0xDB` | ✅ | ⬜ geometry | ⬜ | Cần xác định ground AoE vs channel |
| 378 | Master Flame | `0x1E` | ✅ | 🟡 arc | ⬜ | |
| 483 | Master Flame+ | `0x1E` | ✅ | 🟡 arc | ⬜ | |
| 61–65 | DL/MG specials | `0x1E` | 🟡 | ⬜ | ⬜ | Catalog có — **không** dùng geometry Twister |

**Cách test Lốc (8):**

1. Đứng giữa 2 hàng quái (trái/phải), bắn **một hướng** thẳng.
2. **Pass:** chỉ hàng trên trục lốc mất máu; log `mode=2`, `hits` thấp (1–4).
3. **Fail:** cả 2 bên cùng lúc `hits=10+`, `mode=0` — server chưa deploy bản corridor.

### 2.2 Targeted `0x19` / burst `0xDB`

| ID | Tên | Wire | Server | QA HP giảm | Anim/VFX |
|----|-----|------|--------|-------------|----------|
| 3 | Sét | `0x19` | ✅ | ⬜ | ⬜ |
| 4 | Hỏa cầu | `0x19` | ✅ | ⬜ | ⬜ |
| 5 | Hỏa ngục | `0x19` | ✅ | ⬜ | ⬜ |
| 12 | Độc | `0x19` | ✅ | ⬜ | ⬜ |
| 13 | Inferno | `0xDB` | ✅ | ⬜ | ⬜ |

### 2.3 Client-only (không chặn damage)

| ID | Anim (E) | VFX (F) | Speed (G) |
|----|----------|---------|-----------|
| 9 | ✅ | ✅ | ✅ |
| 8 | ⬜ | ⬜ spawn | 🟡 |
| 55–56, 236–237 | ⬜ | ⬜ | ⬜ |

---

## 3. Checklist theo lớp (mỗi skill “done”)

Đánh ✅ cả **5 cột** mới coi skill xong trên mobile:

| Lớp | Ký hiệu | Evil 9 | Twister 8 | Fire 55 | Evil QA |
|-----|---------|--------|-----------|---------|---------|
| A Input | Gesture | ✅ | ✅ | ✅ | — |
| B Wire TX | Packet | ✅ | ✅ | ✅ | — |
| C Server damage | `[m9]` | ✅ | 🟡 volume | 🟡 volume | — |
| E Char anim | Pose skill | ✅ | ⬜ | ⬜ | — |
| F World VFX | Effect map | ✅ | ⬜ | ⬜ | — |
| G VFX speed | MagicSpeed | ✅ | 🟡 | ⬜ | — |

---

## 4. Thứ tự làm dần (backlog)

1. **QA deploy** Twister `mode=2` trên Atlans (H4) — chặn wide hit.
2. **QA** Fire Slash / Evil Spirit regression (H3, H5).
3. **H8** Inferno / Hell geometry từ OpenMU `SkillsInitializer`.
4. **H9** Tách skill 61–65 khỏi nhóm range 6 omnidirectional.
5. **C4–C6** Animation MG (S1b/S1c rollout plan).
6. **H13** Melee stat parity.
7. Class khác (DK/ELF) — matrix mở rộng trong [SKILL-MATRIX.csv](./SKILL-MATRIX.csv).

---

## 5. Ghi chú kỹ thuật (hit volume)

```text
mode=0  → Chebyshev max(|dx|,|dy|) ≤ range từ (playerX, playerY)
mode=1  → SkillCombatDirection.IsInForwardArc (~140°, Manhattan depth)
mode=2  → IsInForwardCorridor (Twister): perp ≤ 1.5, forward ≤ range
```

Tham chiếu OpenMU: Twister `UseFrustumFilter`, width 1.5; Evil Spirit `UseFrustumFilter: false`.

---

## 6. Cập nhật tài liệu khi tick xong

- [ ] Đánh dấu 🟡 → ✅ trong bảng §2 sau QA APK.
- [ ] Cột `server_hit_volume` / notes trong [SKILL-MATRIX.csv](./SKILL-MATRIX.csv) (tuỳ chọn).
- [ ] Một dòng trong [DEVELOPMENT-LOG](../journal/) ngày verify.

*File này là SSOT tracking; đừng tick ✅ chỉ vì build pass — cần log `[m9]` + quan sát in-game.*
