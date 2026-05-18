# MU client data patches (leaf / character-select worlds)

Small binary patches copied from reference clients when the Takumi `data.zip` bundle is incomplete.

| World | Files | Source (2026-05) |
|-------|--------|------------------|
| `World74` | `leaf01.ozt`, `leaf01.ozj`, `leaf02.ozj` | MuMain-5.2 `src/bin/Data/World74` |
| `World75` | `leaf01.ozt`, `leaf01.ozj`, `leaf02.ozj` | MuMain-5.2 — **`leaf01.ozt` was missing** from `ClientBuild_*/Data/World75` and from `data.zip` |

MU ships leaves as **OZT/OZJ**, not `leaf01.tga`. The client maps `.tga` → `.ozt` in `OpenTga`; without `leaf01.ozt` on World75, logcat shows `File not found Data\World74\leaf01.tga` when swapping worlds.

Apply into a client `Data/` tree and rebuild `data.zip`:

```bash
./scripts/apply-data-patches.sh
./scripts/apply-data-patches.sh --repack-zip   # also refresh ClientBuild_*/data.zip + docker host
```

After repack, Android must re-download `data.zip` (wipe app storage or use a `datafresh` APK build).
