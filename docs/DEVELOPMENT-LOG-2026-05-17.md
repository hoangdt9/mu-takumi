# Development log — 2026-05-17

Ghi tiếp từ **`DEVELOPMENT-LOG-2026-05-16.md`**. Dùng kèm **`server-next/docs/IMPLEMENTATION-CHECKLIST.md`**.

**Tham chiếu commit (`main`, mới → cũ):**

| Commit | Tóm tắt |
|--------|---------|
| `135634f` | Merge `origin/main` — resolve `AndroidLoginUi.cpp` (loopback + protocol drain) |
| `5e729d4` | Level-up **FLARE** spiral (gold tint) + white disc; bỏ `MAGIC+2` ground rings; gỡ log debug |
| `3c36cf0` | `ShopItemValueCache` link vào CMake/Android build |
| `65e0e36` | Client **F3 ED** buy confirm + NPC shop message box |
| `1d77d94` | Shop buy price khớp **F3 E9** + legacy formula (server) |
| `6330de9` | Inventory **0x24** + **F3 10** sync; plain **C4** trên Android; BMD footprints |
| `fd6ddf8` | Register form text alignment; game login parse |

---

## Android client (`Source/5.Main`)

### Level-up VFX (sửa nhầm hiệu ứng)

**Vấn đề:** Log spawn báo `MAGIC+2/3=1` nhưng màn hình vẫn thấy **vòng đỏ phẳng** — do nhầm loại effect:

| Hiệu ứng | Cơ chế | Ghi chú |
|----------|--------|---------|
| Cột xoắn level-up (vanilla) | `BITMAP_FLARE` joints SubType 0 (×15) | Pegasus `ReceiveLevelUp` — **đây** là spiral quanh nhân vật |
| Vòng đỏ mặt đất | `BITMAP_MAGIC+2` SubType **0** | Join map / warp (`TakumiTryCreateJoinWarpRingEffect`) — **không** phải level-up |
| Thử nghiệm cũ | `BITMAP_MAGIC+2` SubType **3** + gold tint | Cùng texture `RenderCircle` với vòng đỏ — khó phân biệt trên GLES |

**Đã làm (`5e729d4`):**

- `TakumiPlayLevelUpEffects`: clear FLARE/45/46 + `MAGIC+1/0` + `MAGIC+2/0` + `MAGIC+2/3`; debounce **1.2s**.
- Spawn: **15× FLARE** (gold `vPriorColor` `(1, 0.92, 0.18)`) + **`MAGIC+1/0`** white disc; master level dùng FLARE 45/46.
- `ZzzEffectJoint.cpp`: giữ `vPriorColor` khi tạo FLARE (không ghi đè bằng `Target->Light`).
- `RenderLevelUpMagicFx`: chỉ white disc; `HasActiveLevelUpGfx` theo FLARE joints + disc.
- Gỡ log tạm: `[LevelUpFx]`, `[Exp]`, `[Combat]` trên Android.

**File chính:** `WSclient.cpp` (`TakumiSpawnLevelUpDiscAndColumn`), `ZzzEffect.cpp`, `ZzzEffectJoint.cpp`, `ZzzScene.cpp`.

### Shop (client)

- **`ShopItemValueCache`**: cache giá từ **`C2 F3 E9`** cho tooltip / debit UI.
- **`ReceiveBuyConfirm`**: xử lý **`F3 ED`** khi server bật `TAKUMI_SHOP_BUY_CONFIRM=1` → message box + `SendRequestBuyConfirm`.
- **`NewUINPCShop`**: click mua dùng giá cache thay hardcode.

### Inventory / wire

- **`0x24` move**: clear footprint client khi pick/drop; server gửi **`0x24` trước `F3 10`** trên inv→inv; resync full túi sau shop buy.
- **Plain `C4 F3 10`**: Android nhận inventory list không SM-decrypt khi header plain.
- **`AndroidLoginUi`**: loopback retry khi split stack chưa có C2 F4 06; drain protocol sau sub-server pick (merge `135634f`).

### UI / login khác

- Register in-game: căn text input (`fd6ddf8`).
- `MsgWin` / stat dialog: tiếp tục từ nhánh 05-16 (IME, pump `F3 06`).

---

## Server (`server-next`)

### Shop commerce

- **`ShopItemValueResolver`**, **`LegacyShopBuyPriceEstimate`**: giá mua khớp client + `ItemValue.txt`.
- **`ShopCommerceHandler`**: buy confirm wire **`F3 ED`**; full **`F3 10`** sau buy.
- **`JoinMapEconomy602`**: zen / economy fields trên join khi cần shop QA.
- Test: **`ShopItemValueResolverTests`**, cập nhật **`ShopCommerceWire602Tests`**.

### Inventory

- **`InventoryBagGrid`**, **`ItemSizeCatalog`**, **`ClientItemFootprintCatalog`**: placement theo footprint BMD.
- **`JoinInventoryLifecycle`**: đồng bộ túi sau thao tác item.
- Script: **`scripts/clear-inventory-account.sh`**; SQL test zen **`012_test_account_zen_2b.sql`**.

### Combat / vitals (tiếp 05-16)

- **`MonsterCombatHandler`**, **`PlayerVitalsLoop`**: hit/die/regen wire tinh chỉnh.
- **`CharacterRegenWire602`**: town respawn parity tests.

---

## Build / QA nhanh

```bash
# Server
cd server-next
./scripts/docker-stack.sh --host-build --recreate --detach

# APK
cd Source/android
./gradlew :app:assembleRealDevicePreloadDefaultDebug
adb install -r app/build/outputs/apk/realDevicePreloadDefault/debug/*.apk
```

**Smoke gợi ý (05-17):**

1. Login → Lorencia → đánh quái → **level-up**: cột xoắn vàng (FLARE) + đĩa trắng; **không** vòng đỏ `MAGIC+2/0` chồng.
2. NPC shop → tooltip giá từ F3 E9 → mua (F3 ED confirm nếu env bật).
3. Kéo item trong túi → `0x24` + `F3 10` khớp server; không mất slot sau buy.

**Docs liên quan:** `server-next/docs/M9-M8-NPC-GAMEPLAY-OWNERSHIP.md`, `server-next/docs/M7-CHARACTER-PERSISTENCE-CHECKLIST.md`, `server-next/docs/M6-GAME-TCP-CHECKLIST.md`.
