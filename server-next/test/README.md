# Takumi server-next — QA test pack (M7–M10)

Checklist từng bước để test trên **Mac LAN + APK Android** sau khi merge `main` (`b882608` trở đi).

**Thiết bị đã cấu hình (ví dụ của bạn):**

| Thành phần | Giá trị |
|------------|---------|
| Mac LAN IP | `192.168.1.50` (`server-next/.env` → `TAKUMI_PUBLIC_HOST`) |
| Connect | `192.168.1.50:44605` |
| Login / game (một socket) | `192.168.1.50:44606` |
| GameHost (M6 split, nếu bật) | `192.168.1.50:55901` |
| data.zip | `http://192.168.1.50:18080/data.zip` |
| Postgres (host) | `127.0.0.1:54444` |
| Tài khoản mặc định | `test` / `test` hoặc `admin` / `admin` |

## Thứ tự chạy

1. **[00-prerequisites.md](./00-prerequisites.md)** — APK, `.env`, tắt stack OpenMU cũ  
2. **[01-docker-stack.md](./01-docker-stack.md)** — rebuild Docker + xem log  
3. **[02-sql-and-etl.md](./02-sql-and-etl.md)** — SQL 004–006 + import M8 (tuỳ chọn DB)  
4. **[M7-vitals-qa.md](./M7-vitals-qa.md)** — HP/MP/zen, Life/Mana wire  
5. **[M8-etl-qa.md](./M8-etl-qa.md)** — monster_spawn, gates, shops (Postgres)  
6. **[M9-monster-combat-qa.md](./M9-monster-combat-qa.md)** — viewport, hit, die, destroy  
7. **[M10-presence-qa.md](./M10-presence-qa.md)** — player position/action broadcast  
8. **[android-logcat.md](./android-logcat.md)** — filter logcat + script có sẵn  

## Lệnh nhanh (từ `server-next/`)

```bash
# Stack + log (foreground)
./scripts/docker-stack.sh --host-build --recreate

# Stack nền, log riêng
./scripts/docker-stack.sh --host-build --recreate --detach
docker compose logs -f legacy-login postgres datazip game-host

# Log Android
./scripts/watch-android-takumi-log.sh
```

## Không cần rebuild APK khi

Chỉ đổi C# server, SQL, `.env` → **chỉ recreate Docker** (xem `docs/IMPLEMENTATION-CHECKLIST.md` § *Client APK, data.zip, and Docker*).

## Tài liệu gốc

- `docs/M7-CHARACTER-PERSISTENCE-CHECKLIST.md`
- `docs/M8-M10-WORLD-RUNTIME-CHECKLIST.md`
- `docs/M9-NPC-MONSTER-CHECKLIST.md`
- `docs/IMPLEMENTATION-CHECKLIST.md`
