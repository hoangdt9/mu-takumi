# All-in-one container (Takumi `server-next`)

Một image Debian chạy **PostgreSQL** + **LegacyLoginHost** (Connect + login) + **GameHost** (cổng game M6), giám sát bởi **supervisord** — gần với tinh thần OpenMU *all-in-one* (một stack nhỏ, dễ chạy QA).

## Yêu cầu

- Docker / Docker Compose v2  
- Thư mục host chứa **`Dec2.dat`** (mount vào `/keys` trong container)

## Chạy

Từ thư mục `server-next/`:

```bash
cp -n .env.lan.example .env
# Sửa .env: TAKUMI_LAN_IP = IP LAN (máy chạy Docker) mà điện thoại tới được

docker compose -f docker-compose.all-in-one.yml up -d --build
docker compose -f docker-compose.all-in-one.yml logs -f
```

Cổng mặc định (publish ra host):

| Dịch vụ | Trong container | Host (mặc định) |
|---------|-------------------|-----------------|
| PostgreSQL | 5432 | 54444 |
| Connect (F4…) | 44605 | 44605 |
| Login / character | 44606 | 44606 |
| Game TCP (M6) | 55901 | 55901 |

Override bằng biến trong `.env`: `TAKUMI_POSTGRES_PUBLISH_PORT`, `TAKUMI_CONNECT_PUBLISH`, `TAKUMI_LEGACY_LOGIN_PUBLISH`, `TAKUMI_GAME_PUBLISH`, `TAKUMI_DEC2_HOST_DIR`, v.v.

## Đã “public LAN” chưa? (client báo không kết nối được)

**Trong `docker-compose.all-in-one.yml`, cổng đã publish dạng `HOST:CONTAINER` mặc định là bind tất cả interface của máy host** (Docker Desktop hiển thị `0.0.0.0:44605`…). **Không** cần thêm flag riêng để “mở LAN”; điện thoại cùng Wi‑Fi vẫn phải trỏ tới **IP LAN của máy Mac** (ví dụ `192.168.x.x`), **không** dùng `127.0.0.1`.

Checklist khi client vẫn báo không kết nối:

1. **`.env` → `TAKUMI_LAN_IP`** phải **trùng** IP Wi‑Fi của Mac (để gói Connect **F4 03** gửi đúng IP/port tới client). Tuỳ chọn **`TAKUMI_PUBLIC_HOST`** nếu cần IP F4 03 khác. Nếu để nhầm IP (ví dụ copy mẫu nhưng Mac thật là IP khác), client có thể **TCP tới sai máy** sau khi chọn server.
2. **Client / APK** phải trỏ **Connect `44605`** (và sau đó login **`44606`**) đúng IP Mac — xem `docs/ANDROID-DEV-MAC.md` và chỗ cấu hình IP trong native (`GameConfig`, v.v.).
3. **Tường lửa macOS:** System Settings → Network → Firewall (hoặc tương đương) — cho phép **Docker Desktop** nhận kết nối đến.
4. **Điện thoại và Mac cùng mạng Wi‑Fi** (không VPN chặn, không guest Wi‑Fi cô lập client-to-client).
5. Trên Mac: `lsof -nP -iTCP:44605 -sTCP:LISTEN` và `44606` — phải thấy listener; thử từ máy khác trong LAN: `nc -vz <IP-Mac> 44605`.

## PostgreSQL trong Docker Desktop

Bạn **sẽ không** thấy container tên `postgres` riêng: DB chạy **bên trong** `takumi-server-next-aio`. Cổng **54444→5432** trên host là Postgres (`Host=…;Port=54444` trong connection string).

## `data.zip` (Android preload)

Không nằm trong image aio. Cùng file compose, bật profile **`datazip`** (thêm nginx, mặc định **18080**):

```bash
./scripts/docker-up-all-in-one.sh --with-datazip
# hoặc: COMPOSE_PROFILES=datazip docker compose -f docker-compose.all-in-one.yml up -d --build
```

Cần file `../docker/data-zip/host/data.zip` (xem `docker-compose.yml` gốc).

## Build image thủ công

```bash
cd server-next
docker build -f deploy/all-in-one/Dockerfile -t takumi/server-next-aio:local .
```

## Ghi chú

- **Không** chạy đồng thời `docker-compose.yml` nếu trùng cổng publish.  
- Nếu aio từng crash khi init DB: `docker compose -f docker-compose.all-in-one.yml down -v` rồi `up --build` lại (volume `takumi_aio_pgdata` trống mới initdb sạch).  
- Mật khẩu Postgres trong `.env` không nên chứa ký tự `'` (hạn chế của script bootstrap SQL).
