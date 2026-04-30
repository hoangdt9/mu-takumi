# ODBC / SQL backlog — Takumi DataServer + JoinServer → OpenMU (Postgres)

Tài liệu đóng mục checklist **trích `QueryManager`/SQL** cho Phase 2 migration. Nguồn: quét `Source/2.DataServer/DataServer/**/*.cpp` và `Source/3.JoinServer/JoinServer/**/*.cpp` (script tĩnh — **không** thay `MuOnline.bak` làm chân lý schema).

**Canonical schema:** restore `MuServer/7.DataBase/MuOnline.bak` (không commit) + so sánh với migrations **OpenMU** (PostgreSQL).

## ODBC (runtime)

| Process | DSN (ini) | File mẫu |
|---------|-----------|----------|
| DataServer | `MuOnline` | `MuServer/2.DataServer/DataServer.ini.example` |
| JoinServer | `MuOnlineJoin` | `MuServer/3.JoinServer/JoinServer.ini.example` |

## Stored procedures — `EXEC WZ_*` / `EXEC WZ_CS_*` (62 tên, duy nhất)

Trích từ codebase C++ (recursive `*.cpp`):

```
EXEC WZ_CONNECT_MEMB
EXEC WZ_CS_GetAllGuildMarkRegInfo
EXEC WZ_CS_GetCalcRegGuildList
EXEC WZ_CS_GetCastleNpcInfo
EXEC WZ_CS_GetCastleTaxInfo
EXEC WZ_CS_GetCastleTotalInfo
EXEC WZ_CS_GetCsGuildUnionInfo
EXEC WZ_CS_GetGuildMarkRegInfo
EXEC WZ_CS_GetOwnerGuildMaster
EXEC WZ_CS_GetSiegeGuildInfo
EXEC WZ_CS_ModifyCastleOwnerInfo
EXEC WZ_CS_ModifyCastleSchedule
EXEC WZ_CS_ModifyGuildGiveUp
EXEC WZ_CS_ModifyGuildMarkReset
EXEC WZ_CS_ModifyMoney
EXEC WZ_CS_ModifySiegeEnd
EXEC WZ_CS_ModifyTaxRate
EXEC WZ_CS_ReqNpcBuy
EXEC WZ_CS_ReqNpcRemove
EXEC WZ_CS_ReqNpcRepair
EXEC WZ_CS_ReqNpcUpdate
EXEC WZ_CS_ReqNpcUpgrade
EXEC WZ_CS_ReqRegAttackGuild
EXEC WZ_CS_ReqRegGuildMark
EXEC WZ_CS_ResetCastleSiege
EXEC WZ_CS_ResetCastleTaxInfo
EXEC WZ_CS_ResetRegSiegeInfo
EXEC WZ_CS_ResetSiegeGuildInfo
EXEC WZ_CS_SetSiegeGuildInfo
EXEC WZ_CS_SetSiegeGuildOK
EXEC WZ_CW_InfoLoad
EXEC WZ_CW_InfoSave
EXEC WZ_CreateCharacter
EXEC WZ_CustomArenaRanking
EXEC WZ_CustomMonsterReward
EXEC WZ_CustomRanking
EXEC WZ_CustomTop
EXEC WZ_DISCONNECT_MEMB
EXEC WZ_DeleteCharacter
EXEC WZ_DesblocAccount
EXEC WZ_GetAccountLevel
EXEC WZ_GetCharacterGensInfo
EXEC WZ_GetCoin
EXEC WZ_GetItemSerial
EXEC WZ_GetMarryInfo
EXEC WZ_GetMasterResetInfo
EXEC WZ_GetResetInfo
EXEC WZ_RankingBloodCastle
EXEC WZ_RankingChaosCastle
EXEC WZ_RankingDevilSquare
EXEC WZ_RankingIllusionTemple
EXEC WZ_RenameCharacter
EXEC WZ_SetAccountLevel
EXEC WZ_SetCoin
EXEC WZ_SetDiemMonter
EXEC WZ_SetDivorceInfo
EXEC WZ_SetMarryInfo
EXEC WZ_SetMasterResetInfo
EXEC WZ_SetResetInfo
EXEC WZ_SetReward
EXEC WZ_SetRewardAll
EXEC WZ_TvTRanking
```

### Gợi ý map OpenMU

- Procs **vanilla MU** (`WZ_CreateCharacter`, siege `WZ_CS_*`, ranking events, `WZ_GetItemSerial`, …) thường có **tương đương** hoặc logic nằm trong **EF / game services** OpenMU — đối chiếu từng proc khi port.
- Procs **custom** (`WZ_Custom*`, `WZ_TvTRanking`, `WZ_SetDiemMonter`, `WZ_SetReward*`, …) nhiều khả năng cần **plugin** hoặc bảng/extension riêng trong fork.

## Bảng — heuristic từ `FROM` / `INTO` / `UPDATE` (51 tên)

Có thể trùng casing (`Character` vs `CHARACTER`, `MEMB_INFO` vs `memb_info`) tùy collation script gốc:

AccountCharacter, CardPhone, CashShopData, CashShopInventory, CashShopPeriodicItem, CHARACTER, Character, CustomAttack, CustomGift, CustomItemBank, CustomNpcQuest, CustomQuest, DataNapGame, EquipInventory, EventInventory, EventLeoTheHelper, EventSantaClaus, ExtWarehouse, GameServerInfo, Gens_Rank, Gens_Reward, Guild, GuildMember, HelperData, ItemMarketData, LOG_CREDITOS, LuckyCoin, LuckyItem, MasterSkillTree, MEMB_INFO, memb_info, MuRummyCard, MuRummyData, MuunInventory, OptionData, PcPointData, PentagramJewel, PShopItemValue, QuestKillCount, QuestWorld, RankingDuel, RankingKingGuild, RankingKingPlayer, SNSData, T_FriendList, T_FriendMail, T_FriendMain, T_PetItem_Info, T_WaitFriend, warehouse, WarehouseGuild.

## SQL Back — script kèm repo (`MuServer/7.DataBase/SQL Back/`)

| File | Mục đích (tóm tắt) | Ghi chú migration |
|------|---------------------|-------------------|
| `6. Backup DB Server.sql` | `BACKUP DATABASE [MuOnline_TakumiUP15]` + shrink | Đường `@dateBackup` hardcode `E:\...` — chỉ ops Windows; không dùng trên Postgres. |
| `SQL mUA VIpchar.sql` | `UPDATE character` một nhân vật (`mLvVIPChar`) trên DB `MuMaThan` | One-off GM; **không** merge vào pipeline OpenMU. |
| `SQLPOINT.sql` | `ALTER PROCEDURE [dbo].[WZ_CreateCharacter]` (script SSMS) | File xuất **UTF-16 LE** (`U S E [...]`) — mở bằng editor hỗ trợ BOM hoặc `iconv`/SSMS; DB tên trong header: `MuOnlinecLASSIC` (sic). Đối chiếu với proc trong C++ backlog. |
| `SQLRest.sql` | Reset stats / level một lượt trên `[Character]` | Dev/GM; không schema. |
| `SQLUp.sql` | DDL custom (đầy đủ file không dài duplicate ở đây): xem **`PHASE2-OPENMU-DATA-MODEL-MAP.md`** §1 + §2 mapping OpenMU EF. |
| `Xoa DB Open.sql` | `DELETE FROM …` nhiều bảng trên `[MuOnline]` | Reset world; có trùng dòng (`Gens_Rank`). Dùng làm checklist bảng “có trên server cũ”. |

**Đã có:** DDL `SQLUp.sql` được tóm và ánh xạ khái niệm OpenMU trong [`PHASE2-OPENMU-DATA-MODEL-MAP.md`](PHASE2-OPENMU-DATA-MODEL-MAP.md). Vẫn cần **export từ `.bak`/MSSQL** để chứng nhận cột không khai trong `SQLUp`.

## Việc tiếp theo

1. Export **DDL** từ DB thật hoặc `.bak` và diff với **EF migrations** fork OpenMU (`Persistence/EntityFramework/Migrations`; schema ví dụ `AccountData`, `Configuration` trong codegen).
2. Ghi **mapping** từng proc/bảng vào issue tracker / spreadsheet (cột: OpenMU entity / WONTFIX / deferred).
3. Cập nhật `docs/protocol/COMPATIBILITY-MATRIX.md` nếu login phụ thuộc cột lạ trong `MEMB_INFO`.

**Liên quan:** `docs/TAKUMI-FULL-FILE-MIGRATION-CHECKLIST.md` §5, §11; `docs/TAKUMI-MIGRATION-OPENMU-CHECKLIST.md` Phase 2; **`PHASE2-OPENMU-DATA-MODEL-MAP.md`** (mapping DDL + ADR nháp); [`tools/db-migrate/README.md`](../../tools/db-migrate/README.md).
