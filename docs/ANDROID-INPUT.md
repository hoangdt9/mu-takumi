# Android input — PC mouse parity

Mapping from desktop MU (Season 6 client) to touch controls in `Source/5.Main/source/android_main.cpp`.

## In-world combat

| PC | Mobile |
|----|--------|
| **Left click** on ground / monster | **Finger 1** on world (not HUD): `MouseLButton*` → `MoveHero()` → normal attack / move (`SendRequestAttack`) |
| **Right click** with skill selected | **Skill ring** (4 buttons): `TriggerVirtualCombat(false, slot)` → `Attack()` → `ExecuteSkill()` |
| Hold left on monster | **ATTACK** button (large, bottom-right): repeat normal melee (`TriggerVirtualCombat(true)`) |
| Second finger on screen | **Finger 2**: `MouseRButton*` → same skill path as PC RMB (uses current `Hero->CurrentSkill` from main-frame hotbar) |

Virtual combat pad is enabled even with **legacy main-frame HUD** (`kUseLegacyMainHud = true`): joystick + attack/skill cluster.

## Inventory — use books / gems / scrolls

| PC | Mobile |
|----|--------|
| **Right click** item in bag | **Long-press ~0.5s** on item while inventory is open → one-frame `VK_RBUTTON` pulse → `TryConsumeItem()` / `SendRequestUse` (learn skill, fruit, etc.) |
| Left click drag | Short touch + drag unchanged (`MouseLButton`) |

Implementation: `TakumiAndroid_HandleInventoryTouch*` + `TakumiAndroid_ConsumeInventoryUsePress()` in `Platform/TakumiAndroidInput.h`.

## Skill assignment (mobile)

- Long-press skill picker (legacy HUD) or assign mode → tap a virtual skill slot to bind.
- Attack button no longer casts a bound skill; it is **normal attack only** (PC LMB).

## Related files

- `android_main.cpp` — virtual pad, SDL touch → mouse, inventory long-press
- `ZzzInterface.cpp` — `MoveHero`, `Attack`, `SelectObjects`
- `NewUIMyInventory.cpp` — `HandleInventoryActions`, `TryConsumeItem`
- `Platform/PlatformDefs.h` — `GetAsyncKeyState(VK_LBUTTON/RBUTTON)` from mouse globals
