# 01 — Docker stack + logs

Chạy từ thư mục **`server-next/`**.

## 1. Dừng stack cũ (nếu có)

```bash
cd /Users/hoangmac/Project/MU/takumi/server-next
docker compose down
```

- [ ] Không còn container `takumi-legacy-login`, `takumi-next-postgres` đang chiếm cổng.

## 2. Build host (tuỳ chọn, nhanh bắt lỗi compile)

```bash
./scripts/docker/docker-stack.sh --host-build --recreate --detach
```

Hoặc **foreground + log** (khuyến nghị lần đầu):

```bash
./scripts/docker/docker-stack.sh --host-build --recreate
```

- [ ] Thấy `COMPOSE_PROFILES` gồm `datazip` (mặc định).
- [ ] Nếu `.env` có `TAKUMI_GAME_PORT=55901` → profile **`gamehost`** được thêm → container `takumi-game-host`.

## 3. Đợi legacy-login sẵn sàng (~1–3 phút)

Trong log tìm:

- [ ] `[legacy-login] build OK — exec LegacyLoginHost`
- [ ] Dòng listen Connect `44605` và login `44606`
- [ ] (Nếu M8 DB) `[m8]` / `InitMonsterSpawnIfEnabled` / `MapGateCatalog`
- [ ] Postgres: `database system is ready to accept connections`

## 4. Kiểm tra cổng

```bash
./scripts/smoke/check-takumi-ports.sh
docker compose ps
```

| Service | Port host | Pass |
|---------|-----------|------|
| legacy-login Connect | 44605 | [ ] |
| legacy-login Login | 44606 | [ ] |
| postgres | 54444 | [ ] |
| datazip | 18080 | [ ] |
| game-host (nếu bật) | 55901 | [ ] |

## 5. Tail log (nếu đã `--detach`)

```bash
docker compose logs -f legacy-login postgres datazip game-host
```

Filter grep hữu ích:

```bash
docker compose logs -f legacy-login 2>&1 | grep -E '\[m9\]|\[m10\]|\[m7\]|decrypted_rx|F3 03|0x13|0x26|combat|presence'
```

## 6. Smoke HTTP data.zip

```bash
curl -sI "http://192.168.1.50:18080/data.zip" | head -3
```

- [ ] `HTTP/1.1 200` (hoặc 206) — APK Preload có thể tải lại nếu xóa app data.
- [ ] Nếu **404**: đặt file `takumi/docker/data-zip/host/data.zip` (xem `docker/data-zip/host/README.md`). APK đã Preload trước đó **không cần** tải lại trừ khi xóa app storage.

## 7. Troubleshooting

| Triệu chứng | Gợi ý |
|-------------|--------|
| `errno=111` sau chọn server | Bật `gamehost` hoặc comment `TAKUMI_GAME_PORT` để một socket 44606 |
| Login OK, không quái | Xem M9 — file MonsterSetBase hoặc fallback Lorencia trong log `[m9]` |
| Build container lâu | Bình thường lần đầu; lần sau dùng `--recreate` chỉ legacy-login |
| Sai IP trên điện thoại | Sửa `.env` + rebuild APK |

## Pass criteria

`docker compose ps` → các service **Up**; log có **build OK** → **[02-sql-and-etl.md](./02-sql-and-etl.md)** (nếu dùng Postgres M8) hoặc thẳng **[M7-vitals-qa.md](./M7-vitals-qa.md)**.
