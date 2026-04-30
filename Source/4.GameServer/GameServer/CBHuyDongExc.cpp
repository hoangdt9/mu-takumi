#include "StdAfx.h"
#include "CBHuyDongExc.h"
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

CBHuyDongExc gBHuyDongExc;

CBHuyDongExc::CBHuyDongExc()
{
}


CBHuyDongExc::~CBHuyDongExc()
{
}

void CBHuyDongExc::LoadConfigXML(char* FilePath)
{
	pugi::xml_document file;
	pugi::xml_parse_result res = file.load_file(FilePath);
	if (res.status != pugi::status_ok) {
		ErrorMessageBox("File %s load fail. Error: %s", FilePath, res.description());
		return;
	}
	//--
	//==== Message
	pugi::xml_node Message = file.child("MessageInfo");
	for (pugi::xml_node msg = Message.child("Message"); msg; msg = msg.next_sibling())
	{
		MESSAGE_HUYDONGEXC info;

		info.Index = msg.attribute("Index").as_int();

		strcpy_s(info.Message, msg.attribute("Text").as_string());

		this->m_MessageInfo.insert(std::pair<int, MESSAGE_HUYDONGEXC>(info.Index, info));
	}
	this->m_HuyExcInfo.clear();
	//==Huy Item
	pugi::xml_node oHuyItem = file.child("HuyExc");
	//-Config
	pugi::xml_node eHuyItemConfig = oHuyItem.child("Config");
	this->mHuyExc_Enabled = eHuyItemConfig.attribute("Enabled").as_int();

	pugi::xml_node oHuyExcConfig = oHuyItem.child("HuyExcConfig");
	this->HExcRS = oHuyExcConfig.attribute("HExcRS").as_int();
	this->HExcLvMin = oHuyExcConfig.attribute("HExcLvMin").as_int();
	this->HExcLvMax = oHuyExcConfig.attribute("HExcLvMax").as_int();
	this->HExcSkill = oHuyExcConfig.attribute("HExcSkill").as_int();
	this->HExcLuck = oHuyExcConfig.attribute("HExcLuck").as_int();
	for (pugi::xml_node oHuyExcInfo = oHuyExcConfig.child("Type"); oHuyExcInfo; oHuyExcInfo = oHuyExcInfo.next_sibling())
	{
		HUYEXC_COIN info;
		info.Index = oHuyExcInfo.attribute("ExcOpt").as_int();
		info.Coin[0] = oHuyExcInfo.attribute("WC").as_int();
		info.Coin[1] = oHuyExcInfo.attribute("WCP").as_int();
		info.Coin[2] = oHuyExcInfo.attribute("GBP").as_int();
		info.Rate = oHuyExcInfo.attribute("Rate").as_int();
		this->m_HuyExcInfo.insert(std::pair<int, HUYEXC_COIN>(info.Index, info));
	}
	LogAdd(LOG_BLUE, "[HuyExc] On: %d TypeMixCount : %d", this->mHuyExc_Enabled, this->m_HuyExcInfo.size());

}

char* CBHuyDongExc::GetMessage(int index) // OK
{
	std::map<int, MESSAGE_HUYDONGEXC>::iterator it = this->m_MessageInfo.find(index);

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
//==========Huy Exc
void CBHuyDongExc::HuyExc_ClientSetItemGS(int aIndex, PMSG_HDCLIENTSENDGS* lpMsg)
{

	if (!OBJMAX_RANGE(aIndex))
	{
		return;
	}


	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->Type != OBJECT_USER || lpObj->m_OfflineMode != 0 /*|| lpObj->IsFakeOnline != 0 */ || !this->mHuyExc_Enabled)
	{
		return;
	}

	if (lpObj->Interface.type != INTERFACE_NONE || lpObj->Interface.use != 0)
	{
		return;
	}

	if (gItemManager.CheckItemMoveToTrade(lpObj, &lpObj->Inventory[lpMsg->Source], 0) == 0)
	{
		return;
	}

	if (gItemManager.ChaosBoxHasItem(lpObj) || gItemManager.TradeHasItem(lpObj))
	{
		return;
	}
	if (!lpObj->Inventory[lpMsg->Source].IsItem())
	{
		//Vi tri nay khong co Item do
		return;
	}
	if (lpObj->Inventory[lpMsg->Source].m_NewOption < 1 || (lpObj->Inventory[lpMsg->Source].m_Index / 512) >= 12)
	{
		gNotice.GCNoticeSend(aIndex, 0, 0, 0, 0, 0, 0, this->GetMessage(10));//"[CongHuong] Item khong phu hop !!");
		//LogAdd(LOG_RED,"[BotAlchemist](%s)(%s) Ancient Item Error",lpObj->Account,lpObj->Name);
		return;
	}

	//===Set Info Item Cache
	if (lpObj->CH_IndexItem[0] == -1) //
	{
		char SendTextNotice[256] = { 0 };
		char StringCat[128] = { 0 };
		sprintf(SendTextNotice, "%s", this->GetMessage(12));
		//==Check Dieu Kien
		if (this->HExcRS != -1 && this->HExcRS > lpObj->Reset)
		{
			sprintf(StringCat, "Reset > %d", this->HExcRS);
			strcat(SendTextNotice, StringCat);
			gNotice.GCNoticeSend(aIndex, 0, 0, 0, 0, 0, 0, SendTextNotice);
			return;
		}
		if (this->HExcLvMin != -1 && this->HExcLvMax != -1)
		{
			if (lpObj->Inventory[lpMsg->Source].m_Level < this->HExcLvMin || lpObj->Inventory[lpMsg->Source].m_Level > this->HExcLvMax)
			{
				sprintf(StringCat, "Item (+%d ~ %d)", this->HExcLvMin, this->HExcLvMax);
				strcat(SendTextNotice, StringCat);
				gNotice.GCNoticeSend(aIndex, 0, 0, 0, 0, 0, 0, SendTextNotice);
				return;
			}
		}
		if (this->HExcSkill != -1 && lpObj->Inventory[lpMsg->Source].m_Option1 == 0)
		{
			sprintf(StringCat, "+ Skill");
			strcat(SendTextNotice, StringCat);
			gNotice.GCNoticeSend(aIndex, 0, 0, 0, 0, 0, 0, SendTextNotice);
			return;
		}
		if (this->HExcLuck != -1 && lpObj->Inventory[lpMsg->Source].m_Option2 == 0)
		{
			sprintf(StringCat, "+ Luck");
			strcat(SendTextNotice, StringCat);
			gNotice.GCNoticeSend(aIndex, 0, 0, 0, 0, 0, 0, SendTextNotice);
			return;
		}
		lpObj->CH_IndexItem[0] = lpObj->Inventory[lpMsg->Source].m_Index;
		lpObj->CH_InfoItem[0] = lpObj->Inventory[lpMsg->Source];
		//=== Send Del item
		gItemManager.InventoryDelItem(aIndex, lpMsg->Source);
		gItemManager.GCItemDeleteSend(aIndex, lpMsg->Source, 1);
		//====
		HuyExc_GSSendInfo(aIndex);
		return;
	}
	else {
		gNotice.GCNoticeSend(aIndex, 0, 0, 0, 0, 0, 0, this->GetMessage(11));//
		return;
	}
}

void CBHuyDongExc::HuyExc_GSSendInfo(int aIndex)
{
	if (!OBJMAX_RANGE(aIndex))
	{
		return;
	}
	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->Type != OBJECT_USER || lpObj->m_OfflineMode != 0 /*|| lpObj->IsFakeOnline != 0*/)
	{
		return;
	}

	if (lpObj->Interface.type != INTERFACE_NONE || lpObj->Interface.use != 0)
	{
		return;
	}

	//===========
	HUYEXC_GSSENDINFO pMsg;
	pMsg.header.set(0xFA, 0x16, sizeof(pMsg));

	for (int i = 0; i < 6; i++)
	{
		std::map<int, HUYEXC_COIN>::iterator it = this->m_HuyExcInfo.find(i);

		if (it == this->m_HuyExcInfo.end())
		{
			pMsg.mWC[i] = 0;
			pMsg.mWP[i] = 0;
			pMsg.mGP[i] = 0;
			pMsg.mRate[i] = 0;
		}
		else
		{
			pMsg.mWC[i] = it->second.Coin[0];
			pMsg.mWP[i] = it->second.Coin[1];
			pMsg.mGP[i] = it->second.Coin[2];
			pMsg.mRate[i] = it->second.Rate;

		}
	}
	pMsg.Request = 1;
	DataSend(lpObj->Index, (BYTE*)& pMsg, pMsg.header.size);
	//==========
}

void CBHuyDongExc::ActionHuyExc(int aIndex, XULY_CGPACKET* lpMsg)
{

	if (!OBJMAX_RANGE(aIndex))
	{
		return;
	}

	int GetNumberSelect = -1;
	int TypeExcOp = lpMsg->ThaoTac;
	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->Type != OBJECT_USER || lpObj->m_OfflineMode != 0 /*|| lpObj->IsFakeOnline != 0 */ || !this->mHuyExc_Enabled || lpObj->CH_IndexItem[0] == -1)
	{
		return;
	}

	switch (TypeExcOp)
	{
	case 1:
	{
		GetNumberSelect = 0;
	}
	break;
	case 2:
	{
		GetNumberSelect = 1;
	}
	break;
	case 4:
	{
		GetNumberSelect = 2;
	}
	break;
	case 8:
	{
		GetNumberSelect = 3;
	}
	break;
	case 16:
	{
		GetNumberSelect = 4;
	}
	break;
	case 32:
	{
		GetNumberSelect = 5;
	}
	break;
	default:
		break;
	}
	if (GetNumberSelect == -1)
	{
		return;
	}

	std::map<int, HUYEXC_COIN>::iterator it = this->m_HuyExcInfo.find(GetNumberSelect);

	if (it == this->m_HuyExcInfo.end())
	{
		return;
	}
	//==Check Coin
	if (lpObj->Coin1 < it->second.Coin[0])
	{
		gNotice.GCNoticeSend(aIndex, 0, 0, 0, 0, 0, 0, this->GetMessage(13), it->second.Coin[0]);//"[CongHuong] WCoin");
		return;
	}
	if (lpObj->Coin2 < it->second.Coin[1])
	{
		gNotice.GCNoticeSend(aIndex, 0, 0, 0, 0, 0, 0, this->GetMessage(14), it->second.Coin[1]);//"[CongHuong] WCoinP");
		return;
	}
	if (lpObj->Coin2 < it->second.Coin[2])
	{
		gNotice.GCNoticeSend(aIndex, 0, 0, 0, 0, 0, 0, this->GetMessage(15), it->second.Coin[2]);//"[CongHuong] GobinP");
		return;
	}
	GDSetCoinSend(lpObj->Index, (it->second.Coin[0] > 0) ? (-it->second.Coin[0]) : 0, (it->second.Coin[0] > 0) ? (-it->second.Coin[1]) : 0, (it->second.Coin[0] > 0) ? (-it->second.Coin[2]) : 0, "Tru Coin Huy Exc");
	gCashShop.CGCashShopPointRecv(lpObj->Index);
	int RandomRate = (GetLargeRand() % 100);
	if (RandomRate > it->second.Rate) //That Bai
	{
		CBHuyDongExc::ClearCache(aIndex);//Tra Lai Item
		gNotice.GCNoticeSend(aIndex, 255, 0, 0, 0, 0, 0, this->GetMessage(9));// That Bai
		return;
	}
	//=== Xu Ly Opt Exc
	BYTE sLife = lpObj->CH_InfoItem[0].IsExtLifeAdd();
	BYTE sMana = lpObj->CH_InfoItem[0].IsExtManaAdd();
	BYTE sDmg = lpObj->CH_InfoItem[0].IsExtDamageMinus();
	BYTE sReflect = lpObj->CH_InfoItem[0].IsExtDamageReflect();
	BYTE sDef = lpObj->CH_InfoItem[0].IsExtDefenseSuccessfull();
	BYTE sMoney = lpObj->CH_InfoItem[0].IsExtMonsterMoney();

	if (sLife != 0 && TypeExcOp == 32)
	{
		lpObj->CH_InfoItem[0].m_NewOption &= ~0x20;
	}
	if (sMana != 0 && TypeExcOp == 16)
	{
		lpObj->CH_InfoItem[0].m_NewOption &= ~0x10;

	}
	if (sDmg != 0 && TypeExcOp == 8)
	{
		lpObj->CH_InfoItem[0].m_NewOption &= ~8;

	}
	if (sReflect != 0 && TypeExcOp == 4)
	{
		lpObj->CH_InfoItem[0].m_NewOption &= ~4;
	}
	if (sDef != 0 && TypeExcOp == 2)
	{
		lpObj->CH_InfoItem[0].m_NewOption &= ~2;
	}
	if (sMoney != 0 && TypeExcOp == 1)
	{
		lpObj->CH_InfoItem[0].m_NewOption &= ~1;
	}
	//===== Set Item
	BYTE SlotRecv = gItemManager.InventoryInsertItem(aIndex, gObj[aIndex].CH_InfoItem[0]); // Roll back lai Item
	if (SlotRecv != 0xFF)
	{
		gItemManager.GCItemModifySend(aIndex, SlotRecv);

	}

	CBHuyDongExc::ClearCache(aIndex, 1);
}

void CBHuyDongExc::ClearCache(int aIndex, int Clear)
{

	if (!Clear)
	{
		BYTE SlotRecv = gItemManager.InventoryInsertItem(aIndex, gObj[aIndex].CH_InfoItem[0]); // Roll back lai Item
		if (SlotRecv != 0xFF)
		{
			gItemManager.GCItemModifySend(aIndex, SlotRecv);
		}

	}
	//=== Item Chinh
	gObj[aIndex].CH_IndexItem[0] = -1;
	gObj[aIndex].CH_InfoItem[0].Clear();
	GDCharacterInfoSaveSend(aIndex);
}