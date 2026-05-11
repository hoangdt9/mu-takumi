# Docker — SQL Server + (tuỳ chọn) MuServer Wine

## Trạng thái máy của bạn (log `anon_mmap_fixed` / `qemu` abort)

Đó là **Wine Win32 trong container `linux/amd64`** đang crash trên **Mac Apple Silicon** (Docker + QEMU). **Server .exe chưa thực sự chạy** trong container đó — mọi `ConnectServer.exe` đều **Aborted**.

- **Khắc phục thực tế cho MuServer `.exe`:** Windows (PC hoặc VM ARM), không phụ thuộc Wine trên ARM.
- **SQL Server trong Docker** (image Linux) **thường chạy được** trên Silicon (amd64 emulated) — dùng profile **`db`** bên dưới.

---

## 0) HTTP `data.zip` cho Android Preload (Docker)

Dùng khi muốn điện thoại/emulator tải `data.zip` từ máy bạn (LAN), giống flow `http://<IP>:18080/data.zip`.

1. Đặt file **`data.zip`** vào `docker/data-zip/host/data.zip` (xem `docker/data-zip/host/README.md`).
2. Chạy:

   ```bash
   cd takumi/docker
   docker compose --profile datazip up -d
   ```

3. Cổng host mặc định **`18080`** (đổi `DATA_ZIP_PUBLISH_PORT` trong `.env` nếu cần).
4. Build Android với URL đúng IP máy chạy Docker (mặc định Gradle: `http://192.168.1.50:18080/data.zip`), hoặc:

   ```bash
   ./gradlew :app:assembleRealDevicePreloadDefaultDebug -PmuDataZipLan=http://YOUR_LAN_IP:18080/data.zip
   ```

`PreloadActivity` thử **`DATA_ZIP_URL_LAN`** trước, sau đó mới tới URL công khai dự phòng.

---

## Chuẩn bị `.env`

```bash
cd takumi/docker
cp .env.example .env
# Sửa MSSQL_SA_PASSWORD cho đủ chính sách SQL Server và TRÙNG DataServer.ini (sa)
```

`DataServer.ini` hiện có `DataServerUSER` / `DataServerPASS` (`sa`/… ). **Đặt SA password của container khớp** với `DataServerPASS`, hoặc đổi file ini cho khớp `.env`.

---

## 1) Chỉ Microsoft SQL Server (khuyến nghị trên Mac ARM)

```bash
docker compose --profile db up -d
docker compose --profile db logs -f sqlserver
```

Hoặc: `./scripts/run-mssql-docker.sh up -d`

- Cổng host: **`1433`** (đổi `MSSQL_PUBLISH_PORT` trong `.env` nếu trùng SQL local).

### Restore `MuOnline.bak`

Khi healthcheck **healthy**:

```bash
chmod +x sql/restore-muonline.sh
./sql/restore-muonline.sh
```

Nếu `RESTORE` báo sai tên logical file, dùng `RESTORE FILELISTONLY` (lệnh có trong comment script) rồi sửa `MOVE` trong `sql/restore-muonline.sh`.

### ODBC / client Windows nối vào DB Docker

- **Trên Windows** (máy thật hoặc VM), tạo DSN **`MuOnline`** / **`MuOnlineJoin`** trỏ **SQL Server** tới:
  - **Cùng máy Docker Desktop:** thường **`127.0.0.1,1433`** hoặc **`host.docker.internal`** tùy mạng.
- `JoinServer.ini` có `ConnectServerAddress` — khi chạy full server trên Windows, giữ `127.0.0.1` nếu Connect cùng máy.

**ODBC bên trong Wine** (container `muserver`) cần **registry + driver Windows trong Wine** — rất hay lỗi; trên ARM Wine còn chưa chạy được nên **chưa tới bước ODBC trong container**.

---

## 2) MuServer qua Wine (profile `wine`)

Chỉ nên thử trên **Linux x86_64** hoặc **Mac Intel** / Docker Rosetta ổn định:

```bash
env -u DOCKER_DEFAULT_PLATFORM docker compose --profile wine up --build -d
docker compose --profile wine logs -f muserver
```

Hoặc: `./scripts/run-muserver-docker.sh up --build -d`

Cần mount **`MuServer`** đầy đủ `.exe` + `Data` + `.ini` (mặc định `../MuServer`).

### Cả SQL + Wine (máy hỗ trợ Wine ổn)

```bash
env -u DOCKER_DEFAULT_PLATFORM docker compose --profile db --profile wine up --build -d
```

Trong mạng nội bộ Compose, hostname **`sqlserver`** (tên service). Cấu hình ODBC trong Wine (khi Wine chạy được) phải trỏ **`sqlserver,1433`**.

---

## Biến môi trường — Wine / MuServer

| Biến | Mặc định | Ý nghĩa |
|------|-----------|---------|
| `MU_SERVER_HOST_PATH` | `../MuServer` | Mount host → `/MuServer` |
| `START_XSHIELD` | `0` | `1` = chạy XShield |
| `START_SECOND_GAMESERVER` | `1` | `0` bỏ GameServer thứ hai |
| `START_DELAY_SECONDS` | `3` | Chờ giữa các process |
| `WINEDEBUG` | `-all` | `+err` khi debug |

## Biến — SQL (`docker/.env`)

| Biến | Ý nghĩa |
|------|---------|
| `MSSQL_SA_PASSWORD` | Bắt buộc; khớp cấu hình `sa` trong `DataServer.ini` |
| `MSSQL_PUBLISH_PORT` | Mặc định `1433` |

---

## Không thay được

Đây vẫn là **Wine + exe**, không native macOS. Kế hoạch server portable: **`docs/SERVER-PORT-PLAN.md`**.
