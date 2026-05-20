# Android debug logging (FPS)

High-frequency `g_ErrorReport` / `logcat TakumiErrorReport` traces were throttling FPS. They are **off by default** and controlled from one header.

## Toggle (rebuild APK after change)

File: `Source/5.Main/source/Utilities/Log/TakumiAndroidDiag.h`

| Macro | Default | When set to `1` |
|--------|---------|------------------|
| `TAKUMI_ANDROID_DEBUG_PROTOCOL` | `0` | Raw recv hex (first N reads), parsed/send packet logs (`ShouldLogAndroidGamePacket`), `[AndroidLogin]` join/teleport/defer, login UI traces, SimpleModulus key lines, `Translate F1`, etc. |
| `TAKUMI_ANDROID_DEBUG_WEAR_INVENTORY` | `0` | `[TakumiWear]`, `[ReceiveInventory]` per-slot + summary, `[ShopPrice]` traces |

Optional: Gradle `cppFlags "-DTAKUMI_ANDROID_DEBUG_PROTOCOL=1"` (same for wear) without editing the header.

## Runtime (socket only)

`TAKUMI_VERBOSE_SOCKET=1` in the environment still forces **all** game packet parse/send logs (`android_link_stubs.cpp`), independent of the compile-time macros above.

## Server-side (Docker)

See `server-next/.env.lan.example`: `TAKUMI_M9_COMBAT_LOG`, `TAKUMI_M9_VIEWPORT_LOG`, and `TAKUMI_VERBOSE` for game-host log volume.

## Touch / combat mapping

See [`ANDROID-INPUT.md`](ANDROID-INPUT.md) — PC LMB/RMB parity (attack button, skill ring, inventory long-press = use item).

## UI FPS (inventory / skill picker)

File: `Source/5.Main/source/Utilities/Log/TakumiAndroidUiPerf.h`

| Macro | Default (Android) | Effect |
|--------|-------------------|--------|
| `TAKUMI_ANDROID_UI_PERF` | `1` | Master switch for all UI perf paths below |
| `TAKUMI_ANDROID_UI_SPARSE_INVENTORY_GRID` | `1` | One grid background + per-cell draws only when occupied/hovered |
| `TAKUMI_ANDROID_UI_LIGHT_INVENTORY_ITEM_3D` | `1` | `RenderItem3D` only for hovered/dragged inventory items and equipment |
| `TAKUMI_ANDROID_UI_SKILL_PICKER_CACHE` | `1` | Skill picker iterates learned skills only (not all 150 slots) |
| `TAKUMI_ANDROID_UI_PNG_HUD_OVERLAY` | `1` | PNG `assets/ui/*.png` for balo/character/settings (top-right); hides duplicate SS2 menu icons |
| `TAKUMI_ANDROID_UI_ITEM_PLACEHOLDER_2D` | `1` | Colored 2D tile when bag item 3D is skipped |
| `TAKUMI_ANDROID_UI_SKIP_HOTKEY_ITEM_3D` | `1` | No `RenderItem3D` on Q/W/E/R hotkey bar |

See `Platform/TakumiAndroidHud.h`. Set any macro to `0` and rebuild the APK to compare FPS against legacy UI cost.
