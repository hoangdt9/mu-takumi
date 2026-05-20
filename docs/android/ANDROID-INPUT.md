# Android input — PC mouse parity

Mapping from desktop MU (Season 6) to touch on Takumi Android. **Hướng dẫn đầy đủ (skill, sách skill, channel):** [MOBILE-SKILL-COMBAT-GUIDE.md](./MOBILE-SKILL-COMBAT-GUIDE.md) §4.

**Timing:** long-press **480ms** · double-tap window **420ms** · cancel long-press if finger moves **>48px** (virtual UI space).

---

## Tóm tắt nút

| PC | Mobile |
|----|--------|
| **LMB** click ground | **Tap ngắn** → walk (`MoveHero`) |
| **LMB** click monster | **Tap ngắn** → delayed melee (~420ms) |
| **RMB** skill on target | **Long-press** or **double-tap** (with skill on hotbar) |
| **RMB** hold channel skill | **Long-press hold** (repeat tick) or **double-tap** (latched channel) |
| **RMB** item in bag | **Long-press** or **double-tap** on item (learn skill, use potion) |
| **Hotbar** key / click skill icon | **Tap** skill ring or Q–R row |
| **Skill picker** | **Tap** main skill box → list → pick skill |
| **Move** (keyboard) | **Joystick** (bottom-left) |
| **Normal attack** | **ATTACK** button only (not bound skill) |

---

## In-world (map)

| PC | Mobile | Notes |
|----|--------|-------|
| LMB ground | Short tap | `MoveHero()` |
| LMB monster | Short tap | `FireWorldMeleeAttack` after tap delay |
| RMB skill | Long-press / double-tap | Requires skill on slot 0 / hotbar; see priority below |
| RMB channel (Evil Spirit, …) | Long-press hold or double-tap latch | `CastDirectionChannelSkill`, not `Attack()` |
| LMB item pickup | Long-press / double-tap when no skill cast | `FireWorldPickUpItem` |
| Operate chest/lever | Long-press / double-tap fallback | `FireWorldOperateObject` |
| LMB NPC | Short tap walk/talk; long-press if no skill | `FireWorldNpcTalk` |
| — | **ATTACK** button | Normal melee only (`TriggerVirtualCombat(true)`) |
| — | **Joystick** | 8-way move |
| Second finger | Legacy `MouseRButton*` | Prefer long-press/double-tap on world |

**Gesture priority** (long-press / double-tap): **skill cast** → **pick up item** → **operate** → **NPC talk**.

**HUD dead zones** (no world gesture): joystick area, ATTACK button, skill bar strip — `TakumiAndroid_IsHudBlockingWorldGesture`.

---

## Inventory (bag open)

| PC | Mobile |
|----|--------|
| **RMB** use item (skill book, fruit, scroll, potion…) | **Long-press** or **double-tap** on item → `TryConsumeItem()` / `SendRequestUse` |
| LMB drag item | Short touch + drag (`MouseLButton`) |

Log: `adb logcat -s TakumiInvUse`

---

## Skill hotbar & picker

| PC | Mobile |
|----|--------|
| Keys 0–9 / Ctrl+keys | Tap hotbar icons (Classic Q–R) or Mobile skill ring |
| Click main skill slot | Tap skill box → toggle picker → tap skill row |
| Assign skill to slot | Assign mode → tap target slot |

Persistence: `C1 F3 30` + Postgres `key_configuration` — [game-spec/SKILL-HOTKEY-PERSISTENCE.md](./game-spec/SKILL-HOTKEY-PERSISTENCE.md).

---

## Channel skills (wizard / MG)

`SendRequestMagicContinue` (`0x1E`) — Evil Spirit, Storm, Fire Slash, Power Slash, … — via `IsDirectionChannelSkillType()` + `CastDirectionChannelSkill`.

Wire + damage: [MOBILE-SKILL-COMBAT-GUIDE.md](./MOBILE-SKILL-COMBAT-GUIDE.md).

---

## Debug

```bash
adb logcat -s TakumiSkillAtk TakumiInvUse SkillPicker
```

---

## Related files

- `android_main.cpp` — virtual pad, SDL touch → mouse, HUD touch routing
- `Platform/TakumiAndroidInput.cpp` — inventory use + world skill gestures
- `ZzzInterface.cpp` — `MoveHero`, `Attack`, `CastDirectionChannelSkill`
- `NewUIMainFrameWindow.cpp` — picker, hotbar, `ApplySelectedSkillIndex`
- `NewUIMyInventory.cpp` — `TryConsumeItem` (skill books)
