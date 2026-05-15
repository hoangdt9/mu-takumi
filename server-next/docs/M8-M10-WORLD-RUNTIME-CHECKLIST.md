# M8–M10 — World data, NPC runtime, movement visibility

**Quy ước:** giống các checklist khác — `[x]` chỉ khi đã merge thật. File này gom **M8 / M9 / M10** vì chúng cùng chuỗi “thế giới sau login”.

**Phụ thuộc:** **M7** (`docs/M7-CHARACTER-PERSISTENCE-CHECKLIST.md`) cho stats persist; **M6** (`docs/M6-GAME-TCP-CHECKLIST.md`) cho listener game TCP; **M1** cho opcode map.

---

## M8 — Dữ liệu tĩnh thế giới (ETL)

- [ ] Import `MonsterSetBase*.txt` (hoặc nguồn tương đương) → bảng spawn Postgres (schema + script trong `sql/init/` hoặc tool riêng).
- [ ] Cửa / shop / `Data/Custom/` — quyết định format lưu (JSON vs bảng) + tài liệu nguồn C++ tham chiếu.
- [ ] Liên kết với `Takumi.Server.Persistence` (reader API cho `Takumi.Server.Game`).

---

## M9 — NPC & monster runtime

- [ ] Spawn theo `map_id` + tọa độ; bảng `monster_id` → stats tối thiểu (HP).
- [ ] Gói **scope** tới client (opcode theo **M1**); chỉ gửi cho session đã join hợp lệ.

---

## M10 — Movement & visibility

- [x] Walk / instant move trên TCP minimal → cập nhật **tile** roster (**M4c** — `LegacyLoginHost`, `GamePortMinimalSession`).
- [ ] **Broadcast** xung quanh player (không chỉ self); đồng bộ với **M7** (vitals / trạng thái khi cần).
- [ ] Anti-flood / rate giữ broadcast (có thể tái sử dụng `TAKUMI_MAX_PACKETS_PER_SECOND` pattern).

---

## M11 — DataServer (xem trước)

- [ ] Quyết định Postgres-only vs bridge MSSQL legacy — **`docs/IMPLEMENTATION-CHECKLIST.md`** §11; API nội bộ cho `Takumi.Server.Game`.
