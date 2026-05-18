# Skill hotkey persistence (client ↔ server ↔ DB)

Takumi client (PC + Android) lưu **10 ô skill hotkey** (phím `0`–`9`, ô chính = slot `0`) và cấu hình Q/W/E/R + game options qua cùng một gói **30 byte**.

## Wire protocol (Season 6 / ENG)

| Hướng | Packet | Handler |
|--------|--------|---------|
| Client → GameServer | `C1 F3 30` + 30 bytes | `SendRequestHotKey` (`wsclientinline.h`) → `SaveOptions()` (`ZzzOpenData.cpp`) |
| GameServer → Client | `C1 F3 30` | `ReceiveOption` (`WSclient.cpp`) |

Layout 30 byte (client `SaveOptions`):

- `[0..19]` — 10 skill hotkeys, mỗi ô 2 byte = **skill type** (`CharacterAttribute->Skill[index]`), `0xFFFF` = trống.
- `[20]` — game options (auto attack, whisper sound, slide help).
- `[21..25]` — item hotkey Q/W/E/R + chat window.
- `[26..29]` — item level Q/W/E/R.

Client map skill **index** (0..`MAX_MAGIC-1`) ↔ server lưu **skill type** (WORD). `ReceiveOption` tra cứu `CharacterAttribute->Skill[j] == iHotKey` rồi `SetSkillHotKey(i, j)`.

**Thứ tự join bắt buộc:** server gửi **`F3 11`** (danh sách skill đã học) **trước** **`F3 30`** (hotkey blob). Nếu `F3 11` trống, hotkey đã lưu trong DB sẽ không khớp skill nào sau relog. Xem **M7j** — bảng `character_skill` + `JoinSkillLifecycle` trong `server-next`.

## Server backends

### Takumi GameServer + DataServer (C++)

- Nhận: `CGOptionDataRecv` → `GDOptionDataSaveSend` (`Protocol.cpp` `0xF3/0x30`).
- Trả về khi load nhân vật: `DGOptionDataRecv` → `PMSG_OPTION_DATA_SEND` (`DSProtocol.cpp`).

### OpenMU (.NET)

- Nhận: `CharacterKeyConfigurationPacketHandlerPlugIn` (`0xF3` / `0x30`).
- Lưu: `SaveKeyConfigurationAction` → `Character.KeyConfiguration` (byte[30], EF/Postgres).
- Áp lại client: `IApplyKeyConfigurationPlugIn` / `ApplyKeyConfiguration` khi vào game (cùng layout 30 byte).

## Client UI (Android legacy HUD)

| File | Vai trò |
|------|---------|
| `NewUIMainFrameWindow.cpp` | Ô skill chính (385×431), picker, hotkey 1–5, `ApplySelectedSkillIndex` |
| `ZzzOpenData.cpp` | `SaveOptions()` gửi lên server |
| `WSclient.cpp` | `ReceiveOption` hydrate sau login / reload |

**Ô skill chính (slot 0):**

- Chưa gán hotkey `0` → chỉ khung `newui_skillbox`, **không** vẽ icon (tránh texture lạ khi mở picker).
- Chọn skill từ picker → `ApplySelectedSkillIndex` → `SetSkillHotKey(0, index)` + `Hero->CurrentSkill` + `SaveOptions()`.

**Android touch:** `TryToggleSkillPickerAtTouch` (finger down) mở/đóng picker; chọn skill trong list → `ApplySelectedSkillIndex` + đóng picker.

## DB (server-next)

**Bảng:** `public.character_roster` — cột **`key_configuration`** (`bytea`, 30 byte).  
**Không** lưu trong `character_domain` (bảng đó là mirror stats/EXP/vị trí).

Chạy patch trước khi test (Docker Postgres mặc định cổng **54444**):

```bash
cd server-next
./scripts/apply-sql.sh "postgresql://takumi:takumi@127.0.0.1:54444/takumi_runtime" sql/patches/015_character_key_configuration.sql
```

Kiểm tra:

```sql
SELECT character_name, length(key_configuration) AS bytes
FROM character_roster WHERE account_login = 'test';
```

Khi client gửi `C1 F3 30`, `CharacterOptionHandler` cập nhật `character_roster.key_configuration` **ngay** (không đợi disconnect).

## QA checklist

- [ ] Nhân vật mới: ô skill chính trống; mở picker không có khung “đang chọn” (skillbox_use) nếu chưa gán skill.
- [ ] Chọn skill Độc → gán ô **0** (picker hoặc chạm ô 0 trên bar) → thoát game / vào lại → vẫn ở ô 0.
- [ ] Gán thêm hotkey 1–9 (Ctrl+phím trên PC hoặc phím sau khi pick trên Android) → relog vẫn giữ.
- [ ] Postgres: `SELECT key_configuration FROM character_roster WHERE character_name = '...';` sau khi gán skill.
- [ ] Takumi C++ GS: kiểm tra bảng option trên DataServer (theo `GDOptionDataSaveSend`).

## Tham chiếu PC

- MuMain / client gốc: cùng `SaveOptions` + `ReceiveOption` pattern.
- OpenMU: `src/GameLogic/PlayerActions/Character/SaveKeyConfigurationAction.cs`, `src/DataModel/Entities/Character.cs` (`KeyConfiguration`).
