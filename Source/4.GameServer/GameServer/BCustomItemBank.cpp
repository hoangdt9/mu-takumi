#include "StdAfx.h"
#include "BCustomItemBank.h"
#include "user.h"
#include "Message.h"
#include "DSProtocol.h"
#include "Notice.h"
#include "ItemManager.h"
#include "DSProtocol.h"
#include "GameMain.h"
#include "Util.h"
#include "Log.h"
#include "ItemLevel.h"
#include "ItemStack.h"
#include "BCustomChoTroi.h"
#include "Map.h"
#include "Viewport.h"
#include "MapManager.h"

DWORD CacheAutoPick = 0;
#if (JEWELBANKVER2)
BCustomItemBank gBCustomItemBank;

BCustomItemBank::BCustomItemBank()
{
	this->Enable = 0;
}


BCustomItemBank::~BCustomItemBank()
{
}
void BCustomItemBank::ReloadInfoChar()
{
	if (!this->Enable)
	{
		return;
	}
	for (int n = OBJECT_START_USER; n < MAX_OBJECT; n++)
	{
		if (gObj[n].Type != OBJECT_USER)
		{
			continue;
		}

		if (gObjIsConnected(n) == false)
		{
			continue;
		}
		if (gObj[n].IsBot >= 1 || gObj[n].m_OfflineMode != 0 || gObj[n].IsFakeOnline != 0)
		{
			continue;
		}
		this->GSSendDSSaveDataUser(n);
		this->GSSendDSGetData(n);
	}
}
void BCustomItemBank::Load(char* FilePath)
{
	this->Enable = 0;
	this->mListItemBank.clear();


	pugi::xml_document file;
	pugi::xml_parse_result res = file.load_file(FilePath);
	if (res.status != pugi::status_ok) {
		ErrorMessageBox("File %s load fail. Error: %s", FilePath, res.description());
		return;
	}
	pugi::xml_node oCustomItemBank = file.child("CustomItemBank");
	this->Enable = oCustomItemBank.attribute("Enable").as_int();
	//= Mess Load
	pugi::xml_node Message = oCustomItemBank.child("Message");
	this->m_MessageInfoBP.clear();
	for (pugi::xml_node msg = Message.child("Msg"); msg; msg = msg.next_sibling())
	{
		MESSAGE_INFO_ITEMBANK info;

		info.Index = msg.attribute("Index").as_int();

		strcpy_s(info.Message, msg.attribute("Text").as_string());

		this->m_MessageInfoBP.insert(std::pair<int, MESSAGE_INFO_ITEMBANK>(info.Index, info));
	}

	pugi::xml_node oListBank = oCustomItemBank.child("ListBank");
	for (pugi::xml_node rCustomItemBank = oListBank.child("Item"); rCustomItemBank; rCustomItemBank = rCustomItemBank.next_sibling())
	{
		DATA_LIST_ITEMBANK info = { 0 };
		info.Active = rCustomItemBank.attribute("Active").as_int();
		info.AutoLoot = rCustomItemBank.attribute("AutoLoot").as_int();
		info.ItemIndex = rCustomItemBank.attribute("ItemIndex").as_int();
		info.ItemLevel = rCustomItemBank.attribute("ItemLevel").as_int();
		info.Cum10Index = rCustomItemBank.attribute("Cum10Index").as_int();
		info.Cum10Level = rCustomItemBank.attribute("Cum10Level").as_int();

		info.Cum20Index = rCustomItemBank.attribute("Cum20Index").as_int();
		info.Cum20Level = rCustomItemBank.attribute("Cum20Level").as_int();

		info.Cum30Index = rCustomItemBank.attribute("Cum30Index").as_int();
		info.Cum30Level = rCustomItemBank.attribute("Cum30Level").as_int();
		info.MaxCountType[0] = rCustomItemBank.attribute("MaxCountType0").as_int();
		info.MaxCountType[1] = rCustomItemBank.attribute("MaxCountType1").as_int();
		info.MaxCountType[2] = rCustomItemBank.attribute("MaxCountType2").as_int();
		info.MaxCountType[3] = rCustomItemBank.attribute("MaxCountType3").as_int();
		this->mListItemBank.push_back(info);
	}
	this->ReloadInfoChar();
	LogAdd(LOG_BLUE, "[CustomItemBank] (%d) Load %d Item In List", this->Enable, this->mListItemBank.size());
}


void BCustomItemBank::GSSendDSGetData(int aIndex) //Gui Yeu Cau Den DS de Lay List SQL
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
	if (!this->Enable)
	{
		return;
	}
	BYTE send[4096];
	GSSENDDS_COUNT_ITEMBANK pMsg;
	pMsg.header.set(0xD9, 0x18, 0);
	int size = sizeof(pMsg);
	memcpy(pMsg.AccountID, gObj[aIndex].Account, sizeof(pMsg.AccountID));
	pMsg.aIndex = aIndex;
	pMsg.Count = 0;

	USER_BANK_DATA info;

	for (std::vector<DATA_LIST_ITEMBANK>::iterator it = this->mListItemBank.begin(); it != this->mListItemBank.end(); it++)
	{
		if (it == this->mListItemBank.end())
		{
			break;
		}
		if (it->Active == 0) continue;
		info.ItemIndex = it->ItemIndex;
		info.ItemLevel = it->ItemLevel;
		info.ItemCount = 0;

		pMsg.Count++;
		memcpy(&send[size], &info, sizeof(info));
		size += sizeof(info);

	}

	pMsg.header.size[0] = SET_NUMBERHB(size);
	pMsg.header.size[1] = SET_NUMBERLB(size);
	// ---
	memcpy(send, &pMsg, sizeof(pMsg));

	gDataServerConnection.DataSend(send, size);
}
#if (SENDJEWELALL)
int BCustomItemBank::CheckInfoListItemBank2(int ItemIndex, int ItemLevel)
{
	int Count = 0;
	for (std::vector<DATA_LIST_ITEMBANK>::iterator it = this->mListItemBank.begin(); it != this->mListItemBank.end(); it++)
	{
		if (it == this->mListItemBank.end())
		{
			break;
		}
		if (it->Active == 0) continue;

		if (it->ItemIndex == ItemIndex && it->ItemLevel == ItemLevel)
		{
			return Count;
		}

		if (it->Cum10Index == ItemIndex && it->Cum10Level == ItemLevel)
		{
			return Count;
		}

		if (it->Cum20Index == ItemIndex && it->Cum20Level == ItemLevel)
		{
			return Count;
		}

		if (it->Cum30Index == ItemIndex && it->Cum30Level == ItemLevel)
		{
			return Count;
		}

		Count++;
	}
	return -1;
}

void BCustomItemBank::JewelBankRecv(PSBMSG_JEWELBANK_RECV1 * lpMsg, int aIndex)
{
	LPOBJ lpObj = &gObj[aIndex];

	int Slot = lpMsg->slot;

	if (gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	if (lpObj->Interface.use != 0)
	{
		return;
	}

	if (lpObj->ChaosLock != 0)
	{
		return;
	}

	if (INVENTORY_FULL_RANGE(Slot) == 0)
	{
		return;
	}

	if (lpObj->Inventory[Slot].IsItem() == 0)
	{
		return;
	}

	int SL1 = BCustomItemBank::GetInventoryItemCountBank(lpObj, lpObj->Inventory[Slot].m_Index, lpObj->Inventory[Slot].m_Level);
	int SL10 = 0;
	int SL20 = 0;
	int SL30 = 0;
	int mSlot = this->CheckInfoListItemBank2(lpObj->Inventory[Slot].m_Index, lpObj->Inventory[Slot].m_Level);

	if (BCustomItemBank::CongTruBank(aIndex, lpObj->Inventory[Slot].m_Index, lpObj->Inventory[Slot].m_Level, +(SL1)))
	{
		BCustomItemBank::DeleteInventoryItemCountBank(lpObj, lpObj->Inventory[Slot].m_Index, lpObj->Inventory[Slot].m_Level, SL1);
	}

	//gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, "SL10 %d ~ %d ~ %d", SL10, this->mListItemBank[mSlot].Cum10Index, mSlot);

	if (mSlot == -1) return;

	if (this->mListItemBank[mSlot].Cum10Index != -1) //Neu Co Type Cum 10
	{
		SL10 = BCustomItemBank::GetInventoryItemCountBank(lpObj, this->mListItemBank[mSlot].Cum10Index, this->mListItemBank[mSlot].Cum10Level);



		if (SL10 > 0)
		{
			if (BCustomItemBank::CongTruBank(aIndex, gObj[aIndex].BankJewelData[mSlot].ItemIndex, gObj[aIndex].BankJewelData[mSlot].ItemLevel, +(SL10 * 10)))
			{
				BCustomItemBank::DeleteInventoryItemCountBank(lpObj, this->mListItemBank[mSlot].Cum10Index, this->mListItemBank[mSlot].Cum10Level, SL10);
			}
		}
	}
	if (this->mListItemBank[mSlot].Cum20Index != -1) //Neu Co Type Cum 20
	{
		SL20 = BCustomItemBank::GetInventoryItemCountBank(lpObj, this->mListItemBank[mSlot].Cum20Index, this->mListItemBank[mSlot].Cum20Level);
		if (SL20 > 0)
		{
			if (BCustomItemBank::CongTruBank(aIndex, gObj[aIndex].BankJewelData[mSlot].ItemIndex, gObj[aIndex].BankJewelData[mSlot].ItemLevel, +(SL20 * 20)))
			{
				BCustomItemBank::DeleteInventoryItemCountBank(lpObj, this->mListItemBank[mSlot].Cum20Index, this->mListItemBank[mSlot].Cum20Level, SL20);
			}
		}
	}
	if (this->mListItemBank[mSlot].Cum30Index != -1) //Neu Co Type Cum 20
	{
		SL30 = BCustomItemBank::GetInventoryItemCountBank(lpObj, this->mListItemBank[mSlot].Cum30Index, this->mListItemBank[mSlot].Cum30Level);
		if (SL30 > 0)
		{
			if (BCustomItemBank::CongTruBank(aIndex, gObj[aIndex].BankJewelData[mSlot].ItemIndex, gObj[aIndex].BankJewelData[mSlot].ItemLevel, +(SL30 * 30)))
			{
				BCustomItemBank::DeleteInventoryItemCountBank(lpObj, this->mListItemBank[mSlot].Cum30Index, this->mListItemBank[mSlot].Cum30Level, SL30);
			}
		}
	}


	if (SL1 < 1 && SL10 < 1 && SL20 < 1 && SL30 < 1)
	{
		gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, this->GetMessage(1), gItemLevel.GetItemName(lpObj->Inventory[Slot].m_Index, lpObj->Inventory[Slot].m_Level));
		return;
	}


}
#endif
int BCustomItemBank::CheckInfoListItemBank(int ItemIndex, int ItemLevel)
{
	int Count = 0;
	for (std::vector<DATA_LIST_ITEMBANK>::iterator it = this->mListItemBank.begin(); it != this->mListItemBank.end(); it++)
	{
		if (it == this->mListItemBank.end())
		{
			break;
		}
		if (it->Active == 0) continue;

		if (it->ItemIndex == ItemIndex && it->ItemLevel == ItemLevel)
		{
			return Count;
		}
		Count++;
	}
	return -1;
}




void BCustomItemBank::DSSetInfoUserBankItem(GSSENDDS_COUNT_ITEMBANK * lpMsg) //Nhan KQ tu DS va Cache Info 0xD9, 0x18
{
	int aIndex = lpMsg->aIndex;

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
	if (!this->Enable)
	{
		return;
	}
	for (int n = 0; n < lpMsg->Count; n++)
	{
		USER_BANK_DATA* lpInfo = (USER_BANK_DATA*)(((BYTE*)lpMsg) + sizeof(GSSENDDS_COUNT_ITEMBANK) + (sizeof(USER_BANK_DATA) * n));

		if (n > 100) break;
		//BANKDATAXML GetInfoBankDB;
		if (this->CheckInfoListItemBank(lpInfo->ItemIndex, lpInfo->ItemLevel) != -1)
		{
			gObj[aIndex].BankJewelData[n].ItemIndex = lpInfo->ItemIndex;
			gObj[aIndex].BankJewelData[n].ItemLevel = lpInfo->ItemLevel;
			gObj[aIndex].BankJewelData[n].ItemCount = lpInfo->ItemCount;
			gObj[aIndex].BankJewelData[n].AutoPick = lpInfo->AutoPick;
			//LogAdd(LOG_BLUE, "SetInfo Bank (%d) Item (%d,%d) COunt [%d]", n, gObj[aIndex].BankJewelData[n].ItemIndex, gObj[aIndex].BankJewelData[n].ItemLevel, gObj[aIndex].BankJewelData[n].ItemCount);
		}
	}
#if (CUSTOM_BCHOTROI)
	//=== Get Item Da Ban Coin
	gBCustomChoTroi.GDReqItemStatus(aIndex);
#endif
#if (DAUGIA)
	gDauGia.NhanLaiItemCoin(aIndex);
#endif

	this->UserSendClientInfo(aIndex);
}

void BCustomItemBank::GSSendDSSaveDataUser(int aIndex) //Gui Yeu Cau Den DS de save
{


	if (gObj[aIndex].Type != OBJECT_USER)
	{
		return;
	}

	if (gObj[aIndex].Map == 107 || gObj[aIndex].Map == 109)
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
	if (!this->Enable)
	{
		return;
	}

	if (gObj[aIndex].CloseCount != -1 && gObj[aIndex].CountSaveData++ > 5)
	{
		return;
	}
	BYTE send[4096];

	GSSENDDS_COUNT_ITEMBANK pMsg;
	pMsg.header.set(0xD9, 0x19, 0);
	int size = sizeof(pMsg);
	memcpy(pMsg.AccountID, gObj[aIndex].Account, sizeof(pMsg.AccountID));
	pMsg.aIndex = aIndex;
	pMsg.Count = 0;

	USER_BANK_DATA info;
	gLog.Output(LOG_BANK_JEWEL, "================LogSaveBank=====[%s][%s]================", gObj[aIndex].Account, gObj[aIndex].Name);
	for (int n = 0; n < 100; n++)
	{
		if (gObj[aIndex].BankJewelData[n].ItemIndex == -1) continue;
		info.ItemIndex = gObj[aIndex].BankJewelData[n].ItemIndex;
		info.ItemLevel = gObj[aIndex].BankJewelData[n].ItemLevel;
		info.ItemCount = gObj[aIndex].BankJewelData[n].ItemCount;
		info.AutoPick = gObj[aIndex].BankJewelData[n].AutoPick;

		pMsg.Count++;
		memcpy(&send[size], &info, sizeof(info));
		size += sizeof(info);
		gLog.Output(LOG_BANK_JEWEL, "NHN SAVE BANK: %4d Lvl: %2d SL: %5d | %s (AutoPick %d)", info.ItemIndex, info.ItemLevel, info.ItemCount, gItemLevel.GetItemName(info.ItemIndex, info.ItemLevel), info.AutoPick);//[Bank] Ban vua Gui vao %d [%s]

	}

	pMsg.header.size[0] = SET_NUMBERHB(size);
	pMsg.header.size[1] = SET_NUMBERLB(size);
	// ---
	memcpy(send, &pMsg, sizeof(pMsg));

	gDataServerConnection.DataSend(send, size);
}


void BCustomItemBank::UserSendClientInfo(int aIndex) //Send Info Ve Client
{
	if (gObj[aIndex].Type != OBJECT_USER)
	{
		return;
	}

	if (gObj[aIndex].Map == 107 || gObj[aIndex].Map == 109)
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
	if (!this->Enable)
	{
		return;
	}

	BYTE send[4096];
	PMSG_NEW_HEALTH_BAR_SEND pMsg = { 0 };
	// ---
	pMsg.header.set(0xFA, 0x19, 0);

	int size = sizeof(pMsg);

	pMsg.count = 0;

	USER_BANK_DATA info;

	for (int n = 0; n < 100; n++)
	{
		if (gObj[aIndex].BankJewelData[n].ItemIndex == -1) continue;
		if (this->CheckInfoListItemBank(gObj[aIndex].BankJewelData[n].ItemIndex, gObj[aIndex].BankJewelData[n].ItemLevel) == -1) continue;
		info.ItemIndex = gObj[aIndex].BankJewelData[n].ItemIndex;
		info.ItemLevel = gObj[aIndex].BankJewelData[n].ItemLevel;
		info.ItemCount = gObj[aIndex].BankJewelData[n].ItemCount;
		info.AutoPick = gObj[aIndex].BankJewelData[n].AutoPick;
		pMsg.count++;
		memcpy(&send[size], &info, sizeof(info));
		size += sizeof(info);

	}

	pMsg.header.size[0] = SET_NUMBERHB(size);
	pMsg.header.size[1] = SET_NUMBERLB(size);
	// ---
	memcpy(send, &pMsg, sizeof(pMsg));

	DataSend(aIndex, send, size);
	//	LogAdd(LOG_RED, "SEnd Info Item Bank ve Client !!");

}
char* BCustomItemBank::GetMessage(int index) // OK
{
	std::map<int, MESSAGE_INFO_ITEMBANK>::iterator it = this->m_MessageInfoBP.find(index);

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
bool BCustomItemBank::CheckItemBankAutoPick(int aIndex, int ItemIndex, int ItemLevel)
{
	if (gObj[aIndex].Type != OBJECT_USER)
	{
		return 0;
	}

	if (gObjIsConnected(aIndex) == false)
	{
		return 0;
	}
	if (gObj[aIndex].IsBot >= 1 || gObj[aIndex].m_OfflineMode != 0 || gObj[aIndex].IsFakeOnline != 0)
	{
		return 0;
	}

	if (!this->Enable)
	{
		return 0;
	}
	for (int n = 0; n < 100; n++)
	{
		if (gObj[aIndex].BankJewelData[n].ItemIndex == -1) continue;
		int mSlot = this->CheckInfoListItemBank(ItemIndex, ItemLevel);
		if (mSlot == -1) continue;
		if (gObj[aIndex].BankJewelData[n].ItemIndex == ItemIndex && gObj[aIndex].BankJewelData[n].ItemLevel == ItemLevel)
		{
			return gObj[aIndex].BankJewelData[n].AutoPick;
		}
	}
	return 0;
}
int BCustomItemBank::CheckCountItemBank(int aIndex, int ItemIndex, int ItemLevel)
{
	if (gObj[aIndex].Type != OBJECT_USER)
	{
		return 0;
	}

	if (gObjIsConnected(aIndex) == false)
	{
		return 0;
	}
	if (gObj[aIndex].IsBot >= 1 || gObj[aIndex].m_OfflineMode != 0 || gObj[aIndex].IsFakeOnline != 0)
	{
		return 0;
	}

	if (!this->Enable)
	{
		return 0;
	}
	for (int n = 0; n < 100; n++)
	{
		if (gObj[aIndex].BankJewelData[n].ItemIndex == -1) continue;
		int mSlot = this->CheckInfoListItemBank(ItemIndex, ItemLevel);
		if (mSlot == -1) continue;
		if (gObj[aIndex].BankJewelData[n].ItemIndex == ItemIndex && gObj[aIndex].BankJewelData[n].ItemLevel == ItemLevel)
		{
			return gObj[aIndex].BankJewelData[n].ItemCount;
		}
	}
	return 0;
}
//=======================================
int BCustomItemBank::GetInventoryItemCountBank(LPOBJ lpObj, int index, int level) // OK
{
	int count = 0;

	int MaxValue = gItemManager.GetInventoryMaxValue(lpObj);

	for (int n = 0; n < MaxValue; n++)
	{
		if (lpObj->Inventory[n].IsItem() != 0 && gItemManager.CheckItemMoveToTrade(lpObj, &lpObj->Inventory[n], 0) == 1)
		{
			if (lpObj->Inventory[n].m_Index == index && (level == -1 || lpObj->Inventory[n].m_Level == level))
			{
				if (gItemStack.GetItemMaxStack(lpObj->Inventory[n].m_Index) == 0)
				{
					count++;
				}
				else
				{
					count += (int)lpObj->Inventory[n].m_Durability;
				}
			}
		}
	}

	return count;
}

void BCustomItemBank::DeleteInventoryItemCountBank(LPOBJ lpObj, int index, int level, int count) // OK
{
	if (count == 0)
	{
		return;
	}
	int MaxValue = gItemManager.GetInventoryMaxValue(lpObj);

	for (int n = 0; n < MaxValue, count > 0; n++)
	{
		if (lpObj->Inventory[n].IsItem() != 0 && gItemManager.CheckItemMoveToTrade(lpObj, &lpObj->Inventory[n], 0) == 1)
		{
			if (lpObj->Inventory[n].m_Index == index && lpObj->Inventory[n].m_Level == level)
			{
				if (gItemStack.GetItemMaxStack(lpObj->Inventory[n].m_Index) <= 0)
				{
					gItemManager.InventoryDelItem(lpObj->Index, n);
					gItemManager.GCItemDeleteSend(lpObj->Index, n, 1);
					count--;
				}
				else
				{
					if (lpObj->Inventory[n].m_Durability > count)
					{
						lpObj->Inventory[n].m_Durability -= count;
						gItemManager.GCItemDurSend(lpObj->Index, n, (BYTE)lpObj->Inventory[n].m_Durability, 0);
						count = 0;
					}
					else
					{
						count -= (int)lpObj->Inventory[n].m_Durability;
						gItemManager.InventoryDelItem(lpObj->Index, n);
						gItemManager.GCItemDeleteSend(lpObj->Index, n, 1);
					}
				}
			}
		}
	}
}
bool BCustomItemBank::ClientSendRutOrGui(REQUEST_RUTGUIBANK * lpMsg, int aIndex)
{
	if (gObj[aIndex].Type != OBJECT_USER)
	{
		return false;
	}

	if (gObj[aIndex].Map == 107 || gObj[aIndex].Map == 109)
	{
		return 0;
	}

	if (gObjIsConnected(aIndex) == false)
	{
		return false;
	}
	if (gObj[aIndex].IsBot >= 1 || gObj[aIndex].m_OfflineMode != 0 || gObj[aIndex].IsFakeOnline != 0)
	{
		return false;
	}
	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->Interface.type != INTERFACE_NONE || lpObj->Interface.use != 0 || lpObj->Transaction == 1)
	{
		return false;
	}

	if (gItemManager.ChaosBoxHasItem(lpObj) || gItemManager.TradeHasItem(lpObj))
	{
		return false;
	}
	int TypeRQ = lpMsg->TypeSend;
	int n = lpMsg->IndexList;
	switch (TypeRQ)
	{
	case eAutoPick:
	{
		gObj[aIndex].BankJewelData[n].AutoPick ^= 1;
		BCustomItemBank::UserSendClientInfo(aIndex);
	}
	break;
	case eRut1:
	{
		int MaxCount = gItemManager.GetInventoryItemSpace(lpObj, gObj[aIndex].BankJewelData[n].ItemIndex); //Check O Trong KHa Dung
		if (MaxCount < 1)
		{
			gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(6));
			return 0;
		}
		if (BCustomItemBank::CongTruBank(aIndex, gObj[aIndex].BankJewelData[n].ItemIndex, gObj[aIndex].BankJewelData[n].ItemLevel, -(1)))
		{
			GDCreateItemSend(aIndex, 235, lpObj->X, lpObj->Y, gObj[aIndex].BankJewelData[n].ItemIndex, gObj[aIndex].BankJewelData[n].ItemLevel, 0, 0, 0, 0, aIndex, 0, 0, 0, 0, 0, 0, 0);
		}
	}
	break;
	case eRut10:
	{
		int mSlot = this->CheckInfoListItemBank(gObj[aIndex].BankJewelData[n].ItemIndex, gObj[aIndex].BankJewelData[n].ItemLevel);
		int MaxCount = gItemManager.GetInventoryItemSpace(lpObj, gObj[aIndex].BankJewelData[n].ItemIndex); //Check O Trong KHa Dung
		if (mSlot == -1) return 0;

		if (this->mListItemBank[mSlot].Cum10Index != -1) //Neu Co Type Cum 10
		{
			if (MaxCount < 1)
			{
				gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(6));
				return 0;
			}
			if (BCustomItemBank::CongTruBank(aIndex, gObj[aIndex].BankJewelData[n].ItemIndex, gObj[aIndex].BankJewelData[n].ItemLevel, -(10))) //-10 ngoc
			{
				GDCreateItemSend(aIndex, 235, lpObj->X, lpObj->Y, this->mListItemBank[mSlot].Cum10Index, this->mListItemBank[mSlot].Cum10Level, 0, 0, 0, 0, aIndex, 0, 0, 0, 0, 0, 0, 0);
			}
		}
		else
		{
			if (MaxCount < 10)
			{
				gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(6));
				return 0;
			}
			if (BCustomItemBank::CongTruBank(aIndex, gObj[aIndex].BankJewelData[n].ItemIndex, gObj[aIndex].BankJewelData[n].ItemLevel, -(10))) //-10 ngoc
			{
				for (int i = 0; i < 10; i++)
				{
					GDCreateItemSend(aIndex, 235, lpObj->X, lpObj->Y, gObj[aIndex].BankJewelData[n].ItemIndex, gObj[aIndex].BankJewelData[n].ItemLevel, 0, 0, 0, 0, aIndex, 0, 0, 0, 0, 0, 0, 0);
				}
			}
		}
	}
	break;
	case eRut20:
	{
		int mSlot = this->CheckInfoListItemBank(gObj[aIndex].BankJewelData[n].ItemIndex, gObj[aIndex].BankJewelData[n].ItemLevel);
		int MaxCount = gItemManager.GetInventoryItemSpace(lpObj, gObj[aIndex].BankJewelData[n].ItemIndex); //Check O Trong KHa Dung
		if (mSlot == -1) return 0;

		if (this->mListItemBank[mSlot].Cum20Index != -1) //Neu Co Type Cum 20
		{
			if (MaxCount < 1)
			{
				gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(6));
				return 0;
			}
			if (BCustomItemBank::CongTruBank(aIndex, gObj[aIndex].BankJewelData[n].ItemIndex, gObj[aIndex].BankJewelData[n].ItemLevel, -(20))) //-20 ngoc
			{
				GDCreateItemSend(aIndex, 235, lpObj->X, lpObj->Y, this->mListItemBank[mSlot].Cum20Index, this->mListItemBank[mSlot].Cum20Level, 0, 0, 0, 0, aIndex, 0, 0, 0, 0, 0, 0, 0);
			}
		}
		else
		{
			if (MaxCount < 20)
			{
				gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(6));
				return 0;
			}
			if (BCustomItemBank::CongTruBank(aIndex, gObj[aIndex].BankJewelData[n].ItemIndex, gObj[aIndex].BankJewelData[n].ItemLevel, -(20))) //-20 ngoc
			{
				for (int i = 0; i < 20; i++)
				{
					GDCreateItemSend(aIndex, 235, lpObj->X, lpObj->Y, gObj[aIndex].BankJewelData[n].ItemIndex, gObj[aIndex].BankJewelData[n].ItemLevel, 0, 0, 0, 0, aIndex, 0, 0, 0, 0, 0, 0, 0);
				}
			}
		}
	}
	break;
	case eRut30:
	{
		int mSlot = this->CheckInfoListItemBank(gObj[aIndex].BankJewelData[n].ItemIndex, gObj[aIndex].BankJewelData[n].ItemLevel);
		int MaxCount = gItemManager.GetInventoryItemSpace(lpObj, gObj[aIndex].BankJewelData[n].ItemIndex); //Check O Trong KHa Dung
		if (mSlot == -1) return 0;

		if (this->mListItemBank[mSlot].Cum30Index != -1) //Neu Co Type Cum 10
		{
			if (MaxCount < 1)
			{
				gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(6));
				return 0;
			}
			if (BCustomItemBank::CongTruBank(aIndex, gObj[aIndex].BankJewelData[n].ItemIndex, gObj[aIndex].BankJewelData[n].ItemLevel, -(30))) //-10 ngoc
			{
				GDCreateItemSend(aIndex, 235, lpObj->X, lpObj->Y, this->mListItemBank[mSlot].Cum30Index, this->mListItemBank[mSlot].Cum30Level, 0, 0, 0, 0, aIndex, 0, 0, 0, 0, 0, 0, 0);
			}
		}
		else
		{
			if (MaxCount < 10)
			{
				gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(6));
				return 0;
			}
			if (BCustomItemBank::CongTruBank(aIndex, gObj[aIndex].BankJewelData[n].ItemIndex, gObj[aIndex].BankJewelData[n].ItemLevel, -(30))) //-10 ngoc
			{
				for (int i = 0; i < 30; i++)
				{
					GDCreateItemSend(aIndex, 235, lpObj->X, lpObj->Y, gObj[aIndex].BankJewelData[n].ItemIndex, gObj[aIndex].BankJewelData[n].ItemLevel, 0, 0, 0, 0, aIndex, 0, 0, 0, 0, 0, 0, 0);
				}
			}
		}
	}
	break;
	case eRutAll:
	{
		//int MaxCount = gItemManager.GetInventoryItemSpace(lpObj, gObj[aIndex].BankJewelData[n].ItemIndex); //Check O Trong KHa Dung
		int MaxCount = 10;

		if (MaxCount > gObj[aIndex].BankJewelData[n].ItemCount)
		{
			MaxCount = gObj[aIndex].BankJewelData[n].ItemCount;
		}
		if (MaxCount < 1)
		{
			gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(6));
			return 0;
		}
		if (BCustomItemBank::CongTruBank(aIndex, gObj[aIndex].BankJewelData[n].ItemIndex, gObj[aIndex].BankJewelData[n].ItemLevel, -(MaxCount)))
		{
			for (int i = 0; i < MaxCount; i++)
			{
				GDCreateItemSend(aIndex, 235, lpObj->X, lpObj->Y, gObj[aIndex].BankJewelData[n].ItemIndex, gObj[aIndex].BankJewelData[n].ItemLevel, 0, 0, 0, 0, aIndex, 0, 0, 0, 0, 0, 0, 0);
			}
		}
	}
	break;
	case eGuiAll:
	{
		int SL1 = BCustomItemBank::GetInventoryItemCountBank(lpObj, gObj[aIndex].BankJewelData[n].ItemIndex, gObj[aIndex].BankJewelData[n].ItemLevel);
		int SL10 = 0;
		int SL20 = 0;
		int SL30 = 0;
		if (BCustomItemBank::CongTruBank(aIndex, gObj[aIndex].BankJewelData[n].ItemIndex, gObj[aIndex].BankJewelData[n].ItemLevel, +(SL1)))
		{
			BCustomItemBank::DeleteInventoryItemCountBank(lpObj, gObj[aIndex].BankJewelData[n].ItemIndex, gObj[aIndex].BankJewelData[n].ItemLevel, SL1);
		}
		int mSlot = this->CheckInfoListItemBank(gObj[aIndex].BankJewelData[n].ItemIndex, gObj[aIndex].BankJewelData[n].ItemLevel);
		if (mSlot == -1) return 0;

		if (this->mListItemBank[mSlot].Cum10Index != -1) //Neu Co Type Cum 10
		{
			SL10 = BCustomItemBank::GetInventoryItemCountBank(lpObj, this->mListItemBank[mSlot].Cum10Index, this->mListItemBank[mSlot].Cum10Level);
			if (SL10 > 0)
			{
				if (BCustomItemBank::CongTruBank(aIndex, gObj[aIndex].BankJewelData[n].ItemIndex, gObj[aIndex].BankJewelData[n].ItemLevel, +(SL10 * 10)))
				{
					BCustomItemBank::DeleteInventoryItemCountBank(lpObj, this->mListItemBank[mSlot].Cum10Index, this->mListItemBank[mSlot].Cum10Level, SL10);
				}
			}
		}
		if (this->mListItemBank[mSlot].Cum20Index != -1) //Neu Co Type Cum 20
		{
			SL20 = BCustomItemBank::GetInventoryItemCountBank(lpObj, this->mListItemBank[mSlot].Cum20Index, this->mListItemBank[mSlot].Cum20Level);
			if (SL20 > 0)
			{
				if (BCustomItemBank::CongTruBank(aIndex, gObj[aIndex].BankJewelData[n].ItemIndex, gObj[aIndex].BankJewelData[n].ItemLevel, +(SL20 * 20)))
				{
					BCustomItemBank::DeleteInventoryItemCountBank(lpObj, this->mListItemBank[mSlot].Cum20Index, this->mListItemBank[mSlot].Cum20Level, SL20);
				}
			}
		}
		if (this->mListItemBank[mSlot].Cum30Index != -1) //Neu Co Type Cum 20
		{
			SL30 = BCustomItemBank::GetInventoryItemCountBank(lpObj, this->mListItemBank[mSlot].Cum30Index, this->mListItemBank[mSlot].Cum30Level);
			if (SL30 > 0)
			{
				if (BCustomItemBank::CongTruBank(aIndex, gObj[aIndex].BankJewelData[n].ItemIndex, gObj[aIndex].BankJewelData[n].ItemLevel, +(SL30 * 30)))
				{
					BCustomItemBank::DeleteInventoryItemCountBank(lpObj, this->mListItemBank[mSlot].Cum30Index, this->mListItemBank[mSlot].Cum30Level, SL30);
				}
			}
		}
		if (SL1 < 1 && SL10 < 1 && SL20 < 1 && SL30 < 1)
		{
			gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(1), gItemLevel.GetItemName(gObj[aIndex].BankJewelData[n].ItemIndex, gObj[aIndex].BankJewelData[n].ItemLevel));
			return 0;
		}
	}
	break;

	default:
		break;
	}

	return 1;
}
bool BCustomItemBank::CongTruBank(int aIndex, int ItemIndex, int ItemLevel, int Vaule, bool Notice)
{
	LPOBJ lpObj = &gObj[aIndex];

	if (gObj[aIndex].Type != OBJECT_USER || Vaule == 0)
	{
		return 0;
	}

	if (gObj[aIndex].Map == 107 || gObj[aIndex].Map == 109)
	{
		return 0;
	}

	if (gObjIsConnected(aIndex) == false)
	{
		return 0;
	}
	if (gObj[aIndex].IsBot >= 1 || gObj[aIndex].m_OfflineMode != 0 || gObj[aIndex].IsFakeOnline != 0)
	{
		return 0;
	}

	if (!this->Enable)
	{
		return 0;
	}

	for (int n = 0; n < 100; n++)
	{
		if (gObj[aIndex].BankJewelData[n].ItemIndex == -1) continue;
		int mSlot = this->CheckInfoListItemBank(ItemIndex, ItemLevel);
		if (mSlot == -1) continue;
		if (gObj[aIndex].BankJewelData[n].ItemIndex == ItemIndex && gObj[aIndex].BankJewelData[n].ItemLevel == ItemLevel)
		{
			if (Vaule > 0)
			{
				if ((Vaule + gObj[aIndex].BankJewelData[n].ItemCount) > this->mListItemBank[mSlot].MaxCountType[gObj[aIndex].AccountLevel])
				{
					gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(3), gItemLevel.GetItemName(this->mListItemBank[mSlot].ItemIndex, this->mListItemBank[mSlot].ItemLevel), this->mListItemBank[mSlot].MaxCountType[gObj[aIndex].AccountLevel]);
					return 0;
				}
				gNotice.GCNoticeSend(aIndex, (Notice) ? eMessageBox : 1, 0, 0, 0, 0, 0, this->GetMessage(4), Vaule, gItemLevel.GetItemName(ItemIndex, ItemLevel));
				gObj[aIndex].BankJewelData[n].ItemCount += Vaule;
				gLog.Output(LOG_BANK_JEWEL, "[BankJewel][MAP: %s] [%s][%s] - Add [+%d] [%s] /[Total: %d]", gMapManager.GetMapName(lpObj->Map), lpObj->Account, lpObj->Name, Vaule, gItemLevel.GetItemName(ItemIndex, ItemLevel), gObj[aIndex].BankJewelData[n].ItemCount);
			}

			if (Vaule < 0) //--
			{
				if ((gObj[aIndex].BankJewelData[n].ItemCount + Vaule) < 0)
				{
					gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(2));
					return 0;
				}
				gObj[aIndex].BankJewelData[n].ItemCount += Vaule;
				gNotice.GCNoticeSend(aIndex, (Notice) ? eMessageBox : 1, 0, 0, 0, 0, 0, this->GetMessage(5), Vaule, gItemLevel.GetItemName(ItemIndex, ItemLevel));
				gLog.Output(LOG_BANK_JEWEL, "[BankJewel][MAP: %s] [%s][%s] - Delete [%d] [%s] /[Total: %d]", gMapManager.GetMapName(lpObj->Map), lpObj->Account, lpObj->Name, Vaule, gItemLevel.GetItemName(ItemIndex, ItemLevel), gObj[aIndex].BankJewelData[n].ItemCount);

			}

			this->UserSendClientInfo(aIndex);
			return 1;
		}
	}

	return 0;
}

int BCustomItemBank::GetSlotIsItem(int ItemIndex, int ItemLevel)
{
	int GetSlot = 0;
	for (std::vector<DATA_LIST_ITEMBANK>::iterator it = this->mListItemBank.begin(); it != this->mListItemBank.end(); it++)
	{
		if (it == this->mListItemBank.end())
		{
			break;
		}
		if (it->Active == 0) continue;

		if (it->ItemIndex != -1 && it->ItemIndex == ItemIndex && it->ItemLevel == ItemLevel ||
			it->Cum10Index != -1 && it->Cum10Index == ItemIndex && it->Cum10Level == ItemLevel ||
			it->Cum10Index != -1 && it->Cum20Index == ItemIndex && it->Cum20Level == ItemLevel ||
			it->Cum10Index != -1 && it->Cum30Index == ItemIndex && it->Cum30Level == ItemLevel)
		{
			return GetSlot;
		}
		GetSlot++;
	}
	return -1;
}
int BCustomItemBank::GetCountItemSlot(int mSlot, int ItemIndex, int ItemLevel)
{
	int GetCount = 0;
	for (std::vector<DATA_LIST_ITEMBANK>::iterator it = this->mListItemBank.begin(); it != this->mListItemBank.end(); it++)
	{
		if (it == this->mListItemBank.end())
		{
			break;
		}
		if (it->Active == 0) continue;

		if (it->ItemIndex == ItemIndex && it->ItemLevel == ItemLevel)
		{
			GetCount += 1;
		}
		if (it->Cum10Index == ItemIndex && it->Cum10Level == ItemLevel)
		{
			GetCount += 10;
		}
		if (it->Cum20Index == ItemIndex && it->Cum20Level == ItemLevel)
		{
			GetCount += 20;
		}
		if (it->Cum30Index == ItemIndex && it->Cum30Level == ItemLevel)
		{
			GetCount += 30;
		}
	}
	return GetCount;
}
bool BCustomItemBank::CongBankItemSlot(int aIndex, int ItemIndex, int ItemLevel, bool Notice)
{
	if (gObj[aIndex].Type != OBJECT_USER)
	{
		return 0;
	}

	if (gObj[aIndex].Map == 107 || gObj[aIndex].Map == 109)
	{
		return 0;
	}

	if (gObjIsConnected(aIndex) == false)
	{
		return 0;
	}
	if (gObj[aIndex].IsBot >= 1 || gObj[aIndex].m_OfflineMode != 0 || gObj[aIndex].IsFakeOnline != 0)
	{
		return 0;
	}

	if (!this->Enable)
	{
		return 0;
	}

	int GetSlotItemBank = gBCustomItemBank.GetSlotIsItem(ItemIndex, ItemLevel);
	if (GetSlotItemBank != -1)
	{
		int SL = gBCustomItemBank.GetCountItemSlot(GetSlotItemBank, ItemIndex, ItemLevel);
		if (gBCustomItemBank.CongTruBank(aIndex, gBCustomItemBank.mListItemBank[GetSlotItemBank].ItemIndex, gBCustomItemBank.mListItemBank[GetSlotItemBank].ItemLevel, +(SL), Notice))
		{
			return 1;
		}
	}
	

	return 0;
}

bool BCustomItemBank::RutNgocSoLuong(REQUEST_RUTGUIBANK * lpMsg, int aIndex)
{
	if (gObj[aIndex].Type != OBJECT_USER)
	{
		return false;
	}

	if (gObj[aIndex].Map == 107 || gObj[aIndex].Map == 109)
	{
		return 0;
	}

	if (gObjIsConnected(aIndex) == false)
	{
		return false;
	}
	if (gObj[aIndex].IsBot >= 1 || gObj[aIndex].m_OfflineMode != 0 || gObj[aIndex].IsFakeOnline != 0)
	{
		return false;
	}
	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->Interface.type != INTERFACE_NONE || lpObj->Interface.use != 0 || lpObj->Transaction == 1)
	{
		return false;
	}

	if (gItemManager.ChaosBoxHasItem(lpObj) || gItemManager.TradeHasItem(lpObj))
	{
		return false;
	}
	//LogAdd(LOG_RED, "SoLuongNgoc %d /  %d !", lpMsg->TypeSend, lpMsg->IndexList);

	int SoLuongNgoc = lpMsg->TypeSend;
	int n = lpMsg->IndexList;

	int mSlot = this->CheckInfoListItemBank(gObj[aIndex].BankJewelData[n].ItemIndex, gObj[aIndex].BankJewelData[n].ItemLevel);
	int MaxCount = gItemManager.GetInventoryItemSpace(lpObj, gObj[aIndex].BankJewelData[n].ItemIndex); //Check O Trong KHa Dung

	if (SoLuongNgoc < 1)
	{
		gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(6));
		return 0;
	}
	if (SoLuongNgoc > gObj[aIndex].BankJewelData[n].ItemCount)
	{
		SoLuongNgoc = gObj[aIndex].BankJewelData[n].ItemCount;
	}
	if (SoLuongNgoc > MaxCount)
	{
		SoLuongNgoc = MaxCount;
	}

	if (gBCustomItemBank.CongTruBank(aIndex, gObj[aIndex].BankJewelData[n].ItemIndex, gObj[aIndex].BankJewelData[n].ItemLevel, -(SoLuongNgoc)))
	{
		for (int i = 0; i < SoLuongNgoc; i++)
		{
			GDCreateItemSend(aIndex, 235, lpObj->X, lpObj->Y, gObj[aIndex].BankJewelData[n].ItemIndex, gObj[aIndex].BankJewelData[n].ItemLevel, 0, 0, 0, 0, aIndex, 0, 0, 0, 0, 0, 0, 0);
		}
	}

	return 1;
}


void BCustomItemBank::ProcAutoPic()
{
	if (GetTickCount() < CacheAutoPick) return;
	CacheAutoPick = GetTickCount() + 3000;
	for (int n = OBJECT_START_USER; n < MAX_OBJECT; n++)
	{
		if (gObj[n].Type == OBJECT_USER && gObj[n].Connected == OBJECT_ONLINE)
		{
			LPOBJ lpObj = &gObj[n];
			if (gObjIsConnectedGP(lpObj->Index) == 0)
			{
				continue;
			}

			if (lpObj->DieRegen != 0)
			{
				continue;
			}

			if (lpObj->Interface.use != 0 && lpObj->Interface.type != INTERFACE_SHOP)
			{
				continue;
			}

			if (lpObj->Transaction == 1)
			{
				continue;
			}

			if (lpObj->Type != OBJECT_USER)
			{
				continue;
			}
			int index = -1;

			for (int n = 0; n < MAX_MAP_ITEM; n++)
			{
				if (gMap[lpObj->Map].m_Item[n].m_Live == 0)
				{
					continue;
				}

				if (gMap[lpObj->Map].m_Item[n].m_State != OBJECT_CREATE && gMap[lpObj->Map].m_Item[n].m_State != OBJECT_PLAYING)
				{
					continue;
				}

				if (gViewport.CheckViewportObjectPosition(lpObj->Index, lpObj->Map, gMap[lpObj->Map].m_Item[n].m_X, gMap[lpObj->Map].m_Item[n].m_Y, gMapManager.GetMapViewRange(lpObj->Map)) != 0)
				{
					index = n;
				}

				if (index < 0)
				{
					continue;
				}

				if (MAP_ITEM_RANGE(index) == 0)
				{
					continue;
				}

				if (MAP_RANGE(lpObj->Map) == 0)
				{
					continue;
				}

				if (gMap[lpObj->Map].CheckItemGive(lpObj->Index, index) == 0)
				{
					continue;
				}

				CMapItem* lpItem = &gMap[lpObj->Map].m_Item[index];

				if (lpItem->IsEventItem() != 0)
				{
					continue;
				}
				int GetSlotItemBank = gBCustomItemBank.GetSlotIsItem(lpItem->m_Index, lpItem->m_Level);
				if (GetSlotItemBank != -1)
				{
					int SL = gBCustomItemBank.GetCountItemSlot(GetSlotItemBank, lpItem->m_Index, lpItem->m_Level);
					if (gBCustomItemBank.CheckItemBankAutoPick(lpObj->Index, gBCustomItemBank.mListItemBank[GetSlotItemBank].ItemIndex, gBCustomItemBank.mListItemBank[GetSlotItemBank].ItemLevel))
					{
						if (gBCustomItemBank.CongTruBank(lpObj->Index, gBCustomItemBank.mListItemBank[GetSlotItemBank].ItemIndex, gBCustomItemBank.mListItemBank[GetSlotItemBank].ItemLevel, +(SL), FALSE))
						{
							gMap[lpObj->Map].ItemGive(lpObj->Index, index);
							::GCPartyItemInfoSend(lpObj->Index, lpItem);
						}
					}
				}
			}
		}
	}

}
void BCustomItemBank::RecvPacket(int aIndex, PMSG_MOVEITEM* aRecv)
{
	if (OBJMAX_RANGE(aIndex) == 0) { return; }

	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->Interface.use != 0)
	{
		return;
	}
	if (gObj[aIndex].Type != OBJECT_USER || (gObj[aIndex].Connected != OBJECT_ONLINE))
	{
		return;
	}
	if (gObj[aIndex].Interface.use != 0 || gObj[aIndex].Teleport != 0 || gObj[aIndex].DieRegen != 0 || gObj[aIndex].PShopOpen != 0)
	{
		gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, gMessage.GetMessage(659));
		return;
	}

	if (gBCustomItemBank.CongBankItemSlot(aIndex, lpObj->Inventory[aRecv->Source].m_Index, lpObj->Inventory[aRecv->Source].m_Level))
	{
		gItemManager.InventoryDelItem(aIndex, aRecv->Source);
		gItemManager.GCItemDeleteSend(aIndex, aRecv->Source, 1);

	}
	//int SL1 = BCustomItemBank::GetInventoryItemCountBank(lpObj, lpObj->Inventory[aRecv->Source].m_Index, lpObj->Inventory[aRecv->Source].m_Level);

	//if (BCustomItemBank::CongTruBank(aIndex, lpObj->Inventory[aRecv->Source].m_Index, lpObj->Inventory[aRecv->Source].m_Level, +(SL1)))
	//{
	//	BCustomItemBank::DeleteInventoryItemCountBank(lpObj, lpObj->Inventory[aRecv->Source].m_Index, lpObj->Inventory[aRecv->Source].m_Level, SL1);
	//}

	return;
}
#endif