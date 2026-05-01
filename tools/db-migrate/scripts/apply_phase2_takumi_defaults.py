#!/usr/bin/env python3
"""Fill PHASE2-MAPPING-TEMPLATE.csv with Takumi-priority defaults (dbo truth, OpenMU EF targets).

Run from repo root: python3 tools/db-migrate/scripts/apply_phase2_takumi_defaults.py
"""
from __future__ import annotations

import csv
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
TARGET = ROOT / "docs/takumi-game-spec/PHASE2-MAPPING-TEMPLATE.csv"


LEGACY_TABLE: dict[str, tuple[str, str, str]] = {
    "AccountCharacter": (
        "data.Character + Account (slot)",
        "deferred",
        "Takumi AccountCharacter — map slot/AccountID với EF Aggregate; inspect --table.",
    ),
    "CashLog": ("Takumi plugin audit/log", "deferred", "Lịch sử CashShop — không trong core OpenMU."),
    "CashShopData": ("plugin + config.Market*", "deferred", "Pricing legacy vs OpenMU store model."),
    "CashShopInventory": ("data.Item / plugin stall", "deferred", "Kho cửa hàng MSSQL."),
    "CashShopPeriodicItem": ("plugin periodic grant", "deferred", ""),
    "Character": (
        "data.Character + takumi_staging.legacy_character",
        "in_progress",
        "Chân lý nhân vật Takumi; Gate2 nhận blob Inventory → ItemStorage.",
    ),
    "CustomAttack": ("Takumi plugin", "deferred", ""),
    "CustomGift": ("Takumi plugin", "deferred", ""),
    "CustomItemBank": (
        "plugin + ItemStorage ext",
        "deferred",
        "PHASE2 §1.2 — jewell bank không chuẩn EF.",
    ),
    "CustomJewelBank": ("Takumi plugin", "deferred", ""),
    "CustomNpcQuest": ("config.Quest + plugin", "deferred", ""),
    "CustomQuest": ("config.Quest + plugin state", "deferred", ""),
    "DataNapGame": ("Takumi plugin nap", "deferred", "PHASE2 §1.2 DataNapGame."),
    "DefaultClassType": ("config.CharacterClass.Number", "deferred", "Map Class id MSSQL."),
    "EquipInventory": ("decode → data.ItemStorage", "deferred", "PHASE2 blob EquipInventory."),
    "EventLeoTheHelper": ("plugin event", "deferred", ""),
    "EventSantaClaus": ("plugin event", "deferred", ""),
    "ExtWarehouse": ("vault extension / plugin", "deferred", "Mở rộng warehouse legacy."),
    "GameServerInfo": ("config GameServer* + deploy JSON", "deferred", "Không import nguyên bản bảng; so INI MU."),
    "Gens_Duprian": ("Takumi gens plugin", "deferred", ""),
    "Gens_Rank": ("Takumi gens plugin", "deferred", ""),
    "Gens_Reward": ("Takumi gens plugin", "deferred", ""),
    "Gens_Varnert": ("Takumi gens plugin", "deferred", ""),
    "Guild": ("guild.Guild", "todo", "Map GUID + tên guild legacy."),
    "GuildMember": ("guild.GuildMember", "todo", ""),
    "HelperData": ("data.Character MuHelperConfiguration", "deferred", "Blob helper Takumi."),
    "ItemLog": ("plugin audit", "deferred", ""),
    "ItemMarketData": ("Takumi market plugin", "deferred", "PHASE2 ItemMarketData."),
    "LOG_CREDITOS": ("plugin credit log", "deferred", ""),
    "LuckyCoin": ("plugin economy", "deferred", ""),
    "LuckyItem": ("plugin loot", "deferred", ""),
    "Marry": ("plugin marry social", "deferred", ""),
    "MasterSkillTree": ("master skills EF/service", "deferred", "Map MasterSkillTree → skill EF."),
    "MEMB_INFO": (
        "data.Account + takumi_staging.legacy_memb_info",
        "in_progress",
        "Chân lý account Takumi; memb___id memb__pwd — policy Gate2 BCrypt.",
    ),
    "MEMB_STAT": ("plugin / obsolete", "deferred", "Kiểm tra join code còn dùng không."),
    "MK_Server": ("infra join registry", "deferred", "Thay bằng compose ConnectServer OpenMU."),
    "MuCastle_DATA": ("siege guild persistence", "deferred", "Gần WZ_CS_* + guild schema."),
    "MuCastle_MONEY_STATISTICS": ("siege stats", "deferred", ""),
    "MuCastle_NPC": ("siege NPC", "deferred", ""),
    "MuCastle_REG_SIEGE": ("siege registration", "deferred", ""),
    "MuCastle_SIEGE_GUILDLIST": ("siege guild list", "deferred", ""),
    "OptionData": ("config.ItemOption*", "deferred", "Tham chiếu item opt server cũ."),
    "QuestKillCount": ("data.CharacterQuestState", "deferred", ""),
    "QuestWorld": ("config.QuestDefinition + progress", "deferred", ""),
    "RankingBloodCastle": ("MiniGameRanking / plugin", "deferred", ""),
    "RankingChaosCastle": ("MiniGameRanking / plugin", "deferred", ""),
    "RankingDevilSquare": ("MiniGameRanking / plugin", "deferred", ""),
    "RankingDuel": ("config Duel + ranking", "deferred", ""),
    "RankingIllusionTemple": ("plugin IT", "deferred", ""),
    "RankingKingGuild": ("plugin", "deferred", ""),
    "RankingKingPlayer": ("plugin", "deferred", ""),
    "RankingTvT": ("Takumi TvT ranking plugin", "deferred", ""),
    "T_CGuid": ("migration id map staging", "deferred", "Nếu cần map Guid MSSQL/OpenMU."),
    "T_FriendList": ("friend.Friend (+ letter)", "deferred", "Friend list legacy."),
    "T_FriendMail": ("data.LetterHeader.Body", "deferred", "Thư MU legacy."),
    "T_FriendMain": ("friend.Friend meta", "deferred", ""),
    "T_PetItem_Info": ("plugin pet", "deferred", ""),
    "T_WaitFriend": ("friend invites", "deferred", ""),
    "warehouse": ("data.Account.Vault ItemStorage", "deferred", "Warehouse blob → vault slots."),
    "WarehouseGuild": ("guild vault plugin", "deferred", ""),
    "WZ_CW_INFO": ("event CW Takumi plugin", "deferred", "WZ_CW_InfoLoad/Save parity."),
}


PROC_PATCH: dict[str, tuple[str, str, str]] = {
    "WZ_CONNECT_MEMB": (
        "Connect + Login EF",
        "in_progress",
        "Takumi join path; không giữ ODBC EXEC — parity ConnectServer OpenMU.",
    ),
    "WZ_DISCONNECT_MEMB": ("session teardown", "in_progress", "Clear session DataServer tương đương OpenMU."),
    "WZ_CreateCharacter": (
        "data.Character create path",
        "in_progress",
        "Inventory + CharacterClass FK takumi so với EF.",
    ),
    "WZ_DeleteCharacter": ("data.Character delete path", "in_progress", ""),
    "WZ_RenameCharacter": ("data.Character rename", "in_progress", ""),
    "WZ_GetItemSerial": ("item serial service", "in_progress", "Serial generator legacy vs OpenMU increment."),
}


DATA_OPENMU: dict[str, tuple[str, str, str]] = {
    "data.Account": (
        "MEMB_INFO (+ promote staging)",
        "in_progress",
        "Đích Gate2; không ghi đè Takumi MSSQL trong staging.",
    ),
    "data.AccountCharacterClass": ("DefaultClassType / vault unlock", "deferred", ""),
    "data.Character": ("dbo.Character", "in_progress", "Chữ lý Takumi EF promote."),
    "data.Item": ("decode Character.Inventory vault …", "deferred", ""),
    "data.ItemStorage": ("inventory + warehouse paths", "in_progress", "Blob MSSQL parsers."),
    "data.LetterHeader": ("T_FriendMail / mail", "deferred", ""),
    "data.LetterBody": ("T_FriendMail", "deferred", ""),
    "data.CharacterQuestState": ("QuestKillCount QuestWorld", "deferred", ""),
    "data.SkillEntry": ("skill bar legacy", "deferred", ""),
    "data.StatAttribute": ("stats str/dex...", "deferred", ""),
}


GUILD_OPENMU_NOTE = ("dbo.Guild + GuildMember", "todo", "Nguồn MSSQL Takumi guild.*")

FRIEND_OPENMU_NOTE = ("T_Friend*", "deferred", "Friend MU legacy không 1:1 EF friend schema")


HEURISTIC_PATCH: dict[str, tuple[str, str, str]] = {
    "CardPhone": ("Takumi không trong dbo bak", "wontfix-bak", "Chỉ C++; bỏ qua restore hiện tại."),
    "CHARACTER": ("dbo.Character", "resolved", "Casing đã chuẩn Character."),
    "EventInventory": ("plugin/EventInventory không trong bak", "wontfix-bak", "Nếu cần bảng tay plugin."),
    "memb_info": ("MEMB_INFO", "resolved", "Chuẩn SQL Server dbo.MEMB_INFO."),
    "MuRummyCard": ("plugin Season rummy", "deferred", "Không có trong snapshot."),
    "MuRummyData": ("plugin Season rummy", "deferred", ""),
    "MuunInventory": ("plugin Muun slots", "deferred", ""),
    "PcPointData": ("cash shop pts plugin", "deferred", ""),
    "PentagramJewel": ("pentagram jewels plugin", "deferred", ""),
    "PShopItemValue": ("personal shop plugin", "deferred", ""),
    "SNSData": ("social SN plugin", "deferred", ""),
}


def csv_escape_hint(s: str) -> None:
    if "\n" in s:
        sys.exit("unsupported newline in csv field")


def main() -> int:
    rows: list[list[str]] = []
    with TARGET.open(newline="", encoding="utf-8") as f:
        rdr = csv.reader(f)
        for row in rdr:
            rows.append(row)
    if not rows:
        sys.exit("empty csv")
    header, body = rows[0], rows[1:]
    if header[:5] != ["kind", "legacy_name", "openmu_or_plugin", "parity_status", "notes"]:
        sys.exit(f"unexpected header: {header!r}")

    out: list[list[str]] = [header[:5]]

    for row in body:
        while len(row) < 5:
            row.append("")
        kind, legacy, tgt, parity, notes = row[0], row[1], row[2], row[3], row[4]
        if kind == "LEGACY_TABLE" and legacy in LEGACY_TABLE:
            t, p, n = LEGACY_TABLE[legacy]
            tgt = t
            parity = p
            notes = n or (
                f"Takumi dbo.{legacy} — PHASE2 §0 / inspector: "
                f"takumi-mssql-inspect --table {legacy}"
            )
        elif kind == "LEGACY_PROC" and legacy in PROC_PATCH:
            tgt, parity, notes = PROC_PATCH[legacy]

        elif kind == "OPENMU_TABLE":
            name = legacy
            schema = legacy.split(".", 1)[0]
            if name in DATA_OPENMU:
                lk, pv, nt = DATA_OPENMU[name]
                tgt = lk
                parity = pv
                notes = nt
            elif schema == "config":
                parity = parity if parity != "todo" else "n/a-openmu-world"
                if not tgt.strip():
                    tgt = "OpenMU EF world config (dbo Takumi n/a)"
                if "OpenMU EF snapshot" in notes or not notes.strip():
                    notes = "Cấu hình MU OpenMU; Takumi chỉ có file INI/Data — dbo không tương đương."
            elif name == "friend.Friend":
                tgt = FRIEND_OPENMU_NOTE[0]
                parity = FRIEND_OPENMU_NOTE[1]
                notes = FRIEND_OPENMU_NOTE[2]
            elif name.startswith("guild."):
                tgt = GUILD_OPENMU_NOTE[0]
                parity = GUILD_OPENMU_NOTE[1]
                notes = GUILD_OPENMU_NOTE[2]
            else:
                parity = parity if parity != "todo" else "deferred"
                notes = notes or ""

        if kind == "LEGACY_PROC" and legacy.startswith("WZ_CS_") and parity == "todo":
            parity = "deferred"
            if not tgt.strip():
                tgt = "guild/siege svc"
            if "siege" not in notes.lower() and not notes.strip():
                notes = "Takumi siege procs; đối chiếu persistence guild OpenMU / plugin."

        elif kind == "HEURISTIC_VERIFY" and legacy in HEURISTIC_PATCH:
            t, p, n = HEURISTIC_PATCH[legacy]
            tgt = t
            parity = p
            notes = n

        csv_escape_hint("|".join([kind, legacy, tgt, parity, notes]))
        out.append([kind, legacy, tgt, parity, notes])

    with TARGET.open("w", newline="", encoding="utf-8") as f:
        w = csv.writer(f, quoting=csv.QUOTE_MINIMAL)
        for row in out:
            w.writerow(row)

    print(f"wrote {len(out) - 1} data rows (+ header) → {TARGET}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
