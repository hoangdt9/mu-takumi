# Chạy MU server trên macOS (đúng hướng dẫn VM + IP 192.168.99.200)

`MuServer` là **Windows (.exe + .bat + ODBC/SQL)** — **macOS không chạy trực tiếp**. Trên **Mac Apple Silicon**, máy ảo VMware kiểu `VMWare/BNS-2020` (Intel) **không bật được**; đường ổn định là **Windows trong VM ARM** hoặc **máy Windows thật / xa**.

### Chỉ có `takumi/MuServer/` — chạy trên Mac được bằng cách nào?

1. **Cài Windows 11 ARM** trong **Parallels Desktop** (dễ dùng, shared folder) hoặc **UTM** (miễn phí — tự tải ISO Windows 11 ARM).
2. Trong guest: đặt **`D:\takumi\MuServer`** trùng với `Start_192.168.99.200.bat` — copy cả **`takumi/MuServer`** vào đó *(hoặc shared folder đổi ổ D:, hoặc sửa dòng `set MUSERVER=...` trong `.bat`)*.
3. **SQL + ODBC**: cài/driver đúng bộ Database mà DataServer dùng (thường team đã có sẵn trong môi trường gốc; nếu thiếu cần hỏi team hoặc restore DB backup).
4. **`Start_192.168.99.200.bat`** → **Run as administrator**; chỉnh firewall nếu cần.
5. Client/Android trỏ tới **IP VM** và port đã mở (xem các mục mạng bên dưới).

**Docker + Wine trên Apple Silicon**: thường không chạy ổn (đã thử trong repo) — không nên làm hướng chính.

**Nhanh nhất** nếu có PC Windows: copy **`MuServer`** sang đó, cài SQL/ODBC, chạy `.bat`. Mac vẫn dùng để build client Android.

---

Trên Windows, anh/em dùng **hai thứ khác nhau** — đừng trộn:

| Thư mục | Nội dung | Việc cần làm |
|---------|----------|---------------|
| Thư mục có **`BNS-2020.vmx`**, `.vmdk` | **Máy ảo VMware Intel (x86)** | Chỉ bật được trên **Mac Intel** hoặc **VMware trên Windows** — **Mac Apple Silicon: không chạy** (lỗi ARM vs x86, KB 84273). |
| **`MuServer`** có `1.ConnectServer`, `2.DataServer`, `Start_*.bat` | **Bộ server MU (Windows)** | Chạy **bên trong** Windows của VM: `Start_192.168.99.200.bat` **Run as administrator**. |

**Cấu trúc gợi ý:**

- **`takumi/VMWare/`** — máy ảo **`BNS-2020.vmx`** + `.vmdk`.
- **`takumi/MuServer/`** — `Start_*.bat`, các thư mục **`1.ConnectServer`**… và **`.exe`**. Chạy **trong Windows** (ổ **`D:\\takumi\\MuServer`** trong VM như trong `.bat`) hoặc dùng thư mục này làm runtime cho Docker/Wine trên Mac.

---

## Mac Apple Silicon (M1/M2/M3…): không bật được VM BNS-2020 (`VMWare/`)

Nếu VMware Fusion báo: **X86 machine architecture** incompatible with **Arm** host (KB **84273**), thì **BNS-2020** là VM **Intel-only**. **Fusion trên Mac ARM không chạy** máy ảo VMware x86 dạng này — không có tùy chọn “ép chạy” an toàn.

**Trên Silicon, coi folder `VMWare/` là không dùng được**, trừ khi bạn chuyển sang **máy Intel / Windows PC** rồi mở bằng VMware Workstation / Fusion Intel.

### Lựa chọn thực tế trên Mac ARM

| Cách | Ghi chú |
|------|--------|
| **PC Windows / máy Intel** | Chạy `MuServer` hoặc máy ảo VMware x86 ở đó. |
| **Windows 11 ARM (Parallels, UTM, …)** | Tạo VM **Windows ARM mới**, **không** dùng `.vmx` BNS. Copy **`takumi/MuServer`** vào trong guest (đặt **`D:\\takumi\\MuServer`** hoặc sửa `.bat`/ODBC). `.exe` Win32 có thể chạy qua lớp dịch x86 của Windows — **cần test** (đặc biệt ODBC, anti‑hack). |
| **Docker + Wine** | Trên Silicon thường vỡ (QEMU/Wine); xem comment trong `docker/docker-compose.yml`. |

---

## 1. Cài VMware Fusion (Mac)

- **Tải chính thức từ Broadcom / VMware** (Fusion yêu cầu đăng nhập tải; cask Homebrew `vmware-fusion` thường bị tắt / cần auth).
- Cài xong, kéo **VMware Fusion** vào **Applications** và mở một lần.

Sau khi cài, mở Fusion một lần để chấp nhận điều khoản / cấp quyền.

---

## 2. Mở máy ảo BNS-2020

1. Mở **VMware Fusion** → **File → Open…**
2. Chọn file:

   ` /Users/hoangmac/Project/MU/takumi/VMWare/BNS-2020.vmx `

3. **Play** để bật Windows guest.

*(Có thể double-click `BNS-2020.vmx` trong Finder nếu `.vmx` đã gắn mở bằng Fusion.)*

---

## 3. Mạng: client kết nối theo IP VM

Ý “**IP 192.168.99.200**” là IP **mà bộ server trong Windows** đang cấu hình (theo tên file `Start_192.168.99.200.bat`).

- **Cách đơn giản (mạng nhà / Wi‑Fi):** Fusion → VM **Settings → Network** → **Bridged** (hoặc **Share with my Mac** nếu chỉ test từ Mac). Gán **IP tĩnh trong Windows** trùng subnet nhà bạn (ví dụ `192.168.1.x`), rồi **sửa lại** IP trong config server + client cho khớp — *hoặc* giữ subnet `192.168.99.0/24` như bên dưới.
- **Giữ đúng 192.168.99.200:** Trong Fusion tạo **custom / host-only** `192.168.99.0/24`, gán cho card mạng ảo của Windows địa chỉ tĩnh **`192.168.99.200`**, gateway/DNS tùy mô hình. Máy Mac hoặc PC client cần route tới `192.168.99.x` (thêm interface hoặc static route) nếu không cùng subnet.

Sau khi VM lên, trong **Windows (CMD):** `ipconfig` → dùng **IPv4** đó cho client **nếu** bạn đã đổi IP trong server thay vì .200.

---

## 4. Start server trong Windows (Run as administrator)

Trong **Windows của VM**, vào thư mục kiểu:

`D:\takumi\MuServer` (đúng như ảnh em gửi)

1. Chuột phải **`Start_192.168.99.200`** → **Run as administrator**
2. Đợi Connect / Data / Join / Game lên (theo batch).

Nếu lỗi firewall: bật rule cho các `.exe` server hoặc tắt firewall tạm trong VM để test.

---

## 5. Client (PC / Android build Takumi)

- **PC client:** trỏ tới **IP thật của VM** (và port trong `ServerInfo` / config client) — trùng với IP đã cấu hình trong server.
- **Android (source Takumi):** trong code có chuỗi placeholder `192.168.99.200`; khi test thật, đổi hoặc nhập IP VM trong app / file cấu hình tương ứng (xem `Source/5.Main/source/android_main.cpp` và `PreloadActivity` / `GameConfig` theo doc android).

---

## 6. Build exe trên Windows, “ném” vào server

Như anh nói: build server từ **Visual Studio** (các solution trong `Source/1.ConnectServer` …), copy `.exe` (và file data cần thiết) **vào đúng thư mục `MuServer` trong Windows** (VM), ghi đè bản cũ, rồi chạy lại `Start_*.bat` (hoặc stop/start từng service nếu batch có).

---

## Script tiện trên Mac (sau khi đã cài Fusion)

Từ thư mục `takumi`:

```bash
./scripts/open-mu-vm.sh
```

Chỉ mở file `.vmx`; **không** thay cho bước chạy `.bat` trong Windows.
