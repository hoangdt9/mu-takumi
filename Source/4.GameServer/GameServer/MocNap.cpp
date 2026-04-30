#include "StdAfx.h"
#include "MocNap.h"
#include "DSProtocol.h"
#include "EffectManager.h"
#include "Map.h"
#include "MemScript.h"
#include "Notice.h"
#include "Util.h"
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
#include "Log.h"
#if(MOCNAP)
gBMocNap gMocNap;

gBMocNap::gBMocNap()
{
}

gBMocNap::~gBMocNap()
{
}

void gBMocNap::LoadFileXML(char* FilePath)
{
	pugi::xml_document file;
	pugi::xml_parse_result res = file.load_file(FilePath);
	if (res.status != pugi::status_ok) {
		ErrorMessageBox("File %s load fail. Error: %s", FilePath, res.description());
		return;
	}
	//--
	//--
	pugi::xml_node oCustomMocNap = file.child("CustomMocNap");
	this->Enable = oCustomMocNap.attribute("Enable").as_int();
	this->Firework = oCustomMocNap.attribute("Firework").as_int();
	this->Notice = oCustomMocNap.attribute("Notice").as_int();
	//= Mess Load
	this->m_MessageInfoBP.clear();
	pugi::xml_node Message = oCustomMocNap.child("MessageInfo");
	for (pugi::xml_node msg = Message.child("Message"); msg; msg = msg.next_sibling())
	{
		MESSAGE_INFO_MocNap info;

		info.Index = msg.attribute("Index").as_int();

		strcpy_s(info.Message, msg.attribute("Text").as_string());

		this->m_MessageInfoBP.insert(std::pair<int, MESSAGE_INFO_MocNap>(info.Index, info));
	}
	//====Load Data Moc Nap
	this->m_DataMocNap.clear();
	pugi::xml_node ConfigMocNap = oCustomMocNap.child("ConfigMocNap");
	int IndexMocNap = 1;
	for (pugi::xml_node MocNap = ConfigMocNap.child("MocNap"); MocNap; MocNap = MocNap.next_sibling())
	{
		DATA_CBMONAP infoData = { 0 };
		infoData.IndexMocNap = IndexMocNap++;
		infoData.GiaTriNap = MocNap.attribute("GiaTriNap").as_int();
		//==Coin Nhan
		pugi::xml_node CoinNhan = MocNap.child("CoinNhan");
		infoData.WC = CoinNhan.attribute("WC").as_int();
		infoData.WP = CoinNhan.attribute("WP").as_int();
		infoData.GP = CoinNhan.attribute("GP").as_int();
		infoData.Ruud = CoinNhan.attribute("Ruud").as_int();
		//===ItemNhan
		infoData.ListItemNhan.clear();
		pugi::xml_node ItemNhan = MocNap.child("ItemNhan");
		for (pugi::xml_node Item = ItemNhan.child("Item"); Item; Item = Item.next_sibling())
		{
			DATA_CBITEMMOCNAP ListItemInfo;
			ListItemInfo.SizeBMD = Item.attribute("SizeBMD").as_float();
			ListItemInfo.Count = Item.attribute("Count").as_int();
			ListItemInfo.IndexItem = Item.attribute("IndexItem").as_int();
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

			ListItemInfo.Class[0] = Item.attribute("DW").as_int();
			ListItemInfo.Class[1] = Item.attribute("DK").as_int();
			ListItemInfo.Class[2] = Item.attribute("ELF").as_int();
			ListItemInfo.Class[3] = Item.attribute("MG").as_int();
			ListItemInfo.Class[4] = Item.attribute("DL").as_int();
			ListItemInfo.Class[5] = Item.attribute("SUM").as_int();
			ListItemInfo.Class[6] = Item.attribute("RF").as_int();
			infoData.ListItemNhan.push_back(ListItemInfo);
		}
		this->m_DataMocNap.insert(std::pair<int, DATA_CBMONAP>(infoData.IndexMocNap, infoData));
	}
	//===Cap Nhat Lai Danh Sach List Nap
	for (int n = OBJECT_START_USER; n < MAX_OBJECT; n++)
	{
		if (gObj[n].Connected == OBJECT_ONLINE)
		{
			if ((gObj[n].Type != OBJECT_USER))
			{
				continue;
			}
			UserSendClientInfo(n);
		}
	}
	LogAdd(LOG_BLUE, "[LoadMocNap] [%d] Size %d ", this->Enable, this->m_DataMocNap.size());
}

char* gBMocNap::GetMessage(int index) // OK
{
	std::map<int, MESSAGE_INFO_MocNap>::iterator it = this->m_MessageInfoBP.find(index);

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
void gBMocNap::UserSendClientInfo(int aIndex) //Send Danh Sach Moc Nap Ve Client
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
	PMSG_CBMONAP_SEND pMsg = { 0 };
	// ---
	pMsg.header.set(0xD3, 0x9A, 0);

	int size = sizeof(pMsg);

	pMsg.count = 0;
	pMsg.NhanMocNap = gObj[aIndex].NhanMocNap;
	pMsg.TongNap = gObj[aIndex].TongNap;

	for (std::map<int, DATA_CBMONAP>::iterator it = this->m_DataMocNap.begin(); it != this->m_DataMocNap.end(); it++)
	{
		if (it == this->m_DataMocNap.end())
		{
			break;
		}
		ListMocNapSend info;
		info.IndexMocNap = it->second.IndexMocNap;;
		info.GiaTriNap = it->second.GiaTriNap;
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
	//LogAdd(LOG_RED, "SendINfo List Moc Nap %s", gObj[aIndex].Name);
}

void CBMONAP_ItemByteConvert(BYTE* lpMsg, DATA_CBITEMMOCNAP* Data) // OK
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

void gBMocNap::SendListNhanThuong(int aIndex, int MocNap) //Send List SendListNhanThuong
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

	if (gObj[aIndex].IsBot >= 1 || gObj[aIndex].m_OfflineMode != 0)
	{
		return;
	}

	BYTE send[4096];

	PMSG_CBLISTTHUONG_SEND pMsg = { 0 };
	// ---
	pMsg.header.set(0xD3, 0x9B, 0);

	int size = sizeof(pMsg);

	pMsg.count = 0;

	std::map<int, DATA_CBMONAP>::iterator it = this->m_DataMocNap.find(MocNap);

	if (it == this->m_DataMocNap.end())
	{
		gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(1)); //Khong co thong tin cua moc nap
		return;
	}
	pMsg.WC = it->second.WC;
	pMsg.WP = it->second.WP;
	pMsg.GP = it->second.GP;
	pMsg.Ruud = it->second.Ruud;

	for (std::vector<DATA_CBITEMMOCNAP>::iterator itItem = it->second.ListItemNhan.begin(); itItem != it->second.ListItemNhan.end(); itItem++)
	{
		if (itItem == it->second.ListItemNhan.end())
		{
			break;
		}

		if (itItem->Class[gObj[aIndex].Class] == 0) //Neu khong phai class duoc active thi bo qua
		{
			continue;
		}
		LISTITEMMOCNAP_SENDINFO info;
		info.SizeBMD = itItem->SizeBMD;
		info.Count = itItem->Count;
		info.Index = itItem->IndexItem;
		info.Dur = itItem->Dur;
		CBMONAP_ItemByteConvert(info.Item, &*itItem);
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
	LogAdd(LOG_RED, "Send List Phan Thuong Moc Nap %s", gObj[aIndex].Name);
}
void gBMocNap::NhanThuongMocNap(int aIndex, int MocNap)
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
	if (gObj[aIndex].IsBot >= 1 || gObj[aIndex].m_OfflineMode != 0)
	{
		return;
	}
	LPOBJ lpObj = &gObj[aIndex];
	//===KIem tra trang thai co duoc add item khong
	if (lpObj->Interface.type != INTERFACE_NONE || lpObj->Interface.use != 0 || lpObj->Transaction == 1)
	{
		return;
	}

	/*if (gItemManager.ChaosBoxHasItem(lpObj) || gItemManager.TradeHasItem(lpObj))
	{
		return;
	}*/
	//===================================================

	if ((MocNap - lpObj->NhanMocNap) > 1)
	{
		gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(2), MocNap - 1); //Nhan MOc Nho hon truoc
		return;
	}
	//===Lay Thong Tin Moc Nap
	std::map<int, DATA_CBMONAP>::iterator it = this->m_DataMocNap.find(MocNap);

	if (it == this->m_DataMocNap.end())
	{
		gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(1)); //Khong co thong tin cua moc nap
		return;
	}
	if (it->second.GiaTriNap > lpObj->TongNap)
	{
		gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(3), NumberFormat(it->second.GiaTriNap)); //Yeu cau gia tri nap
		return;
	}
	if (it->second.IndexMocNap <= lpObj->NhanMocNap)
	{
		gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(4)); //da nhan roi
		return;
	}
	//===Kiem tra thung do
	if (gItemManager.CheckItemInventorySpace(lpObj, 4, 4) == 0)
	{
		gNotice.GCNoticeSend(lpObj->Index, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(6));
		return;
	}


	//===Set Va thong bao
	lpObj->NhanMocNap = MocNap; //Save Moc Nhan
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
	wsprintf(tmp, this->GetMessage(5), lpObj->Name, NumberFormat(it->second.GiaTriNap));
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
	//===============Cong Coin
	if (it->second.WC > 0 || it->second.WP > 0 || it->second.GP > 0)
	{
		GDSetCoinSend(lpObj->Index, it->second.WC, it->second.WP, it->second.GP, "CoinMocNap");
	}
		//if (it->second.Ruud > 0)
		//{
		//	gCashShop.GDCashShopAddPointSaveSend(lpObj->Index, 0, 0, 0, 0, it->second.Ruud);
		//	//-- Ruud Update
		//	gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, "[Ruud] Add Coin %d", it->second.Ruud);

		//}
		//=====
		//================ Add Item
		if (it->second.ListItemNhan.empty() == 0)
		{
			for (int n = 0; n < it->second.ListItemNhan.size(); n++)
			{
				if (it->second.ListItemNhan[n].Class[gObj[aIndex].Class] == 0) //Neu khong phai class duoc active thi bo qua
				{
					continue;
				}
				for (int count = 0; count < it->second.ListItemNhan[n].Count; count++)
				{
					BYTE ItemSocketOption[MAX_SOCKET_OPTION] = { it->second.ListItemNhan[n].SK[0], it->second.ListItemNhan[n].SK[1], it->second.ListItemNhan[n].SK[2], it->second.ListItemNhan[n].SK[3], it->second.ListItemNhan[n].SK[4] };
					time_t t = time(NULL);
					localtime(&t);
					DWORD iTime = (DWORD)t + it->second.ListItemNhan[n].HSD * 60;
					if (it->second.ListItemNhan[n].HSD > 0)
					{
						GDCreateItemSend(lpObj->Index, 0xEB, (BYTE)lpObj->X, (BYTE)lpObj->Y, it->second.ListItemNhan[n].IndexItem, it->second.ListItemNhan[n].LvItem, it->second.ListItemNhan[n].Dur, it->second.ListItemNhan[n].Skill, it->second.ListItemNhan[n].Luck, it->second.ListItemNhan[n].Opt, -1, it->second.ListItemNhan[n].Exc, it->second.ListItemNhan[n].Anc, 0, 0, ItemSocketOption, it->second.ListItemNhan[n].SKBonus, iTime);
						gLog.Output(LOG_MocNap, "%s - %s: nhận mốc:%d có giá trị là %s", lpObj->Account, lpObj->Name, it->second.IndexMocNap, NumberFormat(it->second.GiaTriNap));

					}
					else
					{
						GDCreateItemSend(lpObj->Index, 0xEB, (BYTE)lpObj->X, (BYTE)lpObj->Y, it->second.ListItemNhan[n].IndexItem, it->second.ListItemNhan[n].LvItem, it->second.ListItemNhan[n].Dur, it->second.ListItemNhan[n].Skill, it->second.ListItemNhan[n].Luck, it->second.ListItemNhan[n].Opt, -1, it->second.ListItemNhan[n].Exc, it->second.ListItemNhan[n].Anc, 0, 0, ItemSocketOption, it->second.ListItemNhan[n].SKBonus, 0);
						gLog.Output(LOG_MocNap, "%s - %s: nhận mốc: %d có giá trị là %s - %s+%d Dur:%d Skill:%d Luck:%d Opt:%d Exc:%d Anc:%d",
							lpObj->Account,
							lpObj->Name,
							it->second.IndexMocNap,
							NumberFormat(it->second.GiaTriNap),
							gItemManager.GetItemName(it->second.ListItemNhan[n].IndexItem),
							it->second.ListItemNhan[n].LvItem,
							it->second.ListItemNhan[n].Dur,
							it->second.ListItemNhan[n].Skill,
							it->second.ListItemNhan[n].Luck,
							it->second.ListItemNhan[n].Opt,
							it->second.ListItemNhan[n].Exc,
							it->second.ListItemNhan[n].Anc);
					}
				}
			}
		}
	UserSendClientInfo(aIndex);//Update lai thong tin list

}
#endif