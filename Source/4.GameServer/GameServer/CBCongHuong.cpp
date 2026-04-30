#include "StdAfx.h"
#if (CongHuongV2)
#include "CBCongHuong.h"
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
CBCongHuong gCBCongHuong;

CBCongHuong::CBCongHuong()
{
}


CBCongHuong::~CBCongHuong()
{
}

BOOL CBCongHuong::CHCheckItemCH(CItem ItemCheck) // OK
{

	int cExcOpt = getNumberOfExcOptions(ItemCheck.m_NewOption);
	int cLevel = ItemCheck.m_Level;
	bool cChecker = false;
	bool ItemInMax = true;
	this->CH_CoinMix[0] = -1;
	this->CH_CoinMix[1] = -1;
	this->CH_CoinMix[2] = -1;
	this->CHLevelPlus = 0;
	this->CH_MaxExcCount = 0;
	this->CH_TyLeThanhCong = -1;

	if ((ItemCheck.m_Index / 512) >= 12)
	{
		return cChecker;
	}
	for (std::vector<TYPEMIX_CH>::iterator it = this->m_TypeMixCH.begin(); it != this->m_TypeMixCH.end(); it++)
	{
		if (cLevel >= it->LvMin && cLevel <= it->LvMax &&
			cExcOpt >= it->ExcMin && cExcOpt < it->ExcMax)
		{
			this->CH_CoinMix[0] = it->CH_WC;
			this->CH_CoinMix[1] = it->CH_WCP;
			this->CH_CoinMix[2] = it->CH_GBP;
			this->CHLevelPlus = it->CHLevelMax;
			this->CH_MaxExcCount = it->ExcMax;
			this->CH_TyLeThanhCong = it->CHRate;
			cChecker = true;
			ItemInMax = false;
		}
	}

	if (ItemInMax == true)
	{
		if (this->CHSkill == 1 && ItemCheck.m_Option1 == 0 || this->CHLuck == 1 && ItemCheck.m_Option2 == 0)
		{
			this->CH_CoinMix[0] = this->m_TypeMixCH.back().CH_WC;
			this->CH_CoinMix[1] = this->m_TypeMixCH.back().CH_WCP;
			this->CH_CoinMix[2] = this->m_TypeMixCH.back().CH_GBP;
			this->CHLevelPlus = this->m_TypeMixCH.back().CHLevelMax;
			this->CH_MaxExcCount = this->m_TypeMixCH.back().ExcMax;
			this->CH_TyLeThanhCong = this->m_TypeMixCH.back().CHRate;
			cChecker = true;
		}
	}
	return cChecker;
}
#include "MonsterSetBase.h"
void CBCongHuong::SetNPC()
{
	if (this->mCH_Enabled)
	{
		MONSTER_SET_BASE_INFO info;
		memset(&info, 0, sizeof(info));

		info.Type = 0;
		info.MonsterClass = this->mCH_NPCClass;
		info.Map = this->mCH_NPCMap;
		info.Dis = 0;
		info.Dir = this->mCH_NPCDir;
		info.X = this->mCH_NPCX;
		info.Y = this->mCH_NPCY;

		gMonsterSetBase.SetInfo(info);
	}
}
void CBCongHuong::CH_LoadConfigXML(char* FilePath)
{
	this->CH_ClearConfig();

	pugi::xml_document file;
	pugi::xml_parse_result res = file.load_file(FilePath);
	if (res.status != pugi::status_ok) {
		ErrorMessageBox("File %s load fail. Error: %s", FilePath, res.description());
		return;
	}
	//--
	pugi::xml_node oNPC = file.child("NPC");
	//-Config
	pugi::xml_node eSetNPC = oNPC.child("SetNPC");
	this->mCH_NPCClass = eSetNPC.attribute("NPCClass").as_int();
	this->mCH_NPCMap = eSetNPC.attribute("NPCMap").as_int();
	this->mCH_NPCX = eSetNPC.attribute("NPCX").as_int();
	this->mCH_NPCY = eSetNPC.attribute("NPCY").as_int();
	this->mCH_NPCDir = eSetNPC.attribute("NPCDir").as_int();
	//--
	pugi::xml_node oCongHuongItem = file.child("CongHuongItem");
	//-Config
	pugi::xml_node eCongHuongConfig = oCongHuongItem.child("Config");
	this->mCH_Enabled = eCongHuongConfig.attribute("Enabled").as_int();
	//==== Load Type
	pugi::xml_node eCongHuongType = oCongHuongItem.child("CHConifg");
	this->CH_ResetMix = eCongHuongType.attribute("CHReset").as_int();
	this->CHLuck = eCongHuongType.attribute("CHLuck").as_int();
	this->CHSkill = eCongHuongType.attribute("CHSkill").as_int();
	this->CHLevel = eCongHuongType.attribute("CHLevel").as_int();
	this->CHOptLife = eCongHuongType.attribute("CHOptLife").as_int();
	this->CHCheckPhoiType = eCongHuongType.attribute("CHCheckPhoiType").as_int();
	this->CHKeepItem = eCongHuongType.attribute("CHKeepItem").as_int();
	this->CHAllowAnc = eCongHuongType.attribute("CHAllowAnc").as_int();


	for (pugi::xml_node rCongHuongType = eCongHuongType.child("Type"); rCongHuongType; rCongHuongType = rCongHuongType.next_sibling())
	{
		TYPEMIX_CH info;
		info.LvMin = rCongHuongType.attribute("LvMin").as_int();
		info.LvMax = rCongHuongType.attribute("LvMax").as_int();
		info.ExcMin = rCongHuongType.attribute("ExcMin").as_int();
		info.ExcMax = rCongHuongType.attribute("ExcMax").as_int();
		info.CHLevelMax = rCongHuongType.attribute("CHLevelMax").as_int();
		info.CH_WC = rCongHuongType.attribute("WC").as_int();
		info.CH_WCP = rCongHuongType.attribute("WCP").as_int();
		info.CH_GBP = rCongHuongType.attribute("GBP").as_int();
		info.CHRate = rCongHuongType.attribute("CHRate").as_int();
		this->m_TypeMixCH.push_back(info);
	}
	//==== Message
	pugi::xml_node Message = file.child("MessageInfo");
	for (pugi::xml_node msg = Message.child("Message"); msg; msg = msg.next_sibling())
	{
		MESSAGE_CH info;

		info.Index = msg.attribute("Index").as_int();

		strcpy_s(info.Message, msg.attribute("Text").as_string());

		this->m_MessageInfo.insert(std::pair<int, MESSAGE_CH>(info.Index, info));
	}
}

void CBCongHuong::CH_ClearConfig()
{
	this->CH_ResetMix = -1;

	this->CH_TyLeThanhCong = -1;
	this->CH_CoinMix[0] = -1;
	this->CH_CoinMix[1] = -1;
	this->CH_CoinMix[2] = -1;
	this->CHLevelPlus = 0;
	this->CH_MaxExcCount = 0;
	//==============
	this->mCH_Enabled = 0;
	this->mCH_NPCClass = 0;
	this->mCH_NPCMap = 0;
	this->mCH_NPCX = 0;
	this->mCH_NPCY = 0;
	this->mCH_NPCDir = 0;

	this->CHLuck = 0;
	this->CHSkill = 0;
	this->CHLevel = 0;
	this->CHCheckPhoiType = 0;
	this->CHKeepItem = 0;
	this->CHAllowAnc = 0;

	this->m_TypeMixCH.clear();
	this->m_MessageInfo.clear();

}

void CBCongHuong::CH_ClearDB(int aIndex, int TypeClear, bool RollBackItem)
{
	if (OBJECT_RANGE(aIndex) == 0)
	{
		return;
	}
	bool clear = false;

	if ((TypeClear == 2 || TypeClear == 0) && gObj[aIndex].CH_IndexItem[1] != -1) //item Phoi
	{
		if (RollBackItem)
		{
			BYTE SlotRecv = gItemManager.InventoryInsertItem(aIndex, gObj[aIndex].CH_InfoItem[1]); // Roll back lai Item
			if (SlotRecv != 0xFF)
			{
				gItemManager.GCItemModifySend(aIndex, SlotRecv);

			}
		}
		//=== Item Phoi
		gObj[aIndex].CH_IndexItem[1] = -1;
		gObj[aIndex].CH_InfoItem[1].Clear();
		clear = true;
	}

	if ((TypeClear == 1 || TypeClear == 0) && gObj[aIndex].CH_IndexItem[0] != -1)
	{
		if (gObj[aIndex].CH_IndexItem[1] != -1)
		{
			gNotice.GCNoticeSend(aIndex, 0, 0, 0, 0, 0, 0, this->GetMessage(3));// "[CongHuong] Ban can lay phoi ra truoc !");
			return;
		}
		if (RollBackItem)
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
		clear = true;
	}

	if (clear)
	{
		//=== Xem Truoc
		gObj[aIndex].CH_IndexItem[2] = -1;
		gObj[aIndex].CH_InfoItem[2].Clear();
		gObj[aIndex].CH_InfoItem[3].Clear();
	}
	if (RollBackItem)
	{
		GDCharacterInfoSaveSend(aIndex);
	}
}
void CBCongHuong::CH_GSSendInfoClient(int aIndex) //GUi ve Client Info
{
	if (OBJECT_RANGE(aIndex) == 0)
	{
		return;
	}
	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->Type != OBJECT_USER || lpObj->m_OfflineMode != 0 || lpObj->IsFakeOnline != 0)
	{
		return;
	}
	//===========
	PMSG_GSSendInfoClient pMsg;
	pMsg.header.set(0xFA, 0x15, sizeof(pMsg));
	pMsg.mCH_TyLeThanhCong = this->CH_TyLeThanhCong;
	pMsg.mCH_ResetMix = this->CH_ResetMix;
	pMsg.mCH_TypeMix = this->CHCheckPhoiType;
	pMsg.mCH_CheckLvItem = 1;
	pMsg.mCH_CoinMix[0] = this->CH_CoinMix[0]; //WCoin
	pMsg.mCH_CoinMix[1] = this->CH_CoinMix[1]; //WCoinP
	pMsg.mCH_CoinMix[2] = this->CH_CoinMix[2]; //Gobin

	for (int i = 0; i < 3; i++)
	{
		pMsg.mCH_IndexItem[i] = lpObj->CH_IndexItem[i];
		memset(pMsg.mCH_InfoItem[i], 0x0, sizeof(pMsg.mCH_InfoItem[i]));
		gItemManager.ItemByteConvert(pMsg.mCH_InfoItem[i], lpObj->CH_InfoItem[i]); // Set Info Item Chinh
	}

	DataSend(lpObj->Index, (BYTE*)& pMsg, pMsg.header.size);
	//==========
}
void CBCongHuong::CH_ClientSetItemGS(int aIndex, PMSG_MOVEITEM* lpMsg)
{

	if (!OBJMAX_RANGE(aIndex))
	{
		return;
	}


	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->Type != OBJECT_USER || lpObj->m_OfflineMode != 0 || lpObj->IsFakeOnline != 0)
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
	if (lpObj->Inventory[lpMsg->Source].m_NewOption == 63 && this->CHLevel == 0)
	{
		gNotice.GCNoticeSend(aIndex, 0, 0, 0, 10, 0, 0, "[Cộng Hưởng] Không Thể Dùng Item Full Excellent Để Cộng Hưởng !");
		return;
	}
	if (lpObj->Inventory[lpMsg->Source].m_NewOption < 1 || (lpObj->Inventory[lpMsg->Source].m_Index / 512) >= 12)
	{
		gNotice.GCNoticeSend(aIndex, 0, 0, 0, 10, 0, 0, "[Cộng Hưởng] Item Chính của bạn Không Phải Là Excellent !");//"[CongHuong] Item khong phu hop !!");
		return;
	}

	if (this->CHAllowAnc == 0 && lpObj->Inventory[lpMsg->Source].m_SetOption > 0)
	{
		gNotice.GCNoticeSend(aIndex, 0, 0, 0, 10, 0, 0, "[Cộng Hưởng] Admin Không Cho Cộng Hưởng Đồ Thần !");//"[CongHuong] Item khong phu hop !!");
		return;
	}
	//if (lpObj->Inventory[lpMsg->Source].m_NewOption < 1 || (lpObj->Inventory[lpMsg->Source].m_Index / 512) >= 12)
	//{
	//	gNotice.GCNoticeSend(aIndex, 0, 0, 0, 10, 0, 0, this->GetMessage(4));//"[CongHuong] Item khong phu hop !!");
	//	//LogAdd(LOG_RED,"[BotAlchemist](%s)(%s) Ancient Item Error",lpObj->Account,lpObj->Name);
	//	return;
	//}

	//if (this->CHAllowAnc == 0 && lpObj->Inventory[lpMsg->Source].m_SetOption > 0)
	//{
	//	gNotice.GCNoticeSend(aIndex, 0, 0, 0, 10, 0, 0, this->GetMessage(4));//"[CongHuong] Item khong phu hop !!");
	//	//LogAdd(LOG_RED,"[BotAlchemist](%s)(%s) Ancient Item Error",lpObj->Account,lpObj->Name);
	//	return;
	//}

	//===Set Info Item Cache
	if (lpObj->CH_IndexItem[0] == -1) //
	{
		if (this->CHCheckItemCH(lpObj->Inventory[lpMsg->Source]) == false)
		{
			gNotice.GCNoticeSend(aIndex, 0, 0, 0, 10, 0, 0, this->GetMessage(9));//"[CongHuong] Item cua ban dat muc toi da !!");
			return;
		}
		lpObj->CH_IndexItem[0] = lpObj->Inventory[lpMsg->Source].m_Index;
		lpObj->CH_InfoItem[0] = lpObj->Inventory[lpMsg->Source];
		//=== Send Del item
		gItemManager.InventoryDelItem(aIndex, lpMsg->Source);
		gItemManager.GCItemDeleteSend(aIndex, lpMsg->Source, 1);
		//====
		CH_GSSendInfoClient(aIndex);
	}
	else if (lpObj->CH_IndexItem[1] == -1) //
	{
		if (this->CHCheckPhoiType > 0)
		{
			int fLevel = gItemManager.m_ItemInfo[lpObj->CH_InfoItem[0].m_Index].Level;
			int sLevel = gItemManager.m_ItemInfo[lpObj->Inventory[lpMsg->Source].m_Index].Level;

			if ((sLevel < fLevel && this->CHCheckPhoiType == 1) || (lpObj->CH_InfoItem[0].m_Index != lpObj->Inventory[lpMsg->Source].m_Index && this->CHCheckPhoiType == 2))
			{
				gNotice.GCNoticeSend(aIndex, 0, 0, 0, 10, 0, 0, this->GetMessage(5));//"[CongHuong] vui long dung phoi co trong danh sach !");
				return;
			}

		}

		if ((lpObj->CH_InfoItem[0].m_Index / 512) != (lpObj->Inventory[lpMsg->Source].m_Index / 512))
		{
			gNotice.GCNoticeSend(aIndex, 0, 0, 0, 10, 0, 0, this->GetMessage(4));//"[CongHuong] Item khong phu hop !!");
			return;

		}
		lpObj->CH_IndexItem[1] = lpObj->Inventory[lpMsg->Source].m_Index;
		lpObj->CH_InfoItem[1] = lpObj->Inventory[lpMsg->Source];
		//=== Send Del item
		gItemManager.InventoryDelItem(aIndex, lpMsg->Source);
		gItemManager.GCItemDeleteSend(aIndex, lpMsg->Source, 1);
		//====	
		//=== Tinh Toan Item Xem Truoc
		lpObj->CH_IndexItem[2] = lpObj->CH_IndexItem[0];
		lpObj->CH_InfoItem[2] = lpObj->CH_InfoItem[0];

		//== Set Cache Item Phoi 
		lpObj->CH_InfoItem[3].Clear();
		lpObj->CH_InfoItem[3] = lpObj->CH_InfoItem[1];
		//CItem ItemXemTruoc;
		//gItemManager.ConvertItemByte(&ItemXemTruoc, lpObj->CH_InfoItem[0]);
		//
		//=== Phoi
		BYTE sLife = lpObj->CH_InfoItem[1].IsExtLifeAdd();
		BYTE sMana = lpObj->CH_InfoItem[1].IsExtManaAdd();
		BYTE sDmg = lpObj->CH_InfoItem[1].IsExtDamageMinus();
		BYTE sReflect = lpObj->CH_InfoItem[1].IsExtDamageReflect();
		BYTE sDef = lpObj->CH_InfoItem[1].IsExtDefenseSuccessfull();
		BYTE sMoney = lpObj->CH_InfoItem[1].IsExtMonsterMoney();
		////== Item Goc
		BYTE fLife = lpObj->CH_InfoItem[2].IsExtLifeAdd();
		BYTE fMana = lpObj->CH_InfoItem[2].IsExtManaAdd();
		BYTE fDmg = lpObj->CH_InfoItem[2].IsExtDamageMinus();
		BYTE fReflect = lpObj->CH_InfoItem[2].IsExtDamageReflect();
		BYTE fDef = lpObj->CH_InfoItem[2].IsExtDefenseSuccessfull();
		BYTE fMoney = lpObj->CH_InfoItem[2].IsExtMonsterMoney();

		if (this->CHLuck == 1)
		{
			if (lpObj->CH_InfoItem[2].m_Option2 == 0 && lpObj->CH_InfoItem[1].m_Option2 == 1)
			{
				lpObj->CH_InfoItem[2].m_Option2 = 1;
				lpObj->CH_InfoItem[3].m_Option2 = 0;
			}
		}
		if (this->CHLevel == 1)
		{
			int fLevel = lpObj->CH_InfoItem[2].m_Level;
			int sLevel = lpObj->CH_InfoItem[1].m_Level; //Phoi
			if (sLevel > fLevel)
			{

				int CH_Level = 0;
				if (CHLevelPlus != -1)
				{
					CH_Level = fLevel + CHLevelPlus;
				}
				else
				{
					CH_Level = fLevel + sLevel;
				}
				if (CH_Level > 15) { CH_Level = 15; }


				lpObj->CH_InfoItem[2].m_Level = CH_Level;
				lpObj->CH_InfoItem[3].m_Level = 0;
			}
		}
		if (this->CHSkill == 1)
		{
			if (lpObj->CH_InfoItem[2].m_Index < 3584)
			{
				if (lpObj->CH_InfoItem[2].m_Option1 == 0 && lpObj->CH_InfoItem[1].m_Option1 == 1)
				{
					lpObj->CH_InfoItem[2].m_Option1 = 1;
					lpObj->CH_InfoItem[3].m_Option1 = 0;
				}
			}
		}
		if (this->CHOptLife == 1)
		{
			int fOpt = lpObj->CH_InfoItem[2].m_Option3;
			if (fOpt < 7)
			{
				int sOpt = lpObj->CH_InfoItem[1].m_Option3;
				if ((fOpt + sOpt) > 7)
				{

					fOpt = 7;
				}
				else
				{

					fOpt += sOpt;

				}
				lpObj->CH_InfoItem[2].m_Option3 = fOpt;
				lpObj->CH_InfoItem[3].m_Option3 = 0;
			}
		}
		int CheckOpExcItem = getNumberOfExcOptions(lpObj->CH_InfoItem[2].m_NewOption);
		if (CheckOpExcItem < this->CH_MaxExcCount)
		{

			if (lpObj->CH_InfoItem[2].m_NewOption != 63)
			{
				if (fLife == 0 && sLife != 0)
				{
					lpObj->CH_InfoItem[2].m_NewOption |= 0x20;
					lpObj->CH_InfoItem[3].m_NewOption &= ~0x20;
					CheckOpExcItem++;
				}
				else if (fMana == 0 && sMana != 0 && CheckOpExcItem < this->CH_MaxExcCount)
				{
					lpObj->CH_InfoItem[2].m_NewOption |= 0x10;
					lpObj->CH_InfoItem[3].m_NewOption &= ~0x10;
					CheckOpExcItem++;
				}
				else if (fDmg == 0 && sDmg != 0 && CheckOpExcItem < this->CH_MaxExcCount)
				{
					lpObj->CH_InfoItem[2].m_NewOption |= 8;
					lpObj->CH_InfoItem[3].m_NewOption &= ~8;
					CheckOpExcItem++;

				}
				else if (fReflect == 0 && sReflect != 0 && CheckOpExcItem < this->CH_MaxExcCount)
				{
					lpObj->CH_InfoItem[2].m_NewOption |= 4;
					lpObj->CH_InfoItem[3].m_NewOption &= ~4;
					CheckOpExcItem++;
				}
				else if (fDef == 0 && sDef != 0 && CheckOpExcItem < this->CH_MaxExcCount)
				{
					lpObj->CH_InfoItem[2].m_NewOption |= 2;
					lpObj->CH_InfoItem[3].m_NewOption &= ~2;
					CheckOpExcItem++;
				}
				else if (fMoney == 0 && sMoney != 0 && CheckOpExcItem < this->CH_MaxExcCount)
				{
					lpObj->CH_InfoItem[2].m_NewOption |= 1;
					lpObj->CH_InfoItem[3].m_NewOption &= ~1;
					CheckOpExcItem++;
				}
			}
		}
		CH_GSSendInfoClient(aIndex);
	}
	else
	{
		return;
	}
}
//==== Xu Ly Packet
bool CBCongHuong::TalkNPC(int aIndex, int aNpcIndex) //Click NPC Vao Event
{
	if (!this->mCH_Enabled)
	{
		gNotice.GCNoticeSend(aIndex, 1, 0, 0, 0, 0, 0, this->GetMessage(10));//"Tinh nang hien dang tam khoa, vui long LH Admin !");
		return false;
	}

	if (gObj[aNpcIndex].Class == this->mCH_NPCClass &&
		gObj[aNpcIndex].Map == this->mCH_NPCMap &&
		gObj[aNpcIndex].X == this->mCH_NPCX &&
		gObj[aNpcIndex].Y == this->mCH_NPCY)
	{
		//LogAdd(LOG_RED, "%s TalkNPC %s", gObj[aIndex].Name, gObj[aNpcIndex].Name);
		////=========== Send Open WIndow Cong Huong
		LPOBJ lpUser = &gObj[aIndex];
		XULY_CGPACKET pMsg;
		pMsg.header.set(0xFA, 0x14, sizeof(pMsg));
		pMsg.ThaoTac = 1;	//Index Cua So NPC cong huong
		DataSend(lpUser->Index, (BYTE*)& pMsg, pMsg.header.size);
		//==========
		return 1;
	}
	return 0;
}

void CBCongHuong::CH_MixItemGS(int aIndex, XULY_CGPACKET* lpMsg)
{
	if (!OBJMAX_RANGE(aIndex))
	{
		return;
	}


	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->Type != OBJECT_USER || lpObj->m_OfflineMode != 0 || lpObj->IsFakeOnline != 0 || lpMsg->ThaoTac != 1)
	{
		return;
	}

	if (lpObj->CH_IndexItem[2] == -1)
	{
		gNotice.GCNoticeSend(aIndex, 0, 0, 0, 10, 0, 0, this->GetMessage(6));//"[CongHuong] Vui long bo them phoi item vao");
		CBCongHuong::CH_ClearDB(aIndex, 0);
		CBCongHuong::CH_GSSendInfoClient(aIndex);
		return;
	}
	//==Check Coin
	if (lpObj->Coin1 < this->CH_CoinMix[0])
	{
		gNotice.GCNoticeSend(aIndex, 0, 0, 0, 10, 0, 0, this->GetMessage(0), this->CH_CoinMix[0]);//"[CongHuong] WCoin");
		CBCongHuong::CH_GSSendInfoClient(aIndex);
		return;
	}
	if (lpObj->Coin2 < this->CH_CoinMix[1])
	{
		gNotice.GCNoticeSend(aIndex, 0, 0, 0, 10, 0, 0, this->GetMessage(1), this->CH_CoinMix[1]);//"[CongHuong] WCoinP");
		CBCongHuong::CH_GSSendInfoClient(aIndex);
		return;
	}
	if (lpObj->Coin2 < this->CH_CoinMix[2])
	{
		gNotice.GCNoticeSend(aIndex, 0, 0, 0, 10, 0, 0, this->GetMessage(2), this->CH_CoinMix[2]);//"[CongHuong] GobinP");
		CBCongHuong::CH_GSSendInfoClient(aIndex);
		return;
	}
	//	GDSetCoinSend(lpObj->Index, -this->CH_CoinMix[0], -this->CH_CoinMix[1], -this->CH_CoinMix[2], "CH New - Coin: ");
	GDSetCoinSend(lpObj->Index, (this->CH_CoinMix[0] > 0) ? (-this->CH_CoinMix[0]) : 0, (this->CH_CoinMix[0] > 0) ? (-this->CH_CoinMix[1]) : 0, (this->CH_CoinMix[0] > 0) ? (-this->CH_CoinMix[2]) : 0, "CongHuongNewTruCoin");
	Sleep(1000);
	srand(static_cast<int>(time(NULL)));
	int random = rand() % 100;
	bool failed = false;

	if (random > this->CH_TyLeThanhCong) //Ty le Mix
	{
		failed = true;
	}

	if (!failed)
	{
		//===== Set Item Mix Thanh Cong
		BYTE SlotRecv = gItemManager.InventoryInsertItem(aIndex, gObj[aIndex].CH_InfoItem[2]); // Roll back lai Item
		if (SlotRecv != 0xFF)
		{
			gItemManager.GCItemModifySend(aIndex, SlotRecv);
		}
		//=== Tra Lai phoi
		if (this->CHKeepItem)
		{
			BYTE SlotRecvPhoi = gItemManager.InventoryInsertItem(aIndex, gObj[aIndex].CH_InfoItem[3]); // Roll back lai Item
			if (SlotRecvPhoi != 0xFF)
			{
				gItemManager.GCItemModifySend(aIndex, SlotRecvPhoi);
			}
		}
		gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, "%s Cộng hưởng thành công %s", lpObj->Name, gItemManager.GetItemName(gObj[aIndex].CH_InfoItem[0].m_Index));
		//gNotice.GCNoticeSend(aIndex, 0, 0, 0, 10, 0, 0, this->GetMessage(7));// "[CongHuong] Ban vua Cong Huong thanh cong !");
		gLog.Output(LOG_CONG_HUONG, "[CongHuong][ThanhCong][%s][%s] ItemChinh: Item: %s+%01d, Index %d,Seri %08X, Skil %01d ,Luck %01d, Option %01d , OptionEx %03d, JOH: %03d, Set %03d, SocketOption:%03d, %03d, %03d, %03d, %03d", lpObj->Account, lpObj->Name, gItemManager.GetItemName(gObj[aIndex].CH_InfoItem[0].m_Index), gObj[aIndex].CH_InfoItem[0].m_Level, gObj[aIndex].CH_InfoItem[0].m_Index, gObj[aIndex].CH_InfoItem[0].m_Serial, gObj[aIndex].CH_InfoItem[0].m_Option1, gObj[aIndex].CH_InfoItem[0].m_Option2, gObj[aIndex].CH_InfoItem[0].m_Option3, gObj[aIndex].CH_InfoItem[0].m_NewOption, gObj[aIndex].CH_InfoItem[0].m_JewelOfHarmonyOption, gObj[aIndex].CH_InfoItem[0].m_SetOption, gObj[aIndex].CH_InfoItem[0].m_SocketOption[0], gObj[aIndex].CH_InfoItem[0].m_SocketOption[1], gObj[aIndex].CH_InfoItem[0].m_SocketOption[2], gObj[aIndex].CH_InfoItem[0].m_SocketOption[3], gObj[aIndex].CH_InfoItem[0].m_SocketOption[4]);
		gLog.Output(LOG_CONG_HUONG, "[CongHuong][ThanhCong][%s][%s] Item_Phoi: Item: %s+%01d, Index %d,Seri %08X, Skil %01d ,Luck %01d, Option %01d , OptionEx %03d, JOH: %03d, Set %03d, SocketOption:%03d, %03d, %03d, %03d, %03d", lpObj->Account, lpObj->Name, gItemManager.GetItemName(gObj[aIndex].CH_InfoItem[1].m_Index), gObj[aIndex].CH_InfoItem[1].m_Level, gObj[aIndex].CH_InfoItem[1].m_Index, gObj[aIndex].CH_InfoItem[1].m_Serial, gObj[aIndex].CH_InfoItem[1].m_Option1, gObj[aIndex].CH_InfoItem[1].m_Option2, gObj[aIndex].CH_InfoItem[1].m_Option3, gObj[aIndex].CH_InfoItem[1].m_NewOption, gObj[aIndex].CH_InfoItem[1].m_JewelOfHarmonyOption, gObj[aIndex].CH_InfoItem[1].m_SetOption, gObj[aIndex].CH_InfoItem[1].m_SocketOption[0], gObj[aIndex].CH_InfoItem[1].m_SocketOption[1], gObj[aIndex].CH_InfoItem[1].m_SocketOption[2], gObj[aIndex].CH_InfoItem[1].m_SocketOption[3], gObj[aIndex].CH_InfoItem[1].m_SocketOption[4]);
		gLog.Output(LOG_CONG_HUONG, "[CongHuong][ThanhCong][%s][%s] Item_Nhan: Item: %s+%01d, Index %d,Seri %08X, Skil %01d ,Luck %01d, Option %01d , OptionEx %03d, JOH: %03d, Set %03d, SocketOption:%03d, %03d, %03d, %03d, %03d", lpObj->Account, lpObj->Name, gItemManager.GetItemName(gObj[aIndex].CH_InfoItem[2].m_Index), gObj[aIndex].CH_InfoItem[2].m_Level, gObj[aIndex].CH_InfoItem[2].m_Index, gObj[aIndex].CH_InfoItem[2].m_Serial, gObj[aIndex].CH_InfoItem[2].m_Option1, gObj[aIndex].CH_InfoItem[2].m_Option2, gObj[aIndex].CH_InfoItem[2].m_Option3, gObj[aIndex].CH_InfoItem[2].m_NewOption, gObj[aIndex].CH_InfoItem[2].m_JewelOfHarmonyOption, gObj[aIndex].CH_InfoItem[2].m_SetOption, gObj[aIndex].CH_InfoItem[2].m_SocketOption[0], gObj[aIndex].CH_InfoItem[2].m_SocketOption[1], gObj[aIndex].CH_InfoItem[2].m_SocketOption[2], gObj[aIndex].CH_InfoItem[2].m_SocketOption[3], gObj[aIndex].CH_InfoItem[2].m_SocketOption[4]);
		gLog.Output(LOG_CONG_HUONG, "[CongHuong][ThanhCong][%s][%s] Ty Le: %d\n==", lpObj->Account, lpObj->Name, this->CH_TyLeThanhCong);
	}
	else
	{
		//===== Set Item Chinh Khi Mix Khong Thanh Cong
		BYTE SlotRecv = gItemManager.InventoryInsertItem(aIndex, gObj[aIndex].CH_InfoItem[0]); // Roll back lai Item
		if (SlotRecv != 0xFF)
		{
			gItemManager.GCItemModifySend(aIndex, SlotRecv);

		}
		//=== Tra Lai phoi
		if (this->CHKeepItem)
		{
			BYTE SlotRecvPhoi = gItemManager.InventoryInsertItem(aIndex, gObj[aIndex].CH_InfoItem[3]); // Roll back lai Item
			if (SlotRecvPhoi != 0xFF)
			{
				gItemManager.GCItemModifySend(aIndex, SlotRecvPhoi);
			}
		}
		gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, "%s Cộng Hưởng Thất Bại: %s !", lpObj->Name, gItemManager.GetItemName(gObj[aIndex].CH_InfoItem[0].m_Index));//"[Cộng Hưởng] %s Cộng Hưởng Thành Công !",lpObj->Name);
		//gNotice.GCNoticeSend(aIndex, 0, 0, 0, 10, 0, 0, this->GetMessage(8));//"[CongHuong] Rat tiec, ban da that bai");
		gLog.Output(LOG_CONG_HUONG, "[CongHuong][ThatBai][%s][%s] ItemChinh: Item: %s+%01d, Index %d,Seri %08X, Skil %01d ,Luck %01d, Option %01d , OptionEx %03d, JOH: %03d, Set %03d, SocketOption:%03d, %03d, %03d, %03d, %03d", lpObj->Account, lpObj->Name, gItemManager.GetItemName(gObj[aIndex].CH_InfoItem[0].m_Index), gObj[aIndex].CH_InfoItem[0].m_Level, gObj[aIndex].CH_InfoItem[0].m_Index, gObj[aIndex].CH_InfoItem[0].m_Serial, gObj[aIndex].CH_InfoItem[0].m_Option1, gObj[aIndex].CH_InfoItem[0].m_Option2, gObj[aIndex].CH_InfoItem[0].m_Option3, gObj[aIndex].CH_InfoItem[0].m_NewOption, gObj[aIndex].CH_InfoItem[0].m_JewelOfHarmonyOption, gObj[aIndex].CH_InfoItem[0].m_SetOption, gObj[aIndex].CH_InfoItem[0].m_SocketOption[0], gObj[aIndex].CH_InfoItem[0].m_SocketOption[1], gObj[aIndex].CH_InfoItem[0].m_SocketOption[2], gObj[aIndex].CH_InfoItem[0].m_SocketOption[3], gObj[aIndex].CH_InfoItem[0].m_SocketOption[4]);
		gLog.Output(LOG_CONG_HUONG, "[CongHuong][ThatBai][%s][%s] Item_Phoi: Item: %s+%01d, Index %d,Seri %08X, Skil %01d ,Luck %01d, Option %01d , OptionEx %03d, JOH: %03d, Set %03d, SocketOption:%03d, %03d, %03d, %03d, %03d", lpObj->Account, lpObj->Name, gItemManager.GetItemName(gObj[aIndex].CH_InfoItem[1].m_Index), gObj[aIndex].CH_InfoItem[1].m_Level, gObj[aIndex].CH_InfoItem[1].m_Index, gObj[aIndex].CH_InfoItem[1].m_Serial, gObj[aIndex].CH_InfoItem[1].m_Option1, gObj[aIndex].CH_InfoItem[1].m_Option2, gObj[aIndex].CH_InfoItem[1].m_Option3, gObj[aIndex].CH_InfoItem[1].m_NewOption, gObj[aIndex].CH_InfoItem[1].m_JewelOfHarmonyOption, gObj[aIndex].CH_InfoItem[1].m_SetOption, gObj[aIndex].CH_InfoItem[1].m_SocketOption[0], gObj[aIndex].CH_InfoItem[1].m_SocketOption[1], gObj[aIndex].CH_InfoItem[1].m_SocketOption[2], gObj[aIndex].CH_InfoItem[1].m_SocketOption[3], gObj[aIndex].CH_InfoItem[1].m_SocketOption[4]);
		gLog.Output(LOG_CONG_HUONG, "[CongHuong][ThatBai][%s][%s] Ty Le: %d\n==", lpObj->Account, lpObj->Name, this->CH_TyLeThanhCong);

	}
	CBCongHuong::CH_ClearDB(aIndex, 0, 0);
	CBCongHuong::CH_GSSendInfoClient(aIndex);
}

char* CBCongHuong::GetMessage(int index) // OK
{
	std::map<int, MESSAGE_CH>::iterator it = this->m_MessageInfo.find(index);

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
#endif
