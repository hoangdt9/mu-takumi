# MonoGame client — roadmap (sau migrate server)

Last updated: 2026-05-16

**Mục đích:** Phác thảo hướng client **MonoGame** (.NET) cho Takumi **sau khi** `server-next` đủ ổn định — **không** thay thế `Source/5.Main` (C++/APK) trong giai đoạn migrate server.

**Liên quan:** `IMPLEMENTATION-CHECKLIST.md`, `M1-PROTOCOL-PARITY-MAP.md`, `TAKUMI-MIGRATION-OPENMU-CHECKLIST.md`, repo tham chiếu `muonline-xulek` / `muonline-bernat-main` (MonoGame + Season 6 wire).

---

## 1. Nguyên tắc

| Nguyên tắc | Ý nghĩa |
|------------|---------|
| **Server trước, client MonoGame sau** | Không bắt đầu rewrite render/UI cho đến khi server đạt **Gate S2** (bên dưới). |
| **Hai client song song** | APK/PC C++ (`Source/5.Main`) = **golden parity**; MonoGame = product line mới, cùng wire Takumi. |
| **Chia sẻ protocol với server** | Ưu tiên package C# dùng chung (`Takumi.Client.Protocol` ← mirror `Takumi.Server.Protocol`) thay vì copy struct. |
| **Tái dùng có chọn lọc** | `Client.Data` / scene pattern từ `muonline-xulek` — **không** fork nguyên repo nếu asset season khác. |
| **Không chặn server migration** | Mọi thay đổi wire trên server phải có test golden + cập nhật M1; MonoGame chỉ **consume** API ổn định. |

---

## 2. Cổng (gates) — khi nào được bắt đầu MonoGame

Chỉ mở phase client khi server đạt mốc tương ứng.

| Gate | Điều kiện server (`server-next`) | Cho phép client |
|------|-----------------------------------|-----------------|
| **S0** | M1 doc + `Takumi.Server.Protocol` tests xanh | Spike: TCP connect + decrypt login **không** render |
| **S1** | M4–M5: login + `F3 00` + `F3 03` + roster Postgres (`TAKUMI_ROSTER_DB_PRIMARY`) | Scene Login + Select + empty world stub |
| **S2** | M6–M7: game TCP ổn (split hoặc single port), vitals + `F3 10` inventory | **Bắt đầu MonoGame MVP** (desktop trước) |
| **S3** | M8–M10: monster viewport, walk broadcast, gate/shop stub | In-world: map tile, player move, NPC/monster cơ bản |
| **S4** | M9 combat + M11 social partial ổn | Combat, inventory UI, shop |
| **S5** | SSOT Postgres-only roster; protocol frozen 1 sprint | Mobile head (Android), tối ưu hiệu năng |

**Trạng thái hiện tại (2026-05):** ~**S1 đạt**, **S2** dev xong — sign-off APK: **`../../docs/qa/S2-gate-login-join.md`**. Skeleton: **`../client-mono/`**. Phase 1 MonoGame sau S2 QA milestone.

---

## 3. Kiến trúc đề xuất

```text
┌─────────────────────────────────────────────────────────────┐
│  MuOnline.Client.sln (repo mới hoặc takumi/client-mono/)      │
├─────────────────────────────────────────────────────────────┤
│  Takumi.Client.Protocol    ← shared với server-next (subset) │
│  Takumi.Client.Crypto      ← SimpleModulus, Xor32, Protect* │
│  Client.Data               ← BMD/MAP/ATT/OZB (fork/port xulek)│
│  Client.Main               ← scenes, world, UI, networking    │
│  MuWinGL / MuWinDX / MuMac / MuAndroid / …                  │
└─────────────────────────────────────────────────────────────┘
         │ TCP                           ▲
         ▼                               │ packets
┌─────────────────────────────────────────────────────────────┐
│  server-next: Connect + Login + GameHost + Postgres         │
└─────────────────────────────────────────────────────────────┘

* Protect: chỉ nếu vẫn bắt buộc parity Android; desktop có thể tắt
  `TAKUMI_GAME_CLIENT_PROTECT_WIRE=0` trong QA MonoGame.
```

**Repo layout (đề xuất):**

| Option | Ưu | Nhược |
|--------|-----|-------|
| **A. `takumi/client-mono/`** song song `server-next/` | Rõ boundary server/client | Hai solution |
| **B. Gộp `server-next/src/Takumi.Client.*`** | Chia sẻ project protocol dễ | Solution phình to |
| **C. Fork `muonline-xulek` → branch `takumi-wire`** | Có sẵn MG + Android | Lệch asset S20 vs Takumi `data.zip` |

**Khuyến nghị:** **A** hoặc **C** (branch Takumi) — protocol package reference `Takumi.Server.Protocol` qua project reference hoặc NuGet nội bộ.

---

## 4. Lộ trình client (phases)

### Phase 0 — Chuẩn bị (song song S1, ≤ 1 tuần dev)

- [ ] Quyết **asset contract**: Takumi `data.zip` (cùng Android) vs Season 20 (xulek).
- [ ] Quyết **repo** (A/B/C §3).
- [ ] Tạo `Takumi.Client.Protocol` (hoặc link `Takumi.Server.Protocol` + client-only parsers).
- [ ] Golden vectors: import từ `Takumi.Server.Tests` (`CharacterWireGolden602Tests`, join 131-byte, `F3 10`).
- [ ] Doc wire: link `M1-PROTOCOL-PARITY-MAP.md` — MonoGame **không** đọc `WSclient.cpp` trực tiếp sau phase 0.

### Phase 1 — Networking shell (Gate S1, desktop only)

**Mục tiêu:** Connect → login → character list **không** 3D.

- [ ] `ConnectServerService`: `F4 06` / `F4 03` (plain TCP).
- [ ] `LoginService`: `C1 F1 00`, encrypted `F1 01` (`Dec2.dat` từ `Data/`).
- [ ] `CharacterService`: `F3 00`, parse 34-byte slots.
- [ ] Scene: `LoadScene` → `LoginScene` → `SelectCharacterScene` (list UI tối thiểu).
- [ ] Config: `appsettings.json` — host/port mirror `server-next/.env`.
- [ ] **Không** gProtect trừ khi test split port 55901.

**Exit:** Login `test`/`test`, thấy list nhân vật từ Postgres, log packet hex khớp C++ logcat.

### Phase 2 — Join map stub (Gate S2)

- [ ] `F3 03` + parse `PRECEIVE_JOIN_MAP_SERVER` (131 bytes) — dùng builder ngược từ server tests.
- [ ] `F3 10` inventory list (12-byte items).
- [ ] `GameScene` placeholder: terrain flat hoặc debug grid; hiển thị tọa độ tile từ roster.
- [ ] Walk TX: `0xD4` / `0x15` — server cập nhật roster (đã có trên server).

**Exit:** Vào map, di chuyển, disconnect → reconnect đúng tile (DB).

### Phase 3 — World render (Gate S3)

- [ ] Port/load: `MAP`, `ATT`, `OZB` (từ `Client.Data` xulek hoặc Takumi paths).
- [ ] Camera + tile walk (không float world — `M4-TILE-AND-COORDINATES.md`).
- [ ] RX: `C2 0x12` player viewport, `C2 0x13` monster spawn, `C1 0x15`/`0x18` move.
- [ ] Object pool: player / monster / dropped item (placeholder model).

**Exit:** Hai client MonoGame cùng map thấy nhau di chuyển (server M10).

### Phase 4 — Gameplay UI (Gate S4)

- [ ] Inventory panel (`F3 10` + local state).
- [ ] HP/MP/SD bars (`0x26`/`0x27` RX).
- [ ] NPC shop (`0x31`, buy/sell stub).
- [ ] Combat click → `0x11` / nhận damage packets.

### Phase 5 — Mobile & polish (Gate S5)

- [ ] `MuAndroid`: touch UI, virtual resolution (`appsettings` như xulek).
- [ ] Preload `data.zip` (reuse logic `MuPreload` concept — HTTP từ `datazip` profile).
- [ ] Hiệu năng: batching, no LINQ in Draw/Update.
- [ ] CI: build MuLinux + MuAndroid debug on tag.

---

## 5. Ánh xạ server milestone → client work

| Server (`IMPLEMENTATION-CHECKLIST`) | Client MonoGame |
|-------------------------------------|-----------------|
| M1 Protocol doc | Phase 0 golden tests |
| M2 `Takumi.Server.Protocol` | Shared package / copy-on-build |
| M3 Connect F4 | Phase 1 |
| M4 Roster / M7 vitals | Phase 1–2 select + join stats |
| M5 Handoff / split port | Phase 1 optional; ưu tiên **single port 44606** trước |
| M6 GameHost | Phase 2 (game TCP) |
| M8–M9 world static + monsters | Phase 3–4 |
| M10 presence | Phase 3 multiplayer view |
| M11 warehouse/trade | Phase 4+ |

---

## 6. Tái dùng từ `muonline-xulek` (tham chiếu)

| Thành phần xulek | Takumi MonoGame |
|------------------|-----------------|
| `Client.Main/Scenes/*` | Pattern scene lifecycle — **rewrite** branding/UI |
| `Client.Main/Networking/PacketRouter` | Handler registration — **đổi** opcode map theo M1 Takumi |
| `Client.Data/BMD, MAP, …` | **Tái dùng** nếu cùng format file Takumi `Data/` |
| Season 20 assets | **Không** mặc định — Takumi dùng bundle `data.zip` hiện tại |
| OpenMU endpoints | **Không** — trỏ `server-next` host/port |

---

## 7. Khác biệt cố ý so với client C++ (không parity 100% ngày 1)

- UI MonoGame mới (không port `SEASON3B` / `LoginWin.cpp`).
- Không port `gProtect` trừ khi bắt buộc cho production anti-cheat.
- Custom interface Takumi (`gInterface`, jewel bank, …) — **phase sau** hoặc bỏ trên bản MG.
- `ProtocolCoreEx` custom heads — chỉ khi server-next implement tương ứng.

C++ client vẫn chạy để **so sánh pcap/log** khi nghi lệch wire.

---

## 8. Rủi ro & giảm thiểu

| Rủi ro | Giảm thiểu |
|--------|------------|
| Server đổi wire giữa chừng | Chỉ bắt Phase 2+ sau **S2 freeze** 1 sprint; shared tests |
| Asset season lệch | Chốt Takumi `data.zip` là SSOT asset cho MG |
| Hai team maintain 2 client | C++ = parity QA only; feature mới ưu tiên MG sau S4 |
| MonoGame Android build chậm | Desktop GL trước; MGCB prebuilt như xulek `MuMac` |

---

## 9. Không nằm trong scope (explicit non-goals)

- Thay APK C++ trong Q1 migrate server.
- Unity client.
- Full OpenMU admin / EF `Takumi.Server.Host` trong client.
- Parity toàn bộ `TranslateProtocol` (~100+ heads) trước Phase 4.

---

## 10. Milestone “done” cho product MonoGame v1

1. **Desktop (Win GL):** Connect → login → select → join Lorencia → walk → thấy monster spawn + hit quái.
2. **Cùng server Docker** như `docker-stack.sh` (Postgres + legacy-login hoặc game-host).
3. **Data:** một lệnh preload / path `Data/` documented.
4. **Test:** `dotnet test` protocol + smoke script TCP (không bắt buộc full UI automation).

Sau v1 mới mở **Android head** và sunset dần C++ (quyết định product, không kỹ thuật).

---

## 11. Việc làm ngay (trước Gate S2)

Chỉ doc + prep — **không** team lớn:

1. Chốt repo option (§3) trong issue/README.
2. Khi S2 đạt: tạo solution rỗng + Phase 0 checklist ticket.
3. Giữ `Source/5.Main` là client QA chính cho đến hết M9/M10 server.

**Cập nhật doc này** khi `IMPLEMENTATION-CHECKLIST` đổi gate hoặc khi bắt Phase 1 thật.
