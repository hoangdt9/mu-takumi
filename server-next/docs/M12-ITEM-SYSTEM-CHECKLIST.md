# M12 — Item system parity checklist

Last updated: 2026-05-19.  
**Liên quan:** `docs/IMPLEMENTATION-CHECKLIST.md` (M4/M7/M8/M9), `docs/M4-M7-CHARACTER-ITEM-MIGRATION.md`, `docs/M11-SOCIAL-WAREHOUSE-SKILLS.md`.

Milestone **tổng hợp cho item** (wire 12-byte, tooltip, shop, jewel, socket) — làm dần theo nhóm; đánh dấu `[x]` khi xong + có test hoặc QA note.

---

## Đã có trong docs (không duplicate chi tiết)

| Chủ đề | Tài liệu / code |
|--------|------------------|
| Opcode item (`0x22`–`0x34`, `F3 10`, `F3 E9/ED`) | `docs/M1-PROTOCOL-PARITY-MAP.md`, `docs/M4-M7-CHARACTER-ITEM-MIGRATION.md` §C |
| Shop buy/sell/repair, giá Zen | `ShopCommerceHandler`, `IMPLEMENTATION-CHECKLIST.md` M9c |
| Socket encode shop/NPC | `ItemWire602.WriteShopItem`, `SocketItemTypeCatalog`, `IMPLEMENTATION-CHECKLIST.md` (2026-05-18) |
| Warehouse / trade wire | `docs/M11-SOCIAL-WAREHOUSE-SKILLS.md` |

---

## A. Wire 12-byte & hiển thị (ưu tiên cao)

- [x] **Non-socket item:** bytes 7–11 = `0xFF` khi encode shop (`WriteShopItem`).
- [x] **Sanitize on join / F3 10:** `ItemWireSanitizer` + `SocketItemType.txt` — cánh/vũ khí không còn cột socket rác từ DB.
- [x] **Client tooltip:** `CSocketItemMgr::IsSocketItem(ITEM*)` chỉ theo **loại item** (parity Pegasus), không coi `0xFF` = socket item.
- [ ] **Persist sau sanitize:** join repack ghi lại DB sạch (đi kèm `InventorySlotPersist` khi bật).
- [ ] **Tooltip parity:** excellent / ancient / harmony / 380 / period — so khớp legacy `ZzzInventory.cpp`.
- [ ] **Durability đỏ / max dur:** `Item.txt` + repair; Fenrir 255.

---

## B. Item use (`C1 0x26`)

- [x] HP/MP/SD potion (`InventoryConsumableRules`).
- [x] Fenrir repair: Jewel of Bless → Fenrir hỏng (`InventoryJewelUseRules`).
- [ ] Jewel of Bless → +level item (không phải Fenrir).
- [ ] Jewel of Soul / Life / Harmony / Extension / Elevation.
- [ ] Scroll, fruit, talisman, bundle jewel.
- [ ] `g_byItemUseType` / C3 `0x26` 6-byte nếu client bật `ENABLE_EDIT`.

---

## C. Socket gameplay

- [ ] Gắn seed/sphere (chaos / NPC) — server validate + wire.
- [ ] Socket set bonus (`SocketItemOption.txt`) trên equip.
- [ ] Seed sphere tooltip (`AttachToolTipForSeedSphereItem`) chỉ item sphere.
- [ ] Mix socket item (`MixMgr`) — ngoài scope shop stub.

---

## D. Equipment & inventory UX

- [x] Equip wear 0–11, bag 12–75, `0x24` + `F3 10` sync.
- [x] Pet slot 8 (Fenrir / Dark Horse / spirit).
- [ ] Muun / extended wear slots (`HAISLOTRING`).
- [ ] Item lock / personal store / drop filter.
- [ ] Stack merge (jewel stack, zen pile).

---

## E. Shop & economy

- [x] NPC shop list `C2 0x31`, buy `0x32`, sell `0x33`, repair `0x34`.
- [x] `C2 F3 E9` / `F3 ED` buy confirm (khi bật flag).
- [ ] Personal shop / PShop wire.
- [ ] Item value ancient/socket bonus sell price đầy đủ.

---

## F. Data & ETL

- [ ] `Item.txt`, `ItemOption*.txt` load runtime (một SSOT).
- [ ] `SocketItemType.txt` / `SocketItemOption.txt` hot-reload hoặc versioned deploy.
- [ ] Import `inventory_slot` / warehouse từ legacy SQL — sanitize socket cột.
- [ ] QA seed: không seed socket bytes lên cánh/test wing trong `dev-seed`.

---

## G. QA smoke (Android `TakumiErrorReport`)

| Kịch bản | Kỳ vọng |
|----------|---------|
| Hover **cánh 3** trong kho | Không có block "Socket 1…5" |
| Hover **kiếm socket** thật | Có dòng socket đúng `SocketItemType.txt` |
| Bless → Fenrir hỏng | `0x28` + `0x2A`, log `[m7] fenrir repair` |
| Mở kho sau join | `F3 10` — item không socket ảo |

---

## Thứ tự đề xuất (2–3 sprint nhỏ)

1. **A + B cơ bản** — sanitize, tooltip client, jewel bless/soul (đang làm).
2. **D + E** — trade/warehouse ổn định, stack, muun nếu cần.
3. **C + F** — socket gameplay + ETL sạch dữ liệu cũ.

---

## Ghi chú lỗi "cánh có socket" (2026-05-19)

**Triệu chứng:** Cánh (vd. Cánh Tinh Thần, `type=6183`) hiện 5 dòng socket Lửa trong tooltip.

**Nguyên nhân kép:**

1. **Client:** `IsSocketItem(ITEM*)` trả `TRUE` nếu *bất kỳ* byte socket = `0xFF` — trong khi item thường *đúng chuẩn* cũng có `0xFF` ở cột 7–11 → nhánh tooltip socket bật sai.
2. **Server/DB:** Blob 12-byte cánh có byte 7–11 ≠ `FF` (dữ liệu cũ / import) → `NewUIItemMng` parse ra `SocketCount` > 0 và vẽ seed.

**Sửa:** client parity Pegasus; server `ItemWireSanitizer` trước `F3 10` / join load.
