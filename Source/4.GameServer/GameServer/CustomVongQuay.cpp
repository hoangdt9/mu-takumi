#include "stdafx.h"
#include "CustomVongQuay.h"
#include "MemScript.h"
#include "Util.h"
#include "Notice.h"
#include "DSProtocol.h"
#include "EffectManager.h"
#include "Map.h"
#include "ItemOptionRate.h"
#include "ObjectManager.h"
#include "Guild.h"
#include "Move.h"
#include "Monster.h"
#include "ItemBagManager.h"
#include "Party.h"
#include "CashShop.h"
#include "MapServerManager.h"
#include "ServerInfo.h"
#include "RandomManager.h"
#include "ItemLevel.h"

CCustomVongQuay gCustomVongQuay;

CCustomVongQuay::CCustomVongQuay()
{
	this->Init();
}


CCustomVongQuay::~CCustomVongQuay()
{
}

void CCustomVongQuay::Init()
{
	this->SoVongQuay = 0;
}

void CCustomVongQuay::LoadFileXML(char* FilePath)
{
	pugi::xml_document file;
	pugi::xml_parse_result res = file.load_file(FilePath);
	if (res.status != pugi::status_ok) {
		ErrorMessageBox("File %s load fail. Error: %s", FilePath, res.description());
		return;
	}
	//--
	//--
	pugi::xml_node oCustomVongQuay = file.child("CustomVongQuay");
	this->Enable = oCustomVongQuay.attribute("Enable").as_int();
	this->Firework = oCustomVongQuay.attribute("Firework").as_int();
	this->Notice = oCustomVongQuay.attribute("Notice").as_int();
	//= Mess Load
	this->m_MessageInfoBP.clear();
	pugi::xml_node Message = oCustomVongQuay.child("MessageInfo");
	for (pugi::xml_node msg = Message.child("Message"); msg; msg = msg.next_sibling())
	{
		MESSAGE_INFO_VONGQUAY info;

		info.Index = msg.attribute("Index").as_int();

		strcpy_s(info.Message, msg.attribute("Text").as_string());

		this->m_MessageInfoBP.insert(std::pair<int, MESSAGE_INFO_VONGQUAY>(info.Index, info));
	}
	//====Load Data Moc Nap
	this->m_DataVongQuay.clear();
	pugi::xml_node ConfigVongQuay = oCustomVongQuay.child("ConfigVongQuay");
	int IndexMocNap = 1;
	for (pugi::xml_node VongQuay = ConfigVongQuay.child("VongQuay"); VongQuay; VongQuay = VongQuay.next_sibling())
	{
		DATA_VONGQUAY infoData = { 0 };
		infoData.IndexVongQuay = IndexMocNap++;
		
		infoData.IndexItemYC = SafeGetItem(GET_ITEM(VongQuay.attribute("Type").as_int(), VongQuay.attribute("Index").as_int()));
		infoData.WC = VongQuay.attribute("WC").as_int();
		infoData.WP = VongQuay.attribute("WP").as_int();
		infoData.GP = VongQuay.attribute("GP").as_int();
		infoData.Count = VongQuay.attribute("Count").as_int();
		strncpy_s(infoData.NameVongQuay, VongQuay.attribute("Name").as_string(), sizeof(infoData.NameVongQuay));
		//===ItemNhan
		infoData.ListItemNhan.clear();
		pugi::xml_node ItemNhan = VongQuay.child("ItemNhan");
		
		for (pugi::xml_node Item = ItemNhan.child("Item"); Item; Item = Item.next_sibling())
		{
			if (infoData.ListItemNhan.size() > 11) break;

			DATA_VONGQUAYITEM ListItemInfo;
			ListItemInfo.SizeBMD = Item.attribute("SizeBMD").as_float();
			ListItemInfo.PosX = Item.attribute("PosX").as_float();
			ListItemInfo.PosY = Item.attribute("PosY").as_float();
			ListItemInfo.IndexItem = SafeGetItem(GET_ITEM(Item.attribute("Type").as_int(), Item.attribute("Index").as_int()));
			ListItemInfo.LvItem = Item.attribute("LvItem").as_int();
			ListItemInfo.Dur = Item.attribute("Dur").as_int();
			ListItemInfo.Skill = Item.attribute("Skill").as_int();
			ListItemInfo.Luck = Item.attribute("Luck").as_int();
			ListItemInfo.Opt = Item.attribute("Opt").as_int();
			ListItemInfo.Exc = Item.attribute("Exc").as_int();
			ListItemInfo.Anc = Item.attribute("Anc").as_int();

			ListItemInfo.SK[0] = Item.attribute("SK1").as_int();
			ListItemInfo.SK[1] = Item.attribute("SK2").as_int();
			ListItemInfo.SK[2] = Item.attribute("SK3").as_int();
			ListItemInfo.SK[3] = Item.attribute("SK4").as_int();
			ListItemInfo.SK[4] = Item.attribute("SK5").as_int();

			ListItemInfo.SKBonus = Item.attribute("SKBonus").as_int();
			ListItemInfo.HSD = Item.attribute("HSD").as_int();
			ListItemInfo.Rate = Item.attribute("Rate").as_int();
			//LogAdd(LOG_CAM, "[VongQuay] [%d] index %d size ", ListItemInfo.IndexItem, infoData.ListItemNhan.size());
			infoData.ListItemNhan.push_back(ListItemInfo);
		}
		this->m_DataVongQuay.insert(std::pair<int, DATA_VONGQUAY>(infoData.IndexVongQuay, infoData));
	}
	
	LogAdd(LOG_BLUE, "[VongQuay] [%d] Size %d ", this->Enable, this->m_DataVongQuay.size());
}

char* CCustomVongQuay::GetMessage(int index) // OK
{
	std::map<int, MESSAGE_INFO_VONGQUAY>::iterator it = this->m_MessageInfoBP.find(index);

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


void CCustomVongQuay::UserSendClientInfo(int aIndex) //Send Danh Sach Moc Nap Ve Client
{

	if (gObj[aIndex].Type != OBJECT_USER)
	{
		return;
	}

	if (gObjIsConnected(aIndex) == false)
	{
		return;
	}
	if (gObj[aIndex].IsBot >= 1 || gObj[aIndex].m_OfflineMode != 0 || gObj[aIndex].IsFakeOnline != 0)
	{
		return;
	}
	BYTE send[4096];
	PMSG_VONGQUAY_SEND pMsg = { 0 };
	// ---
	pMsg.header.set(0xD3, 0x8A, 0);

	int size = sizeof(pMsg);

	pMsg.count = 0;
	

	for (std::map<int, DATA_VONGQUAY>::iterator it = this->m_DataVongQuay.begin(); it != this->m_DataVongQuay.end(); it++)
	{
		if (it == this->m_DataVongQuay.end())
		{
			break;
		}
		ListVongQuaySend info;
		info.IndexVongQuay = it->second.IndexVongQuay;;
		memset(info.Name, 0, sizeof(info.Name));
		memcpy(info.Name, it->second.NameVongQuay, sizeof(info.Name));
		if ((size + sizeof(info) >= 4096))
		{
			break;
		}
		pMsg.count++;
		memcpy(&send[size], &info, sizeof(info));
		size += sizeof(info);
	}
	pMsg.header.size[0] = SET_NUMBERHB(size);
	pMsg.header.size[1] = SET_NUMBERLB(size);
	// ---
	memcpy(send, &pMsg, sizeof(pMsg));

	DataSend(aIndex, send, size);
	//LogAdd(LOG_RED, "SendINfo List Vong Quay %s", gObj[aIndex].Name);
	//SendListNhanThuong(gObj[aIndex].Index, 1);
}


void VongQuay_ItemByteConvert(BYTE* lpMsg, DATA_VONGQUAYITEM* Data) // OK
{

	lpMsg[0] = Data->IndexItem & 0xFF;

	lpMsg[1] = 0;
	lpMsg[1] |= Data->LvItem * 8;
	lpMsg[1] |= Data->Skill * 128;
	lpMsg[1] |= Data->Luck * 4;
	lpMsg[1] |= Data->Opt & 3;

	lpMsg[2] = Data->Dur;

	lpMsg[3] = 0;
	lpMsg[3] |= (Data->IndexItem & 0x100) >> 1;
	lpMsg[3] |= ((Data->Opt > 3) ? 0x40 : 0);
	lpMsg[3] |= Data->Exc;

	lpMsg[4] = Data->Anc;

	lpMsg[5] = 0;
	lpMsg[5] |= (Data->IndexItem & 0x1E00) >> 5;
	lpMsg[5] |= ((Data->Exc & 0x80) >> 4);
	lpMsg[5] |= ((Data->HSD & 1) << 2);

	lpMsg[6] = Data->SKBonus;

	lpMsg[7] = Data->SK[0];
	lpMsg[8] = Data->SK[1];
	lpMsg[9] = Data->SK[2];
	lpMsg[10] = Data->SK[3];
	lpMsg[11] = Data->SK[4];
}


void CCustomVongQuay::SendListNhanThuong(int aIndex, int VongQuaySo) //Send List SendListNhanThuong
{
	if (!this->Enable)
	{
		gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, this->GetMessage(0)); //
		return;
	}

	if (gObj[aIndex].Type != OBJECT_USER)
	{
		return;
	}

	if (gObjIsConnected(aIndex) == false)
	{
		return;
	}
	if (gObj[aIndex].IsBot >= 1 || gObj[aIndex].m_OfflineMode != 0 || gObj[aIndex].IsFakeOnline != 0)
	{
		return;
	}
	BYTE send[4096];
	PMSG_YCVONGQUAY_SEND pMsg = { 0 };
	// ---
	pMsg.header.set(0xD3, 0x8B, 0);

	int size = sizeof(pMsg);

	pMsg.count = 0;

	std::map<int, DATA_VONGQUAY>::iterator it = this->m_DataVongQuay.find(VongQuaySo);

	if (it == this->m_DataVongQuay.end())
	{
		gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(1)); //Khong co thong tin cua moc nap
		return;
	}
	pMsg.IndexYC = it->second.IndexItemYC;
	pMsg.CountItem = it->second.Count;
	pMsg.WCYC = it->second.WC;
	pMsg.WPYC = it->second.WP;
	pMsg.GPYC = it->second.GP;

	for (std::vector<DATA_VONGQUAYITEM>::iterator itItem = it->second.ListItemNhan.begin(); itItem != it->second.ListItemNhan.end(); itItem++)
	{
		if (itItem == it->second.ListItemNhan.end())
		{
			break;
		}

		
		LISTITEMVONGQUAY_SENDINFO info;
		info.SizeBMD = itItem->SizeBMD;
		info.PosX = itItem->PosX;
		info.PosY = itItem->PosY;
		info.Index = itItem->IndexItem;
		info.Dur = itItem->Dur;
		VongQuay_ItemByteConvert(info.Item, &*itItem);
		time_t t = time(NULL);
		localtime(&t);
		DWORD iTime = (DWORD)t + itItem->HSD * 60;
		if ((itItem->HSD) > 0)
		{
			info.PeriodTime = iTime;
		}
		else
		{
			info.PeriodTime = itItem->HSD;
		}
		if ((size + sizeof(info) >= 4096))
		{
			break;
		}
		pMsg.count++;
		memcpy(&send[size], &info, sizeof(info));
		size += sizeof(info);
	}
	pMsg.header.size[0] = SET_NUMBERHB(size);
	pMsg.header.size[1] = SET_NUMBERLB(size);
	// ---
	memcpy(send, &pMsg, sizeof(pMsg));

	DataSend(aIndex, send, size);
	//LogAdd(LOG_RED, "Send List Item Vong Quay %s", gObj[aIndex].Name);
}

void CCustomVongQuay::MakeItem(int aIndex,int type)
{
	LPOBJ lpObj = &gObj[aIndex];
	if (type != 1)
	{
		goto Next;
	}
	if (lpObj->SauVongQuay > 0 && GetTickCount() - lpObj->SauVongQuay > 5000)
		Next:
	{
		lpObj->SauVongQuay = 0;

		std::map<int, DATA_VONGQUAY>::iterator it = this->m_DataVongQuay.find(this->SoVongQuay);

		CRandomManager RandomManager;
		WORD iIndex = 0;

		CRandomManager RandomMng;

		for (int n = 0; n < it->second.ListItemNhan.size(); n++)
		{
			RandomMng.AddElement(n, it->second.ListItemNhan[n].Rate);
		}

		RandomMng.GetRandomElement(&iIndex);

		BYTE ItemSocketOption[MAX_SOCKET_OPTION] = { it->second.ListItemNhan[iIndex].SK[0], it->second.ListItemNhan[iIndex].SK[1], it->second.ListItemNhan[iIndex].SK[2], it->second.ListItemNhan[iIndex].SK[3], it->second.ListItemNhan[iIndex].SK[4] };
		time_t t = time(NULL);
		localtime(&t);
		DWORD iTime = (DWORD)t + it->second.ListItemNhan[iIndex].HSD * 60;
		if (it->second.ListItemNhan[iIndex].HSD > 0)
		{
			GDCreateItemSend(lpObj->Index, 0xEB, (BYTE)lpObj->X, (BYTE)lpObj->Y, it->second.ListItemNhan[iIndex].IndexItem, it->second.ListItemNhan[iIndex].LvItem, it->second.ListItemNhan[iIndex].Dur, it->second.ListItemNhan[iIndex].Skill, it->second.ListItemNhan[iIndex].Luck, it->second.ListItemNhan[iIndex].Opt, -1, it->second.ListItemNhan[iIndex].Exc, it->second.ListItemNhan[iIndex].Anc, 0, 0, ItemSocketOption, it->second.ListItemNhan[iIndex].SKBonus, iTime);
		}
		else
		{
			GDCreateItemSend(lpObj->Index, 0xEB, (BYTE)lpObj->X, (BYTE)lpObj->Y, it->second.ListItemNhan[iIndex].IndexItem, it->second.ListItemNhan[iIndex].LvItem, it->second.ListItemNhan[iIndex].Dur, it->second.ListItemNhan[iIndex].Skill, it->second.ListItemNhan[iIndex].Luck, it->second.ListItemNhan[iIndex].Opt, -1, it->second.ListItemNhan[iIndex].Exc, it->second.ListItemNhan[iIndex].Anc, 0, 0, ItemSocketOption, it->second.ListItemNhan[iIndex].SKBonus, 0);
		}

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
		wsprintf(tmp, this->GetMessage(5), lpObj->Name, gItemLevel.GetItemName(it->second.ListItemNhan[iIndex].IndexItem, it->second.ListItemNhan[iIndex].LvItem));
		if (this->Notice == 1) { //Thong Bao trong Sub
			gNotice.GCNoticeSend(lpObj->Index, 0, 0, 0, 0, 0, 0, tmp);
		}
		else if (this->Notice == 2)
		{ //Thong Bao Toan Sub

			GDGlobalNoticeSend(gMapServerManager.GetMapServerGroup(), 0, 0, 0, 0, 0, 0, tmp);
		}
		else if (this->Notice == 3)
		{ //Thong Bao Toan Sub
			wsprintf(tmp2, "%s %s", gServerInfo.m_ServerName, tmp);
			GDGlobalNoticeSend(gMapServerManager.GetMapServerGroup(), 0, 0, 0, 0, 0, 0, tmp2);
		}

		XULY_CGPACKET_VONGQUAY pMsg;
		pMsg.header.set(0xD3, 0x8C, sizeof(pMsg));
		pMsg.StartRoll = 0;	//Clear Item Cache O Client
		pMsg.IndexWin = iIndex;
		DataSend(lpObj->Index, (BYTE*)&pMsg, pMsg.header.size);
	}
}

void CCustomVongQuay::ActionVongQuay(int aIndex, int MocNap , int solan)
{
	if (!this->Enable)
	{
		gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, this->GetMessage(0)); //
		return;
	}

	if (gObj[aIndex].Type != OBJECT_USER)
	{
		return;
	}

	

	if (gObjIsConnected(aIndex) == false)
	{
		return;
	}
	if (gObj[aIndex].IsBot >= 1 || gObj[aIndex].m_OfflineMode != 0 || gObj[aIndex].IsFakeOnline != 0)
	{
		return;
	}
	LPOBJ lpObj = &gObj[aIndex];

	//if ((GetTickCount() - lpObj->ClickClientSend) < 2000)
	//{
	//	gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, this->GetMessage(8));
	//	return;
	//}

	//===KIem tra trang thai co duoc add item khong
	if (lpObj->Interface.type != INTERFACE_NONE || lpObj->Interface.use != 0 || lpObj->Transaction == 1 )
	{
		return;
	}

	if (gItemManager.ChaosBoxHasItem(lpObj) || gItemManager.TradeHasItem(lpObj))
	{
		return;
	}
	//===================================================

	
	//===Lay Thong Tin Moc Nap
	std::map<int, DATA_VONGQUAY>::iterator it = this->m_DataVongQuay.find(MocNap);
	this->SoVongQuay = MocNap;
	if (it == this->m_DataVongQuay.end())
	{
		gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(1)); //Khong co thong tin cua moc nap
		return;
	}
	if (it->second.WC > lpObj->Coin1)
	{
		gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(3), NumberFormat(it->second.WC)); //Yeu cau gia tri nap
		return;
	}

	if (it->second.WP > lpObj->Coin2)
	{
		gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(3), NumberFormat(it->second.WP)); //Yeu cau gia tri nap
		return;
	}

	if (it->second.GP > lpObj->Coin3)
	{
		gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(3), NumberFormat(it->second.GP)); //Yeu cau gia tri nap
		return;
	}

	if (it->second.Count > 0)
	{
		int count = gItemManager.GetInventoryItemCount(lpObj, it->second.IndexItemYC, 0);
		if (count < it->second.Count)
		{
			gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, this->GetMessage(7), it->second.Count, gItemManager.GetItemName(it->second.IndexItemYC));
			return ;
		}

		gItemManager.DeleteInventoryItemCount(lpObj, it->second.IndexItemYC, 0, it->second.Count);
	}
		
	
	//===Kiem tra thung do
	if (gItemManager.CheckItemInventorySpace(lpObj, 4, 4) == 0)
	{
		gNotice.GCNoticeSend(lpObj->Index, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(6));
		return;
	}


	
	
	//===============Cong Coin
	if (it->second.WC > 0 || it->second.WP > 0 || it->second.GP > 0)
	{
		GDSetCoinSend(lpObj->Index, -it->second.WC, -it->second.WP, -it->second.GP, "Tru Coin Vong Quay");
	}
	
	

	XULY_CGPACKET_VONGQUAY pMsg;
	pMsg.header.set(0xD3, 0x8C, sizeof(pMsg));
	pMsg.StartRoll = 1;	//Clear Item Cache O Client
	pMsg.IndexWin = -1;
	DataSend(lpObj->Index, (BYTE*)&pMsg, pMsg.header.size);
	if (solan == 1)
	{
		lpObj->SauVongQuay = GetTickCount();
	}
	else
	{
		this->MakeItem(lpObj->Index,2);
	}
	//lpObj->ClickClientSend = GetTickCount();
}


