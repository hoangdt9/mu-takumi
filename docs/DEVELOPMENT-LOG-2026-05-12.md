# Nhật ký phát triển — 2026-05-12

Tài liệu tổng hợp thay đổi client native (màn chọn nhân vật trên Android/iOS) và trỏ tới checklist server. Chi tiết build Mac: `ANDROID-DEV-MAC.md`.

## Client — màn hình chọn nhân vật (`CHARACTER_SCENE`)

**Mã chính:** `Source/5.Main/source/ZzzScene.cpp` (`CreateCharacterScene`, `NewMoveCharacterScene`, `CharacterSelect_UpdateTapRayForPick`, `StartGame`).

### IME / bàn phím ảo

- Trong `CreateCharacterScene()` (Android/iOS): gọi `MU_MobileStopTextInput()` và `SetFocus(nullptr)` để sau login không giữ cầu nối IME / ô Win32 giả khiến `android_main` gọi lại `StartTextInput`.
- Trong `NewMoveCharacterScene()`: nếu `m_CharSelMainWin` đang hiện, không mở `m_CharMakeWin`, và `AndroidHasFocusedTextInput()` vẫn true thì lặp lại `SetFocus(nullptr)` + `MU_MobileStopTextInput()` (trường hợp chuyển từ login sang chọn nhân mà không chạy lại `CreateCharacterScene`).

### Touch: double-tap / long-press để vào game (tương đương H.tất / nút Kết nối)

**Nguyên nhân gốc:** trong một frame, `RenderScene` → `UpdateSceneState()` gọi `g_pNewKeyInput->ScanAsyncKeyState()` rồi `NewMoveCharacterScene()` **trước** `MainScene()` → `CInput::Update()`. Cờ `CInput::IsLBtnDn()` **không** khớp thời điểm chạm trong bước Move; double-tap trên nhân vật gần như không kích hoạt nhánh xử lý.

**Cách xử lý:**

- Dùng `SEASON3B::IsPress(VK_LBUTTON)` và `SEASON3B::IsRepeat(VK_LBUTTON)` (cùng nguồn trạng thái phím đã quét trong frame) thay cho `IsLBtnDn()` cho nhánh mobile chọn nhân.
- Bypass `IsCursorOnUI()` khi ngón tay đang active (`IsPress` hoặc `IsRepeat`) và cửa sổ chọn nhân đang mở, không có modal chặn (Msg / tạo nhân / SysMenu / Option).
- **Double-tap:** hai lần nhấn trong cửa sổ ~0,75s, cùng slot; lần hai được phép ray miss nếu `SelectedHero` vẫn trùng slot lần đầu.
- **Long-press (~0,48s):** giữ trên vùng 3D (`!IsCursorOnUI()`), đã có `SelectedHero` hợp lệ → `StartGame()` giống luồng nút Connect.
- Nếu trong lúc giữ (`IsRepeat`) ngón tay trượt lên vùng UI, hủy anchor hold để tránh vào game nhầm khi kéo lại 3D.

### Ray pick 3D

- `CharacterSelect_UpdateTapRayForPick()`: lặp lại chuỗi camera / `BeginOpengl` / `CreateFrustrum` / `CreateScreenVector` / `EndOpengl` thống nhất với `NewRenderCharacterScene` trước `SelectCharacter(KIND_PLAYER)` để tia chọn khớp khung hình.

### Ghi chú build

- Mọi thay đổi dưới `Source/5.Main` → **cần rebuild APK** (Gradle `assembleRealDevicePreloadDefaultDebug` hoặc flavor tương ứng). Server-only không cần cài lại APK.

### File song song

- `Source/5.Main/source/Scenes/CharacterScene.cpp` có logic tương tự cũ dùng `IsLBtnDn()`; trong solution Windows hiện tại thường chỉ biên dịch `ZzzScene.cpp`. Nếu sau này gộp build dùng `CharacterScene.cpp`, cần đồng bộ cùng mô hình `SEASON3B::IsPress` / long-press.

## Server (`server-next`)

Không đổi schema hay handler trong phiên này; chỉ cập nhật checklist và liên kết tài liệu. MVP vẫn tham chiếu `server-next/docs/IMPLEMENTATION-CHECKLIST.md` (một session TCP login, join map sau `F3 03`, v.v.).

**Tiếp theo (phiên sau):** IME toàn cục trên Android, modal, xóa nhân vật, JNI — [`DEVELOPMENT-LOG-2026-05-14.md`](DEVELOPMENT-LOG-2026-05-14.md).
