# Nhật ký phát triển — 2026-05-14

Tổng hợp **phiên làm việc gần nhất** trên client native Android/iOS (IME, modal, xóa nhân vật) và liên kết QA với **`server-next`**. Phiên **2026-05-12** (màn chọn nhân vật, touch → game): vẫn tham chiếu [`DEVELOPMENT-LOG-2026-05-12.md`](DEVELOPMENT-LOG-2026-05-12.md).

## Đã commit trên `main` (2026-05-13)

Các commit gần nhất trên repo (trích ý nghĩa, không liệt kê đủ diff):

- **Luồng vào game sau chọn nhân vật:** cập nhật `ZzzScene` / UI — double-tap lên nhân vật, click vào nhân vật, menu ESC hoạt động, chọn nhân vật ổn định hơn.
- **Mạng LAN + login:** chỉnh bootstrap server (xem `docs/journal/DEVELOPMENT-LOG-2026-05-12.md` và `server-next` README) để máy dev trong LAN kết nối đúng cổng/host.

## Working tree (chưa `git commit` tại thời điểm ghi nhận — 2026-05-14)

Các file đang sửa (đối chiếu `git status`): `LoginWin.cpp`, `MsgWin.cpp` / `MsgWin.h`, `NewUISystem.cpp`, `UIMng.cpp`, `android_main.cpp`, `MuMainNativeActivity.java`, `server-next/takumi-roster/test.json`.

### IME / focus (Android & iOS)

- **`CUIMng::ShowWin`:** gọi `Active(true)` **ngay** khi hiện cửa sổ, tránh trường hợp modal (`MsgWin`, …) chưa active trong cùng một pass `Update` → phím / IME không ăn đến frame sau.
- **`CUIMng::Update`:** khi đóng cửa sổ trên cùng (`Active(false)`), nếu vẫn báo có text input Android thì `SetFocus(nullptr)` để gỡ kẹt IME.
- **`SEASON3B::CNewUISystem::Update`:** nếu `UpdateMouseEvent()` **không** tiêu thụ sự kiện chuột, có `IsLBtnDn()` và `AndroidHasFocusedTextInput()` → `SetFocus(nullptr)` (tap ra ngoài mọi panel Season3B, cùng ý đồ tap-outside legacy).
- **`CLoginWin::UpdateWhileShow`:** trên mobile, nếu không đang trên nút OK/Cancel (và các nút đăng ký nếu bật macro) mà vẫn `AndroidHasFocusedTextInput()` → `SetFocus(nullptr)` để tránh IME dính sau khi rời ô nhập.

### `SDL_FINGERDOWN` / tọa độ chuột ảo

- **`android_main.cpp`:** đồng bộ `UpdateMouseFromTouch` **trước** các handler có thể `return` sớm (chat, picker, …). Trước đây `MsgWin::UpdateWhileShow` có thể đọc **MouseX/Y cũ**, coi tap là “ngoài ô”, gọi `SetFocus(nullptr)` — bàn phím mở rồi tắt ngay (double-tap).

### Khởi tạo ô nhập SDL (`g_iChatInputType == 1`)

- **`android_main.cpp`:** sau khi tạo `CUIManager`, gọi `Init` cho `g_pMercenaryInputBox`, `g_pSingleTextInputBox`, `g_pSinglePasswdInputBox` (tương tự `Winmain.cpp`). Không có bước này thì `GiveFocus` trên modal mật khẩu/resident **không** mở được SDL text input trên Android.

### Đồng bộ IME sau cập nhật UI trong frame

- Logic `AndroidSyncImeWithFocusedTextInput()` được **rút ra** khỏi đầu `RunAndroidGameFrame` và gọi **sau** `Scene(...)` trong frame, để khớp focus mà UI vừa đặt trong cùng frame (tránh lệch một frame).

### IME action “Done” trên thanh bàn phím Android

- **`MuMainNativeActivity.java`:** JNI `nativeOnImeEditorAction(int actionCode)`.
- **`android_main.cpp`:** hàng đợi synthetic `SDL_KEYDOWN` / `SDL_KEYUP` cho `SDLK_RETURN` — cùng đường với phím Enter cứng cho các luồng UI đọc `WM_CHAR` / `SetEnterPressed`.

### `CMsgWin` — xóa nhân vật (`MESSAGE_DELETE_CHARACTER_RESIDENT`)

- **Captcha 6 chữ số phía client:** sinh ngẫu nhiên, hiển thị dòng hướng dẫn (ASCII) + mã ở dòng 2; `ManageOKClick` chỉ cho phép tiếp tục khi người chơi gõ **đúng 6 ký tự** (tránh gửi xóa với resident rỗng / sai định dạng). Gói tin gửi server vẫn qua `RequestDeleteCharacter()` → `SendRequestDeleteCharacter` với resident lấy từ `g_pSinglePasswdInputBox->GetText(InputText[0])` khi `g_iChatInputType == 1`.
- **`SetMsg`:** dùng `snprintf` vào buffer hai dòng; **không** dùng `strncpy_s(..., _TRUNCATE)` trên Android (macro `_TRUNCATE` kiểu MSVC khiến Bionic FORTIFY coi count quá lớn và có thể abort).
- **`SetCtrlPosition`:** `SetSize` cho `g_pSinglePasswdInputBox` theo kích thước sprite ô nhập (virtual 640×480).
- **`UpdateWhileShow`:** tap vào vùng hit mở rộng (sprite + padding) để `GiveFocus` ngay frame PopUp; tap ngoài (không phải nút) + đang có IME → `SetFocus(nullptr)`.
- **`AndroidTryFocusDeleteResidentInput`:** gọi từ `HandleVirtualFingerDown` **trước** virtual pad để touch không bị nuốt mất.
- **`InitResidentNumInput`:** `InitResidentNumInput()` gọi **sau** `ShowWin` trong `PopUp`; giới hạn `InputTextMax` / text box = 6; `UIOPTION_NUMBERONLY`; trên Android/iOS **không** `GiveFocus` ngay trong `Init` (tránh race với touch).
- **`ManageCancelClick`:** xóa text + ẩn passwd box; `MU_MobileStopTextInput` cho flow xóa nhân vật.

### Dữ liệu roster thử nghiệm

- **`server-next/takumi-roster/test.json`:** bổ sung thêm mẫu nhân vật (`dk002`, `ef001`, …) phục vụ script/QA roster (không thay đổi giao thức server).

## Build / QA

- Mọi thay đổi dưới `Source/5.Main` và Java JNI → **rebuild APK** (Gradle flavor đang dùng).
- Kiểm thử gợi ý: login → chọn nhân vật → vào game (đã log 2026-05-12); thêm: **SysMenu / Season3B panel** tap outside tắt IME; **xóa nhân vật** (nếu bật trên build) — captcha 6 số, Done trên bàn phím, hủy modal; xác nhận server `F3 02` vẫn nhận resident đúng độ dài Takumi.

## Tài liệu liên quan

- Checklist host: [`../server-next/docs/milestones/IMPLEMENTATION-CHECKLIST.md`](../server-next/docs/milestones/IMPLEMENTATION-CHECKLIST.md)
- Mac + Docker + APK: [`ANDROID-DEV-MAC.md`](ANDROID-DEV-MAC.md)
