#include "stdafx.h"
#if CB_VIP_CHAR
#include "BCustomVIPChar.h"
#include "Message.h"
#include "DSProtocol.h"
#include "Notice.h"
#include "ItemManager.h"
#include "GameMain.h"
#include "Util.h"
#include "Log.h"
#include "ItemLevel.h"
#include "ObjectManager.h"
#include "Map.h"
#include "ServerInfo.h"
#include "MapServerManager.h"
#include "ItemOption.h"
#include "CustomRankUser.h"
#include "CashShop.h"

BCustomVIPChar gBCustomVIPChar;


void BCustomVIPChar::Load(char* FilePath)
{
	this->Enable = 0;
	pugi::xml_document file;
	pugi::xml_parse_result res = file.load_file(FilePath);
	if (res.status != pugi::status_ok) {
		ErrorMessageBox("File %s load fail. Error: %s", FilePath, res.description());
		return;
	}
	pugi::xml_node oCustomVIPChar = file.child("CustomVIPChar");
	this->Enable = oCustomVIPChar.attribute("Enable").as_int();
	this->Firework = oCustomVIPChar.attribute("Firework").as_int();
	this->Notice = oCustomVIPChar.attribute("Notice").as_int();
	strncpy_s(this->Msg, oCustomVIPChar.attribute("Msg").as_string(), sizeof(this->Msg));

	this->m_MessageInfoBP.clear();
	//==Load Mess
	pugi::xml_node Message = oCustomVIPChar.child("MessageInfo");
	for (pugi::xml_node msg = Message.child("Message"); msg; msg = msg.next_sibling())
	{
		CUSTOM_VIPCHAR_MESSAGE info;

		info.Index = msg.attribute("Index").as_int();

		strncpy_s(info.Message, msg.attribute("Text").as_string(), sizeof(info.Message));

		this->m_MessageInfoBP.insert(std::pair<int, CUSTOM_VIPCHAR_MESSAGE>(info.Index, info));
	}
	//=======Load Data Item >> Point VIP
	this->m_ItemPoint.clear();
	pugi::xml_node ItemPoint = oCustomVIPChar.child("ItemPoint");
	for (pugi::xml_node Item = ItemPoint.child("Item"); Item; Item = Item.next_sibling())
	{
		DATA_ITEMVIPPOINT infoItemPoint;
		infoItemPoint.IndexItem = Item.attribute("IndexItem").as_int();
		infoItemPoint.Level = Item.attribute("Level").as_int();
		infoItemPoint.ExlOp = Item.attribute("ExlOp").as_int();
		infoItemPoint.PointVIP = Item.attribute("PointVIP").as_int();
		this->m_ItemPoint.push_back(infoItemPoint);
	}
	//== Load Config VIP level
	this->m_DataVIPChar.clear();
	pugi::xml_node LevelUpVip = oCustomVIPChar.child("LevelUpVip");
	for (pugi::xml_node Config = LevelUpVip.child("Config"); Config; Config = Config.next_sibling())
	{
		DATA_VIP_CHAR infoConfig;
		infoConfig.LevelVIP = Config.attribute("LevelVIP").as_int();
		infoConfig.PointVIP = Config.attribute("PointVIP").as_int();
		infoConfig.Coin[0] = Config.attribute("WC").as_int();
		infoConfig.Coin[1] = Config.attribute("WP").as_int();
		infoConfig.Coin[2] = Config.attribute("GP").as_int();

		infoConfig.TypeOp[0] = Config.attribute("TypeOp1").as_int(0);
		infoConfig.ValueOp[0] = Config.attribute("ValueOp1").as_int(0);
		//==
		infoConfig.TypeOp[1] = Config.attribute("TypeOp2").as_int(0);
		infoConfig.ValueOp[1] = Config.attribute("ValueOp2").as_int(0);
		//==
		infoConfig.TypeOp[2] = Config.attribute("TypeOp3").as_int(0);
		infoConfig.ValueOp[2] = Config.attribute("ValueOp3").as_int(0);
		//==
		infoConfig.TypeOp[3] = Config.attribute("TypeOp4").as_int(0);
		infoConfig.ValueOp[3] = Config.attribute("ValueOp4").as_int(0);
		//==
		infoConfig.TypeOp[4] = Config.attribute("TypeOp5").as_int(0);
		infoConfig.ValueOp[4] = Config.attribute("ValueOp5").as_int(0);
		//==
		infoConfig.TypeOp[5] = Config.attribute("TypeOp6").as_int(0);
		infoConfig.ValueOp[5] = Config.attribute("ValueOp6").as_int(0);

		this->m_DataVIPChar.insert(std::pair<int, DATA_VIP_CHAR>(infoConfig.LevelVIP, infoConfig));
	}
	LogAdd(LOG_BLUE, "[VIPChar] Enable %d, Date ItemPoint %d, Data LevelVip %d", this->Enable, this->m_ItemPoint.size(), this->m_DataVIPChar.size());
}

char* BCustomVIPChar::GetMessage(int index) // OK
{
	std::map<int, CUSTOM_VIPCHAR_MESSAGE>::iterator it = this->m_MessageInfoBP.find(index);

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

DATA_VIP_CHAR* BCustomVIPChar::GetInfoVIPChar(int LvVIPChar)
{
	std::map<int, DATA_VIP_CHAR>::iterator it = this->m_DataVIPChar.find(LvVIPChar);
	if (it != this->m_DataVIPChar.end())
	{
		return &it->second;
	}

	return 0;
}

void BCustomVIPChar::CheckItemPointVIP(LPOBJ lpObj, int SlotItem)
{
	if (gObjIsConnected(lpObj->Index) == false)
	{
		return;
	}

	if (!this->Enable || !this->m_DataVIPChar.size())
	{
		gNotice.GCNoticeSend(lpObj->Index, 255, 0, 0, 0, 0, 0, this->GetMessage(0));
		return;
	}
	for (std::vector<DATA_ITEMVIPPOINT>::iterator it = this->m_ItemPoint.begin(); it != this->m_ItemPoint.end(); it++)
	{
		if (lpObj->Inventory[SlotItem].m_Index == it->IndexItem && (lpObj->Inventory[SlotItem].m_Level == it->Level || it->Level == 255) && (lpObj->Inventory[SlotItem].m_NewOption == it->ExlOp || it->ExlOp == 255))
		{
			int SL1 = gItemManager.GetInventoryItemCount(lpObj, lpObj->Inventory[SlotItem].m_Index, lpObj->Inventory[SlotItem].m_Level);
			//gItemManager.InventoryDelItem(lpObj->Index, SlotItem);
			//gItemManager.GCItemDeleteSend(lpObj->Index, SlotItem, 1);
			//gNotice.GCNoticeSend(lpObj->Index, 255, 0, 0, 0, 0, 0, "%d ",SL1);

			gItemManager.DeleteInventoryItemCount(lpObj, lpObj->Inventory[SlotItem].m_Index, lpObj->Inventory[SlotItem].m_Level, SL1);
			lpObj->PointVIPChar += it->PointVIP * SL1;
			SendInfoToClient(lpObj->Index, 1);
		}
	}
	return;

}
//===Active Dmg Bonus
void BCustomVIPChar::ActiveBonus(LPOBJ lpObj, bool flag)
{
	if (flag != 0)
	{
		return;
	}
	if (!this->Enable || !this->m_DataVIPChar.size())
	{
		return;
	}

	//if (lpObj->CB_Status == 1)
	//{
	//	return;
	//}

	if (lpObj->LvVIPChar > 0)
	{
		DATA_VIP_CHAR* GetInfoVIPChar = this->GetInfoVIPChar(lpObj->LvVIPChar);
		if (!GetInfoVIPChar) { return; }
		for (int n = 0; n < 6; n++)
		{
			gItemOption.CBInsertOption(lpObj, GetInfoVIPChar->TypeOp[n], GetInfoVIPChar->ValueOp[n]);
		}
	}

	//LogAdd(LOG_RED, "ActiveBonusSkin");
}
//=Thang Cap
void BCustomVIPChar::UpLevelVIPChar(LPOBJ lpObj)
{
	if (gObjIsConnected(lpObj->Index) == false)
	{
		return;
	}

	if (!this->Enable || !this->m_DataVIPChar.size())
	{
		gNotice.GCNoticeSend(lpObj->Index, 255, 0, 0, 0, 0, 0, this->GetMessage(0));
		return;
	}
	DATA_VIP_CHAR* GetInfoVIPChar = this->GetInfoVIPChar(lpObj->LvVIPChar + 1);

	if (!GetInfoVIPChar)
	{
		gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, this->GetMessage(6));
		return;
	}
	//===Del Coin
	if (lpObj->Coin1 < GetInfoVIPChar->Coin[0] || lpObj->Coin2 < GetInfoVIPChar->Coin[1] || lpObj->Coin3 < GetInfoVIPChar->Coin[2] || lpObj->PointVIPChar < GetInfoVIPChar->PointVIP)
	{
		gNotice.GCNoticeSend(lpObj->Index, 255, 0, 0, 0, 0, 0, this->GetMessage(4));
		return;
	}

	lpObj->PointVIPChar -= GetInfoVIPChar->PointVIP;

	if (GetInfoVIPChar->Coin[0] > 0 || GetInfoVIPChar->Coin[1] > 0 || GetInfoVIPChar->Coin[2] > 0)
	{
		GDSetCoinSend(lpObj->Index, -(GetInfoVIPChar->Coin[0]), -(GetInfoVIPChar->Coin[1]), -(GetInfoVIPChar->Coin[2]), "TruCoinUPVipChar");
		gCashShop.CGCashShopPointRecv(lpObj->Index);
	}

	lpObj->LvVIPChar++;
	gObjectManager.CharacterCalcAttribute(lpObj->Index);
	gCustomRankUser.GCReqRankLevelUser(lpObj->Index, lpObj->Index);
	SendInfoToClient(lpObj->Index, 2);

	//==Send Effect
	if (this->Firework == 1)
	{
		GCServerCommandSend(lpObj->Index, 0, lpObj->X, lpObj->Y);
	}
	else if (this->Firework == 2)
	{
		GCServerCommandSend(lpObj->Index, 2, lpObj->X, lpObj->Y);
	}
	else if (this->Firework == 3)
	{
		GCServerCommandSend(lpObj->Index, 58, SET_NUMBERHB(lpObj->Index), SET_NUMBERLB(lpObj->Index));
	}
	char tmp[255];
	char tmp2[255];
	if (this->Notice == 1) { //Thong Bao trong Sub
		gNotice.GCNoticeSend(lpObj->Index, 0, 0, 0, 0, 0, 0, this->Msg, lpObj->Name, lpObj->LvVIPChar);
	}
	else if (this->Notice == 2)
	{ //Thong Bao Toan Sub
		wsprintf(tmp, this->Msg, lpObj->Name, lpObj->LvVIPChar);
		GDGlobalNoticeSend(gMapServerManager.GetMapServerGroup(), 0, 0, 0, 0, 0, 0, tmp);
	}
	else if (this->Notice == 3)
	{ //Thong Bao Toan Sub
		wsprintf(tmp, this->Msg, lpObj->Name, lpObj->LvVIPChar);
		wsprintf(tmp2, "%s %s", gServerInfo.m_ServerName, tmp);
		GDGlobalNoticeSend(gMapServerManager.GetMapServerGroup(), 0, 0, 0, 0, 0, 0, tmp2);
	}
	gNotice.GCNoticeSend(lpObj->Index, 255, 0, 0, 0, 0, 0, this->GetMessage(3));
}
//===Open WIndow -> Send Info to Client
void BCustomVIPChar::SendListItemPoint(int Index)
{
	if (gObjIsConnected(Index) == false)
	{
		return;
	}

	if (!this->Enable || !this->m_DataVIPChar.size())
	{
		//gNotice.GCNoticeSend(Index, 1, 0, 0, 0, 0, 0, this->GetMessage(0));
		return;
	}
	LPOBJ lpObj = &gObj[Index];
	BYTE send[8192];

	VCSEV_COUNTLIST pMsg;
	pMsg.header.set(0xD3, 0x24, sizeof(pMsg));

	int size = sizeof(pMsg);

	pMsg.Count = 0;

	for (int i = 0; i < this->m_ItemPoint.size(); i++)
	{
		if (size + sizeof(this->m_ItemPoint[i]) > 8192)
		{
			LogAdd(LOG_RED, "[BCustomVIPChar] Data qua dai !!");
			return;
		}
		memcpy(&send[size], &this->m_ItemPoint[i], sizeof(this->m_ItemPoint[i]));
		size += sizeof(this->m_ItemPoint[i]);

		pMsg.Count++;
	}


	pMsg.header.size[0] = SET_NUMBERHB(size);

	pMsg.header.size[1] = SET_NUMBERLB(size);

	memcpy(send, &pMsg, sizeof(pMsg));
	DataSend(Index, send, size);
}
//===Open WIndow -> Send Info to Client
void BCustomVIPChar::SendInfoToClient(int Index, int Type)
{
	if (gObjIsConnected(Index) == false)
	{
		return;
	}

	if (!this->Enable || !this->m_DataVIPChar.size())
	{
		gNotice.GCNoticeSend(Index, 1, 0, 0, 0, 0, 0, this->GetMessage(0));
		return;
	}


	LPOBJ lpObj = &gObj[Index];
	//== Type = 0 ; Open Window
	//=== =  1 ; ItemPoint Update Exp Point
	//=== = 2 ; Update Level VIP Char 
	BYTE send[4096];

	BPMSG_CUSTOM_VIPCHAR_SEND pMsg;

	pMsg.header.set(0xD3, 0x22, 0);

	int size = sizeof(pMsg);


	//LogAdd(LOG_RED, "%s VipChar %d", lpObj->Name, lpObj->LvVIPChar);
	DATA_VIP_CHAR* GetInfoVIPChar = this->GetInfoVIPChar(lpObj->LvVIPChar + 1);
	if (!GetInfoVIPChar)
	{
		gNotice.GCNoticeSend(Index, 1, 0, 0, 0, 0, 0, this->GetMessage(6));
		return;
	}

	pMsg.TypeSend = Type;
	pMsg.UserPointVIP = lpObj->PointVIPChar;

	pMsg.LevelVIP = GetInfoVIPChar->LevelVIP;
	pMsg.PointVIP = GetInfoVIPChar->PointVIP;
	//==Coin
	pMsg.Coin[0] = GetInfoVIPChar->Coin[0];
	pMsg.Coin[1] = GetInfoVIPChar->Coin[1];
	pMsg.Coin[2] = GetInfoVIPChar->Coin[2];

	for (int n = 0; n < 6; n++)
	{
		pMsg.TypeOp[n] = GetInfoVIPChar->TypeOp[n];
		pMsg.ValueOp[n] = GetInfoVIPChar->ValueOp[n];
	}

	pMsg.header.size[0] = SET_NUMBERHB(size);
	pMsg.header.size[1] = SET_NUMBERLB(size);
	// ---
	memcpy(send, &pMsg, sizeof(pMsg));

	DataSend(Index, send, size);
}
#endif