# Kế hoạch cải thiện `data.zip` — merge Data từ các bộ source

**Trạng thái:** backlog (chưa làm đủ pipeline; chỉ có patch thủ công đầu tiên)  
**Ngày:** 2026-05-18  
**Mục tiêu:** Một bundle `data.zip` **đủ file** cho client Takumi (Android + PC `ClientBuild_*`), bổ sung từ các repo reference khi bản build hiện tại thiếu hoặc lỗi thời — **không** commit toàn bộ client MU proprietary vào git.

---

## Vì sao cần làm

Client Takumi đọc asset từ thư mục `Data/` (Android: preload HTTP `data.zip` → giải nén). Thiếu file gây:

- Map đen / terrain không vẽ (texture world lệch sau char select).
- Logcat `File not found Data\WorldXX\...` (thường là **OZT/OZJ**, không phải `.tga` thật).
- Model/item BMD thiếu, UI texture lỗi.

Ví dụ đã gặp (2026-05): `World75/leaf01.ozt` có trong MuMain-5.2 nhưng **không** có trong `ClientBuild_*/Data/World75` và `data.zip` → load lá cây fail khi đổi map 74↔75.

---

## SSOT (single source of truth) đề xuất

| Lớp | Vai trò | Ghi chú |
|-----|---------|---------|
| **`ClientBuild_*/Data/`** (local, gitignore) | Cây Data “đầy đủ” trên máy dev | Nguồn để đóng `data.zip` |
| **`assets/data-patches/`** (git) | Chỉ file **nhỏ / thiếu** đã audit | Merge vào ClientBuild trước khi zip |
| **`docker/data-zip/host/data.zip`** | File HTTP cho Android Preload | Copy từ ClientBuild sau repack |
| **Reference repos** (không commit) | MuMain, Pegasus, ThangCuoi, … | Đọc-only khi audit/merge |

**Không** dùng trực tiếp `data.zip` trên điện thoại làm SSOT — khó diff và dễ stale.

---

## Bộ source reference (đường dẫn gợi ý)

Các repo thường có `Data/World*` hoặc `Client/Data/World*`:

| Repo | Đường dẫn Data client | Ghi chú |
|------|------------------------|---------|
| **MuMain-5.2** | `src/bin/Data/` | Season 6 client data; nhiều world đủ OZT/OZJ |
| **muonline-bernat-main** / **muonline-xulek** | `Data/` (theo cấu hình MonoGame) | Season 20 — **không** merge mù vào Takumi S6 trừ khi đã map |
| **Source Pegasus 5.2** | `Client/Data/` | Gần bản Takumi C++ |
| **SRC ThangCuoi** | `Client/Data/` hoặc tương đương | Kiểm tra season trước khi copy |
| **OpenMU** | Không phải client `Data/` đầy đủ | Chủ yếu server; dùng cho protocol/config, không phải zip terrain |
| **takumi `ClientBuild_*`** | `Data/` | Baseline hiện tại |

Luôn ghi **repo + commit/path** trong manifest khi copy một file vào `assets/data-patches/`.

---

## Định dạng file client (khi audit)

Client map tên logic → file thật:

| Code gọi | File thường có trên đĩa |
|----------|-------------------------|
| `*.tga` | `*.ozt` (alpha TGA packed) |
| `*.jpg` | `*.ozj` (JPEG packed) |
| `EncTerrainNN.map` | Heightmap |
| `EncTerrainNN.att` | Walkability |
| `TileGround01.ozj` + `TileGround01` tileset | Terrain render |

Loader: `GlobalBitmap::LoadImage` — `MapManager.cpp` load world; Android: `OpenTga` / `OpenJpeg` tìm OZT/OZJ.

---

## Hiện trạng trong repo (2026-05-18)

| Thành phần | Mô tả |
|------------|--------|
| `assets/data-patches/World74`, `World75`, `World1`, `World58` | Patch lá cây từ MuMain-5.2 |
| `assets/data-patches/Monster/Monster03.bmd` | Rồng Con classic S6 (~88KB); thay mesh S20 ~268KB |
| `scripts/apply-data-patches.sh` | Copy patch → `ClientBuild_*/Data`; `--repack-zip` rebuild zip |
| `scripts/sync-data-patches-android.sh` | `adb push` patch → `Android/data/.../files/Data/` (khi `DEV_SKIP_DATA_ZIP`) |
| `assets/data-patches/README.md` | Ghi chú patch nhỏ |
| Sửa code | `MapManager` load leaf: ozt → ozj → tga → jpg; `GlobalBitmap` nhận `.ozt`/`.ozj` |

**Chưa có:** audit toàn bundle, diff tự động, CI, manifest đầy đủ.

---

## Quy trình merge (làm sau — từng phase)

### Phase 0 — Baseline

1. Chọn một `ClientBuild_<LAN>/Data` làm baseline (hoặc export từ máy đang chạy OK).
2. Liệt kê world bắt buộc cho QA Takumi: login (**World73**), char select (**World74/78**), map gameplay (**World1–4**, … theo seed QA).
3. Ghi version / nguồn gốc baseline (ngày, máy, client gốc).

### Phase 1 — Audit thiếu file

Mục tiêu: danh sách `MISSING` / `SIZE_MISMATCH` so với reference.

Gợi ý lệnh (chạy local, chỉnh `BASE` / `REF`):

```bash
BASE="/path/to/takumi/ClientBuild_192.168.99.200/Data"
REF="/path/to/MuMain-5.2/src/bin/Data"

# So sánh một world (ví dụ World75)
diff <(cd "$BASE/World75" && find . -type f | sort) \
     <(cd "$REF/World75" && find . -type f | sort)

# Tìm file có ở REF mà không có ở BASE (toàn Data)
comm -23 \
  <(cd "$REF" && find . -type f | sed 's|^\./||' | sort) \
  <(cd "$BASE" && find . -type f | sed 's|^\./||' | sort) \
  > /tmp/missing-from-base.txt
```

Ưu tiên audit:

- `World74`, `World75`, `World78` (char select).
- `World4` (Noria — map id 3 gameplay).
- `Item/`, `Monster/`, `Player/` (BMD wear) — nếu thiếu gây crash khi equip.
- `Dec1.dat`, `Dec2.dat`, `Enc1.dat` (crypto login).

**Output mong muốn:** `docs/manifests/TAKUMI-DATAZIP-AUDIT.csv` (đường dẫn, kích thước BASE/REF, quyết định copy/skip).

### Phase 2 — Quy tắc merge

| Quy tắc | Chi tiết |
|---------|----------|
| **Ưu tiên reference cùng season** | Takumi client S6 → MuMain S6 / Pegasus trước; tránh xulek S20 trừ file đã xác nhận tương thích |
| **Chỉ copy khi thiếu hoặc size=0** | Không ghi đè file lớn tùy tiện (terrain, BMD) nếu baseline đã chạy |
| **Patch nhỏ → `assets/data-patches/`** | File &lt; ~50KB hoặc đã review; kèm dòng manifest |
| **File lớn / hàng loạt** | Chỉ cập nhật `ClientBuild` local + `data.zip`; **không** commit vào git |
| **Không merge** | `*.log`, `Thumbs.db`, config máy cá nhân, account, SQL |

### Phase 3 — Apply + repack

```bash
# 1) Copy patch đã commit
./scripts/apply-data-patches.sh

# 2) Copy thủ công từ REF (ví dụ) — làm sau khi có audit list
# cp -n "$REF/World75/leaf01.ozt" "$BASE/World75/"

# 3) Repack zip (ClientBuild có thể cần zip qua /tmp nếu permission)
cd ClientBuild_192.168.99.200
zip -qr /tmp/takumi-data.zip Data -x "*.DS_Store"
cp -f /tmp/takumi-data.zip data.zip
cp -f data.zip ../docker/data-zip/host/data.zip
```

### Phase 3b — Sync patch lên Android (`adb`) — nhanh cho dev

Dùng khi APK dev **bỏ qua tải** `data.zip` (`DEV_SKIP_DATA_ZIP` + marker `.mu_data_ready_v1` — xem logcat `MuPreload: DEV_SKIP_DATA_ZIP`). Điện thoại vẫn giữ `Data/` cũ (Season 20) trong khi `ClientBuild` / `docker/data-zip/host/data.zip` đã được patch.

**Triệu chứng thường gặp:** Nhện / Rồng Con Lorencia model lạ; `Monster03.bmd` trên máy ~268KB thay vì ~88KB (MuMain S6).

| Cách | Khi nào |
|------|---------|
| **`./scripts/sync-data-patches-android.sh`** | Chỉ cần vài file BMD/OZT; không muốn xóa app / tải lại zip lớn |
| **`apply-data-patches.sh --repack-zip` + tải lại zip** | Muốn đồng bộ toàn bộ `Data/` với ClientBuild |
| **Xóa storage app** | Buộc Preload tải lại `http://<LAN>:18080/data.zip` (build `datafresh` hoặc gỡ marker) |

**Đường dẫn trên máy (external storage, package mặc định `com.muonline.client`):**

```text
/storage/emulated/0/Android/data/com.muonline.client/files/Data/
```

Logcat `cwd=.../files` khi login — cùng cây thư mục này (không có thêm `files/files`).

**Script (khuyên dùng):**

```bash
# USB debugging bật; một thiết bị trong `adb devices`
cd /path/to/takumi

# Chỉ Monster/ (Monster03.bmd Rồng Con, …)
./scripts/sync-data-patches-android.sh

# Monster + patch World* (leaf OZT/OZJ)
./scripts/sync-data-patches-android.sh --worlds

# Cập nhật ClientBuild local rồi push từ assets/data-patches
./scripts/sync-data-patches-android.sh --apply-local

# Xem lệnh adb sẽ chạy
./scripts/sync-data-patches-android.sh --dry-run
```

**Lệnh `adb` thủ công (tương đương Monster):**

```bash
PKG=com.muonline.client
REMOTE=/storage/emulated/0/Android/data/${PKG}/files/Data

# Áp patch vào ClientBuild trên máy dev (tuỳ chọn)
./scripts/apply-data-patches.sh

CB="$(ls -d ClientBuild_*/Data 2>/dev/null | head -1)"
CB="${CB%/Data}"

adb shell mkdir -p "${REMOTE}/Monster"
adb push "${CB}/Data/Monster/Monster03.bmd" "${REMOTE}/Monster/Monster03.bmd"
adb push "${CB}/Data/Monster/Monster04.bmd" "${REMOTE}/Monster/Monster04.bmd"

adb shell ls -la "${REMOTE}/Monster/Monster03.bmd" "${REMOTE}/Monster/Monster04.bmd"
```

**Kích thước mong đợi (MuMain-5.2 / Pegasus S6):**

| File | Bytes (xấp xỉ) | Ghi chú |
|------|----------------|---------|
| `Monster03.bmd` | 87 994 | Class 2 Budge Dragon — client load `Monster` + (Type+1) |
| `Monster04.bmd` | 166 065 | Class 3 Spider |

**Sau khi push:** force-stop app → mở lại (BMD load lúc vào game). Vào Lorencia: Rồng Con nhỏ, Nhện 8 chân classic.

**Map class → file BMD:** server gửi `MonsterClass` trong `C2 0x13`; client `OpenMonsterModel(Type)` → `Data/Monster/Monster{Type+1}.bmd` (class 2 → `Monster03.bmd`, class 3 → `Monster04.bmd`).

**Không thay thế repack zip khi:** cần đồng bộ hàng trăm MB terrain/BMD mới — dùng Phase 3 `--repack-zip` + Phase 4 (xóa data app hoặc flavor `datafresh`).

### Phase 4 — Validate

| Kiểm tra | Cách |
|----------|------|
| Zip integrity | `unzip -t data.zip` |
| Entry quan trọng | `unzip -l data.zip \| grep -E 'World7[45]/leaf|Dec2.dat\|Monster/Monster03'` |
| Android preload | Xóa app data hoặc APK `datafresh`; tải `http://<LAN>:18080/data.zip` |
| Android adb patch | `./scripts/sync-data-patches-android.sh` → `ls -la` Monster03 ~87994 trên device |
| In-game | Login → char select → vào Noria (map 3): terrain không đen; logcat không lặp `File not found` cho world đang chơi |
| Monster model | Lorencia: Spider + Budge Dragon giống MU S6 (không mesh S20 lạ) |
| PC smoke | `ClientBuild` + `server-next` nếu có bản Win |

### Phase 5 — Phát hành nội bộ

- Cập nhật `TAKUMI_DATA_ZIP_URL` / nginx host nếu đổi máy.
- Ghi trong `DEVELOPMENT-LOG-*.md` hoặc PR: **hash/size zip**, worlds đã merge, reference commit.

---

## Automation (backlog — chưa implement)

| Task | Mô tả |
|------|--------|
| `scripts/audit-data-zip.sh` | Diff BASE vs REF → CSV |
| `scripts/merge-data-from-ref.sh` | Copy có `--dry-run`, `--only-world World75` |
| `scripts/sync-data-patches-android.sh` | `adb push` `assets/data-patches` → device `files/Data/` |
| Manifest generator | `TAKUMI-DATAZIP-MANIFEST.txt` (path, sha256, source repo) |
| CI (optional) | Chỉ verify patch folder + script syntax; **không** build zip 500MB trên CI |

---

## Liên kết tài liệu khác

| Doc | Nội dung |
|-----|----------|
| [`ANDROID-DEV-MAC.md`](ANDROID-DEV-MAC.md) | Preload `data.zip`, `DEV_SKIP_DATA_ZIP`, flavor `datafresh` |
| [`server-next/docs/docker/DOCKER-BUILD-RUN.md`](../server-next/docs/docker/DOCKER-BUILD-RUN.md) | Profile `datazip`, port 18080 |
| [`assets/data-patches/README.md`](../assets/data-patches/README.md) | Patch lá World74/75 |
| [`TAKUMI-FULL-FILE-MIGRATION-CHECKLIST.md`](TAKUMI-FULL-FILE-MIGRATION-CHECKLIST.md) | ClientBuild / season parity |
| [`game-spec/GAMESERVER-DATA-FOLDER-MAP.md`](game-spec/GAMESERVER-DATA-FOLDER-MAP.md) | Data **server** (khác client zip) |

---

## Checklist nhanh (khi bắt đầu sprint merge)

- [ ] Chốt `BASE` ClientBuild và `REF` (MuMain / Pegasus).
- [ ] Chạy audit Phase 1 → CSV thiếu file.
- [ ] Merge theo Phase 2; commit chỉ patch nhỏ vào `assets/data-patches/`.
- [ ] Repack + copy `docker/data-zip/host/data.zip` **hoặc** `sync-data-patches-android.sh` nếu dev skip zip.
- [ ] QA Android: char select + map 3 + Lorencia monsters + relog.
- [ ] Ghi log thay đổi zip (size, ngày, worlds).

---

## Ghi chú pháp lý / repo

- Asset MU là dữ liệu game gốc — giữ **ngoài git** trừ patch tối thiểu đã review.
- Repo public / chia sẻ: không upload `data.zip` đầy đủ; chỉ script + manifest + patch nhỏ.
