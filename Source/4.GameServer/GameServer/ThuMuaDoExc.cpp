#include "stdafx.h"
#include "ThuMuaDoExc.h"
#include "Monster.h"
#include "Protocol.h"
#include "Util.h"
#include "MemScript.h"
#include "Message.h"
#include "Notice.h"
#include "User.h"
#include "ItemManager.h"
#include "Map.h"
#include "ScheduleManager.h"
#include "ObjectManager.h"
#include "DSProtocol.h"
#include "SkillManager.h"
#include "MuunSystem.h"
#include "GensSystem.h"
#include "PcPoint.h"
#include "Trade.h"
#include "CashShop.h"
#include "ItemBagManager.h"
#include "Log.h"
#if (BOTHUMUADORAC == 1)

BotThuMuaer BotThuMua;

//CMemScript objScript;
void BotThuMuaer::Read(char * FilePath)
{
	if (this->Enabled == true)
	{
		for (int botNum = 0; botNum<MAX_BOTTHUMUA; botNum++)
		{
			if (this->bot[botNum].Enabled == true)
			{
				int bIndex = this->bot[botNum].index;
				if (gObjIsConnected(bIndex) == TRUE)
				{
					gObjDel(bIndex);
				}
			}
		}
	}
	this->m_DoiItem.clear();
	this->m_DoiItemList.clear();
	ZeroMemory(this->CacheSerial, sizeof(this->CacheSerial));

	int CountBot = 0;
	int CountItem = 0;
	pugi::xml_document file;
	pugi::xml_parse_result res = file.load_file(FilePath);
	memset(&this->bot, 0, sizeof(this->bot));
	if (res.status != pugi::status_ok){
		ErrorMessageBox("File %s load fail. Error: %s", FilePath, res.description());
		return;
	}

	pugi::xml_node BotThuMua = file.child("BotThuMua");
	this->Enabled = BotThuMua.attribute("EnableBot").as_bool();

	pugi::xml_node BCreateBot = BotThuMua.child("CreateBot");
	for (pugi::xml_node CreateBot = BCreateBot.child("NewBot"); CreateBot; CreateBot = CreateBot.next_sibling()){
		int BotNum = CreateBot.attribute("IndexBot").as_int();
		if (BotNum < 0 || BotNum > MAX_BOTTHUMUA - 1){
			ErrorMessageBox("BotThuMua error: Bot Index:%d out of range!", BotNum);
			return;
		}
		this->bot[BotNum].Class = CreateBot.attribute("ClassBot").as_int();
		this->bot[BotNum].Rate = CreateBot.attribute("SucessRateBot").as_int();
		this->bot[BotNum].Type = CreateBot.attribute("Type").as_int(); //Loai
		strcpy_s(this->bot[BotNum].Name, CreateBot.attribute("NameBot").as_string());
		this->bot[BotNum].Map = CreateBot.attribute("IDMapBot").as_int();
		this->bot[BotNum].X = CreateBot.attribute("CoordXBot").as_int();
		this->bot[BotNum].Y = CreateBot.attribute("CoordYBot").as_int();
		this->bot[BotNum].Dir = CreateBot.attribute("DirBot").as_int();
		this->bot[BotNum].Enabled = true;
		CountBot++;
	}
	//===
	pugi::xml_node ItemBot = BotThuMua.child("ItemBot");
	for (pugi::xml_node ItemsBot = ItemBot.child("ItemsBot"); ItemsBot; ItemsBot = ItemsBot.next_sibling()){
		int BotNum = ItemsBot.attribute("IndexBot").as_int();
		if (BotNum < 0 || BotNum > MAX_BOTTHUMUA - 1){
			ErrorMessageBox("BotThuMua error: Bot Index:%d out of range!", BotNum);
			return;
		}
		if (this->bot[BotNum].Enabled == false){
			ErrorMessageBox("BotThuMua error: Bot Index:%d doesnt exist", BotNum);
			return;
		}
		int Slot = ItemsBot.attribute("SlotItemBot").as_int();
		if (Slot < 0 || Slot > 8){
			ErrorMessageBox("BotThuMua error: Min Slot 0 ; Max Slot 8");
			return;
		}
		int iType = ItemsBot.attribute("TypeItemBot").as_int();
		int iIndex = ItemsBot.attribute("IndexItemBot").as_int();
		this->bot[BotNum].body[Slot].num = GET_ITEM(iType, iIndex);
		this->bot[BotNum].body[Slot].level = ItemsBot.attribute("LevelItemBot").as_int();
		this->bot[BotNum].body[Slot].opt = ItemsBot.attribute("OptionItemBot").as_int();
		this->bot[BotNum].body[Slot].Enabled = true;
	}
	//====
	pugi::xml_node cDoiItem = BotThuMua.child("DoiItem");
	for (pugi::xml_node oDoiItem = cDoiItem.child("OpBot"); oDoiItem; oDoiItem = oDoiItem.next_sibling())
	{
		DoiItem info;

		info.IndexBot = oDoiItem.attribute("IndexBot").as_int();
		info.OnlyVip = oDoiItem.attribute("OnlyVip").as_int();
		info.LoaiTien = oDoiItem.attribute("LoaiTien").as_int();
		info.MinCoin = oDoiItem.attribute("MinCoin").as_int();
		info.MaxCoin = oDoiItem.attribute("MaxCoin").as_int();
		info.SeriCheck = oDoiItem.attribute("SeriCheck").as_int();
		info.ItemExc = oDoiItem.attribute("ItemExc").as_int();
		info.ItemAnc = oDoiItem.attribute("ItemAnc").as_int();
		info.ItemBag = oDoiItem.attribute("ItemBag").as_int();
		this->m_DoiItem.insert(std::pair<int, DoiItem>(info.IndexBot, info));
	}
	//====
	pugi::xml_node cDoiItemList = BotThuMua.child("DoiItemList");
	for (pugi::xml_node oDoiItemList = cDoiItemList.child("ListItem"); oDoiItemList; oDoiItemList = oDoiItemList.next_sibling())
	{
		DoiItemList info;
		info.Index = CountItem;
		info.IndexBot = oDoiItemList.attribute("IndexBot").as_int();
		info.ItemIndex = oDoiItemList.attribute("ItemIndex").as_int();
		info.LoaiTien = oDoiItemList.attribute("LoaiTien").as_int();
		info.MinCoin = oDoiItemList.attribute("MinCoin").as_int();
		info.MaxCoin = oDoiItemList.attribute("MaxCoin").as_int();
		info.SeriCheck = oDoiItemList.attribute("SeriCheck").as_int();
		info.ItemBag = oDoiItemList.attribute("ItemBag").as_int();
		this->m_DoiItemList.insert(std::pair<int, DoiItemList>(info.Index, info));
		CountItem++;
	}

	pugi::xml_node Message = BotThuMua.child("MessageInfo");
	for (pugi::xml_node msg = Message.child("Message"); msg; msg = msg.next_sibling())
	{
		MESSAGE_INFO_TM info;

		info.Index = msg.attribute("Index").as_int();

		strcpy_s(info.Message, msg.attribute("Text").as_string());

		this->m_MessageInfoTM.insert(std::pair<int, MESSAGE_INFO_TM>(info.Index, info));
	}


	LogAdd(LOG_BLUE, "[BotThuMua]Bot: %d DoiItem: %d  DoiItemList:%d", CountBot, this->m_DoiItem.size(), this->m_DoiItemList.size());
}

DoiItem* BotThuMuaer::GetInfoDoiItem(int Numbot) // Lay thong tin
{
	for (std::map<int, DoiItem>::iterator it = this->m_DoiItem.begin(); it != this->m_DoiItem.end(); it++)
	{
		if (Numbot == it->second.IndexBot)
		{
			return &it->second;
		}
	}
	return 0;
}

char* BotThuMuaer::GetMessage(int index) // OK
{
	std::map<int, MESSAGE_INFO_TM>::iterator it = this->m_MessageInfoTM.find(index);

	if (it == this->m_MessageInfoTM.end())
	{
		char Error[256];
		wsprintf(Error, "Could not find message %d!", index);
		return Error;
	}
	else
	{
		return it->second.Message;
	}
}


DoiItemList* BotThuMuaer::GetInfoDoiItemList(int Numbot, int ItemIndex) // Lay thong tin
{
	for (std::map<int, DoiItemList>::iterator it = this->m_DoiItemList.begin(); it != this->m_DoiItemList.end(); it++)
	{
		if (Numbot == it->second.IndexBot && ItemIndex == it->second.ItemIndex)
		{
			return &it->second;
		}
	}
	return 0;
}



bool BotThuMuaer::GetCoinDoiItem(int Numbot, int* Coin) // OK
{
	for (std::map<int, DoiItem>::iterator it = this->m_DoiItem.begin(); it != this->m_DoiItem.end(); it++)
	{
		if (Numbot == it->second.IndexBot)
		{
				(*Coin) += random(it->second.MinCoin,it->second.MaxCoin);
				return 1;
		}
	}

	return 0;
}


int BotThuMuaer::GetBotIndex(int aIndex)
{
	for (int i = 0; i<MAX_BOTTHUMUA; i++)
	{
		if (this->bot[i].Enabled == 1)
		{
			if (this->bot[i].index == aIndex)
				return i;
		}
	}
	return -1;
}

void BotThuMuaer::MakeBot()
{

	if (this->Enabled == 1)
	{

		for (int botNum = 0; botNum<MAX_BOTTHUMUA; botNum++)
		{
			if (this->bot[botNum].Enabled == 1)
			{

				int result = gObjAddSummon();

				if (result >= 0)
				{
					this->bot[botNum].index = result;
					gObj[result].PosNum = (WORD)-1;
					gObj[result].X = this->bot[botNum].X;
					gObj[result].Y = this->bot[botNum].Y;
					gObj[result].OldX = this->bot[botNum].X;
					gObj[result].OldY = this->bot[botNum].Y;
					gObj[result].TX = this->bot[botNum].X;
					gObj[result].TY = this->bot[botNum].Y;
					gObj[result].MTX = this->bot[botNum].X;
					gObj[result].MTY = this->bot[botNum].Y;
					gObj[result].Dir = this->bot[botNum].Dir;
					gObj[result].Map = this->bot[botNum].Map;
					gObj[result].Live = 1;
					gObj[result].PathCount = 0;
					gObj[result].IsBot = 2;
					gObj[result].BotPower = 0;

					gObjSetBots(result, this->bot[botNum].Class);

					gObj[result].ChangeUp = this->bot[botNum].Class;
					gObj[result].Class = this->bot[botNum].Class;
					gObj[result].Level = 400;
					gObj[result].Life = 1;
					gObj[result].MaxLife = 2;
					gObj[result].Mana = 1;
					gObj[result].MaxMana = 2;
					gObj[result].Experience = 0;
					gObj[result].DBClass = this->bot[botNum].Class;
					gObj[result].PKLevel = this->bot[botNum].ChangeColorName;
					gObj[result].GensFamily = this->bot[botNum].GensFamily;
					gObj[result].Inventory = new CItem[INVENTORY_SIZE];
					gObj[result].Inventory1 = new CItem[INVENTORY_SIZE];
					gObj[result].InventoryMap1 = new BYTE[INVENTORY_SIZE];
					gObj[result].InventoryMap = new BYTE[INVENTORY_SIZE];


					for (int i = 0; i<INVENTORY_SIZE; i++)
					{
						gObj[result].Inventory[i].Clear();
						gObj[result].Inventory1[i].Clear();
					}

					memset(&gObj[result].InventoryMap[0], (BYTE)-1, INVENTORY_SIZE);
					memset(&gObj[result].InventoryMap1[0], (BYTE)-1, INVENTORY_SIZE);

#if (GAMESERVER_UPDATE >= 801)
					gObj[result].MuunInventory = new CItem[MUUN_INVENTORY_SIZE];
					gObj[result].MuunInventoryMap = new BYTE[MUUN_INVENTORY_SIZE];

					for (int i = 0; i<MUUN_INVENTORY_SIZE; i++)
					{
						gObj[result].MuunInventory[i].Clear();
					}

					memset(&gObj[result].MuunInventoryMap[0], (BYTE)-1, MUUN_INVENTORY_SIZE); //novo
#endif

					//strcpy_s(gObj[result].Name, this->bot[botNum].Name);

					strncpy_s(gObj[result].Name, this->bot[botNum].Name,sizeof(gObj[result].Name));

					for (int i = 0; i<9; i++)
					{
						if (this->bot[botNum].body[i].num >= 0 && this->bot[botNum].body[i].Enabled == 1)
						{
							CItem item;
							item.m_Level = this->bot[botNum].body[i].level;
							item.m_Option1 = 0;
							item.m_Option2 = 1;
							item.m_Option3 = this->bot[botNum].body[i].opt;
							item.m_Durability = 255.0f;
							item.m_JewelOfHarmonyOption = 0;
							item.m_ItemOptionEx = 0;
							item.m_SocketOption[1] = 0;
							item.m_SocketOption[2] = 0;
							item.m_SocketOption[3] = 0;
							item.m_SocketOption[4] = 0;
							item.m_SocketOption[5] = 0;
							item.Convert(this->bot[botNum].body[i].num, item.m_Option1, item.m_Option2, item.m_Option3, item.m_NewOption, item.m_SetOption, item.m_JewelOfHarmonyOption, item.m_ItemOptionEx, item.m_SocketOption, item.m_SocketOptionBonus);
							gObj[result].Inventory[i].m_Option1 = item.m_Option1;
							gObj[result].Inventory[i].m_Option2 = item.m_Option2;
							gObj[result].Inventory[i].m_Option3 = item.m_Option3;
							gObj[result].Inventory[i].m_JewelOfHarmonyOption = item.m_JewelOfHarmonyOption;
							gObj[result].Inventory[i].m_ItemOptionEx = item.m_ItemOptionEx;

							item.m_Number = 0;
							gObjInventoryInsertItemPos(gObj[result].Index, item, i, 0);
						}
					}

					gObj[result].Inventory1 = gObj[result].Inventory;
					gObj[result].InventoryMap1 = gObj[result].InventoryMap;

					gObjectManager.CharacterMakePreviewCharSet(result);

					gObj[result].AttackType = 0;
					gObj[result].BotSkillAttack = 0;

					gObj[result].Attribute = 100;
					gObj[result].TargetNumber = (WORD)-1;
					gObj[result].ActionState.Emotion = 0;
					gObj[result].ActionState.Attack = 0;
					gObj[result].ActionState.EmotionCount = 0;
					gObj[result].PathCount = 0;
					gObj[result].BotPower = 0;
					gObj[result].BotDefense = this->bot[botNum].OnlyVip;
					gObj[result].BotLife = 1;
					gObj[result].BotMaxLife = 1;

					gObj[result].BotLvlUpDefense = 1;
					gObj[result].BotLvlUpPower = 1;
					gObj[result].BotLvlUpLife = 1;
					gObj[result].BotLvlUpMana = 1;
					gObj[result].BotLvlUpExp = 1;
					gObj[result].BotLvlUpMaxLevel = 1;

					gObj[result].MoveRange = 1;
					gObj[result].BotFollowMe = 0;
					gObj[result].NextExperience = gLevelExperience[gObj[result].Level];


					GCSummonLifeSend(gObj[result].SummonIndex, (int)gObj[result].MaxLife, (int)gObj[result].Life);

					GCActionSend(&gObj[result], ACTION_SALUTE1, result, result);

				}
			}
		}
	}
}


BYTE BotThuMuaer::gObjInventoryInsertItemPos(int aIndex, CItem item, int pos, BOOL RequestCheck) //OK
{

	if (OBJECT_RANGE(aIndex) == 0)
	{
		return -1;
	}

	/*if(pos < 0 || pos > INVENTORY_SIZE_CUSTOM(aIndex,false))
	{
	return -1;
	}*/

	LPOBJ lpObj;

	int useClass = 0;

	lpObj = &gObj[aIndex];

	if (lpObj->Inventory[pos].IsItem() == 1)
	{
		return -1;
	}

	if (item.IsItem() == 0)
	{
		return -1;
	}

	if (pos < 12)
	{
		if (lpObj->IsBot == 0)
		{
			useClass = item.IsClassBot((char)lpObj->Class, lpObj->ChangeUp);

			if (useClass == 0)
			{
				LogAdd(LOG_BLUE, "[Using Class Error] Error UseClass %d", useClass);
				return -1;
			}

		}
	}

	lpObj->Inventory[pos] = item;

	return pos;
}

BOOL BotThuMuaer::IsInTrade(int aIndex)
{
	int number = this->GetBotIndex(gObj[aIndex].TargetNumber);
	if (number != -1)
		return 1;
	else
		return 0;
}


int BotThuMuaer::getNumberOfExcOptions(int checksum)
{
	int optionscount = 0;
	int ExcOrgArr[6];

	ExcOrgArr[0] = 1;
	ExcOrgArr[1] = 2;
	ExcOrgArr[2] = 4;
	ExcOrgArr[3] = 8;
	ExcOrgArr[4] = 16;
	ExcOrgArr[5] = 32;

	if (checksum>0)
	{
		for (int i = 0; i<6; i++)
		{
			int and_val = checksum & ExcOrgArr[i];
			if (and_val != 0)
				optionscount += 1;
		}
	}

	return optionscount;
}


bool BotThuMuaer::AllowExc(BYTE BotNum, BYTE ExcOpt)
{
	int dwExOpCount = this->getNumberOfExcOptions(ExcOpt);

	if (dwExOpCount < this->bot[BotNum].MaxExc)
	{
		return true;
	}
	return false;
}

BOOL BotThuMuaer::XuLyItemThuMua(int aIndex, int BotNum)
{
	char sbuf[512] = { 0 };
	int fitem = -1;
	int count = 0;

	int Number = aIndex - OBJECT_START_USER;

	if (gObjIsConnected(aIndex) == 0)
	{
		return 0;
	}
	int failed = false;
	int Coin = 0;
	int LoaiTienTe = -1;
	int LoaiItemBag = -1;


	LPOBJ lpObj = &gObj[aIndex];

	if (this->bot[BotNum].Type == 1) //Doi Item THeo List
	{
		//DoiItemList *cGetInfoDoiItemList = this->GetInfoDoiItemList(BotNum);
		int CacheItemIndex = -1;
		for (int n = 0; n < TRADE_SIZE; n++)
		{
			if (lpObj->Trade[n].IsItem() != 0)
			{
				//LogAdd(LOG_RED, "DEBUG BOT THUMUA 0x%.8X Item %d", lpObj->Trade[n].m_Serial, lpObj->Trade[n].m_Index);
				DoiItemList *cGetInfoDoiItemList;
				if (CacheItemIndex == -1) {

					cGetInfoDoiItemList = this->GetInfoDoiItemList(BotNum, lpObj->Trade[n].m_Index);
					CacheItemIndex = lpObj->Trade[n].m_Index;
				}
				if (cGetInfoDoiItemList != 0 && CacheItemIndex == lpObj->Trade[n].m_Index)
				{
					if (((cGetInfoDoiItemList->SeriCheck == 1 && lpObj->Trade[n].m_Serial != 0x00000000) || cGetInfoDoiItemList->SeriCheck == 0) && lpObj->Trade[n].m_Serial != this->CacheSerial[Number])
					{
						if (cGetInfoDoiItemList->MinCoin < cGetInfoDoiItemList->MaxCoin) {
							Coin += random(cGetInfoDoiItemList->MinCoin, cGetInfoDoiItemList->MaxCoin);
						}
						else {
							Coin += cGetInfoDoiItemList->MaxCoin;
						}

						LoaiTienTe = cGetInfoDoiItemList->LoaiTien;
						LoaiItemBag = cGetInfoDoiItemList->ItemBag;		
						this->CacheSerial[Number] = lpObj->Trade[n].m_Serial;
						count++;
					}
					else {

						failed = true;
					}

					
					

				}
				else {
					failed = true;
				}

			}
		}
	}
	else {

		DoiItem *cGetInfoDoiItem = this->GetInfoDoiItem(BotNum);
		if (!cGetInfoDoiItem)
		{
			ChatSend(&gObj[this->bot[BotNum].index], this->GetMessage(5));
			this->TradeCancel(aIndex);
			gTrade.ResetTrade(aIndex);
			gTrade.GCTradeResultSend(aIndex, 5);
			return 0;

		}
		for (int n = 0; n < TRADE_SIZE; n++)
		{
			if (lpObj->Trade[n].IsItem() != 0)
			{
				int Slot = gItemManager.GetItemSlotByIndex(lpObj->Trade[n].m_Index);

				if ((cGetInfoDoiItem->ItemAnc == 1 && !lpObj->Trade[n].IsSetItem()))
				{
					failed = true;
				}
				else if ((cGetInfoDoiItem->ItemExc == 1 && !lpObj->Trade[n].IsExcItem()))
				{
					failed = true;
				}

	 
				else if (Slot >= 2 && Slot <= 6 || Slot >= 0 && Slot <= 1 || Slot >= 9 && Slot <= 10) 
				{
					if (((cGetInfoDoiItem->SeriCheck == 1 && lpObj->Trade[n].m_Serial != 0x00000000) || cGetInfoDoiItem->SeriCheck == 0) && lpObj->Trade[n].m_Serial != this->CacheSerial[Number])
					{
						if (cGetInfoDoiItem->MinCoin < cGetInfoDoiItem->MaxCoin) 
						{
							Coin += random(cGetInfoDoiItem->MinCoin, cGetInfoDoiItem->MaxCoin);
						}
						else 
						{
							Coin += cGetInfoDoiItem->MaxCoin;
						}

						LoaiTienTe = cGetInfoDoiItem->LoaiTien;
						LoaiItemBag = cGetInfoDoiItem->ItemBag;			
						this->CacheSerial[Number] = lpObj->Trade[n].m_Serial;
						count++;
					}
					else {

						failed = true;
					}
				}
				else {

					failed = true;
				}


			}
		}

	}
	
	
	if (!failed && LoaiTienTe != -1)
	{
		srand(static_cast<int>(time(NULL)));
		int random = rand() % 100;
		bool failed = false;

			if (random < this->bot[BotNum].Rate)
			{

				ChatSend(&gObj[this->bot[BotNum].index], this->GetMessage(0));
				//LogAdd(LOG_BLUE, "[THUMUA]BotNum[%d] [%s] Da Doi (%d) Item Nhan Duoc (%d) Coin Type(%d)", BotNum, lpObj->Name, count, Coin, LoaiTienTe);
				//gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, "[% s] Đã đổi(% d) Item Nhận được (% d) WCoinC", lpObj->Name, count, Coin, 0, 0);
				//gLog.Output(LOG_TRADE,"[THUMUA]BotNum[%d] [%s] Da Doi (%d) Item Nhan Duoc (%d) Coin Type(%d) | ItemBag: %d   ", BotNum, lpObj->Name, count, Coin, LoaiTienTe, LoaiItemBag );
				gLog.Output(LOG_THUMUA, "[ThuMuaEx]: Account:[%s]- Name[%s] ", lpObj->Account, lpObj->Name);
				for (int n = 0; n < TRADE_SIZE; n++)
				{
					if (lpObj->Trade[n].IsItem() != 0)
					{
						gLog.Output(LOG_THUMUA, "[ThuMuaEx] - [%s +%d] - DB[%.0f] TC[%d] Luck[%d] Op[%d] Exc[%d] Set[%d] oEx[%d] Har[%d] Socket[%03d,%03d,%03d,%03d,%03d][%d] Seri[%08X]",
							gItemManager.GetItemName(lpObj->Trade[n].m_Index),
							lpObj->Trade[n].m_Level,
							lpObj->Trade[n].m_Durability,
							lpObj->Trade[n].m_Option1,
							lpObj->Trade[n].m_Option2,
							lpObj->Trade[n].m_Option3,
							lpObj->Trade[n].m_NewOption,
							lpObj->Trade[n].m_SetOption,
							lpObj->Trade[n].m_ItemOptionEx,
							lpObj->Trade[n].m_JewelOfHarmonyOption,
							lpObj->Trade[n].m_SocketOption[0],
							lpObj->Trade[n].m_SocketOption[1],
							lpObj->Trade[n].m_SocketOption[2],
							lpObj->Trade[n].m_SocketOption[3],
							lpObj->Trade[n].m_SocketOption[4],
							lpObj->Trade[n].m_SocketOptionBonus,
							lpObj->Trade[n].m_Serial);
					}
				}
				switch (LoaiTienTe)
				{
					case 1:
					{
						GDSetCoinSend(lpObj->Index, +Coin, 0, 0, "ThuMuaDoRac");
						gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, gMessage.GetMessage(565), lpObj->Name, Coin, 0, 0);

						gLog.Output(LOG_WC, "[THUMUA][%s] Da Doi (%d) Item Nhan Duoc (%d) CoinC ", lpObj->Name, count, Coin);
					} break;
					case 2:
					{
						GDSetCoinSend(lpObj->Index, 0, +Coin, 0, "ThuMuaDoRac");
						//gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, gMessage.GetMessage(565), lpObj->Name, 0, Coin, 0);
					} break;
					case 3:
					{
						GDSetCoinSend(lpObj->Index, 0, 0, +Coin, "ThuMuaDoRac");
						//gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, gMessage.GetMessage(565), lpObj->Name, 0, 0, Coin);
					} break;
					default:
					{
						lpObj->Money += Coin;
						//gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, gMessage.GetMessage(565), lpObj->Name, 0, 0, Coin);
					} break;

				}
				if (LoaiItemBag != -1)
				{
					gItemBagManager.DropItemByIndexItemBag(LoaiItemBag, lpObj); //Gui Item cho NV
				}
			}
			else
			{
				ChatSend(&gObj[this->bot[BotNum].index], this->GetMessage(1));
			}

			gObjInventoryCommit(aIndex);
			gObjectManager.CharacterMakePreviewCharSet(aIndex);
			GDCharacterInfoSaveSend(aIndex);
			GDPetItemInfoSend(aIndex, 0);
			lpObj->Interface.use = 0;
			lpObj->Interface.type = INTERFACE_NONE;
			lpObj->Interface.state = 0;
			lpObj->TargetNumber = -1;
			lpObj->TradeOk = 0;
			lpObj->TradeOkTime = 0;
			lpObj->TradeMoney = 0;
			GCMoneySend(aIndex, lpObj->Money);
			gTrade.GCTradeResultSend(aIndex, 1);
			gItemManager.GCItemListSend(aIndex);
			gTrade.ClearTrade(lpObj);
			return 1;
	}
	else 
	{
		ChatSend(&gObj[this->bot[BotNum].index], this->GetMessage(3));
		this->TradeCancel(aIndex);
		gTrade.ResetTrade(aIndex);
		gTrade.GCTradeResultSend(aIndex, 5);
		return 0;
	}



}


void BotThuMuaer::TradeOk(int aIndex)
{
	int MixNum=-1;
	int MixItem=-1;

	int number = this->GetBotIndex(gObj[aIndex].TargetNumber);

	if(number == -1)
	{			
		gObj[aIndex].Interface.state = 0;
		gObj[aIndex].Interface.type = 0;
		gObj[aIndex].TargetShopNumber = -1;
		gObj[aIndex].Transaction = 0;
		this->TradeCancel(aIndex);
		gTrade.ClearTrade(&gObj[aIndex]);
		gTrade.ResetTrade(aIndex);
		gTrade.GCTradeResultSend(aIndex, 5);
		return;
	}
	this->XuLyItemThuMua(aIndex, number);
}

BOOL BotThuMuaer::TradeOpen(int index, int nindex)
{
	if(gObjIsConnected(index) == 0)
	{
		return 0;
	}
	if(gObjIsConnected(nindex) == 0)
	{
		return 0;
	}

	int number = this->GetBotIndex(nindex);
	if(number == -1) return 0;

	char sbuf[512]={0};
	LPOBJ lpObj = &gObj[index];
	LPOBJ lpBot = &gObj[nindex];

	if(this->bot[number].OnlyVip == 1 && gObj[index].AccountLevel == 0)
	{
		ChatSend(&gObj[this->bot[number].index],this->GetMessage(2));
		LogAdd(LOG_RED,"[BotThuMua][Bot:%d](%s) Account is not VIP",number,gObj[index].Account);
		return 0;
	}
	
	if(this->Enabled == TRUE)
	{
		//Fix Dupe
		if (lpObj->Interface.type != INTERFACE_NONE || lpObj->Interface.use != 0)
		{
			return 0;
		}
		else
		{	
			gTrade.ClearTrade(lpObj);
			gObjInventoryTransaction(lpObj->Index);
			gTrade.GCTradeResponseSend(lpObj->Index,1,lpBot->Name, 400, 0);
			gTrade.GCTradeOkButtonSend(lpObj->Index, 1);
			lpObj->Interface.state = 1;
			lpObj->Interface.use = 1;
			lpObj->Interface.type = 1;
			lpObj->TradeMoney = 0;
			lpObj->TargetNumber =lpBot->Index;
			lpObj->Transaction = 1;

			char wbuf[1024]={0};
			wsprintf(wbuf, "[BotThuMua] (%s)(%s) OPEN Type[%d],State[%d], Use[%d]", gObj[index].Account, gObj[index].Name, lpObj->Interface.type, lpObj->Interface.state, lpObj->Interface.use);
			LogAdd(LOG_RED,wbuf);
			ChatSend(&gObj[this->bot[number].index],this->GetMessage(4));			
		}
	}
	return 1;
}

void BotThuMuaer::TradeCancel(int aIndex)
{
	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	
	if(lpObj->Interface.use == 0 || lpObj->Interface.type != INTERFACE_TRADE || lpObj->Interface.state == 0)
	{
		return;
	}
	gTrade.ResetTrade(aIndex);
	gTrade.GCTradeResultSend(aIndex, 0);

}


#endif
