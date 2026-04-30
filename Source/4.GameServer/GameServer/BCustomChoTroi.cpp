#include "stdafx.h"
#include "BCustomChoTroi.h"
#include "Monster.h"
#include "ItemManager.h"
#include "ObjectManager.h"
#include "SkillManager.h"
#include "EffectManager.h"
#include "MasterSkillTree.h"
#include "ServerInfo.h"
#include "MemScript.h"
#include "Util.h"
#include "CashShop.h"
#include "Message.h"
#include "Notice.h"
#include "Trade.h"
#include "DSProtocol.h"
#include "Log.h"
#include "ItemLevel.h"
#include "GameMain.h"
#include "CSProtocol.h"
//#include "CustomJewelBank.h"
#include "MapServerManager.h"
//#include "BankUpgrade.h"
#include "BCustomItemBank.h"

#if(CUSTOM_BCHOTROI)
BCustomChoTroi gBCustomChoTroi;
char* TypeCoin[7] = { "NULL", "WC" ,"WP","GP","B","S","C" };
void BCustomChoTroi::Load(char* FilePath)
{
	this->m_DataVAT.clear();
	this->m_DataItemBlock.clear();
	this->Enable = 0;
	this->TypeHSD = 0;

	this->OnCointType = 0;
	pugi::xml_document file;
	pugi::xml_parse_result res = file.load_file(FilePath);
	if (res.status != pugi::status_ok) {
		ErrorMessageBox("File %s load fail. Error: %s", FilePath, res.description());
		return;
	}
	pugi::xml_node oCustomChoTroi = file.child("CustomChoTroi");
	this->Enable = oCustomChoTroi.attribute("Enable").as_int();
	this->TypeHSD = oCustomChoTroi.attribute("TypeHSD").as_int();
	if (oCustomChoTroi.attribute("OnWC").as_int() == 1)
	{
		this->OnCointType += 1;
	}
	if (oCustomChoTroi.attribute("OnWP").as_int() == 1)
	{
		this->OnCointType += 2;
	}
	if (oCustomChoTroi.attribute("OnGP").as_int() == 1)
	{
		this->OnCointType += 4;
	}
	if (oCustomChoTroi.attribute("OnB").as_int() == 1)
	{
		this->OnCointType += 8;
	}
	if (oCustomChoTroi.attribute("OnS").as_int() == 1)
	{
		this->OnCointType += 16;
	}
	if (oCustomChoTroi.attribute("OnC").as_int() == 1)
	{
		this->OnCointType += 32;
	}

	this->m_MessageInfoBP.clear();
	pugi::xml_node Message = oCustomChoTroi.child("MessageInfo");
	for (pugi::xml_node msg = Message.child("Message"); msg; msg = msg.next_sibling())
	{
		CUSTOM_CHOTROI_MESSAGE info;

		info.Index = msg.attribute("Index").as_int();

		strncpy_s(info.Message, msg.attribute("Text").as_string(), sizeof(info.Message));

		this->m_MessageInfoBP.insert(std::pair<int, CUSTOM_CHOTROI_MESSAGE>(info.Index, info));
	}

	//=== LOad COnfig VAT
	pugi::xml_node oConfigVAT = oCustomChoTroi.child("ConfigVAT");
	for (pugi::xml_node ConfigVAT = oConfigVAT.child("Type"); ConfigVAT; ConfigVAT = ConfigVAT.next_sibling())
	{
		VAT_DATA_INFO info = { 0 };
		info.TypeItem = ConfigVAT.attribute("TypeItem").as_int();
		info.WCRate = ConfigVAT.attribute("WCRate").as_int();
		info.WPRate = ConfigVAT.attribute("WPRate").as_int();
		info.GPRate = ConfigVAT.attribute("GPRate").as_int();
		info.BRate = ConfigVAT.attribute("BRate").as_int();
		info.SRate = ConfigVAT.attribute("SRate").as_int();
		info.CRate = ConfigVAT.attribute("CRate").as_int();
		this->m_DataVAT.insert(std::pair<int, VAT_DATA_INFO>(info.TypeItem, info));
	}
	//===== Load COnfig Item Block
	pugi::xml_node oConfigItemBlock = oCustomChoTroi.child("ConfigItemBlock");
	for (pugi::xml_node ConfigItemBlock = oConfigItemBlock.child("Item"); ConfigItemBlock; ConfigItemBlock = ConfigItemBlock.next_sibling())
	{
		DATA_ITEMBLOCK info = { 0 };
		info.IndexMin = ConfigItemBlock.attribute("IndexMin").as_int();
		info.IndexMax = ConfigItemBlock.attribute("IndexMax").as_int();
		info.LvMin = ConfigItemBlock.attribute("LvMin").as_int();
		info.LvMax = ConfigItemBlock.attribute("LvMax").as_int();
		info.Luck = ConfigItemBlock.attribute("Luck").as_int();
		info.Skill = ConfigItemBlock.attribute("Skill").as_int();
		info.Opt = ConfigItemBlock.attribute("Opt").as_int();
		info.ExcOpt = ConfigItemBlock.attribute("ExcOpt").as_int();
		this->m_DataItemBlock.push_back(info);
	}
	LogAdd(LOG_BLUE, "[CustomChoTroi] Load VAT %d, Load ItemBlock %d", this->m_DataVAT.size(), this->m_DataItemBlock.size());
}

char* BCustomChoTroi::GetMessage(int index) // OK
{
	std::map<int, CUSTOM_CHOTROI_MESSAGE>::iterator it = this->m_MessageInfoBP.find(index);

	if (it == this->m_MessageInfoBP.end())
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
int BCustomChoTroi::GetRateTaxTypeItem(int TypeItem, int TypeCoin) // OK
{
	if (TypeItem > 10 || TypeItem < 0) TypeItem = 0;
	std::map<int, VAT_DATA_INFO>::iterator it = this->m_DataVAT.find(TypeItem);
	if (it == this->m_DataVAT.end())
	{
		return 10;
	}
	else
	{
		if (TypeCoin == eMarketPriceWC) return it->second.WCRate;
		else if (TypeCoin == eMarketPriceWP) return it->second.WPRate;
		else if (TypeCoin == eMarketPriceGP) return it->second.GPRate;
		else if (TypeCoin == eMarketPriceB) return it->second.BRate;
		else if (TypeCoin == eMarketPriceS) return it->second.SRate;
		else if (TypeCoin == eMarketPriceC) return it->second.CRate;
	}
	return 10;
}
bool BCustomChoTroi::CheckItemBlockSend(CItem * lpItem)
{
	//LogAdd(LOG_RED, "Check Item %d", lpItem->m_Index);
	for (std::vector<DATA_ITEMBLOCK>::iterator it = this->m_DataItemBlock.begin(); it != this->m_DataItemBlock.end(); it++)
	{
		if (it->IndexMin != -1 && lpItem->m_Index < it->IndexMin)
		{
			//LogAdd(LOG_RED, "Check Item 1");
			continue;
		}
		if (it->IndexMax != -1 && lpItem->m_Index > it->IndexMax)
		{
			//LogAdd(LOG_RED, "Check Item 2");
			continue;
		}
		if (it->LvMin != -1 && lpItem->m_Level >= it->LvMin)
		{
			//LogAdd(LOG_RED, "Check Item 3");
			continue;
		}
		if (it->LvMax != -1 && lpItem->m_Level <= it->LvMax)
		{
			//LogAdd(LOG_RED, "Check Item 4");
			continue;
		}
		if (it->Skill != -1 && !lpItem->m_Option1)
		{
			//LogAdd(LOG_RED, "Check Item 5");
			continue;
		}
		if (it->Luck != -1 && !lpItem->m_Option2)
		{
			//	LogAdd(LOG_RED, "Check Item 6");
			continue;
		}
		if (it->Opt != -1 && lpItem->m_Option3 != it->Opt)
		{
			//	LogAdd(LOG_RED, "Check Item 7");
			continue;
		}
		if (it->ExcOpt != -1 && lpItem->m_NewOption != it->ExcOpt)
		{
			//	LogAdd(LOG_RED, "Check Item 8");
			continue;
		}
		return 1;
	}

	return 0;
}
void BCustomChoTroi::RollBack(int aIndex, XULY_CGPACKET * lpMsg)
{
	if (!OBJMAX_RANGE(aIndex))
	{
		return;
	}


	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->Type != OBJECT_USER || !this->Enable)
	{
		return;
	}
	BYTE SlotRecv = gItemManager.InventoryInsertItem(aIndex, gObj[aIndex].CH_InfoItem[0]); // Roll back lai Item
	if (SlotRecv != 0xFF)
	{
		gItemManager.GCItemModifySend(aIndex, SlotRecv);

	}
	gObj[aIndex].StatusCacheItem = -1;
	gObj[aIndex].CH_IndexItem[0] = -1;
	gObj[aIndex].StatusCacheItem = -1;
	gObj[aIndex].CH_InfoItem[0].Clear();
	GDCharacterInfoSaveSend(aIndex);
	//====
	XULY_CGPACKET pMsg;
	pMsg.header.set(0xD3, 0x01, sizeof(pMsg));
	pMsg.ThaoTac = 0;	//Clear Item Cache O Client
	DataSend(lpObj->Index, (BYTE*)& pMsg, pMsg.header.size);
	return;
}
void BCustomChoTroi::ClientSendItemRaoBan(int aIndex, PMSG_MOVEITEM * lpMsg)
{

	if (!OBJMAX_RANGE(aIndex))
	{
		return;
	}


	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->Type != OBJECT_USER || !this->Enable)
	{
		return;
	}

	if (lpObj->Interface.type != INTERFACE_NONE || lpObj->Interface.use != 0)
	{
		return;
	}

	if (gItemManager.CheckItemMoveToTrade(lpObj, &lpObj->Inventory[lpMsg->Source], 0) == 0)
	{
		gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(4));//
		return;
	}

	if (gItemManager.ChaosBoxHasItem(lpObj) || gItemManager.TradeHasItem(lpObj))
	{
		return;
	}
	if (!lpObj->Inventory[lpMsg->Source].IsItem())
	{
		//Vi tri nay khong co Item do
		gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(4));//
		return;
	}

	//=== Kiem Tra Item List Block
	if (this->CheckItemBlockSend(&lpObj->Inventory[lpMsg->Source]))
	{
		gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(4));//
		return;
	}
	if (lpObj->StatusCacheItem != -1)
	{
		gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(4));//
		return;
	}

	if (lpObj->CH_IndexItem[0] == -1) //
	{
		lpObj->CH_IndexItem[0] = lpObj->Inventory[lpMsg->Source].m_Index;
		lpObj->CH_InfoItem[0] = lpObj->Inventory[lpMsg->Source];
		lpObj->StatusCacheItem = CACHE_ITEMCHOTROI;
		//=== Send Del item
		gItemManager.InventoryDelItem(aIndex, lpMsg->Source);
		gItemManager.GCItemDeleteSend(aIndex, lpMsg->Source, 1);
		//====
		XULY_CGPACKET pMsg;
		pMsg.header.set(0xD3, 0x01, sizeof(pMsg));
		pMsg.ThaoTac = 1;	//Show Item Cache
		DataSend(lpObj->Index, (BYTE*)& pMsg, pMsg.header.size);
		return;
	}
	else
	{
		gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(3));//
		return;
	}
}

void BCustomChoTroi::CGReqItemSell(PMSG_REQ_MARKET_SELL * lpMsg, int aIndex)
{
	if (lpMsg->ItemPrice < 0) { return; }
	if (lpMsg->ItemPriceType < eMarketPriceWC && lpMsg->ItemPriceType > eMarketPriceC) { return; }



	int ItemPriceType = lpMsg->ItemPriceType;
	int ItemPrice = lpMsg->ItemPrice;
	if (!OBJECT_RANGE(aIndex))
	{
		return;
	}

	switch (ItemPriceType)
	{
	case eMarketPriceWC:
	{
		if (!(this->OnCointType & 1))
		{
			gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(11));//
			return;
		}
	}
	break;
	case eMarketPriceWP:
	{
		if (!(this->OnCointType & 2))
		{
			gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(11));//
			return;
		}
	}
	break;
	case eMarketPriceGP:
	{
		if (!(this->OnCointType & 4))
		{
			gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(11));//
			return;
		}
	}
	break;
	case eMarketPriceB:
	{
		if (!(this->OnCointType & 8))
		{
			gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(11));//
			return;
		}
	}
	break;
	case eMarketPriceS:
	{
		if (!(this->OnCointType & 16))
		{
			gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(11));//
			return;
		}
	}
	break;

	case eMarketPriceC:
	{
		if (!(this->OnCointType & 32))
		{
			gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(11));//
			return;
		}
	}
	break;
	default:
		break;
	}

	LPOBJ lpUser = &gObj[aIndex];
	if (lpUser->Interface.type == INTERFACE_CHAOS_BOX
		|| lpUser->Interface.type == INTERFACE_TRADE
		|| lpUser->Interface.type == INTERFACE_PARTY
		|| lpUser->Interface.type == INTERFACE_WAREHOUSE
		|| lpUser->Interface.type == INTERFACE_PERSONAL_SHOP
		|| lpUser->Interface.type == INTERFACE_CASH_SHOP
		|| lpUser->Interface.type == INTERFACE_TRAINER
		|| lpUser->Interface.use != 0
		|| lpUser->State == OBJECT_DELCMD
		|| lpUser->DieRegen != 0
		|| lpUser->Teleport != 0
		|| lpUser->PShopOpen != 0
		|| lpUser->ChaosLock != 0
		|| lpUser->SkillSummonPartyTime != 0)
	{
		gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(0));//
		return;
	}

	if (lpUser->CH_IndexItem[0] == -1)
	{
		gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(1));//
		return;
	}
	if (ItemPriceType < 1 || ItemPriceType >6 || ItemPrice < 1)
	{
		gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(0));//
		return;
	}
	SDHP_REQ_MARKET_SELL pMsg;

	pMsg.h.set((LPBYTE)& pMsg, 0xEC, 0x01, sizeof(pMsg));

	pMsg.Result = 1;

	pMsg.aIndex = aIndex;

	pMsg.PriceType = ItemPriceType;

	pMsg.Price = ItemPrice;

	pMsg.ItemDay = lpMsg->ItemDay;//Ngay ban

	pMsg.Pass = lpMsg->Pass;//Ngay ban
	//pMsg.Account[10] = 0;

	memcpy(pMsg.Account, lpUser->Account, sizeof(pMsg.Account) - 1);

#if(MARKET_NAME_DEV)

	//pMsg.Name[10] = 0;

	memcpy(pMsg.Name, lpUser->Name, sizeof(pMsg.Name));

#endif

	pMsg.ItemPos = 0;


	gItemManager.DBItemByteConvert((LPBYTE)pMsg.ItemData, &lpUser->CH_InfoItem[0]);

	if (lpUser->CH_InfoItem[0].m_IsPeriodicItem)
	{
		gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(4));//
		//LogAdd(LOG_RED, "ItemMakert : Sell Item Thoi Han Error");
		return;
	}

	if (lpUser->CH_InfoItem[0].m_LoadPeriodicItem)
	{
		gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(4));//
		//LogAdd(LOG_RED, "ItemMakert : Sell Item Thoi Han Error");
		return;
	}

	if (lpUser->CH_InfoItem[0].m_PeriodicItemTime)
	{
		gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(4));//
		//LogAdd(LOG_RED, "ItemMakert : Sell Item Thoi Han Error");
		return;
	}
	if (lpUser->CH_InfoItem[0].m_JewelOfHarmonyOption)
	{
		gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(4));//
		//LogAdd(LOG_RED, "ItemMakert : Sell Item Thoi Han Error");
		return;
	}
	// --


	//==Phan Loai Item
	int mTypeItem = lpUser->CH_InfoItem[0].m_Slot;
	if (lpUser->CH_InfoItem[0].m_Slot == 0 || lpUser->CH_InfoItem[0].m_Slot == 1) { mTypeItem = 1; }
	if (lpUser->CH_InfoItem[0].m_Slot == 12) { mTypeItem = 8; }
	if (lpUser->CH_InfoItem[0].m_Slot == 13 || lpUser->CH_InfoItem[0].m_Slot == 14) { mTypeItem = 9; }
	pMsg.TypeItem = mTypeItem;
	//gNotice.GCNoticeSendToAll(0,0,0,0,0,0,"%d",mTypeItem);
	//Set Delay
	lpUser->ClickClientSend = GetTickCount();
	gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, this->GetMessage(12), lpUser->Name, gItemLevel.GetItemName(lpUser->CH_InfoItem[0].m_Index, lpUser->CH_InfoItem[0].m_Level), lpUser->CH_InfoItem[0].m_Level);
	gLog.Output(LOG_TRADE, "[ChoTroi][%s][%s] Rao Ban item (%s | Index: %04d Gia: %d [%d] Level: %02d, Serial: %08X, Option1: %01d, Option2: %01d, Option3: %01d, NewOption: %03d, JewelOfHarmonyOption: %03d, ItemOptionEx: %03d, SocketOption: %03d, %03d, %03d, %03d, %03d)"
		, lpUser->Account, lpUser->Name, gItemLevel.GetItemName(lpUser->CH_InfoItem[0].m_Index, lpUser->CH_InfoItem[0].m_Level), lpUser->CH_InfoItem[0].m_Index, ItemPrice, ItemPriceType, lpUser->CH_InfoItem[0].m_Level, lpUser->CH_InfoItem[0].m_Serial, lpUser->CH_InfoItem[0].m_Option1, lpUser->CH_InfoItem[0].m_Option2, lpUser->CH_InfoItem[0].m_Option3, lpUser->CH_InfoItem[0].m_NewOption, lpUser->CH_InfoItem[0].m_JewelOfHarmonyOption, lpUser->CH_InfoItem[0].m_ItemOptionEx, lpUser->CH_InfoItem[0].m_SocketOption[0], lpUser->CH_InfoItem[0].m_SocketOption[1], lpUser->CH_InfoItem[0].m_SocketOption[2], lpUser->CH_InfoItem[0].m_SocketOption[3], lpUser->CH_InfoItem[0].m_SocketOption[4]);
	if (ItemPriceType == eMarketPriceWC)
	{
		gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(9), gItemLevel.GetItemName(lpUser->CH_InfoItem[0].m_Index, lpUser->CH_InfoItem[0].m_Level), ItemPrice, "WC");//
	}
	else if (ItemPriceType == eMarketPriceWP)
	{
		gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(9), gItemLevel.GetItemName(lpUser->CH_InfoItem[0].m_Index, lpUser->CH_InfoItem[0].m_Level), ItemPrice, "WP");//
	}
	else if (ItemPriceType == eMarketPriceGP)
	{
		gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(9), gItemLevel.GetItemName(lpUser->CH_InfoItem[0].m_Index, lpUser->CH_InfoItem[0].m_Level), ItemPrice, "GP");//
	}
	else if (ItemPriceType == eMarketPriceB)
	{
		gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(9), gItemLevel.GetItemName(lpUser->CH_InfoItem[0].m_Index, lpUser->CH_InfoItem[0].m_Level), ItemPrice, "Bless");//
	}
	else if (ItemPriceType == eMarketPriceS)
	{
		gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(9), gItemLevel.GetItemName(lpUser->CH_InfoItem[0].m_Index, lpUser->CH_InfoItem[0].m_Level), ItemPrice, "Soul");//
	}
	else if (ItemPriceType == eMarketPriceC)
	{
		gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(9), gItemLevel.GetItemName(lpUser->CH_InfoItem[0].m_Index, lpUser->CH_InfoItem[0].m_Level), ItemPrice, "Chaos");//
	}
	gDataServerConnection.DataSend((BYTE*)& pMsg, pMsg.h.size);

	gObj[aIndex].CH_IndexItem[0] = -1;
	gObj[aIndex].CH_InfoItem[0].Clear();
	gObj[aIndex].StatusCacheItem = -1;
	//====Set Clear Cache Item Client
	XULY_CGPACKET cMsg;
	cMsg.header.set(0xD3, 0x01, sizeof(cMsg));
	cMsg.ThaoTac = 2;	//Show Item Cache
	DataSend(lpUser->Index, (BYTE*)& cMsg, cMsg.header.size);
}

//===Send Get List DS
void BCustomChoTroi::CGOpenListChoTroi(int aIndex)
{
	SDHP_REQ_MARKET_ITEM pMsg;

	pMsg.h.set(0xEC, 0x00, sizeof(pMsg));
	pMsg.Result = 1;
	pMsg.aIndex = aIndex;
	pMsg.PriceType = -1;
	pMsg.GetTypeItem = 0;
	memset(pMsg.Account, 0, sizeof(pMsg.Account));
	memcpy(pMsg.Account, &gObj[aIndex].Account, sizeof(pMsg.Account));
	pMsg.PageNumber = 1;
	pMsg.TypeHSD = this->TypeHSD;//GG
	gDataServerConnection.DataSend((BYTE*)& pMsg, pMsg.h.size);
}
void BCustomChoTroi::CGReqItemList(PMSG_REQ_MARKET_ITEM * lpMsg, int aIndex)
{
	SDHP_REQ_MARKET_ITEM pMsg;

	pMsg.h.set(0xEC, 0x00, sizeof(pMsg));
	pMsg.Result = 1;
	pMsg.aIndex = aIndex;
	pMsg.PriceType = lpMsg->PriceType;
	pMsg.GetTypeItem = lpMsg->GetTypeItem;
	memset(pMsg.Account, 0, sizeof(pMsg.Account));
	memcpy(pMsg.Account, &gObj[aIndex].Account, sizeof(pMsg.Account));
	pMsg.PageNumber = lpMsg->PageNumber;
	pMsg.TypeHSD = this->TypeHSD;//GG
	gDataServerConnection.DataSend((BYTE*)& pMsg, pMsg.h.size);
}
//===DS Recv
void BCustomChoTroi::DGAnsItemList(CBCUSTOM_LOAD_COUNT * lpMsg)
{
	if (!this->Enable)
	{
		return;
	}

	int aIndex = lpMsg->aIndex;


	if (gObjIsConnected(aIndex) == false)
	{
		return;
	}
	// ----
	this->m_ListDataChoTroi[aIndex].clear();

	for (int n = 0; n < lpMsg->count; n++)
	{
		MARKET_DATA* lpInfo = (MARKET_DATA*)(((BYTE*)lpMsg) + sizeof(CBCUSTOM_LOAD_COUNT) + (sizeof(MARKET_DATA) * n));

		//	LogAdd(LOG_RED,"GCSetListChoTroi %d  %s  %x %d", lpInfo->ID, lpInfo->Name, lpInfo->Item,lpInfo->Pass );
		this->m_ListDataChoTroi[aIndex].push_back(*lpInfo);
	}
	this->SendListUser(aIndex);
}


void BCustomChoTroi::SendListUser(int Index)
{
	if (!this->Enable)
	{
		return;
	}


	if (gObjIsConnected(Index) == false)
	{
		return;
	}
	// ----

	BYTE send[8192];

	BCSEV_COUNTLIST pMsg;

	pMsg.header.set(0xD3, 0x20, 0);

	int size = sizeof(pMsg);

	pMsg.Count = 0;

	pMsg.Type = this->OnCointType;

	MARKET_DATA info;

	for (int i = 0; i < this->m_ListDataChoTroi[Index].size(); i++)
	{

		info.ID = this->m_ListDataChoTroi[Index][i].ID;

		memcpy(info.Name, this->m_ListDataChoTroi[Index][i].Name, sizeof(info.Name));
		//	LogAdd(LOG_CAM, "%s", info.Name);
		CItem item;

		if (gItemManager.ConvertItemByte(&item, this->m_ListDataChoTroi[Index][i].Item) == 0)
		{
			continue;
		}
		gItemManager.ItemByteConvert(info.Item, item); // Set Info Item
		//memcpy(info.Item, this->m_ListDataChoTroi[i].Item, sizeof(info.Item) - 1);
		info.PriceType = this->m_ListDataChoTroi[Index][i].PriceType;
		info.Price = this->m_ListDataChoTroi[Index][i].Price;
		info.TypeItem = this->m_ListDataChoTroi[Index][i].TypeItem;
		info.TimeItemRaoBan = this->m_ListDataChoTroi[Index][i].TimeItemRaoBan;
		info.Pass = this->m_ListDataChoTroi[Index][i].Pass;
		//LogAdd(LOG_RED, "SendList Model Skin OK !!");
		pMsg.Count++;
		memcpy(&send[size], &info, sizeof(info));
		size += sizeof(info);

	}

	pMsg.header.size[0] = SET_NUMBERHB(size);
	pMsg.header.size[1] = SET_NUMBERLB(size);
	// ---
	memcpy(send, &pMsg, sizeof(pMsg));

	DataSend(Index, send, size);

	//LogAdd(LOG_RED, "SendList Model Skin OK !!");
}

bool BCustomChoTroi::GetCheckMoney(int aIndex, int PriceType, int PriceValue)
{
	bool Result = true;

	LPOBJ lpUser = &gObj[aIndex];


	// --------->
	if (PriceType == eMarketPriceWC)
	{
		if (PriceValue > lpUser->Coin1)
		{
			gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(2), PriceValue, "WC");//
			Result = false;
		}
	}
	// --------->
	if (PriceType == eMarketPriceWP)
	{
		if (PriceValue > lpUser->Coin2)
		{
			gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(2), PriceValue, "WP");//
			Result = false;
		}
	}
	if (PriceType == eMarketPriceGP)
	{
		if (PriceValue > lpUser->Coin3)
		{
			gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(2), PriceValue, "GP");//
			Result = false;
		}
	}
	if (PriceType == eMarketPriceB)
	{
		int countitem = gBCustomItemBank.CheckCountItemBank(aIndex, 7181, 0);
		if (PriceValue > countitem)
			//if (PriceValue > lpUser->JewelBlessCount)
		{
			gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(2), PriceValue, "B");//
			Result = false;
		}

	}
	if (PriceType == eMarketPriceS)
	{
		int countitem = gBCustomItemBank.CheckCountItemBank(aIndex, 7182, 0);
		if (PriceValue > countitem)
			//if (PriceValue > lpUser->JewelSoulCount)
		{
			gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(2), PriceValue, "S");//
			Result = false;
		}
	}
	if (PriceType == eMarketPriceC)
	{
		int countitem = gBCustomItemBank.CheckCountItemBank(aIndex, 6159, 0);
		if (PriceValue > countitem)
		{
			gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(2), PriceValue, "C");//
			Result = false;
		}
	}
	// --------->
	return Result;
}

void BCustomChoTroi::CGReqItemBuy(PMSG_REQ_MARKET_BUY * lpMsg, int aIndex)
{
	if (!OBJECT_RANGE(aIndex))
	{
		return;
	}
	int ID = lpMsg->ID;
	LPOBJ lpUser = &gObj[aIndex];
	if (lpUser->Interface.type == INTERFACE_CHAOS_BOX
		|| lpUser->Interface.type == INTERFACE_TRADE
		|| lpUser->Interface.type == INTERFACE_PARTY
		|| lpUser->Interface.type == INTERFACE_WAREHOUSE
		|| lpUser->Interface.type == INTERFACE_PERSONAL_SHOP
		|| lpUser->Interface.type == INTERFACE_CASH_SHOP
		|| lpUser->Interface.type == INTERFACE_TRAINER
		|| lpUser->Interface.use != 0
		|| lpUser->State == OBJECT_DELCMD
		|| lpUser->DieRegen != 0
		|| lpUser->Teleport != 0
		|| lpUser->PShopOpen != 0
		|| lpUser->ChaosLock != 0
		|| lpUser->SkillSummonPartyTime != 0)
	{
		gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(0));//
		return;
	}
	// Delay Khi Click
	if ((GetTickCount() - lpUser->ClickClientSend) < 2000)
	{
		gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(5));//
		return;
	}
	//LogAdd(LOG_RED, "%d",lpMsg->Pass);

	SDHP_REQ_MARKET_BUY pMsg;

	pMsg.h.set(0xEC, 0x02, sizeof(pMsg));

	pMsg.aIndex = aIndex;

	pMsg.ID = ID;

	pMsg.Result = 0;

	if (!lpMsg->Result) return;
	for (int i = 0; i < this->m_ListDataChoTroi[aIndex].size(); i++)
	{
		if (ID == this->m_ListDataChoTroi[aIndex][i].ID)
		{
			CItem item;

			if (gItemManager.ConvertItemByte(&item, this->m_ListDataChoTroi[aIndex][i].Item) == 0)
			{
				gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(0));//
				return;
			}
			if (!gItemManager.GetInventoryItemSpace(lpUser, item.m_Index))
			{
				gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(6));//
				return;
			}

			if (lpMsg->Result == 1)
			{
				if (!this->GetCheckMoney(aIndex, this->m_ListDataChoTroi[aIndex][i].PriceType, this->m_ListDataChoTroi[aIndex][i].Price))
				{
					return;
				}
				if (this->m_ListDataChoTroi[aIndex][i].Pass != lpMsg->Pass)
				{
					gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, "SaiPass");//
					return;
				}
			}


			pMsg.Result = lpMsg->Result;
			break;
		}
	}

	//Set Delay
	lpUser->ClickClientSend = GetTickCount();

	gDataServerConnection.DataSend((BYTE*)& pMsg, pMsg.h.size);
}

void BCustomChoTroi::DGAnsItemBuy(SDHP_ANS_MARKET_BUY * lpMsg)
{
	int aIndex = lpMsg->aIndex;

	if (!OBJECT_RANGE(aIndex))
	{
		return;
	}
	LPOBJ lpUser = &gObj[aIndex];
	//LogAdd(LOG_RED, "DGAnsItemBuy %d", lpMsg->Result);
	if (lpMsg->Result == 1 || lpMsg->Result == 2)
	{
		CItem item;
		int Count = 0;
		for (int i = 0; i < this->m_ListDataChoTroi[aIndex].size(); i++)
		{
			if (lpMsg->ID == this->m_ListDataChoTroi[aIndex][i].ID)
			{
				gItemManager.ConvertItemByte(&item, this->m_ListDataChoTroi[aIndex][i].Item);
				if (lpMsg->Result == 1) //MUA
				{
					//===Tru Coin
					if (this->m_ListDataChoTroi[aIndex][i].PriceType == eMarketPriceWC)
					{
						GDSetCoinSend(aIndex, -(this->m_ListDataChoTroi[aIndex][i].Price), 0, 0, "ChoTroi - WCoinC");
						gCashShop.CGCashShopPointRecv(aIndex);
					}
					if (this->m_ListDataChoTroi[aIndex][i].PriceType == eMarketPriceWP)
					{
						GDSetCoinSend(aIndex, 0, -(this->m_ListDataChoTroi[aIndex][i].Price), 0, "ChoTroi - WCoinP");
						gCashShop.CGCashShopPointRecv(aIndex);
					}
					if (this->m_ListDataChoTroi[aIndex][i].PriceType == eMarketPriceGP)
					{
						GDSetCoinSend(aIndex, 0, 0, -(this->m_ListDataChoTroi[aIndex][i].Price), "ChoTroi - GobinP");
						gCashShop.CGCashShopPointRecv(aIndex);
					}

					if (this->m_ListDataChoTroi[aIndex][i].PriceType == eMarketPriceB)
					{
						int GetSlotItemBank = gBCustomItemBank.CheckInfoListItemBank(7181, 0);
						if (GetSlotItemBank != -1)
						{
							gBCustomItemBank.CongTruBank(aIndex, 7181, 0, -(this->m_ListDataChoTroi[aIndex][i].Price));
						}
						//gCustomJewelBank.GDCustomJewelBankDelJewel(lpUser,0,this->m_ListDataChoTroi[aIndex][i].Price);
						////GDSetCoinSend(aIndex, 0, 0, -(this->m_ListDataChoTroi[aIndex][i].Price), "ChoTroi - GobinP");
						//gCashShop.CGCashShopPointRecv(aIndex);
					}
					if (this->m_ListDataChoTroi[aIndex][i].PriceType == eMarketPriceS)
					{
						int GetSlotItemBank = gBCustomItemBank.CheckInfoListItemBank(7182, 0);
						if (GetSlotItemBank != -1)
						{
							gBCustomItemBank.CongTruBank(aIndex, 7182, 0, -(this->m_ListDataChoTroi[aIndex][i].Price));
						}
						//gCustomJewelBank.GDCustomJewelBankDelJewel(lpUser,1,this->m_ListDataChoTroi[aIndex][i].Price);
						//GDSetCoinSend(aIndex, 0, 0, -(this->m_ListDataChoTroi[aIndex][i].Price), "ChoTroi - GobinP");
						//gCashShop.CGCashShopPointRecv(aIndex);
					}
					if (this->m_ListDataChoTroi[aIndex][i].PriceType == eMarketPriceC)
					{
						int GetSlotItemBank = gBCustomItemBank.CheckInfoListItemBank(6159, 0);
						if (GetSlotItemBank != -1)
						{
							gBCustomItemBank.CongTruBank(aIndex, 6159, 0, -(this->m_ListDataChoTroi[aIndex][i].Price));
						}
						//gCustomJewelBank.GDCustomJewelBankDelJewel(lpUser,7,this->m_ListDataChoTroi[aIndex][i].Price);
						//GDSetCoinSend(aIndex, 0, 0, -(this->m_ListDataChoTroi[aIndex][i].Price), "ChoTroi - GobinP");
						//gCashShop.CGCashShopPointRecv(aIndex);
					}



					gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(7), gItemLevel.GetItemName(item.m_Index, item.m_Level));//
					int ListID = this->m_ListDataChoTroi[aIndex][i].ID;
					int ItemPrice = this->m_ListDataChoTroi[aIndex][i].Price;
					int ItemPriceType = this->m_ListDataChoTroi[aIndex][i].PriceType;
					gLog.Output(LOG_TRADE, "[ChoTroi][%s][%s] Vua mua Item [%d] (%s | Index: %04d Gia: %d [%d] Level: %02d, Serial: %08X, Option1: %01d, Option2: %01d, Option3: %01d, NewOption: %03d, JewelOfHarmonyOption: %03d, ItemOptionEx: %03d, SocketOption: %03d, %03d, %03d, %03d, %03d)"
						, lpUser->Account, lpUser->Name, ListID, gItemLevel.GetItemName(item.m_Index, item.m_Level), item.m_Index, ItemPrice, ItemPriceType, item.m_Level, item.m_Serial, item.m_Option1, item.m_Option2, item.m_Option3, item.m_NewOption, item.m_JewelOfHarmonyOption, item.m_ItemOptionEx, item.m_SocketOption[0], item.m_SocketOption[1], item.m_SocketOption[2], item.m_SocketOption[3], item.m_SocketOption[4]);

				}
				else //Thu hoi
				{
					gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(8), gItemLevel.GetItemName(item.m_Index, item.m_Level));//
					int ListID = this->m_ListDataChoTroi[aIndex][i].ID;
					int ItemPrice = this->m_ListDataChoTroi[aIndex][i].Price;
					int ItemPriceType = this->m_ListDataChoTroi[aIndex][i].PriceType;
					gLog.Output(LOG_TRADE, "[ChoTroi][%s][%s] Vua thu hoi Item [%d] (%s | Index: %04d Gia: %d [%d] Level: %02d, Serial: %08X, Option1: %01d, Option2: %01d, Option3: %01d, NewOption: %03d, JewelOfHarmonyOption: %03d, ItemOptionEx: %03d, SocketOption: %03d, %03d, %03d, %03d, %03d)"
						, lpUser->Account, lpUser->Name, ListID, gItemLevel.GetItemName(item.m_Index, item.m_Level), item.m_Index, ItemPrice, ItemPriceType, item.m_Level, item.m_Serial, item.m_Option1, item.m_Option2, item.m_Option3, item.m_NewOption, item.m_JewelOfHarmonyOption, item.m_ItemOptionEx, item.m_SocketOption[0], item.m_SocketOption[1], item.m_SocketOption[2], item.m_SocketOption[3], item.m_SocketOption[4]);

				}

				BYTE btItemPos = gItemManager.InventoryInsertItem(lpMsg->aIndex, item);
				gItemManager.GCItemModifySend(lpMsg->aIndex, btItemPos);
				this->m_ListDataChoTroi[aIndex].erase(this->m_ListDataChoTroi[aIndex].begin() + Count);
				this->SendListUser(aIndex);

				break;
			}
			Count++;
		}
	}

	else
	{
		gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(1));//
		return;
	}

	//PMSG_ANS_MARKET_BUY pMsg;
	//
	//pMsg.h.set((LPBYTE)& pMsg, 0xEC, 0x01, sizeof(pMsg));
	//
	//pMsg.Result = 1;
	//
	//DataSend(aIndex, (LPBYTE)& pMsg, sizeof(pMsg));
}

void BCustomChoTroi::GDReqItemStatus(int aIndex)
{
	if (!OBJECT_RANGE(aIndex))
	{
		return;
	}

	LPOBJ lpUser = &gObj[aIndex];

	SDHP_REQ_MARKET_STATUS pMsg;

	pMsg.h.set(0xEC, 0x03, sizeof(pMsg));

	pMsg.Result = 1;

	pMsg.aIndex = aIndex;

	memcpy(pMsg.Account, lpUser->Account, sizeof(pMsg.Account) - 1);

	gDataServerConnection.DataSend((BYTE*)& pMsg, pMsg.h.size);
}
void BCustomChoTroi::DGAnsItemStatus(CBCUSTOM_LOAD_COUNT * lpMsg)
{
	LPOBJ lpUser = &gObj[lpMsg->aIndex];
	for (int n = 0; n < lpMsg->count; n++)
	{
		SDHP_ANS_MARKET_STATUS* lpInfo = (SDHP_ANS_MARKET_STATUS*)(((BYTE*)lpMsg) + sizeof(CBCUSTOM_LOAD_COUNT) + (sizeof(SDHP_ANS_MARKET_STATUS) * n));
		//LogAdd(LOG_RED, "DEBUG CONg Coin -> %s Coint %d", gObj[lpMsg->aIndex].Name, lpInfo->PriceValue);
		CItem item;
		if (!gItemManager.ConvertItemByte(&item, lpInfo->ItemData))
		{
			continue;
		}
		int ItemPrice = lpInfo->PriceValue;
		int ItemPriceType = lpInfo->PriceType;


		int RateTax = this->GetRateTaxTypeItem(lpInfo->TypeItem, lpInfo->PriceType);
		int CoinAfterTax = lpInfo->PriceValue - ((lpInfo->PriceValue * RateTax) / 100);


		//===Cong Coin
		if (ItemPriceType == eMarketPriceWC)
		{
			GDSetCoinSend(lpUser->Index, +CoinAfterTax, 0, 0, "ChoTroi + WCoinC");
			gCashShop.CGCashShopPointRecv(lpUser->Index);
		}
		else if (ItemPriceType == eMarketPriceWP)
		{
			GDSetCoinSend(lpUser->Index, 0, +CoinAfterTax, 0, "ChoTroi + WCoinP");
			gCashShop.CGCashShopPointRecv(lpUser->Index);
		}
		else if (ItemPriceType == eMarketPriceGP)
		{
			GDSetCoinSend(lpUser->Index, 0, 0, +CoinAfterTax, "ChoTroi + GobinP");
			gCashShop.CGCashShopPointRecv(lpUser->Index);
		}

		else if (ItemPriceType == eMarketPriceB)
		{
			int GetSlotItemBank = gBCustomItemBank.CheckInfoListItemBank(7181, 0);
			if (GetSlotItemBank != -1)
			{
				gBCustomItemBank.CongTruBank(lpUser->Index, 7181, 0, +(CoinAfterTax));
			}
			//gCustomJewelBank.GDCustomJewelBankAddJewel(lpUser,0,CoinAfterTax);
			//GDSetCoinSend(lpUser->Index, 0, 0, +CoinAfterTax, "ChoTroi + GobinP");
			//gCashShop.CGCashShopPointRecv(lpUser->Index);
		}
		else if (ItemPriceType == eMarketPriceS)
		{
			int GetSlotItemBank = gBCustomItemBank.CheckInfoListItemBank(7182, 0);
			if (GetSlotItemBank != -1)
			{
				gBCustomItemBank.CongTruBank(lpUser->Index, 7182, 0, +(CoinAfterTax));
			}
			//gCustomJewelBank.GDCustomJewelBankAddJewel(lpUser,1,CoinAfterTax);
			//GDSetCoinSend(lpUser->Index, 0, 0, +CoinAfterTax, "ChoTroi + GobinP");
			//gCashShop.CGCashShopPointRecv(lpUser->Index);
		}
		else if (ItemPriceType == eMarketPriceC)
		{
			int GetSlotItemBank = gBCustomItemBank.CheckInfoListItemBank(6159, 0);
			if (GetSlotItemBank != -1)
			{
				gBCustomItemBank.CongTruBank(lpUser->Index, 6159, 0, +(CoinAfterTax));
			}
			//gCustomJewelBank.GDCustomJewelBankAddJewel(lpUser,7,CoinAfterTax);
			//GDSetCoinSend(lpUser->Index, 0, 0, +CoinAfterTax, "ChoTroi + GobinP");
			//gCashShop.CGCashShopPointRecv(lpUser->Index);
		}
		//===

		this->GuiThuThongBao(lpUser, this->GetMessage(10), "=====[Successfully Sold]===== \n► TAX %d%% (%s) += %d \n► %s (Price: %d [%s]) \n* Level: %02d\n* Serial:%08X\n* Skill: %01d\n* Luck: %01d\n* Opt: %01d\n* ExcOpt: %03d\n* JewelOfHarmonyOption: %03d\n* ItemOptionEx: %03d\n* SocketOption: %03d, %03d, %03d, %03d, %03d",
			RateTax, TypeCoin[ItemPriceType], CoinAfterTax, gItemLevel.GetItemName(item.m_Index, item.m_Level), ItemPrice, TypeCoin[ItemPriceType],
			item.m_Level, item.m_Serial, item.m_Option1, item.m_Option2, item.m_Option3, item.m_NewOption, item.m_JewelOfHarmonyOption, item.m_ItemOptionEx, item.m_SocketOption[0], item.m_SocketOption[1], item.m_SocketOption[2], item.m_SocketOption[3], item.m_SocketOption[4]);

		gLog.Output(LOG_TRADE, "[ChoTroi][%s][%s] Ban Thanh Cong Item (%s | Index: %04d Gia: %d [%s] Level: %02d, Serial: %08X, Option1: %01d, Option2: %01d, Option3: %01d, NewOption: %03d, JewelOfHarmonyOption: %03d, ItemOptionEx: %03d, SocketOption: %03d, %03d, %03d, %03d, %03d)"
			, lpUser->Account, lpUser->Name, gItemLevel.GetItemName(item.m_Index, item.m_Level), item.m_Index, ItemPrice, TypeCoin[ItemPriceType], item.m_Level, item.m_Serial, item.m_Option1, item.m_Option2, item.m_Option3, item.m_NewOption, item.m_JewelOfHarmonyOption, item.m_ItemOptionEx, item.m_SocketOption[0], item.m_SocketOption[1], item.m_SocketOption[2], item.m_SocketOption[3], item.m_SocketOption[4]);

	}
}

void FriendMessageSend(PMSG_FRIEND_MEMO * lpMsg, int aIndex, int bIndex)
{
	if (!gObjIsConnectedGP(aIndex))
	{
		return;
	}


	char szName[11];

	memset(szName, 0, sizeof(szName));

	memcpy(szName, lpMsg->Name, sizeof(lpMsg->Name));

	FHP_FRIEND_MEMO_SEND pMsg;
	int bsize = lpMsg->MemoSize + sizeof(pMsg) - sizeof(pMsg.Memo);

	pMsg.h.set(0x70, bsize);
	pMsg.WindowGuid = lpMsg->WindowGuid;
	pMsg.MemoSize = lpMsg->MemoSize;
	pMsg.Number = aIndex;
	pMsg.Dir = lpMsg->Dir;
	pMsg.Action = lpMsg->Action;

	memcpy(pMsg.Subject, lpMsg->Subject, sizeof(pMsg.Subject));
	memcpy(pMsg.Name, lpMsg->Name, sizeof(pMsg.Name));
	memcpy(pMsg.Photo, gObj[bIndex].CharSet, sizeof(pMsg.Photo));
	memset(pMsg.ToName, 0, sizeof(pMsg.ToName));
	memcpy(pMsg.ToName, gObj[aIndex].Name, sizeof(lpMsg->Name));
	memcpy(pMsg.Memo, lpMsg->Memo, lpMsg->MemoSize);

	CSDataSend((BYTE*)& pMsg, bsize);
}
void BCustomChoTroi::GuiThuThongBao(LPOBJ lpTarget, char* Subject, char* message, ...)
{
	PMSG_FRIEND_MEMO pMsg;

	pMsg.WindowGuid = 0;

	//strcpy_s(pMsg.Name,"AFK");

	memcpy(pMsg.Name, "[He Thong]", sizeof(pMsg.Name));
	memcpy(pMsg.Subject, Subject, sizeof(pMsg.Subject));

	pMsg.Action = 143;
	pMsg.Dir = 2;

	char buff[1000] = { 0 };

	va_list arg;
	va_start(arg, message);
	vsprintf_s(buff, message, arg);
	va_end(arg);


	memcpy(pMsg.Memo, buff, sizeof(pMsg.Memo));
	pMsg.MemoSize = sizeof(buff);


	FriendMessageSend(&pMsg, lpTarget->Index, lpTarget->Index);
}

#endif