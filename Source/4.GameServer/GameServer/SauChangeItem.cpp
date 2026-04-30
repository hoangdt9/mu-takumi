#include "stdafx.h"
#include "SauChangeItem.h"
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
#include "MonsterSetBase.h"
#include "MapServerManager.h"

#if(SAUDOIITEM)

SauChangeItem gSauChangeItem;

SauChangeItem::SauChangeItem()
{
	this->Enable = 0;
	this->mNPCData = new NPC_DATA_DOIITEM;
}

SauChangeItem::~SauChangeItem()
{

}

void SauChangeItem::SetNPC()
{
	if (gSauChangeItem.Enable)
	{
		MONSTER_SET_BASE_INFO info;
		memset(&info, 0, sizeof(info));

		info.Type = 0;
		info.MonsterClass = gSauChangeItem.mNPCData->NPCClass;
		info.Map = gSauChangeItem.mNPCData->NPCMap;
		info.Dis = 0;
		info.Dir = gSauChangeItem.mNPCData->NPCDir;
		info.X = gSauChangeItem.mNPCData->NPCX;
		info.Y = gSauChangeItem.mNPCData->NPCY;

		gMonsterSetBase.SetInfo(info);
	}
}
void SauChangeItem::LoadConfig(char* FilePath)
{
	this->m_GroupDoiItemData.clear();
	this->Enable = 0;
	this->mNPCData->Clear();

	pugi::xml_document file;
	pugi::xml_parse_result res = file.load_file(FilePath);
	if (res.status != pugi::status_ok) {
		ErrorMessageBox("File %s load fail. Error: %s", FilePath, res.description());
		return;
	}
	pugi::xml_node oDungLuyenItem = file.child("DoiItem");
	this->Enable = oDungLuyenItem.attribute("Enable").as_int();

	//= Mess Load
	pugi::xml_node Message = oDungLuyenItem.child("Message");
	for (pugi::xml_node msg = Message.child("Msg"); msg; msg = msg.next_sibling())
	{
		DOIITEM_MSG info;

		info.Index = msg.attribute("Index").as_int();

		strncpy_s(info.Message, msg.attribute("Text").as_string(), sizeof(info.Message));

		this->m_MessageInfoBP.insert(std::pair<int, DOIITEM_MSG>(info.Index, info));
	}

	//= Config NPC
	pugi::xml_node EventConfigNPC = oDungLuyenItem.child("NPC");
	this->mNPCData->NPCClass = EventConfigNPC.attribute("NPCClass").as_int();
	this->mNPCData->NPCMap = EventConfigNPC.attribute("NPCMap").as_int();
	this->mNPCData->NPCX = EventConfigNPC.attribute("NPCX").as_int();
	this->mNPCData->NPCY = EventConfigNPC.attribute("NPCY").as_int();
	this->mNPCData->NPCDir = EventConfigNPC.attribute("NPCDir").as_int();
	//=== Load Data
	int mIndex = 0;
	for (pugi::xml_node GroupDungLuyen = oDungLuyenItem.child("GroupDoiItem"); GroupDungLuyen; GroupDungLuyen = GroupDungLuyen.next_sibling())
	{
		GROUPDOIITEM_DATA infoGroup;
		infoGroup.Index = mIndex++;
		infoGroup.Notice = GroupDungLuyen.attribute("Notice").as_int();
		strncpy_s(infoGroup.Name, GroupDungLuyen.attribute("Name").as_string(), sizeof(infoGroup.Name));
		infoGroup.Wcoin = GroupDungLuyen.attribute("Wcoin").as_int();

		//Config ItemChinh
		pugi::xml_node ItemChinh = GroupDungLuyen.child("ItemChinh");
		infoGroup.ItemChinh.IndexItem = ItemChinh.attribute("IndexItem").as_int();
		infoGroup.ItemChinh.Level = ItemChinh.attribute("Level").as_int();
		infoGroup.ItemChinh.Skill = ItemChinh.attribute("Skill").as_int();
		infoGroup.ItemChinh.Luck = ItemChinh.attribute("Luck").as_int();
		infoGroup.ItemChinh.Option = ItemChinh.attribute("Option").as_int();
		infoGroup.ItemChinh.Exc = ItemChinh.attribute("Exc").as_int();

		//Config KetQua
		pugi::xml_node ItemKetQua = GroupDungLuyen.child("ItemKetQua");
		infoGroup.ItemKetQua.IndexItem = ItemKetQua.attribute("IndexItem").as_int();
		infoGroup.ItemKetQua.IndexItem1 = ItemKetQua.attribute("IndexItem1").as_int();
		infoGroup.ItemKetQua.IndexItem2 = ItemKetQua.attribute("IndexItem2").as_int();
		infoGroup.ItemKetQua.IndexItem3 = ItemKetQua.attribute("IndexItem3").as_int();
		infoGroup.ItemKetQua.IndexItem4 = ItemKetQua.attribute("IndexItem4").as_int();
		infoGroup.ItemKetQua.IndexItem5 = ItemKetQua.attribute("IndexItem5").as_int();

		this->m_GroupDoiItemData.insert(std::pair<int, GROUPDOIITEM_DATA>(infoGroup.Index, infoGroup));

	}

	LogAdd(LOG_BLUE, "[DoiItem] Load OK Enable = %d, Size %d", this->Enable, this->m_GroupDoiItemData.size());
}


char* SauChangeItem::GetMessage(int index) // OK
{
	std::map<int, DOIITEM_MSG>::iterator it = this->m_MessageInfoBP.find(index);

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
void SauChangeItem::SetStateInterface(int aIndex, int Type)
{

	if (Type == 1)
	{
		gObj[aIndex].Interface.state = 1;
		gObj[aIndex].Interface.use = 1;
		gObj[aIndex].Interface.type = INTERFACE_DOIITEM;
		gObj[aIndex].Transaction = 0;
		gObjInventoryTransaction(aIndex);
	}
	else
	{


		if (gObj[aIndex].SAU_StatusMix > 0) //SAVE Neu Da Mix
		{
			if (gObj[aIndex].SAU_StatusMix == 1)
			{
				gItemManager.InventoryInsertItem(aIndex, gObj[aIndex].SAU_ItemChinh);
			}
			else
			{
				for (int n = 0; n < 6; n++)
				{
					gItemManager.InventoryInsertItem(aIndex, gObj[aIndex].SAU_ItemKetQua[n]);
				}

			}
		}
		else
		{
			gObjInventoryRollback(aIndex);
		}

		//==Clear Cache
		gObj[aIndex].SAU_IndexMix = -1;
		gObj[aIndex].SAU_ItemChinh.Clear();
		for (int n = 0; n < 6; n++)
		{
			gObj[aIndex].SAU_ItemKetQua[n].Clear();
		}

		gObj[aIndex].SAU_StatusMix = -1;
		SendInfoItemCache(aIndex);
		gItemManager.GCItemListSend(aIndex);
		gObjectManager.CharacterMakePreviewCharSet(aIndex);
		gItemManager.GCItemEquipmentSend(aIndex);

		gObj[aIndex].Interface.use = 0;
		gObj[aIndex].Interface.type = INTERFACE_NONE;
		gObj[aIndex].Interface.state = 0;
		gObj[aIndex].TargetNumber = -1;

	}
}



void SauChangeItem::SendInfoItemCache(int aIndex)
{
	if (!OBJMAX_RANGE(aIndex))
	{
		return;
	}


	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->Type != OBJECT_USER || lpObj->m_OfflineMode != 0 || lpObj->IsFakeOnline != 0 || !this->Enable)
	{
		return;

	}

	if (lpObj->Interface.type != INTERFACE_DOIITEM)
	{

		return;
	}

	std::map<int, GROUPDOIITEM_DATA>::iterator it = this->m_GroupDoiItemData.find(lpObj->SAU_IndexMix);

	BYTE send[8192];

	INFOITEM_DOIITEM_CLIENT pMsg;

	pMsg.header.set(0xD3, 0x6B, 0);

	int size = sizeof(pMsg);

	pMsg.ActiveMix = 1;

	if (!lpObj->SAU_ItemChinh.IsItem()
		|| !lpObj->SAU_ItemChinh.IsItem())
		//|| !lpObj->DL_ItemKetQua.IsItem()

	{

		pMsg.ActiveMix = 0;
	}

	gItemManager.ItemByteConvert(pMsg.ItemChinh, lpObj->SAU_ItemChinh); // Set Info Item
	for (int n = 0; n < 6; n++)
	{
		gItemManager.ItemByteConvert(pMsg.ItemKetQua[n], lpObj->SAU_ItemKetQua[n]); // Set Info Item
	}
	pMsg.Rate = it->second.Wcoin;

	pMsg.header.size[0] = SET_NUMBERHB(size);
	pMsg.header.size[1] = SET_NUMBERLB(size);
	// ---
	memcpy(send, &pMsg, sizeof(pMsg));

	DataSend(aIndex, send, size);
}

void SauChangeItem::ProcItemSend(int aIndex, int SlotItem)
{
	if (!OBJMAX_RANGE(aIndex))
	{
		return;
	}


	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->Type != OBJECT_USER || lpObj->m_OfflineMode != 0 || lpObj->IsFakeOnline != 0 || !this->Enable)
	{
		return;

	}

	if (lpObj->Interface.type != INTERFACE_DOIITEM)
	{

		return;
	}

	if (gItemManager.CheckItemMoveToTrade(lpObj, &lpObj->Inventory[SlotItem], 0) == 0)
	{
		gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(1));//
		return;
	}

	if (gItemManager.ChaosBoxHasItem(lpObj) || gItemManager.TradeHasItem(lpObj))
	{
		return;
	}
	if (!lpObj->Inventory[SlotItem].IsItem())
	{
		//Vi tri nay khong co Item do
		gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(1));//
		return;
	}
	if (gObj[aIndex].SAU_StatusMix > 0)
	{
		gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, "Vui Long Dong Dung Luyen Va Mo Lai!!");// That Bai
		return;
	}
	//==Cache Item Chinh
	if (!lpObj->SAU_ItemChinh.IsItem())
	{
		for (std::map<int, GROUPDOIITEM_DATA>::iterator it = this->m_GroupDoiItemData.begin(); it != this->m_GroupDoiItemData.end(); it++)
		{
			if (it == this->m_GroupDoiItemData.end())
			{
				gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(1));//
				return;
			}
			if (lpObj->Inventory[SlotItem].m_Index == it->second.ItemChinh.IndexItem)

			{
				if ((it->second.ItemChinh.Level != -1 && it->second.ItemChinh.Level > lpObj->Inventory[SlotItem].m_Level)
					|| (it->second.ItemChinh.Skill != -1 && it->second.ItemChinh.Skill > lpObj->Inventory[SlotItem].m_Option1)
					|| (it->second.ItemChinh.Luck != -1 && it->second.ItemChinh.Luck > lpObj->Inventory[SlotItem].m_Option2)
					|| (it->second.ItemChinh.Option != -1 && it->second.ItemChinh.Option > lpObj->Inventory[SlotItem].m_Option3)
					|| it->second.ItemChinh.Exc != 0 && !lpObj->Inventory[SlotItem].IsExcItem())
				{
					gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(1));//
					return;
				}


				lpObj->SAU_IndexMix = it->second.Index;
				lpObj->SAU_ItemChinh = lpObj->Inventory[SlotItem];
				//=== Set Item Ket Qua


				lpObj->SAU_ItemKetQua[0] = lpObj->Inventory[SlotItem];
				lpObj->SAU_ItemKetQua[1] = lpObj->Inventory[SlotItem];
				lpObj->SAU_ItemKetQua[2] = lpObj->Inventory[SlotItem];
				lpObj->SAU_ItemKetQua[3] = lpObj->Inventory[SlotItem];
				lpObj->SAU_ItemKetQua[4] = lpObj->Inventory[SlotItem];
				lpObj->SAU_ItemKetQua[5] = lpObj->Inventory[SlotItem];

				lpObj->SAU_ItemKetQua[0].m_Index = it->second.ItemKetQua.IndexItem;
				lpObj->SAU_ItemKetQua[1].m_Index = it->second.ItemKetQua.IndexItem1;
				lpObj->SAU_ItemKetQua[2].m_Index = it->second.ItemKetQua.IndexItem2;
				lpObj->SAU_ItemKetQua[3].m_Index = it->second.ItemKetQua.IndexItem3;
				lpObj->SAU_ItemKetQua[4].m_Index = it->second.ItemKetQua.IndexItem4;
				lpObj->SAU_ItemKetQua[5].m_Index = it->second.ItemKetQua.IndexItem5;

				lpObj->SAU_ItemKetQua[0].m_JewelOfHarmonyOption = 0;
				lpObj->SAU_ItemKetQua[1].m_JewelOfHarmonyOption = 0;
				lpObj->SAU_ItemKetQua[2].m_JewelOfHarmonyOption = 0;
				lpObj->SAU_ItemKetQua[3].m_JewelOfHarmonyOption = 0;
				lpObj->SAU_ItemKetQua[4].m_JewelOfHarmonyOption = 0;
				lpObj->SAU_ItemKetQua[5].m_JewelOfHarmonyOption = 0;

				lpObj->SAU_ItemKetQua[0].m_ItemOptionEx = 0;
				lpObj->SAU_ItemKetQua[1].m_ItemOptionEx = 0;
				lpObj->SAU_ItemKetQua[2].m_ItemOptionEx = 0;
				lpObj->SAU_ItemKetQua[3].m_ItemOptionEx = 0;
				lpObj->SAU_ItemKetQua[4].m_ItemOptionEx = 0;
				lpObj->SAU_ItemKetQua[5].m_ItemOptionEx = 0;

				//=== Send Del item
				gItemManager.InventoryDelItem(aIndex, SlotItem);
				gItemManager.GCItemDeleteSend(aIndex, SlotItem, 1);
				this->SendInfoItemCache(aIndex);
				//====
				return;

			}
		}

	}
}
void SauChangeItem::BackItem(int aIndex, int BackSlot)
{
	if (!OBJMAX_RANGE(aIndex))
	{
		return;
	}


	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->Type != OBJECT_USER || lpObj->m_OfflineMode != 0 || lpObj->IsFakeOnline != 0 || !this->Enable)
	{
		return;

	}

	std::map<int, GROUPDOIITEM_DATA>::iterator it = this->m_GroupDoiItemData.find(lpObj->SAU_IndexMix);

	if (lpObj->Interface.type != INTERFACE_DOIITEM)
	{

		return;
	}
	if (BackSlot >= 1 && lpObj->Coin1 < it->second.Wcoin)
	{
		gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(2));//
		return;
	}


	BYTE SlotRecv = 0xFF;
	switch (BackSlot)
	{
	case 0: //Item Chinh
	{
		if (lpObj->SAU_ItemChinh.IsItem())
		{
			SlotRecv = gItemManager.InventoryInsertItem(aIndex, lpObj->SAU_ItemChinh); // Roll back lai Item
			lpObj->SAU_ItemChinh.Clear();
			for (int n = 0; n < 6; n++)
			{
				lpObj->SAU_ItemKetQua[n].Clear();
			}
		}
	}
	break;
	case 1: //Item Phu 1
	{
		if (lpObj->SAU_ItemKetQua[0].m_Index < 0)
		{
			gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(3));//
			return;
		}

		if (lpObj->SAU_ItemChinh.IsItem())
		{
			SlotRecv = gItemManager.InventoryInsertItem(aIndex, lpObj->SAU_ItemKetQua[0]); // Roll back lai Item
			gLog.Output(LOG_GENERAL, "[Doi Item] Char [%s] Doi Thanh Cong Item[%s] Sang [%s]", lpObj->Name, gItemLevel.GetItemName(lpObj->SAU_ItemChinh.m_Index, lpObj->SAU_ItemChinh.m_Level), gItemLevel.GetItemName(lpObj->SAU_ItemKetQua[BackSlot - 1].m_Index, lpObj->SAU_ItemKetQua[BackSlot - 1].m_Level));
			lpObj->SAU_ItemChinh.Clear();
			for (int n = 0; n < 6; n++)
			{
				lpObj->SAU_ItemKetQua[n].Clear();
			}
		}
		gObjInventoryCommit(aIndex); //Save Trang Thai Da Mix
	}
	break;
	case 2: //Item Phu 2
	{
		if (lpObj->SAU_ItemKetQua[1].m_Index < 0)
		{
			gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(3));//
			return;
		}

		if (lpObj->SAU_ItemChinh.IsItem())
		{
			SlotRecv = gItemManager.InventoryInsertItem(aIndex, lpObj->SAU_ItemKetQua[1]); // Roll back lai Item
			gLog.Output(LOG_GENERAL, "[Doi Item] Char [%s] Doi Thanh Cong Item [%s] Sang [%s] ", lpObj->Name, gItemLevel.GetItemName(lpObj->SAU_ItemChinh.m_Index, lpObj->SAU_ItemChinh.m_Level), gItemLevel.GetItemName(lpObj->SAU_ItemKetQua[BackSlot - 1].m_Index, lpObj->SAU_ItemKetQua[BackSlot - 1].m_Level));
			lpObj->SAU_ItemChinh.Clear();
			for (int n = 0; n < 6; n++)
			{
				lpObj->SAU_ItemKetQua[n].Clear();
			}
		}
		gObjInventoryCommit(aIndex); //Save Trang Thai Da Mix
	}
	break;
	case 3: //Item Phu 3
	{

		if (lpObj->SAU_ItemKetQua[2].m_Index < 0)
		{
			gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(3));//
			return;
		}

		if (lpObj->SAU_ItemChinh.IsItem())
		{
			SlotRecv = gItemManager.InventoryInsertItem(aIndex, lpObj->SAU_ItemKetQua[2]); // Roll back lai Item
			gLog.Output(LOG_GENERAL, "[Doi Item] Char [%s] Doi Thanh Cong Item [%s] Sang [%s]", lpObj->Name, gItemLevel.GetItemName(lpObj->SAU_ItemChinh.m_Index, lpObj->SAU_ItemChinh.m_Level), gItemLevel.GetItemName(lpObj->SAU_ItemKetQua[BackSlot - 1].m_Index, lpObj->SAU_ItemKetQua[BackSlot - 1].m_Level));
			lpObj->SAU_ItemChinh.Clear();
			for (int n = 0; n < 6; n++)
			{
				lpObj->SAU_ItemKetQua[n].Clear();
			}
		}
		gObjInventoryCommit(aIndex); //Save Trang Thai Da Mix
	}
	break;
	case 4: //Item KQ
	{
		if (lpObj->SAU_ItemKetQua[3].m_Index < 0)
		{
			gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(3));//
			return;
		}

		if (lpObj->SAU_ItemChinh.IsItem())
		{
			SlotRecv = gItemManager.InventoryInsertItem(aIndex, lpObj->SAU_ItemKetQua[3]); // Roll back lai Item
			gLog.Output(LOG_GENERAL, "[Doi Item] Char [%s] Doi Thanh Cong Item [%s] Sang [%s]", lpObj->Name, gItemLevel.GetItemName(lpObj->SAU_ItemChinh.m_Index, lpObj->SAU_ItemChinh.m_Level), gItemLevel.GetItemName(lpObj->SAU_ItemKetQua[BackSlot - 1].m_Index, lpObj->SAU_ItemKetQua[BackSlot - 1].m_Level));
			lpObj->SAU_ItemChinh.Clear();
			for (int n = 0; n < 6; n++)
			{
				lpObj->SAU_ItemKetQua[n].Clear();
			}
		}
		gObjInventoryCommit(aIndex); //Save Trang Thai Da Mix
	}
	break;

	case 5: //Item KQ
	{

		if (lpObj->SAU_ItemKetQua[4].m_Index < 0)
		{
			gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(3));//
			return;
		}

		if (lpObj->SAU_ItemChinh.IsItem())
		{
			SlotRecv = gItemManager.InventoryInsertItem(aIndex, lpObj->SAU_ItemKetQua[4]); // Roll back lai Item
			gLog.Output(LOG_GENERAL, "[Doi Item] Char [%s] Doi Thanh Cong Item [%s] Sang [%s] ", lpObj->Name, gItemLevel.GetItemName(lpObj->SAU_ItemChinh.m_Index, lpObj->SAU_ItemChinh.m_Level), gItemLevel.GetItemName(lpObj->SAU_ItemKetQua[BackSlot - 1].m_Index, lpObj->SAU_ItemKetQua[BackSlot - 1].m_Level));
			lpObj->SAU_ItemChinh.Clear();
			for (int n = 0; n < 6; n++)
			{
				lpObj->SAU_ItemKetQua[n].Clear();
			}
		}
		gObjInventoryCommit(aIndex); //Save Trang Thai Da Mix
	}
	break;

	case 6: //Item KQ
	{

		if (lpObj->SAU_ItemKetQua[5].m_Index < 0)
		{
			gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, this->GetMessage(3));//
			return;
		}

		if (lpObj->SAU_ItemChinh.IsItem())
		{
			SlotRecv = gItemManager.InventoryInsertItem(aIndex, lpObj->SAU_ItemKetQua[5]); // Roll back lai Item
			gLog.Output(LOG_GENERAL, "[Doi Item] Char [%s] Doi Thanh Cong Item [%s] Sang [%s] ", lpObj->Name, gItemLevel.GetItemName(lpObj->SAU_ItemChinh.m_Index, lpObj->SAU_ItemChinh.m_Level), gItemLevel.GetItemName(lpObj->SAU_ItemKetQua[BackSlot - 1].m_Index, lpObj->SAU_ItemKetQua[BackSlot - 1].m_Level));
			lpObj->SAU_ItemChinh.Clear();
			for (int n = 0; n < 6; n++)
			{
				lpObj->SAU_ItemKetQua[n].Clear();
			}
		}
		gObjInventoryCommit(aIndex); //Save Trang Thai Da Mix
	}
	break;

	default:
		break;
	}
	if (BackSlot >= 1)
	{
		GDSetCoinSend(lpObj->Index, -it->second.Wcoin, 0, 0, "Coin");

	}
	char tmp[255];
	char tmp2[255];

	if (it->second.Notice == 1)
	{ //Thong Bao trong Sub
		gNotice.GCNoticeSend(lpObj->Index, 0, 0, 0, 0, 0, 0, this->GetMessage(4), lpObj->Name, it->second.Name);
	}
	else if (it->second.Notice == 2)
	{ //Thong Bao Toan Sub
		wsprintf(tmp, this->GetMessage(4), lpObj->Name, it->second.Name);
		GDGlobalNoticeSend(gMapServerManager.GetMapServerGroup(), 0, 0, 0, 0, 0, 0, tmp);
	}
	else if (it->second.Notice == 3)
	{ //Thong Bao Toan Sub
		wsprintf(tmp, this->GetMessage(4), lpObj->Name, it->second.Name);
		GDGlobalNoticeSend(gMapServerManager.GetMapServerGroup(), 0, 0, 0, 0, 0, 0, tmp);
		//gLog.Output(LOG_GENERAL, "[Doi Item] Char [%s] Doi Thanh Cong Item [%]", lpObj->Name,it->second.Name );
	}

	if (SlotRecv != 0xFF)
	{
		gItemManager.GCItemModifySend(aIndex, SlotRecv);
		this->SendInfoItemCache(aIndex);
	}
}

bool SauChangeItem::Dialog(LPOBJ lpObj, LPOBJ lpNpc)
{
	if (!this->Enable || lpObj->Interface.type != INTERFACE_NONE)
	{
		gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, this->GetMessage(0));
		return false;
	}

	if (lpNpc->Class == this->mNPCData->NPCClass &&
		lpNpc->Map == this->mNPCData->NPCMap &&
		lpNpc->X == this->mNPCData->NPCX &&
		lpNpc->Y == this->mNPCData->NPCY)
	{
		//lpObj->DL_StatusMix = TRUE;
		//====Open Window
		XULY_CGPACKET pMsg;
		pMsg.header.set(0xD3, 0x6A, sizeof(pMsg));
		pMsg.ThaoTac = 111;	//Open NPC
		DataSend(lpObj->Index, (BYTE*)& pMsg, pMsg.header.size);
		SetStateInterface(lpObj->Index, 1);
		return true;
	}

	return false;
}











#endif