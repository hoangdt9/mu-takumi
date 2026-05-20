# 02 — SQL migrations + M8 ETL

Chỉ cần khi bật **`TAKUMI_ROSTER_DB_SYNC`**, **`TAKUMI_MONSTER_SPAWN_DB`**, **`TAKUMI_WORLD_STATIC_DB`**.

## 1. Volume Postgres mới (lần đầu `docker compose up`)

- [ ] `sql/init/001` … `006` đã chạy qua `/docker-entrypoint-initdb.d` (xem log postgres lúc tạo volume).

## 2. Volume đã tồn tại — apply thủ công

Từ Mac (cần `psql` hoặc dùng container):

```bash
cd /Users/hoangmac/Project/MU/takumi/server-next
./scripts/db/apply-sql.sh "postgresql://takumi:takumi@127.0.0.1:54444/takumi_runtime"
```

- [ ] Không lỗi `ON_ERROR_STOP` trên `004`, `005`, `006`.

Hoặc trong container:

```bash
docker compose exec -T legacy-login sh -c \
  'for f in /app/sql/init/*.sql; do psql "postgresql://takumi:takumi@postgres:5432/takumi_runtime" -v ON_ERROR_STOP=1 -f "$f"; done'
```

## 3. Import monster spawn (M8)

Trên **host** (đường dẫn file thật trên Mac):

```bash
export TAKUMI_PG_CONNECTION_STRING='postgresql://takumi:takumi@127.0.0.1:54444/takumi_runtime'
export TAKUMI_MONSTER_SET_BASE_PATH='/path/to/MuServer/4.GameServer/Data/Monster/MonsterSetBase.txt'
./scripts/db/import-monster-spawn.sh
```

- [ ] Test filter `MonsterSpawnPostgresEtlTests` pass.
- [ ] Kiểm tra row count:

```bash
psql "$TAKUMI_PG_CONNECTION_STRING" -c 'SELECT COUNT(*) FROM monster_spawn;'
```

## 4. Import gates / shops / custom (M8)

```bash
export TAKUMI_GAMESERVER_DATA_PATH='/path/to/MuServer/4.GameServer/Sub 1/Data'
./scripts/db/import-world-static-data.sh
```

- [ ] `map_gate`, `npc_shop` có dữ liệu (nếu file nguồn tồn tại).

## 5. Restart legacy-login sau ETL

```bash
docker compose up -d --force-recreate --no-deps legacy-login
docker compose logs -f legacy-login | head -80
```

- [ ] Log: monster spawn loaded from DB / `MapGateCatalog` initialized.

## Pass criteria

DB có bảng + (tuỳ chọn) row ETL → tiếp **M7** / **M8** / **M9** trên client.
