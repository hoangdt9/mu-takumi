# Checklist đầy đủ: Takumi server + data → OpenMU (multi-platform, client cũ)

**Mục tiêu:** Chạy **stack server** trên **Linux / macOS / Docker** bằng **OpenMU (.NET)** làm nền; **client hiện tại** (Android `Source/android`, PC `ClientBuild_*`) chỉ là **chuẩn tương thích gói tin & hành vi** — không port client trong checklist này.

**Đọc song song:** checklist pha và gate lộ trình: [`docs/TAKUMI-MIGRATION-OPENMU-CHECKLIST.md`](TAKUMI-MIGRATION-OPENMU-CHECKLIST.md).  
**OpenMU tham chiếu (repo ngoài):** ví dụ `OpenMU/deploy/all-in-one`, `OpenMU/src/MUnique.OpenMU.sln`, `OpenMU/docs/`.

**Cách đọc checklist:**

- Checkbox `[ ]`: việc phải **rà/kéo vào backlog** và **đóng khi có parity hoặc quyết định chủ đích (WONTFIX)**.
- **Migrate** không nghĩa là copy C++ sang C#. Nghĩa là **hành vi + dữ liệu Takumi được tái hiện** trong fork OpenMU (GameLogic / Network / Persistence / Plugins).
- Mọi nhánh **`_real`** / **`Sub 1`** trong `MuServer` đều phải xác định **cái nào production** và **duplicate** chỉ được coi là bản nhân bản để đối chiếu.

---

## Tiến độ (baseline đã làm — 2026-04-30)

| Mục | Trạng thái |
|-----|------------|
| §0 — Cổng, Data/Join/Connect, 2 shard GS / GSCS | **Xong** — [`docs/protocol/TAKUMI-SERVER-NETWORK-BASELINE.md`](protocol/TAKUMI-SERVER-NETWORK-BASELINE.md) |
| §1 — Manifest snapshot | **Xong** — tiêu đề `# Snapshot` + `# Git SHA…` đầu mỗi file trong [`docs/takumi-manifests/`](takumi-manifests/) |
| §2 — Macro / build 603 | **Xong** — [`docs/takumi-game-spec/SEASON-AND-DEFINES.md`](takumi-game-spec/SEASON-AND-DEFINES.md) |
| §3 — Coupling `Source/Util/` (theo vcxproj) | **Xong** — ghi trong `SEASON-AND-DEFINES.md` |
| §5 (SQL) — Trích proc/bảng từ C++ | **Xong** — [`docs/takumi-game-spec/TAKUMI-SQL-BACKLOG.md`](takumi-game-spec/TAKUMI-SQL-BACKLOG.md) |
| §11 / Phase 2 — `SQLUp.sql` + map OpenMU (concept) | **Khung** — [`PHASE2-OPENMU-DATA-MODEL-MAP.md`](takumi-game-spec/PHASE2-OPENMU-DATA-MODEL-MAP.md); [`PHASE2-MAPPING-TEMPLATE.csv`](takumi-game-spec/PHASE2-MAPPING-TEMPLATE.csv) + [`takumi-mssql-inspect`](../tools/db-migrate/README.md); ETL Postgres + điền đủ sheet vẫn TODO |
| §8 — Drift `Sub 1/Data` vs `Data`; **`4.GameServer` vs `4.GameServer_real`** | **Xong** — [`DATA-SUB1-DRIFT.md`](takumi-game-spec/DATA-SUB1-DRIFT.md), [`GAMESERVER-VS-GAMESERVER-REAL.md`](takumi-game-spec/GAMESERVER-VS-GAMESERVER-REAL.md) |
| §4 — `1.ConnectServer` vs `1.ConnectServer_real` | **Xong** (INI đồng nhất) — [`CONNECT-SERVER-REAL-DRIFT.md`](takumi-game-spec/CONNECT-SERVER-REAL-DRIFT.md) |
| §7–§8a — Ánh xạ thư mục `Data/` | **Khung** — [`GAMESERVER-DATA-FOLDER-MAP.md`](takumi-game-spec/GAMESERVER-DATA-FOLDER-MAP.md) |
| §17 — Tracker mẫu (sheet/issue) | **Khung** — [`MANIFEST-TRACKER-TEMPLATE.md`](MANIFEST-TRACKER-TEMPLATE.md) |
| §12 — Batch / docker / script | **Xong** — [`docs/OPERATIONS-MIGRATION-NOTES.md`](OPERATIONS-MIGRATION-NOTES.md) |
| Repo remote | **Đã push:** [github.com/hoangdt9/mu-takumi](https://github.com/hoangdt9/mu-takumi) — các manifest `# commit:` đồng bộ với HEAD sau `git push` (`git rev-parse HEAD`) |
| §4–§14 — Parity OpenMU / migrate từng module | **Chưa** |
| Phase 4.1 — Chỉ mục dispatcher packet (discovery) | **Khung** — [`protocol/TAKUMI-PROTOCOL-DISPATCH-INDEX.md`](protocol/TAKUMI-PROTOCOL-DISPATCH-INDEX.md) |
| Ma trận gói tin (spike) | **Khung** — [`docs/protocol/COMPATIBILITY-MATRIX.md`](protocol/COMPATIBILITY-MATRIX.md) |

---

## 0 — Thứ tự process chạy máy chủ Takumi hiện tại (baseline hành vi)

Tham chiếu: `MuServer/Start_192.168.99.200.bat` (điều chỉnh `%MUSERVER%` máy dev).

| Bước | Thành phần | Ghi chú migration |
|------|------------|-------------------|
| 1 | `1.ConnectServer/ConnectServer.exe` | OpenMU Connect / server list |
| 2 | `2.DataServer/DataServer.exe` | Luồng Data → DB / abstraction OpenMU (Postgres hoặc tương đương) |
| 3 | `3.JoinServer/JoinServer.exe` | Login / nhân vật / chọn server trong OpenMU |
| 4 (optional) | `5.Antihack/XShield.exe` | **Không** có trong OpenMU vanilla — quyết: bỏ, thay rule server-side, hay dịch vụ riêng |
| 5 | `4.GameServer/GameServer/GameServerCS.exe` | **Castle / CS shard** — cấu hình và port trong `GameServerInfo - Common.ini` tương ứng |
| 6 | `4.GameServer/Sub 1/GameServer/GameServer.exe` | **Shard chính** — bộ `Data/` + ini song song |

- [x] Ghi **IP/ports / CS vs non-CS** từ cả hai `GameServer/Data/GameServerInfo - Common.ini` (và bản trong `Sub 1`) vào `docs/protocol/` — **đã ghi:** [`docs/protocol/TAKUMI-SERVER-NETWORK-BASELINE.md`](protocol/TAKUMI-SERVER-NETWORK-BASELINE.md).

---

## 1 — Manifest tự động (inventory đầy đủ, tránh sót file)

Đã có trong repo (đường dẫn tương đối từ root `takumi/`):

| File | Nội dung |
|------|----------|
| [`docs/takumi-manifests/TAKUMI-SERVER-SOURCE-MANIFEST.txt`](takumi-manifests/TAKUMI-SERVER-SOURCE-MANIFEST.txt) | Mọi `*.cpp` server `1.Connect` … `4.Game` + **`6.GetMainInfo`** (~385 file nguồn; GameServer ~281). |
| [`docs/takumi-manifests/TAKUMI-SERVER-HEADERS-MANIFEST.txt`](takumi-manifests/TAKUMI-SERVER-HEADERS-MANIFEST.txt) | Mọi `*.h` / `*.hpp` trong cùng prefix (đọc ghép đôi với `.cpp`). |
| [`docs/takumi-manifests/TAKUMI-MUSERVER-GAMEDATA-FILES.txt`](takumi-manifests/TAKUMI-MUSERVER-GAMEDATA-FILES.txt) | Danh sách **từng file** dưới `MuServer/4.GameServer/Data/` (bundle nội dung game trong repo). |
| [`docs/takumi-manifests/TAKUMI-MUSERVER-CONFIG-MANIFEST.txt`](takumi-manifests/TAKUMI-MUSERVER-CONFIG-MANIFEST.txt) | `*.ini` / `*.txt` / `*.dat` / `*.bat` / `*.sql` tại các service + runtime GameServer (**không** gồm toàn bộ bulk `Data/`, không gồm log). |

### Tái tạo manifest (khi cây repo thay đổi)

Chạy từ máy có `bash`/`find` (macOS):

```bash
TAK=/Users/hoangmac/Project/MU/takumi
# Script đầy đủ: docs/takumi-manifests/README.md
```

- [x] Gắn **snapshot + `# commit:`** vào đầu mỗi manifest (`git rev-parse HEAD` sau mỗi lần tái sinh); hướng dẫn: [`docs/takumi-manifests/README.md`](takumi-manifests/README.md).

---

## 2 — Solution / project MSVC (điểm neo build cũ)

- [x] `Source/1.ConnectServer/ConnectServer/ConnectServer.vcxproj`
- [x] `Source/2.DataServer/DataServer/DataServer.vcxproj`
- [x] `Source/3.JoinServer/JoinServer/JoinServer.vcxproj`
- [x] `Source/4.GameServer/GameServer/GameServer.vcxproj`
- [x] `Source/6.GetMainInfo/GetMainInfo/GetMainInfo.vcxproj` (tool/client patch list — không phải process server Linux nhưng **ảnh hưởng client**)
- [x] Đối chiếu **define preprocessor / SDK version** trong từng `.vcxproj` với nhãn fork OpenMU — **đã tóm tắt:** [`docs/takumi-game-spec/SEASON-AND-DEFINES.md`](takumi-game-spec/SEASON-AND-DEFINES.md).

*(Client PC `Source/5.Main` có solution riêng — **ngoài phạm vi server checklist** nhưng cần khi chứng minh protocol từ `Main`/resource.)*

---

## 3 — `Source/` ngoài 4 server: phải rà để biết coupling

- [x] **`Source/Util/`** — `cryptopp`, `detours`, `lua`, `mapm`: **theo `.vcxproj` server** Join dùng `MD5.*`, Game dùng `CCRC32`/`Math`; các thư khác không được kéo trực tiếp — chi tiết [`SEASON-AND-DEFINES.md`](takumi-game-spec/SEASON-AND-DEFINES.md). (Backlog: grep thư viện ẩn nếu linker dùng `.lib` không khai trong vcxproj.)
- [x] **`Source/_safety_backup/`** — chỉ benchmark / diff khi nghi có lệch bản vá; không đưa vào build OpenMU.
- [ ] **`Source/android/`** — client; **chưa spike pcap** endpoint/cipher — checklist **§13b** (đồng bộ khi test thật).
- [x] **`Source/device_screen_*` / `latest_logcat*.txt`** — chỉ QA; không thuộc server port.

---

## 4 — `1.ConnectServer` — từng file `*.cpp` (đủ)

| File | Migrate / chứng minh trong OpenMU |
|------|-----------------------------------|
| [ ] `ClientManager.cpp` | Session / blacklist / concurrency |
| [ ] `ConnectServer.cpp` | Entry, lifecycle |
| [ ] `ConnectServerProtocol.cpp` | **Danh bạ packet CS↔client** — dispatcher đã rút gọn trong [`protocol/TAKUMI-PROTOCOL-DISPATCH-INDEX.md`](protocol/TAKUMI-PROTOCOL-DISPATCH-INDEX.md) §3; parity OpenMU từng field vẫn backlog. |
| [ ] `CriticalSection.cpp` | Chỉ tham khảo threading — OpenMU có model riêng |
| [ ] `IpManager.cpp` | Allow/block IP |
| [ ] `Log.cpp` | Chuẩn log tương đương |
| [ ] `MemScript.cpp` | Parser script INI của Connect |
| [ ] `MiniDump.cpp` | Thay Observability (.NET dumps / logs) |
| [ ] `Protect.cpp` | Pack / anticrack — có thể bỏ trên fork |
| [ ] `Queue.cpp` | Worker queue |
| [ ] `ServerDisplayer.cpp` | UI/console — không mang sang |
| [ ] `ServerList.cpp` | **Map sang server list OpenMU / DB** |
| [ ] `SocketManager.cpp` | TCP lifecycle |
| [ ] `SocketManagerUdp.cpp` | UDP discovery (nếu client Takumi có) |
| [ ] `stdafx.cpp` | N/A |
| [ ] `Util.cpp` | Tiện ích chung |

- [x] Cặp **`ConnectServer.ini`**, **`ServerList.ini`** (`MuServer/1.ConnectServer*` và `1.ConnectServer_real`): cổng đã vào [`TAKUMI-SERVER-NETWORK-BASELINE.md`](protocol/TAKUMI-SERVER-NETWORK-BASELINE.md); **`_real`** vs chuẩn: **cùng nội dung INI** @ 2026-04-30 — [`CONNECT-SERVER-REAL-DRIFT.md`](takumi-game-spec/CONNECT-SERVER-REAL-DRIFT.md) (khác chủ yếu tên/binary deploy).
- **Discovery dispatcher client→Connect:** [`protocol/TAKUMI-PROTOCOL-DISPATCH-INDEX.md`](protocol/TAKUMI-PROTOCOL-DISPATCH-INDEX.md) §3 (`0xF4`/subs).

---

## 5 — `2.DataServer` — từng file `*.cpp` (đủ)

| File |
|------|
| [ ] `AllowableIpList.cpp` |
| [ ] `ArcaBattle.cpp` |
| [ ] `BadSyntax.cpp` |
| [ ] `CashShop.cpp` |
| [ ] `CastleDBSet.cpp` |
| [ ] `CB_AutoNapGame.cpp` |
| [ ] `CharacterManager.cpp` |
| [ ] `ChoTroi.cpp` |
| [ ] `CommandManager.cpp` |
| [ ] `CriticalSection.cpp` |
| [ ] `CSProtocol.cpp` |
| [ ] `DataServer.cpp` |
| [ ] `DataServerProtocol.cpp` |
| [ ] `ESProtocol.cpp` |
| [ ] `EventInventory.cpp` |
| [ ] `GensSystem.cpp` |
| [ ] `GuildManager.cpp` |
| [ ] `GuildMatching.cpp` |
| [ ] `Helper.cpp` |
| [ ] `Log.cpp` |
| [ ] `LuckyCoin.cpp` |
| [ ] `LuckyItem.cpp` |
| [ ] `MasterSkillTree.cpp` |
| [ ] `MemScript.cpp` |
| [ ] `MiniDump.cpp` |
| [ ] `MuRummy.cpp` |
| [ ] `MuunSystem.cpp` |
| [ ] `NewUIMyInventory.cpp` |
| [ ] `NpcTalk.cpp` |
| [ ] `PartyMatching.cpp` |
| [ ] `PcPoint.cpp` |
| [ ] `PentagramSystem.cpp` |
| [ ] `PersonalShop.cpp` |
| [ ] `Protect.cpp` |
| [ ] `QueryManager.cpp` | **Neo SQL / ODBC — map bảng & proc** |
| [ ] `Quest.cpp` |
| [ ] `QuestWorld.cpp` |
| [ ] `Queue.cpp` |
| [ ] `ReiDoMU.cpp` |
| [ ] `ServerDisplayer.cpp` |
| [ ] `ServerManager.cpp` |
| [ ] `SocketManager.cpp` |
| [ ] `stdafx.cpp` |
| [ ] `Util.cpp` |
| [ ] `Warehouse.cpp` |

- [x] **`MuServer/2.DataServer`**: các key `DataServerPort`, ODBC DSN **đã baseline** trong [`protocol/TAKUMI-SERVER-NETWORK-BASELINE.md`](protocol/TAKUMI-SERVER-NETWORK-BASELINE.md) (không dán PASS vào markdown). Rotation `sa`/PWD khi chuyển Postgres.
- **Điều phối GS↔Data** (discovery): **`DSProtocol.cpp`** — không rút gọn trong doc protocol; backlog parity theo luồng DataServer/OpenMU abstraction.
- [x] Trích **`QueryManager.cpp` + mọi `ExecQuery`/`EXEC` trong Data/Join `*.cpp`** — **đã lưu:** [`docs/takumi-game-spec/TAKUMI-SQL-BACKLOG.md`](takumi-game-spec/TAKUMI-SQL-BACKLOG.md) (62 proc + 51 bảng heuristic).

---

## 6 — `3.JoinServer` — từng file `*.cpp` (đủ)

| File |
|------|
| [ ] `AccountManager.cpp` |
| [ ] `AllowableIpList.cpp` |
| [ ] `CriticalSection.cpp` |
| [ ] `JoinServer.cpp` |
| [ ] `JoinServerProtocol.cpp` | **Đăng nhập / nhân vật / handshake** |
| [ ] `Log.cpp` |
| [ ] `MemScript.cpp` |
| [ ] `MiniDump.cpp` |
| [ ] `Protect.cpp` |
| [ ] `QueryManager.cpp` |
| [ ] `Queue.cpp` |
| [ ] `ServerDisplayer.cpp` |
| [ ] `ServerManager.cpp` |
| [ ] `SocketManager.cpp` |
| [ ] `SocketManagerUdp.cpp` |
| [ ] `stdafx.cpp` |
| [ ] `Util.cpp` |

- [x] **`MuServer/3.JoinServer`**: cổng + liên kết Connect **đã baseline** trong [`protocol/TAKUMI-SERVER-NETWORK-BASELINE.md`](protocol/TAKUMI-SERVER-NETWORK-BASELINE.md); các file TXT còn lại rà khi vào parity login OpenMU.
- **Discovery dispatcher** `JoinServerProtocolCore` (tin nội bộ GS→Join): [`protocol/TAKUMI-PROTOCOL-DISPATCH-INDEX.md`](protocol/TAKUMI-PROTOCOL-DISPATCH-INDEX.md) §4.

---

## 7 — `4.GameServer/GameServer` — 281 file `*.cpp` (đủ trong manifest)

- [x] Baseline cổng game **55901 / 55920**, serial, version → [`docs/protocol/TAKUMI-SERVER-NETWORK-BASELINE.md`](protocol/TAKUMI-SERVER-NETWORK-BASELINE.md).

Danh sách **đầy đủ từng đường dẫn** nằm trong [`docs/takumi-manifests/TAKUMI-SERVER-SOURCE-MANIFEST.txt`](takumi-manifests/TAKUMI-SERVER-SOURCE-MANIFEST.txt), mục `## 4.GameServer GameServer/*.cpp`.

**Điều phối client→GS (discovery):** [`docs/protocol/TAKUMI-PROTOCOL-DISPATCH-INDEX.md`](protocol/TAKUMI-PROTOCOL-DISPATCH-INDEX.md).

- [ ] Tạo bảng theo dõi thật (sheet / GitHub Projects): một dòng một path trong manifest § 7; cột theo [**`MANIFEST-TRACKER-TEMPLATE.md`**](MANIFEST-TRACKER-TEMPLATE.md).
- [x] **Khung `Data/*` theo chức năng** (discovery, không thay manifest từng file): [`GAMESERVER-DATA-FOLDER-MAP.md`](takumi-game-spec/GAMESERVER-DATA-FOLDER-MAP.md).
- [x] **Nhóm ưu tiên đọc** `.cpp` (overlap với `GameServer/Data` và client thật) — thứ tự gợi ý; **không** thay danh sách đầy đủ trong manifest:

| Gợi nhóm chức năng | Ví dụ file (grep manifest) |
|--------------------|-----------------------------|
| Core / lifecycle | `GameServer.cpp`, `Server.cpp`, `User.cpp`, `Viewport.cpp`, `Util.cpp`, `HackCheck*` |
| Mạng & packet game | `Protocol.cpp`, `DSProtocol*.cpp`, `JSProtocol*.cpp`, `SocketManager*.cpp`, `PacketManager*` (nếu có) |
| Combat & damage | `Attack.cpp`, `Skill*.cpp`, `ObjectManager*.cpp`, `DarkSpirit`, `PeriodicItem` |
| Map / NPC / Monster | `Map*.cpp`, `Monster*.cpp`, `NpcTalk.cpp`, bot `Bot*` |
| Kinh tế | `CashShop.cpp`, `Trade.cpp`, `PersonalShop*.cpp`, `ZenDrop.cpp`, warehouse |
| Guild / siege / PvP systems | `Guild*.cpp`, `CastleSiege*.cpp`, `Crywolf*.cpp`, … |
| Dungeon / events | `BloodCastle.cpp`, `ChaosCastle.cpp`, `DevilSquare*.cpp`, `IllusionTemple.cpp`, … |
| Custom Takumi (`BCustom*`, `CB_*`, localized names) | Rà trong manifest — thường **không** có trong OpenMU vanilla |

- [x] **Hai shard chạy thật** (`GameServerCS` + `Sub 1`): **EXE, path outdir `.vcxproj`, port game, ServerCode** → [`protocol/TAKUMI-SERVER-NETWORK-BASELINE.md`](protocol/TAKUMI-SERVER-NETWORK-BASELINE.md) và [`SEASON-AND-DEFINES.md`](takumi-game-spec/SEASON-AND-DEFINES.md). Map sang **multiple game servers** trong OpenMU vẫn **TODO** fork.

---

## 8 — `MuServer/4.GameServer` — runtime, `Data/`, và bản nhân bản `Sub 1`

### 8a. Thư mục `MuServer/4.GameServer/Data/*` — kiểu dữ liệu (inventory đầy đủ trong manifest)

- [x] **Đã có danh sách đường dẫn từng file** trong [`TAKUMI-MUSERVER-GAMEDATA-FILES.txt`](takumi-manifests/TAKUMI-MUSERVER-GAMEDATA-FILES.txt) — checkbox dưới đây là **đọc/import** theo từng hệ subsystem (kéo backlog); **bản đồ thư mục** (chức năng ↔ OpenMU): [`GAMESERVER-DATA-FOLDER-MAP.md`](takumi-game-spec/GAMESERVER-DATA-FOLDER-MAP.md).
- [ ] **`CashShop/`** — gói/ sản phẩm zen/cash (`CashShop*.txt`).
- [ ] **`Character/DefaultClassInfo.txt`** — căn chỉnh với **class/version** OpenMU fork.
- [ ] **`Custom/*.txt|.ini`** — toàn bộ custom Takumi (**26+ pattern** trong `Custom/` repo).
- [ ] **`Event/*.dat`** — BossGuild, BonusManager, BloodCastle, CastleDeep, ChaosCastle, Crywolf, DevilSquare, DoubleGoer, IllusionTemple, ImperialGuardian, Invasion*, Kanturu, LoanChien, MossMerchant, MuCastle*, Raklion, ReiDoMU, SummonScroll, TvT, v.v.; kèm thư con `CTCMini`, `BossGuild`.
- [ ] **`EventItemBag/`** — từng túi rơi (file đánh số `000 …` — không bỏ qua chỉ vì đông).
- [ ] **`Hack/*.txt`** — tích hợp policy OpenMU hay bỏ nếu thay subsystem.
- [ ] **`Item/*.txt`** — item options, jewels, mixtures, stacking rules.
- [ ] **`Monster/*.txt|.dat`** — spawn, ai, leveling.
- [ ] **`Move/*.txt`** — gate / move requirements.
- [ ] **`Quest/`, `QuestWorld/`**.
- [ ] **`Ruud/`**.
- [ ] **`Shop/*.txt`**.
- [ ] **`Skill/*.txt`**.
- [ ] **`Terrain/*.att|.map|…`** (theo đuôi thực tế trong repo).
- [ ] **`Util/*.txt|.dat`** helpers.

- [x] **`MuServer/4.GameServer/Sub 1/Data/`**: `diff -rq` với **`MuServer/4.GameServer/Data/`** — **không chênh** @ 2026-04-30; ghi trong [`docs/takumi-game-spec/DATA-SUB1-DRIFT.md`](takumi-game-spec/DATA-SUB1-DRIFT.md).

*(Xem **`TAKUMI-MUSERVER-GAMEDATA-FILES.txt`** để checkbox từng file nếu cần kỹ luật tuyệt đối.)*

### 8b. Ini / config runtime trong cây `4.GameServer` (không nằm gọn trong `Data/` đã liệt kê ở manifest config)

- [x] `GameServer/Data/GameServerInfo - Common.ini` — **cổng & nhánh GS/GSCS** đã baseline → [`protocol/TAKUMI-SERVER-NETWORK-BASELINE.md`](protocol/TAKUMI-SERVER-NETWORK-BASELINE.md); toàn bộ tuning khác khi parity OpenMU.
- [ ] `GameServer/Data/GameServerInfo - Character.ini`
- [ ] `GameServer/Data/GameServerInfo - Skill.ini`
- [ ] `GameServer/Data/GameServerInfo - ChaosMix.ini`
- [ ] `GameServer/Data/GameServerInfo - Command.ini`
- [ ] `GameServer/Data/GameServerInfo - Event.ini`
- [ ] `GameServer/Data/GameServerInfo - Custom.ini`
- [ ] `GameServer/Data/CustomConfig.ini`
- [ ] `GameServer/GHRSReset.ini`
- [ ] Toàn bộ mục tương ứng dưới **`Sub 1/GameServer/...`** (đã có trong **`TAKUMI-MUSERVER-CONFIG-MANIFEST.txt`**)

### 8c. Cây **`4.GameServer_real`** (snapshot / backup layout)

- [x] Diff với **`4.GameServer`** — **ghi:** [`docs/takumi-game-spec/GAMESERVER-VS-GAMESERVER-REAL.md`](takumi-game-spec/GAMESERVER-VS-GAMESERVER-REAL.md). **Authority tree:** `4.GameServer` (đủ CS + Sub 1); `_real` chủ yếu khác layout / localhost / tuning (`Common.ini`, `GHRSReset.ini`, `MapServerInfo.dat`).

---

## 9 — Antihack & binary phụ (`MuServer/5.*`, `GS antihack`)

- [ ] **`5.Antihack/`**: `XShield.exe`, `Configs.ini`, `AllowableIpList.txt`, `BlackList.txt` và DLL kèm (`libcurl.dll`, VC runtime) — quyết thay thế trong OpenMU.
- [ ] **`GS antihack/`**: các `*.exe` (Connect/Join/GS facade) — xác nhận client Takumi có **hard depend** không (pcap handshake).
- [ ] Log `5.Antihack/LOG` — không migrate; chỉ tham khảo.

---

## 10 — DatEditor và công cụ biên soạn (`MuServer/6.DatEditor`)

- [ ] Workflow editor → file output vào **`4.GameServer/Data`** — tái hiện bằng OpenMU tooling / script import (hoặc doc “manual”).
- [ ] Thư `items/*/…` trong editor — không chạy trên Linux nhưng **tệp đầu ra** phải nằm trong pipeline parity.

---

## 11 — Cơ sở dữ liệu MSSQL hiện tại (`MuServer/7.DataBase`)

- [ ] **`MuOnline.bak`** — nguồn schema + dữ liệu thật để spike ETL Postgres (**chân lý** so với `SQLUp.sql` — đã map patch trong [`PHASE2-OPENMU-DATA-MODEL-MAP.md`](takumi-game-spec/PHASE2-OPENMU-DATA-MODEL-MAP.md)).
- [x] **`SQL Back/*.sql`** — **đã tóm trong** [`docs/takumi-game-spec/TAKUMI-SQL-BACKLOG.md`](takumi-game-spec/TAKUMI-SQL-BACKLOG.md) (mục **SQL Back**); vẫn cần diff chi tiết DDL `SQLUp.sql` ↔ `.bak` / OpenMU khi vào Gate 2.
- [x] **Trích proc/bảng từ server C++** (bổ sung cho `.bak`) — [`docs/takumi-game-spec/TAKUMI-SQL-BACKLOG.md`](takumi-game-spec/TAKUMI-SQL-BACKLOG.md).
- [ ] **`docker/sql/restore-muonline.sh`** (repo) — dev container MSSQL cho golden compare song song Postgres OpenMU — quy ước ETL trong [`tools/db-migrate/README.md`](../tools/db-migrate/README.md).

---

## 12 — Script vận hành Takumi trong repo + VM

| Path | Kiểm tra |
|------|----------|
| [x] `MuServer/Start_*.bat`, `Stop_MuServer.bat`, … | **Đã ghi** thứ tự + hướng OpenMU — [`docs/OPERATIONS-MIGRATION-NOTES.md`](OPERATIONS-MIGRATION-NOTES.md) |
| [x] `docker/docker-compose.yml`, `docker/server/*`, `docker/sql/*` | Cùng doc § «Repo Docker» |
| [x] `scripts/open-mu-vm.sh`, `run-mssql-docker.sh`, `run-muserver-docker.sh` | Cùng doc § «Script local» |
| [x] `VMWare/*.vmx` (nếu có) | **`.gitignore`** — không clone VM; chỉ dev local |

---

## 13 — Client (chỉ verify tương thích với server OpenMU)

### 13a. PC `ClientBuild_192.168.99.200/`

- [ ] `Mu.ini`, `config.ini`, `StartClient.bat` — IP, port, version string.
- [ ] `data.zip` / `Data/**` — **không** mirror sang server; nhưng **version file** phải khớp season server.

### 13b. Android `Source/android/`

- [ ] Gradle / `AndroidManifest.xml` — network cleartext, deep link.
- [ ] Java entry: `MuMainActivity.java`, `MuMainNativeActivity.java`, `PreloadActivity.java`.
- [ ] Bootstrap SDL bundle `org/libsdl/app/*.java` — không đổi trừ khi build break.
- [ ] JNI / native MU core thường nằm trong artifact build (ngoài `main/java`) — locating `*.so` và **trace connect string** trong binary/patch.

*(Client C++ **`Source/5.Main`** không liệt kê từng file ở đây — thuộc client build roadmap; chỉ **cần** khi sửa client hoặc khi chứng minh mismatch opcode.)*

---

## 14 — `6.GetMainInfo` — 26 file `*.cpp` (tool/MainInfo cho client patch)

Đường dẫn đầy đủ trong manifest § **6.GetMainInfo**. Basename checklist:

| File |
|------|
| [ ] `CustomBuyVip.cpp` |
| [ ] `CustomCloak.cpp` |
| [ ] `CustomCommandInfo.cpp` |
| [ ] `CustomCrossBow.cpp` |
| [ ] `CustomDmgColor.cpp` |
| [ ] `CustomGloves.cpp` |
| [ ] `CustomItem.cpp` |
| [ ] `CustomItemPosition.cpp` |
| [ ] `CustomJewel.cpp` |
| [ ] `CustomMessage.cpp` |
| [ ] `CustomMonster.cpp` |
| [ ] `CustomMonsterGlow.cpp` |
| [ ] `CustomNpcName.cpp` |
| [ ] `CustomPet.cpp` |
| [ ] `CustomPetEffect.cpp` |
| [ ] `CustomWing.cpp` |
| [ ] `CustomWIngEffect.cpp` |
| [ ] `DynamicEffect.cpp` |
| [ ] `GetMainInfo.cpp` |
| [ ] `ItemToolTip.cpp` |
| [ ] `MemScript.cpp` |
| [ ] `Message.cpp` |
| [ ] `MonsterEffect.cpp` |
| [ ] `stdafx.cpp` |
| [ ] `TooltipBuff.cpp` |
| [ ] `UIMapName.cpp` |

- [ ] Đối chiếu **output của tool** client đang chờ (checksum / file list) và quyết: **inject qua fork OpenMU** (config export) hay giữ toolchain Windows cho giai đoạn chuyển đổi.

---

## 15 — Headers (396+) — không liệt kê từng dòng tại đây

- [x] Dùng [`TAKUMI-SERVER-HEADERS-MANIFEST.txt`](takumi-manifests/TAKUMI-SERVER-HEADERS-MANIFEST.txt); khi audit một `*.cpp`, đánh dấu **`*.h`** include đã đọc.
- [x] **`stdafx.h` / pch / macro season** — tóm trong [`docs/takumi-game-spec/SEASON-AND-DEFINES.md`](takumi-game-spec/SEASON-AND-DEFINES.md) (mở rộng cờ `CUSTOM_*` sau).

---

## 16 — Repo OpenMU (fork) — vùng sẽ chạm khi tái hiện Takumi

Ghi vào backlog fork (chưa sửa code OpenMU trong repo Takumi):

- [x] `MUnique.OpenMU.ConnectServer` hoặc tương đương listener.
- [x] `MUnique.OpenMU.LoginServer` / join flow.
- [x] `MUnique.OpenMU.GameServer` — game logic, maps, plugs.
- [x] `MUnique.OpenMU.Network` — packet parsers/writers.
- [x] `DataModel` / `Persistence` — EF PostgreSQL migrations.
- [x] `PlugIns` hoặc `Custom` nhánh của bạn — gom customization Takumi.
- [x] Docker `deploy/` — ảnh production đa platform.

---

## 17 — Đầu ra chứng nhận “không sót” checklist file

Mẫu cột cho sheet/issue: **[`MANIFEST-TRACKER-TEMPLATE.md`](MANIFEST-TRACKER-TEMPLATE.md)**.

- [ ] Mọi dòng **`TAKUMI-SERVER-SOURCE-MANIFEST.txt`** có trạng thái trong tracker nội bộ **hoặc** được gộp chủ đích (vd. nhóm `Protect.cpp` = “skipped security parity”).
- [ ] Mọi dòng **`TAKUMI-MUSERVER-GAMEDATA-FILES.txt`** được phân loại: **đã có converter / nhập tay / không dùng / deferred**.
- [ ] **Gate khách quan:** client Takumi vào được **staging OpenMU**, chơi các scenario trong [`docs/protocol/COMPATIBILITY-MATRIX.md`](protocol/COMPATIBILITY-MATRIX.md) — đóng các gap của manifest sau spike.

---

**Kết:** Checklist trong file này + **manifest** trong `docs/takumi-manifests/` là **bộ đầy đủ** để không bỏ sót artifact Takumi trong quá trình chuyển sang OpenMU. Lộ trình theo gate thời gian vẫn dùng [`TAKUMI-MIGRATION-OPENMU-CHECKLIST.md`](TAKUMI-MIGRATION-OPENMU-CHECKLIST.md).
