# Android input — PC mouse parity

Mapping from desktop MU (Season 6 client) to touch controls in `Source/5.Main/source/android_main.cpp`.

## In-world combat

| PC | Mobile |
|----|--------|
| **Left click** on ground / monster | **Finger 1** tap → `MouseLButton*` → `MoveHero()` (move / normal melee) |
| **Right click** skill on monster | **Long-press ~0.48s** or **double-tap** on target → `Attack(Hero)` + hotbar `CurrentSkill` |
| **Right click** skill on ground (AoE) | Same gesture on terrain → `CheckTarget` + `Attack` at cursor |
| **Right click** buff / heal | Same gesture → player / NPC / self under cursor |
| **Left click** item on ground | Short **tap** (priority in `SelectObjects`) |
| **Long-press / double-tap** item on ground | **Pick up** (synthesized LMB + `MOVEMENT_GET`) when skill cast does not apply |
| **Long-press / double-tap** operate object | **Operate** (chest, lever, … — `MOVEMENT_OPERATE`) |
| **Left click** NPC | Short **tap** → walk / talk |
| **Long-press NPC** (no skill / skill failed) | Talk, or buff skill if friendly skill selected |
| **Mobile HUD** skill ring | Tap slot → `TriggerVirtualCombat(false, slot)` |
| **Legacy HUD** skill bar (Q–R row) | Tap icon → `AndroidTriggerHotKeySkillTap` (same as PC hotkey click) |
| **ATTACK** button (bottom-right) | Normal melee only — both Classic and Mobile HUD |
| Second finger | Legacy `MouseRButton*`; prefer long-press/double-tap on world |

Gesture priority (long-press / double-tap): **skill** → **pick up item** → **operate** → **NPC talk**.

Virtual pad: **joystick + ATTACK** on both HUD modes. Skill **ring** (4 slots) on Mobile HUD; **hotbar row** overlay on Classic HUD (`RenderAndroidLegacySkillHotBar`).

## Inventory — use books / gems / scrolls

| PC | Mobile |
|----|--------|
| **Right click** item in bag | **Long-press ~0.5s** on item while inventory is open → one-frame `VK_RBUTTON` pulse → `TryConsumeItem()` / `SendRequestUse` (learn skill, fruit, etc.) |
| Left click drag | Short touch + drag unchanged (`MouseLButton`) |

Implementation: `TakumiAndroid_HandleInventoryTouch*` + `TakumiAndroid_ConsumeInventoryUsePress()` in `Platform/TakumiAndroidInput.h`.

World skill attack uses the same timing constants as inventory use (`480ms` long-press, `420ms` double-tap window): `TakumiAndroid_HandleWorldSkillTouch*` in `Platform/TakumiAndroidInput.cpp`.

## Skill assignment (mobile)

- Long-press skill picker (legacy HUD) or assign mode → tap a virtual skill slot to bind.
- Attack button no longer casts a bound skill; it is **normal attack only** (PC LMB).
- Classic HUD: tap the rendered Q–R skill icons (uses `m_iHotKeySkillType`); open full list via the current-skill box (unchanged).
- Mobile HUD: virtual skill ring + assign mode; consumable quick slots when enabled.

## Debug

```bash
adb logcat -s TakumiSkillAtk
```

## Related files

- `android_main.cpp` — virtual pad, SDL touch → mouse, inventory / world skill gestures
- `Platform/TakumiAndroidInput.cpp` — inventory use + world skill long-press / double-tap
- `ZzzInterface.cpp` — `MoveHero`, `Attack`, `SelectObjects`
- `NewUIMyInventory.cpp` — `HandleInventoryActions`, `TryConsumeItem`
- `Platform/PlatformDefs.h` — `GetAsyncKeyState(VK_LBUTTON/RBUTTON)` from mouse globals
