#include "StdAfx.h"
#include "CB_BotTrader.h"
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
#include <unordered_set>

CB_BotTrader	gCB_BotTrader;


CB_BotTrader::CB_BotTrader()
{
}


CB_BotTrader::~CB_BotTrader()
{
}
char* CB_BotTrader::GetMessage(int index) // OK
{
	std::map<int, MESSAGE_INFO>::iterator it = this->m_MessageInfo.find(index);

	if (it == this->m_MessageInfo.end())
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
void CB_BotTrader::ClearObjBot()
{
	for (auto it = m_DataCreateBot.begin(); it != m_DataCreateBot.end(); ++it)
	{
		int bIndex = it->second.Cache_aIndex;
		if (bIndex != -1)
		{
			if (gObjIsConnected(bIndex) == TRUE)
			{
				gObjDel(bIndex);
				it->second.Cache_aIndex = -1;
			}
		}
	}
}
void CB_BotTrader::LoadXML(char* FilePath)
{
	CB_BotTrader::ClearObjBot();

	pugi::xml_document file;
	pugi::xml_parse_result res = file.load_file(FilePath);
	if (res.status != pugi::status_ok) {
		ErrorMessageBox("File %s load fail. Error: %s", FilePath, res.description());
		return;
	}

	pugi::xml_node BotTrade = file.child("BotTrade");
	this->Enabled = BotTrade.attribute("Enabled").as_int();


	this->m_DataCreateBot.clear();
	pugi::xml_node BCreateBot = BotTrade.child("CreateBot");
	for (pugi::xml_node CreateBot = BCreateBot.child("NewBot"); CreateBot; CreateBot = CreateBot.next_sibling())
	{
		DATA_CREATE_BOT info;
		info.m_DataItemBot.clear();
		info.MixIndexMin = -1;
		info.MixIndexMax = -1;
		info.Cache_aIndex = -1;


		info.IndexBot = CreateBot.attribute("IndexBot").as_int();
		info.Class = CreateBot.attribute("Class").as_int();
		info.Enable = CreateBot.attribute("Enabled").as_int();
		info.OnylVip = CreateBot.attribute("OnlyVip").as_int();
		strcpy_s(info.BotName, CreateBot.attribute("BotName").as_string());
		info.Map = CreateBot.attribute("MapID").as_int();
		info.CoordX = CreateBot.attribute("CoordX").as_int();
		info.CoordY = CreateBot.attribute("CoordY").as_int();
		info.Dir = CreateBot.attribute("Dir").as_int();
		info.CustomShowInfo = CreateBot.attribute("CustomShowInfo").as_int();
		
		this->m_DataCreateBot.insert(std::pair<int, DATA_CREATE_BOT>(info.IndexBot, info));
	}

	pugi::xml_node bItemsBot = BotTrade.child("ItemsBot");
	for (pugi::xml_node ItemsBot = bItemsBot.child("Items"); ItemsBot; ItemsBot = ItemsBot.next_sibling())
	{
		int bIndexBot = ItemsBot.attribute("IndexBot").as_int();
		DATA_ITEM_BOT infoItem;
		infoItem.SlotItem = ItemsBot.attribute("SlotItem").as_int();
		infoItem.TypeItem = ItemsBot.attribute("TypeItem").as_int();
		infoItem.IndexItem = ItemsBot.attribute("IndexItem").as_int();
		infoItem.LevelItem = ItemsBot.attribute("LevelItem").as_int();
		infoItem.OptionItem = ItemsBot.attribute("OptionItem").as_int();

		std::map<int, CB_BotTrader::DATA_CREATE_BOT>::iterator it = this->m_DataCreateBot.find(bIndexBot);
		if (it == this->m_DataCreateBot.end())
		{
			continue;
		}

		it->second.m_DataItemBot.push_back(infoItem);
	}
	pugi::xml_node bBotAsociation = BotTrade.child("BotAsociation");
	for (pugi::xml_node BotAsociation = bBotAsociation.child("Bot"); BotAsociation; BotAsociation = BotAsociation.next_sibling())
	{
		int cIndexBot = BotAsociation.attribute("IndexBot").as_int();

		std::map<int, CB_BotTrader::DATA_CREATE_BOT>::iterator it = this->m_DataCreateBot.find(cIndexBot);
		if (it == this->m_DataCreateBot.end())
		{
			continue;
		}
		it->second.MixIndexMin = BotAsociation.attribute("MixIndexMin").as_int();
		it->second.MixIndexMax = BotAsociation.attribute("MixIndexMax").as_int();
	}

	//===
	this->m_DataMix.clear();
	pugi::xml_node bMixData = BotTrade.child("MixData");
	for (pugi::xml_node MixData = bMixData.child("Mix"); MixData; MixData = MixData.next_sibling())
	{
		DATA_MIX_DATA infoMixData;
		infoMixData.mDataNeedItem.clear();
		infoMixData.mDataResultItem.clear();

		infoMixData.MixIndex = MixData.attribute("MixIndex").as_int();
		strcpy_s(infoMixData.MixName, MixData.attribute("MixName").as_string());
		infoMixData.SuccessRate = MixData.attribute("SuccessRate").as_int();
		infoMixData.PriceType = MixData.attribute("PriceType").as_int();
		infoMixData.PriceCount = MixData.attribute("PriceCount").as_int();
		infoMixData.FailMes = MixData.attribute("FailMes").as_int();
		infoMixData.SuccessMes = MixData.attribute("SuccessMes").as_int();

		this->m_DataMix.insert(std::pair<int, DATA_MIX_DATA>(infoMixData.MixIndex, infoMixData));
	}


	//===

	pugi::xml_node bNeedItem = BotTrade.child("NeedItem");
	for (pugi::xml_node NeedItem = bNeedItem.child("Mix"); NeedItem; NeedItem = NeedItem.next_sibling())
	{
		DATA_NEED_ITEM infoNeedItem;
		int nMixIndex = NeedItem.attribute("MixIndex").as_int();

		infoNeedItem.Type = NeedItem.attribute("Type").as_int();
		infoNeedItem.Index = NeedItem.attribute("Index").as_int();
		infoNeedItem.MinLevel= NeedItem.attribute("MinLevel").as_int();
		infoNeedItem.MaxLevel= NeedItem.attribute("MaxLevel").as_int();
		infoNeedItem.MinOpt = NeedItem.attribute("MinOpt").as_int();
		infoNeedItem.MaxOpt = NeedItem.attribute("MaxOpt").as_int();
		infoNeedItem.Luck = NeedItem.attribute("Luck").as_int();
		infoNeedItem.Skill = NeedItem.attribute("Skill").as_int();
		infoNeedItem.NeedExc = NeedItem.attribute("NeedExc").as_int();
		infoNeedItem.Durability = NeedItem.attribute("Durability").as_int();
		infoNeedItem.KeepLevel = NeedItem.attribute("KeepLevel").as_int();
		infoNeedItem.KeepOption = NeedItem.attribute("KeepOption").as_int();
		infoNeedItem.KeepItemMixFail = NeedItem.attribute("KeepItemMixFail").as_int();

		std::map<int, CB_BotTrader::DATA_MIX_DATA>::iterator it = this->m_DataMix.find(nMixIndex);
		if (it == this->m_DataMix.end())
		{
			continue;
		}
		it->second.mDataNeedItem.push_back(infoNeedItem);

	}

	pugi::xml_node bDataResult = BotTrade.child("DataResult");
	for (pugi::xml_node DataResult = bDataResult.child("ItemResult"); DataResult; DataResult = DataResult.next_sibling())
	{
		DATA_RESULT infoResultItem;
		int rMixIndex = DataResult.attribute("MixIndex").as_int();

		infoResultItem.Type = DataResult.attribute("Type").as_int();
		infoResultItem.Index = DataResult.attribute("Index").as_int();
		infoResultItem.LevelMin = DataResult.attribute("LevelMin").as_int();
		infoResultItem.LevelMax = DataResult.attribute("LevelMax").as_int();
		infoResultItem.OptMin = DataResult.attribute("OptMin").as_int();
		infoResultItem.OptMax = DataResult.attribute("OptMax").as_int();
		infoResultItem.Luck = DataResult.attribute("Luck").as_int();
		infoResultItem.Skill = DataResult.attribute("Skill").as_int();
		infoResultItem.Exc = DataResult.attribute("Exc").as_int();
		infoResultItem.Durability = DataResult.attribute("Durability").as_int();


		std::map<int, CB_BotTrader::DATA_MIX_DATA>::iterator it = this->m_DataMix.find(rMixIndex);
		if (it == this->m_DataMix.end())
		{
			continue;
		}
		it->second.mDataResultItem.push_back(infoResultItem);
	}



	this->m_MessageInfo.clear();
	pugi::xml_node Message = BotTrade.child("MessageInfo");
	for (pugi::xml_node msg = Message.child("Message"); msg; msg = msg.next_sibling())
	{
		MESSAGE_INFO info;

		info.Index = msg.attribute("Index").as_int();

		strcpy_s(info.Message, msg.attribute("Text").as_string());

		this->m_MessageInfo.insert(std::pair<int, MESSAGE_INFO>(info.Index, info));
	}

	LogAdd(LOG_BLUE, "[BotTrade] Load COnffig OK Enable %d", this->Enabled);
}


BYTE CB_BotTrader::gObjInventoryInsertItemPos(int aIndex, CItem item, int pos, BOOL RequestCheck) //OK
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
void CB_BotTrader::MakeBot()
{

	if (this->Enabled == 1)
	{
		for (auto it = m_DataCreateBot.begin(); it != m_DataCreateBot.end(); ++it)
		{
			if (it->second.Enable == 1)
			{

				int result = gObjAddSummon();

				if (result >= 0)
				{
					it->second.Cache_aIndex = result;
					gObj[result].PosNum = (WORD)-1;
					gObj[result].X = it->second.CoordX;
					gObj[result].Y = it->second.CoordY;
					gObj[result].OldX = it->second.CoordX;
					gObj[result].OldY = it->second.CoordY;
					gObj[result].TX = it->second.CoordX;
					gObj[result].TY = it->second.CoordY;
					gObj[result].MTX = it->second.CoordX;
					gObj[result].MTY = it->second.CoordY;
					gObj[result].Dir = it->second.Dir;
					gObj[result].Map = it->second.Map;
					gObj[result].Live = 1;
					gObj[result].PathCount = 0;
					gObj[result].IsBot = 2;
					gObj[result].BotPower = 0;

					gObjSetBots(result, it->second.Class);

					gObj[result].ChangeUp = it->second.Class;
					gObj[result].Class = it->second.Class;
					gObj[result].Level = 400;
					gObj[result].Life = 1;
					gObj[result].MaxLife = 2;
					gObj[result].Mana = 1;
					gObj[result].MaxMana = 2;
					gObj[result].Experience = 0;
					gObj[result].DBClass = it->second.Class;
					gObj[result].PKLevel = 0;
					gObj[result].GensFamily = -1;
					gObj[result].Inventory = new CItem[INVENTORY_SIZE];
					gObj[result].Inventory1 = new CItem[INVENTORY_SIZE];
					gObj[result].InventoryMap1 = new BYTE[INVENTORY_SIZE];
					gObj[result].InventoryMap = new BYTE[INVENTORY_SIZE];


					for (int i = 0; i < INVENTORY_SIZE; i++)
					{
						gObj[result].Inventory[i].Clear();
						gObj[result].Inventory1[i].Clear();
					}

					memset(&gObj[result].InventoryMap[0], (BYTE)-1, INVENTORY_SIZE);
					memset(&gObj[result].InventoryMap1[0], (BYTE)-1, INVENTORY_SIZE);

#if (GAMESERVER_UPDATE >= 801)
					gObj[result].MuunInventory = new CItem[MUUN_INVENTORY_SIZE];
					gObj[result].MuunInventoryMap = new BYTE[MUUN_INVENTORY_SIZE];

					for (int i = 0; i < MUUN_INVENTORY_SIZE; i++)
					{
						gObj[result].MuunInventory[i].Clear();
					}

					memset(&gObj[result].MuunInventoryMap[0], (BYTE)-1, MUUN_INVENTORY_SIZE); //novo
#endif

					//strcpy_s(gObj[result].Name, this->bot[botNum].Name);

					strncpy_s(gObj[result].Name, it->second.BotName, sizeof(gObj[result].Name));

					//for (int i = 0; i < 9; i++)
					for (auto itItemAdd = it->second.m_DataItemBot.begin(); itItemAdd != it->second.m_DataItemBot.end(); ++itItemAdd)
					{
						int i = itItemAdd->SlotItem;
						CItem item;
						item.m_Level = itItemAdd->LevelItem;
						item.m_Option1 = 0;
						item.m_Option2 = 1;
						item.m_Option3 = itItemAdd->OptionItem;
						item.m_Durability = 255.0f;
						item.m_JewelOfHarmonyOption = 0;
						item.m_ItemOptionEx = 0;
						item.m_SocketOption[1] = 0;
						item.m_SocketOption[2] = 0;
						item.m_SocketOption[3] = 0;
						item.m_SocketOption[4] = 0;
						item.m_SocketOption[5] = 0;
						item.Convert(GET_ITEM(itItemAdd->TypeItem, itItemAdd->IndexItem), item.m_Option1, item.m_Option2, item.m_Option3, item.m_NewOption, item.m_SetOption, item.m_JewelOfHarmonyOption, item.m_ItemOptionEx, item.m_SocketOption, item.m_SocketOptionBonus);
						gObj[result].Inventory[i].m_Option1 = item.m_Option1;
						gObj[result].Inventory[i].m_Option2 = item.m_Option2;
						gObj[result].Inventory[i].m_Option3 = item.m_Option3;
						gObj[result].Inventory[i].m_JewelOfHarmonyOption = item.m_JewelOfHarmonyOption;
						gObj[result].Inventory[i].m_ItemOptionEx = item.m_ItemOptionEx;

						item.m_Number = 0;
						gObjInventoryInsertItemPos(gObj[result].Index, item, i, 0);
						
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
					gObj[result].BotDefense = it->second.OnylVip;
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

CB_BotTrader::DATA_CREATE_BOT* CB_BotTrader::GetBotIndex(int aIndex)
{
	for (auto it = m_DataCreateBot.begin(); it != m_DataCreateBot.end(); ++it)
	{
		if (it->second.Enable == 1)
		{
			if (it->second.Cache_aIndex == aIndex)
			{
				return &it->second;
			}
		}
	}
	return 0;
}
BOOL CB_BotTrader::IsInTrade(int aIndex)
{
	CB_BotTrader::DATA_CREATE_BOT* DataBot = this->GetBotIndex(gObj[aIndex].TargetNumber);
	if (DataBot)
		return 1;
	else
		return 0;
}
int CB_BotTrader::ItemCount(int aIndex)
{
	int Count = 0;
	for (int n = 0; n < TRADE_SIZE; n++)
	{
		if (gObj[aIndex].Trade[n].IsItem() == 1)
		{
			Count++;
		}
	}
	return Count;
}
int CB_BotTrader::GetItemCountMixIndex(int MixIndex)
{
	std::map<int, CB_BotTrader::DATA_MIX_DATA>::iterator it = this->m_DataMix.find(MixIndex);

	if (it == this->m_DataMix.end())
	{
		return 0;
	}
	return it->second.mDataNeedItem.size();
}

BYTE CB_BotTrader::BoxExcOptions(int maxOptions)
{
	BYTE NumberOfOptions = 0;
	BYTE RetOption = 0, TempOption = 0, deadlock = 15;
	BYTE ExcOrgArr[6];

	if (maxOptions > 0)
	{
		if (maxOptions > 6)
			NumberOfOptions = 6;
		else
			NumberOfOptions = rand() % maxOptions + 1;

		//Exc Options IDs
		ExcOrgArr[0] = 8;
		ExcOrgArr[1] = 16;
		ExcOrgArr[2] = 2;
		ExcOrgArr[3] = 4;
		ExcOrgArr[4] = 32;
		ExcOrgArr[5] = 1;

		if (NumberOfOptions >= 6)
		{
			RetOption = ExcOrgArr[0] + ExcOrgArr[1] + ExcOrgArr[2] + ExcOrgArr[3] + ExcOrgArr[4] + ExcOrgArr[5];
			return RetOption;
		}

		if (NumberOfOptions == 5)
		{
			TempOption = ExcOrgArr[rand() % 6];

			RetOption = ExcOrgArr[0] + ExcOrgArr[1] + ExcOrgArr[2] + ExcOrgArr[3] + ExcOrgArr[4] + ExcOrgArr[5] - TempOption;
			return RetOption;
		}

		if (NumberOfOptions == 4)
		{
			RetOption = ExcOrgArr[0] + ExcOrgArr[1] + ExcOrgArr[2] + ExcOrgArr[3] + ExcOrgArr[4] + ExcOrgArr[5];
			while (true)
			{
				TempOption = ExcOrgArr[rand() % 6];

				if ((RetOption & TempOption) == TempOption)
				{
					RetOption -= TempOption;
					NumberOfOptions += 1;
				}

				deadlock -= 1;
				if (NumberOfOptions == 6 || deadlock == 0)
					break;
			}
			return RetOption;
		}

		if (NumberOfOptions == 3)
		{
			RetOption = 0;
			while (true)
			{
				TempOption = ExcOrgArr[rand() % 6];

				if ((RetOption & TempOption) != TempOption)
				{
					RetOption += TempOption;
					NumberOfOptions -= 1;
				}

				deadlock -= 1;
				if (NumberOfOptions == 0 || deadlock == 0)
					break;
			}
			return RetOption;
		}

		if (NumberOfOptions == 2)
		{
			RetOption = 0;
			while (true)
			{
				TempOption = ExcOrgArr[rand() % 6];

				if ((RetOption & TempOption) != TempOption)
				{
					RetOption += TempOption;
					NumberOfOptions -= 1;
				}

				deadlock -= 1;
				if (NumberOfOptions == 0 || deadlock == 0)
					break;
			}
			return RetOption;
		}

		if (NumberOfOptions == 1)
		{
			RetOption = ExcOrgArr[rand() % 6];
			return RetOption;
		}
	}
	return RetOption;
}
BOOL CB_BotTrader::XuLyItemTrade(int aIndex, int BotNum)
{
	char sbuf[512] = { 0 };
	int fitem = -1;
	int count = 0;

	int Number = aIndex - OBJECT_START_USER;

	if (gObjIsConnected(aIndex) == 0)
	{
		return 0;
	}
	LPOBJ lpObj = &gObj[aIndex];

	int failed = false;
	int Coin = 0;
	int LoaiTienTe = -1;
	int LoaiItemBag = -1;


	int mIndex = -1;
	int KeepLevel = -1;
	int KeepOption = -1;

	CItem* ItemBackup = nullptr;

	std::map<int, CB_BotTrader::DATA_CREATE_BOT>::iterator it = this->m_DataCreateBot.find(BotNum);

	if (it == this->m_DataCreateBot.end())
	{
		return 0;
	}
	int aIndexBot = it->second.Cache_aIndex;
	for (int i = it->second.MixIndexMin; i <= it->second.MixIndexMax; i++)
	{
		int foundItem = 0;
		std::map<int, CB_BotTrader::DATA_MIX_DATA>::iterator itDBMix = this->m_DataMix.find(i);
		if (itDBMix == this->m_DataMix.end())
		{
			continue;
		}

		int NeedItemCount = itDBMix->second.mDataNeedItem.size();

		// Kiểm tra nếu số lượng item trong giao dịch bằng với yêu cầu
		if (NeedItemCount == this->ItemCount(aIndex))
		{
			foundItem = 0;
			std::unordered_set<int> SlotTradeCache; //danh sach slot da khop

			for (auto itGetNeed = itDBMix->second.mDataNeedItem.begin(); itGetNeed != itDBMix->second.mDataNeedItem.end(); ++itGetNeed)
			{
				bool foundMatchForThisItem = false; // Kiểm tra xem có tìm thấy item hợp lệ cho yêu cầu này hay chưa

				for (int n = 0; n < TRADE_SIZE; n++)
				{
					if (SlotTradeCache.find(n) != SlotTradeCache.end())
					{
						continue;
					}

					if (lpObj->Trade[n].IsItem() == 1)
					{
						// Kiểm tra item có cùng loại và chỉ số
						if (GET_ITEM(itGetNeed->Type, itGetNeed->Index) == lpObj->Trade[n].m_Index)
						{
							// Kiểm tra cấp độ
							if (lpObj->Trade[n].m_Level >= itGetNeed->MinLevel && lpObj->Trade[n].m_Level <= itGetNeed->MaxLevel)
							{
								// Kiểm tra Option3
								if (lpObj->Trade[n].m_Option3 >= itGetNeed->MinOpt && lpObj->Trade[n].m_Option3 <= itGetNeed->MaxOpt)
								{
									// Kiểm tra Luck
									if (lpObj->Trade[n].m_Option2 >= itGetNeed->Luck)
									{
										// Kiểm tra Skill
										if (lpObj->Trade[n].m_Option1 >= itGetNeed->Skill)
										{
											// Kiểm tra NewOption (Exc Option)
											if (lpObj->Trade[n].m_NewOption >= itGetNeed->NeedExc)
											{
												// Kiểm tra Durability (độ bền)
												if (lpObj->Trade[n].m_Durability >= itGetNeed->Durability)
												{
													// Nếu có yêu cầu giữ level, giữ lại cấp độ
													if (itGetNeed->KeepLevel == 1) KeepLevel = lpObj->Trade[n].m_Level;
													if (itGetNeed->KeepOption == 1) KeepOption = lpObj->Trade[n].m_NewOption;

													// Nếu yêu cầu giữ item khi thất bại, lưu item
													if (itGetNeed->KeepItemMixFail)
													{
														ItemBackup = &lpObj->Trade[n];
													}

													foundItem++;
													SlotTradeCache.insert(n); // Thêm slot vào danh sách đã kiểm tra
													foundMatchForThisItem = true; // Đánh dấu đã tìm thấy item này
													break; // Không cần tiếp tục kiểm tra các slot khác cho item này
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}

		// Nếu tìm thấy đủ số lượng item hợp lệ
		if (foundItem == NeedItemCount)
		{
			mIndex = i;
			goto OK;
		}		
	}	
	ChatSend(&gObj[aIndexBot], "Vui long dat dung vat pham");
	goto Cancel;
OK:
	if (mIndex != -1)
	{
		std::map<int, CB_BotTrader::DATA_MIX_DATA>::iterator itDBMixResult = this->m_DataMix.find(mIndex);

		if (itDBMixResult == this->m_DataMix.end())
		{
			ChatSend(&gObj[aIndexBot], "Khong co KQ Mix !!");
			goto Cancel;
		}
		if (itDBMixResult->second.PriceType == 0)
		{
			if (lpObj->Money < itDBMixResult->second.PriceCount)
			{
				wsprintf(sbuf, "Ban Can Co %d Zen de Mix", itDBMixResult->second.PriceCount);
				ChatSend(&gObj[aIndexBot], sbuf);
				goto Cancel;
			}
		}
		else if (itDBMixResult->second.PriceType == 1)
		{
			if (lpObj->Coin1 < itDBMixResult->second.PriceCount)
			{
				wsprintf(sbuf, "Ban Can Co %d WC de Mix", itDBMixResult->second.PriceCount);
				ChatSend(&gObj[aIndexBot], sbuf);
				goto Cancel;
			}
		}
		else if (itDBMixResult->second.PriceType == 2)
		{
			if (lpObj->Coin2 < itDBMixResult->second.PriceCount)
			{
				wsprintf(sbuf, "Ban Can Co %d WP de Mix", itDBMixResult->second.PriceCount);
				ChatSend(&gObj[aIndexBot], sbuf);
				goto Cancel;
			}
		}
		else if (itDBMixResult->second.PriceType == 3)
		{
			if (lpObj->Coin3 < itDBMixResult->second.PriceCount)
			{
				wsprintf(sbuf, "Ban Can Co %d GP de Mix", itDBMixResult->second.PriceCount);
				ChatSend(&gObj[aIndexBot], sbuf);
				goto Cancel;
			}
		}
		else
		{
			wsprintf(sbuf, "Wrong currency! Contact Admin check config!!!");
			ChatSend(&gObj[aIndexBot], sbuf);
			goto Cancel;
		}
		int random = GetLargeRand() % 100;
		//== Thanh Cong
		if (random <= itDBMixResult->second.SuccessRate)
		{
			if (itDBMixResult->second.mDataResultItem.empty())
			{
				ChatSend(&gObj[aIndexBot], "Loi khong co item KQ 2");
				goto Cancel;
			}
			srand(static_cast<int>(time(NULL)));
			int rItem = (itDBMixResult->second.mDataResultItem.size() > 1 ? rand() % itDBMixResult->second.mDataResultItem.size() : 0);
			int Level = 0;
			int Opt = 0;
			int Exc = 0;
			int Luck = 0;
			int Skill = 0;



			int IndexItem = GET_ITEM(itDBMixResult->second.mDataResultItem[rItem].Type, itDBMixResult->second.mDataResultItem[rItem].Index);

			//==Level
			if (itDBMixResult->second.mDataResultItem[rItem].LevelMin == itDBMixResult->second.mDataResultItem[rItem].LevelMax)
			{
				Level = itDBMixResult->second.mDataResultItem[rItem].LevelMin;
			}
			else
			{
				srand(static_cast<int>(time(NULL)));
				Level = rand() % (itDBMixResult->second.mDataResultItem[rItem].LevelMax - itDBMixResult->second.mDataResultItem[rItem].LevelMin) + itDBMixResult->second.mDataResultItem[rItem].LevelMin;
			}
			if (KeepLevel != -1)
			{
				Level = KeepLevel;
			}
			//==Opt
			if (itDBMixResult->second.mDataResultItem[rItem].OptMax == itDBMixResult->second.mDataResultItem[rItem].OptMin)
			{
				Opt = itDBMixResult->second.mDataResultItem[rItem].OptMax;
			}
			else
			{
				srand(static_cast<int>(time(NULL)));
				Opt = rand() % (itDBMixResult->second.mDataResultItem[rItem].OptMax - itDBMixResult->second.mDataResultItem[rItem].OptMin) + itDBMixResult->second.mDataResultItem[rItem].OptMin;
			}
			//==Exc
			if (itDBMixResult->second.mDataResultItem[rItem].Exc > 0)
			{
				Exc = this->BoxExcOptions(itDBMixResult->second.mDataResultItem[rItem].Exc);
			}
			if (KeepOption != -1)
			{
				Exc = KeepOption;
			}
			if (itDBMixResult->second.mDataResultItem[rItem].Luck == 0)
			{
				Luck = 0;
			}
			else
			{
				srand(static_cast<int>(time(NULL)));
				BYTE lRnd = rand() % 100;
				if (lRnd <= itDBMixResult->second.mDataResultItem[rItem].Luck)
				{
					Luck = 1;
				}
				else
				{
					Luck = 0;
				}
			}
			if (itDBMixResult->second.mDataResultItem[rItem].Skill == 0)
			{
				Skill = 0;
			}
			else
			{
				srand(static_cast<int>(time(NULL)));
				BYTE sRnd = rand() % 100;
				if (sRnd <= itDBMixResult->second.mDataResultItem[rItem].Skill)
				{
					Skill = 1;
				}
				else
				{
					Skill = 0;
				}
			}



			gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, this->GetMessage(itDBMixResult->second.SuccessMes), lpObj->Name, itDBMixResult->second.MixName);
			GDCreateItemSend(lpObj->Index, 0xEB, 0, 0, IndexItem, Level, 0, Skill, Luck, Opt, -1, Exc, 0, 0, 0, 0, 0xFF, 0);
			gLog.Output(LOG_TRADEBOT, "Mix OK [%s %s] %s - (%d) Item %d %d %d %d %d %d %d %d", lpObj->Account, lpObj->Name, itDBMixResult->second.MixName, rItem, IndexItem, Level, 0, Skill, Luck, Opt, -1, Exc);

			for (int n = 0; n < TRADE_SIZE; n++)
			{
				if (lpObj->Trade[n].IsItem() != 0)
				{
					gLog.Output(LOG_TRADEBOT, "[LOG_TRADEBOT Ex] - [%s +%d] - [DB[%.0f] TC[%d] Luck[%d] Op[%d]] Exc[%d] Set[%d] oEx[%d] Har[%d] Socket[%03d,%03d,%03d,%03d,%03d][%d] Seri[%08X]",
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


		}
		else //that bai
		{
			gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, this->GetMessage(itDBMixResult->second.FailMes), lpObj->Name, itDBMixResult->second.MixName);
			gLog.Output(LOG_TRADEBOT, "Mix Fail [%s %s] %s", lpObj->Account, lpObj->Name, itDBMixResult->second.MixName);

			for (int n = 0; n < TRADE_SIZE; n++)
			{
				if (lpObj->Trade[n].IsItem() != 0)
				{
					gLog.Output(LOG_TRADEBOT, "[TRADEBOT Ex] - [%s +%d] - [%.0f,%d,%d,%d] ex[%d] set[%d] oEx[%d] ho[%d] sk[%03d,%03d,%03d,%03d,%03d][%d] seri[%08X]",
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
			if (ItemBackup)
			{
				GDCreateItemSend(lpObj->Index, 0xEB, 0, 0, ItemBackup->m_Index, ItemBackup->m_Level, 0, ItemBackup->m_Option1, ItemBackup->m_Option2, ItemBackup->m_Option3, -1, ItemBackup->m_NewOption, 0, 0, 0, 0, 0xFF, 0);
			}
		}


		if (itDBMixResult->second.PriceType == 0)
		{
			lpObj->Money -= itDBMixResult->second.PriceCount;
			GCMoneySend(lpObj->Index, lpObj->Money);

		}
		else if (itDBMixResult->second.PriceType == 1)
		{
			GDSetCoinSend(lpObj->Index, -(itDBMixResult->second.PriceCount), 0, 0, "BotTraderSubCoin");

			//gCashShop.GDCashShopSubPointSaveSend(lpObj->Index, 0, itDBMixResult->second.PriceCount, 0, 0, 0, "BotTraderSubCoin");
			//gLog.Output(LOG_WC, "[BotTraderSubCoin] [%s - %s) Tru (%d) WCoinC Con %d WC  ", lpObj->Account, lpObj->Name, itDBMixResult->second.PriceCount, lpObj->Coin1);
		}
		else if (itDBMixResult->second.PriceType == 2)
		{
			GDSetCoinSend(lpObj->Index, 0, -(itDBMixResult->second.PriceCount), 0, "BotTraderSubCoin");
			//gCashShop.GDCashShopSubPointSaveSend(lpObj->Index, 0, 0, itDBMixResult->second.PriceCount, 0, 0, "BotTraderSubCoin");
			//gLog.Output(LOG_WC, "[BotTraderSubCoin] [%s - %s)  Tru (%d) WP Con %d WP  ", lpObj->Account, lpObj->Name, itDBMixResult->second.PriceCount, lpObj->Coin2);
		}
		else if (itDBMixResult->second.PriceType == 3)
		{
			GDSetCoinSend(lpObj->Index, 0, 0, -(itDBMixResult->second.PriceCount), "BotTraderSubCoin");
			//gCashShop.GDCashShopSubPointSaveSend(lpObj->Index, 0, 0, 0, itDBMixResult->second.PriceCount, 0, "BotTraderSubCoin");
		}

		gObjInventoryCommit(aIndex);
		gItemManager.GCItemListSend(aIndex);
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
		gTrade.GCTradeResultSend(aIndex, 1);
		gTrade.ClearTrade(lpObj);
		return 1;
	}

Cancel:
	
	this->TradeCancel(aIndex);
	//DataSend(aIndex, (BYTE*)&pMsg, pMsg.h.size);
	return 0;
}


void CB_BotTrader::TradeOk(int aIndex)
{
	int MixNum = -1;
	int MixItem = -1;

	CB_BotTrader::DATA_CREATE_BOT* DataBot = this->GetBotIndex(gObj[aIndex].TargetNumber);

	if (!DataBot)
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
	LogAdd(LOG_RED, "CB_BotTrader::TradeOk %s", DataBot->BotName);
	this->XuLyItemTrade(aIndex, DataBot->IndexBot);
}

BOOL CB_BotTrader::TradeOpen(int index, int nindex)
{
	if (gObjIsConnected(index) == 0)
	{
		return 0;
	}
	if (gObjIsConnected(nindex) == 0)
	{
		return 0;
	}

	CB_BotTrader::DATA_CREATE_BOT* DataBot = this->GetBotIndex(nindex);
	if (!DataBot) return 0;

	char sbuf[512] = { 0 };
	LPOBJ lpObj = &gObj[index];
	LPOBJ lpBot = &gObj[nindex];

	if (DataBot->OnylVip == 1 && gObj[index].AccountLevel == 0)
	{
		ChatSend(&gObj[DataBot->Cache_aIndex], this->GetMessage(2));
		LogAdd(LOG_RED, "[BotTrade][Bot:%d](%s) Account is not VIP", DataBot->IndexBot, gObj[index].Account);
		return 0;
	}

	if (this->Enabled == TRUE)
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
			gTrade.GCTradeResponseSend(lpObj->Index, 1, lpBot->Name, 400, 0);
			gTrade.GCTradeOkButtonSend(lpObj->Index, 1);
			lpObj->Interface.state = 1;
			lpObj->Interface.use = 1;
			lpObj->Interface.type = 1;
			lpObj->TradeMoney = 0;
			lpObj->TargetNumber = lpBot->Index;
			lpObj->Transaction = 1;

			char wbuf[1024] = { 0 };
			wsprintf(wbuf, "[BotTrade] (%s)(%s) OPEN Type[%d],State[%d], Use[%d]", gObj[index].Account, gObj[index].Name, lpObj->Interface.type, lpObj->Interface.state, lpObj->Interface.use);
			LogAdd(LOG_RED, wbuf);
			ChatSend(&gObj[DataBot->Cache_aIndex], this->GetMessage(0));

			if(DataBot->CustomShowInfo) this->SendDataIsTrade(lpObj->Index, DataBot->IndexBot, eSendRecvListDataMix);

		}
	}
	return 1;
}

void CB_BotTrader::TradeCancel(int aIndex)
{
	LPOBJ lpObj = &gObj[aIndex];

	if (gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}


	if (lpObj->Interface.use == 0 || lpObj->Interface.type != INTERFACE_TRADE || lpObj->Interface.state == 0)
	{
		return;
	}
	gTrade.ResetTrade(aIndex);
	gTrade.GCTradeResultSend(aIndex, 0);

}


void CB_BotTrader::SendDataIsTrade(int aIndex, int IndexBot, int TypeSend, int MixIndex)
{
	if (gObj[aIndex].Type != OBJECT_USER)
	{
		return;
	}

	if (gObjIsConnected(aIndex) == false)
	{
		return;
	}

	if (gObj[aIndex].IsBot >= 1 || gObj[aIndex].m_OfflineMode != 0)
	{
		return;
	}

	BYTE send[4096];

	PMSG_CUSTOM_LIST pMsg = { 0 };
	// ---
	pMsg.header.set(0xD3, 0x2E, 0);

	int size = sizeof(pMsg);

	pMsg.count = 0;

	pMsg.TypeSend = TypeSend;

	switch (TypeSend)
	{
		case CB_BotTrader::eSendRecvListDataMix:
		{
			std::map<int, CB_BotTrader::DATA_CREATE_BOT>::iterator itBot = this->m_DataCreateBot.find(IndexBot);

			if (itBot == this->m_DataCreateBot.end())
			{
				return;
			}
			for (int i = itBot->second.MixIndexMin; i <= itBot->second.MixIndexMax; i++)
			{
				std::map<int, CB_BotTrader::DATA_MIX_DATA>::iterator itDBMix = this->m_DataMix.find(i);

				if (itDBMix != this->m_DataMix.end())
				{
					LIST_MIX_TRADER info;
					info.MixIndex = itDBMix->second.MixIndex;
					memcpy(&info.NameMix, itDBMix->second.MixName, sizeof(info.NameMix));

					if ((size + sizeof(info)) > sizeof(send)) break;

					pMsg.count++;
					memcpy(&send[size], &info, sizeof(info));
					size += sizeof(info);
				}
			}

		}
		break;
		case CB_BotTrader::eSendRecvListItemMix:
		{
			if (MixIndex == -1) return;
			std::map<int, CB_BotTrader::DATA_MIX_DATA>::iterator itDBMix = this->m_DataMix.find(MixIndex);

			if (itDBMix == this->m_DataMix.end())
			{
				return;
			}
			pMsg.TypeCoin = itDBMix->second.PriceType;
			pMsg.Coin = itDBMix->second.PriceCount;
			pMsg.Rate = itDBMix->second.SuccessRate;
			pMsg.KeepLevel = 0;
			pMsg.KeepOption = 0;
			pMsg.KeepItemMixFail = 0;

			for (auto it = itDBMix->second.mDataNeedItem.begin(); it != itDBMix->second.mDataNeedItem.end(); ++it)
			{
				DATA_ITEMSEND info = { 0 };
				info.IndexItem =  GET_ITEM(it->Type, it->Index);
				info.MinLevel = it->MinLevel;
				info.MaxLevel = it->MaxLevel;
				info.MinOpt = it->MinOpt;
				info.MaxOpt = it->MaxOpt;
				info.Luck = it->Luck;
				info.Skill = it->Skill;
				info.NeedExc = it->NeedExc;
				info.Durability = it->Durability;
				if (it->KeepLevel == 1)  pMsg.KeepLevel = 1;
				if (it->KeepOption == 1)  pMsg.KeepOption = 1;
				if (it->KeepItemMixFail == 1)  pMsg.KeepItemMixFail = 1;

				if ((size + sizeof(info)) > sizeof(send)) break;
				pMsg.count++;
				memcpy(&send[size], &info, sizeof(info));
				size += sizeof(info);
			}

		}
		break;
		default:
			return;
			break;
	}

	
	pMsg.header.size[0] = SET_NUMBERHB(size);
	pMsg.header.size[1] = SET_NUMBERLB(size);
	// ---
	memcpy(send, &pMsg, sizeof(pMsg));

	DataSend(aIndex, send, size);
	//LogAdd(LOG_RED, "SendDataIsTrade %s IndexBot %d", gObj[aIndex].Name, IndexBot);
}