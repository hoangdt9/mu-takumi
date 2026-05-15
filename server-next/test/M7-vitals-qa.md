# M7 — Character vitals (HP / MP / zen / Life-Mana wire)

**Phạm vi đã merge:** roster vitals, join `F3 03`, outbound `C1 0x26`/`0x27`, seed + disconnect save.

## Chuẩn bị

- [ ] Docker stack Up (**[01-docker-stack.md](./01-docker-stack.md)**).
- [ ] (Khuyến nghị) `TAKUMI_ROSTER_DB_SYNC=1` + SQL `004` applied.
- [ ] APK → server `192.168.1.50:44605`.

## Bước test trên client

### A — Join lần đầu (seed vitals)

1. [ ] Login `test` / `test` (hoặc `admin` / `admin`).
2. [ ] Chọn nhân vật → vào map.
3. [ ] Thanh HP/MP hiển thị hợp lý (không 0/0 vô cớ).

**Server log (legacy-login):**

```bash
docker compose logs legacy-login 2>&1 | grep -E 'F3 03|vitals|0x26|0x27|LifeMana|roster'
```

- [ ] Có gửi join map (`F3 03` / join length 131).
- [ ] Nếu `TAKUMI_SEND_LIFE_MANA_AFTER_JOIN=1`: sau join có outbound life/mana.

### B — Roster persist (JSON + DB)

1. [ ] Di chuyển vài ô, thoát game (disconnect).
2. [ ] Kiểm tra file (trên host bind-mount):

```bash
ls -la server-next/takumi-roster/
cat server-next/takumi-roster/test.json | head -40
```

- [ ] Có `currentHp`, `maxHp`, `currentMp`, `maxMp`, `zen` (camelCase) sau seed.

3. [ ] (Nếu DB sync) Postgres:

```bash
psql "postgresql://takumi:takumi@127.0.0.1:54444/takumi_runtime" \
  -c "SELECT account_id, char_name, current_hp, max_hp, current_mp, max_mp, zen FROM character_roster LIMIT 5;"
```

### C — Login lại (overlay vitals)

1. [ ] Login lại cùng nhân vật.
2. [ ] HP/MP khớp lần thoát trước (trong phạm vi stub, chưa combat đầy đủ).

### D — Life/Mana wire giữa phiên

- [ ] Trong log host: khi client nhận packet life/mana, tracker cập nhật (nếu client gửi/nhận `0x26`/`0x27`).
- [ ] **Open / chưa làm:** damage combat liên tục cập nhật HP — xem M9.

## Pass criteria

| # | Kết quả |
|---|---------|
| 1 | Join OK, vitals hiển thị |
| 2 | Disconnect → JSON (và DB nếu bật) có vitals |
| 3 | Re-login giữ vitals đã seed |

Fail → gửi đoạn log `legacy-login` quanh `F3 03` + nội dung `takumi-roster/<account>.json`.
