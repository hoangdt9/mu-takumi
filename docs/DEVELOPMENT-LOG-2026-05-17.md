# Development log — 2026-05-17

Ghi tiếp từ **`DEVELOPMENT-LOG-2026-05-16.md`**. Dùng kèm **`server-next/docs/IMPLEMENTATION-CHECKLIST.md`**.

**Tham chiếu commit (`main`, mới → cũ):**

| Commit | Tóm tắt |
|--------|---------|
| *(pending)* | NPC shop tooltip buy/sell parity (Android): `Sell` flag, exc sell `ItemValue`, potion `/3`, two-tap buy, confirm debounce |
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

### Shop (client) — sáng + chiều 2026-05-17

**Sáng (đã commit trước):**

- **`ShopItemValueCache`**: cache giá từ **`C2 F3 E9`** cho tooltip / debit UI.
- **`ReceiveBuyConfirm`**: xử lý **`F3 ED`** khi server bật `TAKUMI_SHOP_BUY_CONFIRM=1` → message box + `SendRequestBuyConfirm`.
- **`NewUINPCShop`**: click mua dùng giá cache thay hardcode.

**Chiều (tooltip Zen parity — QA Android Noria):**

| Triệu chứng | Nguyên nhân | Sửa |
|-------------|-------------|-----|
| Tooltip **Giá mua** ~382k, chat **Đã trả** ~191k (bình máu x255) | `TOOLTIP_TYPE_NPC_SHOP` gọi `RenderItemInfo(..., Sell=true)` → nhánh **giá bán** `ItemValue(ip,0)` | Shop stock: `Sell=false` → **Giá mua** từ F3 E9 |
| Tooltip **Giá bán** ~16M, nhận ~40M (quần exc +9) | Cache F3 E9 fallback `exc=0` / `buy/3` thấp hơn `ItemValue(ip,0)` với excellent | Đồ exc: `ItemValue(ip,0)`; `TryGetSell` / `TryGetBuyExact` chỉ khớp key chính xác |
| Confirm mua bấm OK 2 lần / mua đôi | Msgbox + `ReceiveBuyConfirm` re-entry | Debounce OK, khóa shop khi msgbox, `LockOkButton`, Android press-on-OK |
| Hai chạm: hover = tooltip, tap 2 = mua | UX mobile | `m_bShopTooltipPinned` + `OpenBuyConfirmDialog` lần 2 |

**File chính (chiều):** `NewUIInventoryCtrl.cpp`, `ZzzInventory.cpp` (`ResolveNpcShopSellZen`), `ShopItemValueCache.cpp`, `NewUINPCShop.cpp`, `NewUICommonMessageBox.cpp`, `WSclient.cpp` (`ShopNotifyZenDelta` / `s_pendingShopZenSpend`).

**Server (chiều):** `LegacyShopBuyPriceEstimate.EstimatePotionBuy` — parity `ItemValue(ip,1)` (1500 cho potion +3/+6, `×255`, `/3`); test `ResolveBuy_large_healing_potion_stack_matches_client_itemvalue_buy`. **`NpcShopTaxWire602`** (`B2 1A`) khi mở shop.

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

- **`ShopItemValueResolver`**, **`LegacyShopBuyPriceEstimate`**: giá mua khớp client + `ItemValue.txt`; potion stack dùng **`EstimatePotionBuy`** (`/3`, base 1500 cho +3/+6).
- **`ShopCommerceHandler`**: buy confirm wire **`F3 ED`**; charge = **`ResolveChargedBuy`**; sell = **`ShopItemPricing.SellPrice`** → **`ResolveSell`** (exc = legacy estimate `/3`).
- **`ShopItemValueSender`**: F3 E9 chỉ stock shop (không merge túi player — tránh overwrite giá).
- **`PlayerShopSession`**: tax % per session; gửi tax info khi mở shop.
- Test: **`ShopItemValueResolverTests`** (+ potion stack), cập nhật **`ShopCommerceWire602Tests`**.

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

# APK (release QA shop — đường dẫn đúng)
cd Source/android
./gradlew :app:assembleRealDevicePreloadDefaultRelease -PmuBootstrapAdbReverse=true
adb install -r app/build/outputs/apk/realDevicePreloadDefault/release/app-realDevice-preloadDefault-release.apk
```

**Smoke gợi ý (05-17):**

1. Login → Lorencia → đánh quái → **level-up**: cột xoắn vàng (FLARE) + đĩa trắng; **không** vòng đỏ `MAGIC+2/0` chồng.
2. NPC shop Noria: hover **bình máu** trong shop → **Giá mua** khớp Zen trừ khi mua; hover **đồ exc** trong túi → **Giá bán** ~40M khớp chat **Nhận … Zen** khi bán.
3. Mua: tap 1 = tooltip, tap 2 = confirm (F3 ED nếu env bật); OK một lần, không mua đôi.
4. Kéo item trong túi → `0x24` + `F3 10` khớp server; không mất slot sau buy.

**Docs liên quan:** `server-next/docs/M9-M8-NPC-GAMEPLAY-OWNERSHIP.md`, `server-next/docs/M7-CHARACTER-PERSISTENCE-CHECKLIST.md`, `server-next/docs/M6-GAME-TCP-CHECKLIST.md`.
