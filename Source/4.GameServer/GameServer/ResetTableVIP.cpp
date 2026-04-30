// ResetTableVIP.cpp: implementation of the CResetTableVIP class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ResetTableVIP.h"
#if(CB_AUTORESETINFO)
#include "ResetTable.h"
#include "MemScript.h"
#include "ServerInfo.h"
#include "Util.h"
#include "ItemManager.h"
#include "Notice.h"
#include "Message.h"
#include "User.h"
#include "DSProtocol.h"

//#include "CustomGHRS.h"
//#include "BCustomItemBank.h"
CResetTableVIP gResetTableVip;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CResetTableVIP::CResetTableVIP() // OK
{
	this->m_AutoResetVIPInfo.clear();
}

CResetTableVIP::~CResetTableVIP() // OK
{

}

void CResetTableVIP::Load(char* path) // OK
{
	pugi::xml_document file;
	pugi::xml_parse_result res = file.load_file(path);
	if (res.status != pugi::status_ok)
	{
		ErrorMessageBox("File %s load fail. Error: %s", path, res.description());
		return;
	}
	//=== Load MSG
	this->m_MessageInfoBP.clear();
	pugi::xml_node Message = file.child("MessageInfo");
	for (pugi::xml_node msg = Message.child("Message"); msg; msg = msg.next_sibling())
	{
		MESSAGE_INFO_RSVIP info;

		info.Index = msg.attribute("Index").as_int();

		strcpy_s(info.Message, msg.attribute("Text").as_string());//

		this->m_MessageInfoBP.insert(std::pair<int, MESSAGE_INFO_RSVIP>(info.Index, info));
	}
	//==AutoResetLoad
	this->m_AutoResetVIPInfo.clear();
	this->AutoResetVIPEnable = 0;
	pugi::xml_node pAutoResetTableVIP = file.child("ResetTableVIP");
	this->AutoResetVIPEnable = pAutoResetTableVIP.attribute("Enable").as_int();
	this->AutoResetVIPNotice = pAutoResetTableVIP.attribute("Notice").as_int();
	this->AutoResetVIPType = pAutoResetTableVIP.attribute("TypeReset").as_int();
	for (pugi::xml_node oReset = pAutoResetTableVIP.child("Reset"); oReset; oReset = oReset.next_sibling())
	{
		AutoResetVIPInfo infoReset;
		infoReset.MinReset = oReset.attribute("MinReset").as_int();
		infoReset.MaxReset = oReset.attribute("MaxReset").as_int();
		//==
		pugi::xml_node oLevelReset = oReset.child("LevelReset");
		infoReset.LevelReset[0] = oLevelReset.attribute("Level_AL0").as_int();
		infoReset.LevelReset[1] = oLevelReset.attribute("Level_AL1").as_int();
		infoReset.LevelReset[2] = oLevelReset.attribute("Level_AL2").as_int();
		infoReset.LevelReset[3] = oLevelReset.attribute("Level_AL3").as_int();
		//==
		pugi::xml_node oPointReset = oReset.child("PointReset");
		infoReset.PointReset[0] = oPointReset.attribute("Point_AL0").as_int();
		infoReset.PointReset[1] = oPointReset.attribute("Point_AL1").as_int();
		infoReset.PointReset[2] = oPointReset.attribute("Point_AL2").as_int();
		infoReset.PointReset[3] = oPointReset.attribute("Point_AL3").as_int();
		//==
		pugi::xml_node oNguyenLieu = oReset.child("NguyenLieu");
		infoReset.Zen = oNguyenLieu.attribute("Zen").as_int();
		infoReset.WC = oNguyenLieu.attribute("WC").as_int();
		infoReset.WP = oNguyenLieu.attribute("WP").as_int();
		infoReset.GP = oNguyenLieu.attribute("GP").as_int();
		infoReset.CheckCode = oNguyenLieu.attribute("CheckCode").as_int();
		infoReset.m_ItemCheck.clear();
		for (pugi::xml_node oItemCheck = oNguyenLieu.child("Item"); oItemCheck; oItemCheck = oItemCheck.next_sibling())
		{
			VNGUYENLIEUITEM_DATA infoItemCheck;
			infoItemCheck.SL = oItemCheck.attribute("SL").as_int();
			infoItemCheck.Type = oItemCheck.attribute("Type").as_int();
			infoItemCheck.IndexItem = oItemCheck.attribute("IndexItem").as_int();
			infoItemCheck.Level = oItemCheck.attribute("Level").as_int();
			infoItemCheck.Skill = oItemCheck.attribute("Skill").as_int();
			infoItemCheck.Luck = oItemCheck.attribute("Luck").as_int();
			infoItemCheck.Option = oItemCheck.attribute("Option").as_int();
			infoItemCheck.ExlOp = oItemCheck.attribute("ExlOp").as_int();
			infoReset.m_ItemCheck.push_back(infoItemCheck);
		}
		this->m_AutoResetVIPInfo.push_back(infoReset);
	}


	LogAdd(LOG_BLUE, "[ResetTableVIP]Enable (%d) Load Config  %d", this->AutoResetVIPEnable, this->m_AutoResetVIPInfo.size());
}


char* CResetTableVIP::GetMessage(int index) // OK
{
	std::map<int, MESSAGE_INFO_RSVIP>::iterator it = this->m_MessageInfoBP.find(index);

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
int CResetTableVIP::GetStatusCheckCode(LPOBJ lpObj) // OK
{
	for (std::vector<AutoResetVIPInfo>::iterator it = this->m_AutoResetVIPInfo.begin(); it != this->m_AutoResetVIPInfo.end(); it++)
	{
		if ((lpObj->Reset + 1) >= it->MinReset && (lpObj->Reset + 1) <= it->MaxReset)
		{
			return it->CheckCode;
		}
	}

	return 0;
}

int CResetTableVIP::GetResetLevel(LPOBJ lpObj) // OK
{

	for (std::vector<AutoResetVIPInfo>::iterator it = this->m_AutoResetVIPInfo.begin(); it != this->m_AutoResetVIPInfo.end(); it++)
	{
		if ((lpObj->Reset + 1) >= it->MinReset && (lpObj->Reset + 1) <= it->MaxReset)
		{
			if (it->LevelReset[lpObj->AccountLevel] == -1)
			{
				return gServerInfo.m_CommandResetLevel[lpObj->AccountLevel];
			}
			else
			{
				return it->LevelReset[lpObj->AccountLevel];
			}
		}
	}

	return gServerInfo.m_CommandResetLevel[lpObj->AccountLevel];
}

int CResetTableVIP::GetResetMoney(LPOBJ lpObj) // OK
{
	for (std::vector<AutoResetVIPInfo>::iterator it = this->m_AutoResetVIPInfo.begin(); it != this->m_AutoResetVIPInfo.end(); it++)
	{
		if ((lpObj->Reset + 1) >= it->MinReset && (lpObj->Reset + 1) <= it->MaxReset)
		{
			if (it->Zen == -1)
			{
				return gServerInfo.m_CommandResetMoney[lpObj->AccountLevel];
			}
			else
			{
				return it->Zen;
			}
		}
	}

	return 0;
}

int CResetTableVIP::GetResetPoint(LPOBJ lpObj, bool GetPointMasterReset) // OK
{
	int point = 0;

	for (int n = 1; n <= lpObj->Reset + 1; n++)
	{
		int AddPoint = gServerInfo.m_CommandResetPoint[lpObj->AccountLevel];

		for (std::vector<AutoResetVIPInfo>::iterator it = this->m_AutoResetVIPInfo.begin(); it != this->m_AutoResetVIPInfo.end(); it++)
		{
			if (n >= it->MinReset && n <= it->MaxReset)
			{
				if (it->PointReset[lpObj->AccountLevel] == -1)
				{
					AddPoint = gServerInfo.m_CommandResetPoint[lpObj->AccountLevel];
					break;
				}
				else
				{
					AddPoint = it->PointReset[lpObj->AccountLevel];
					break;
				}
			}
		}

		point += AddPoint;
	}
	//=== Point Relife
	if (GetPointMasterReset)
	{
		if (gServerInfo.m_CommandMasterResetType == 1)
		{
			point += gServerInfo.m_CommandMasterResetPoint[lpObj->AccountLevel] * lpObj->MasterReset;
		}
	}
	return point;
}
int CResetTableVIP::GetResetPointKeep(LPOBJ lpObj) // OK
{

	for (std::vector<AutoResetVIPInfo>::iterator it = this->m_AutoResetVIPInfo.begin(); it != this->m_AutoResetVIPInfo.end(); it++)
	{
		if ((lpObj->Reset + 1) >= it->MinReset && (lpObj->Reset + 1) <= it->MaxReset)
		{
			if (it->PointReset[lpObj->AccountLevel] == -1)
			{
				return gServerInfo.m_CommandResetPoint[lpObj->AccountLevel];
			}
			else
			{
				return it->PointReset[lpObj->AccountLevel];
			}

		}
	}

	return 0;
}
AutoResetVIPInfo* CResetTableVIP::GetInfoAutoReset(LPOBJ lpObj)
{
	if (OBJECT_RANGE(lpObj->Index) == 0)
	{
		return 0;
	}
	if (lpObj->Type != OBJECT_USER)
	{
		return 0;
	}
	for (std::vector<AutoResetVIPInfo>::iterator it = this->m_AutoResetVIPInfo.begin(); it != this->m_AutoResetVIPInfo.end(); it++)
	{
		if ((lpObj->Reset + 1) >= it->MinReset && (lpObj->Reset + 1) <= it->MaxReset)
		{
			return &(*it);
		}
	}
	return 0;

}


bool CResetTableVIP::CheckItemNguyenLieu(LPOBJ lpObj, std::vector<VNGUYENLIEUITEM_DATA> DataNhomNguyenLieu, bool ClearItem)
{
	bool KQCheck = true;

	for (std::vector<VNGUYENLIEUITEM_DATA>::iterator it = DataNhomNguyenLieu.begin(); it != DataNhomNguyenLieu.end(); it++)
	{
		if (it->SL > 0)
		{
			//-- Check Bank
			//if (it->Type == 1)
			//{
			//	if (it->SL > gBCustomItemBank.GetCountItemBank(lpObj->Index, it->IndexItem, it->Level))
			//	{
			//		gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, this->GetMessage(1), it->SL, gItemManager.GetItemName(it->IndexItem));
			//		KQCheck = false;
			//		return KQCheck;
			//	}
			//}
			//else
			{
				if (it->SL > GetItemCountInInventory(lpObj->Index, it->IndexItem, it->Level, it->ExlOp, it->Skill, it->Luck, it->Option))
				{
					gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, this->GetMessage(1), it->SL, gItemManager.GetItemName(it->IndexItem));
					KQCheck = false;
					return KQCheck;
				}
			}
		}

	}
	if (ClearItem)
	{
		//==Xoa Item
		if (KQCheck)
		{
			for (std::vector<VNGUYENLIEUITEM_DATA>::iterator it = DataNhomNguyenLieu.begin(); it != DataNhomNguyenLieu.end(); it++)
			{
				if (it->SL > 0)
				{
					//-- Check Bank
					//if (it->Type == 1)
					//{
					//	gBCustomItemBank.CongTruItemBankTheoItem(lpObj->Index, it->IndexItem, it->Level, -(it->SL));
					//}
					//else
					{
						DeleteItemsCount(lpObj->Index, it->IndexItem, it->Level, it->SL, it->ExlOp, it->Skill, it->Luck, it->Option);
					}
				}
			}
		}
	}
	return KQCheck;
}

void CResetTableVIP::SendInfoReset(LPOBJ lpObj)
{
	if (gObjIsConnected(lpObj->Index) == 0)
	{
		return;
	}
	AutoResetVIPInfo* GetInfoReset = this->GetInfoAutoReset(lpObj);
	if (GetInfoReset)
	{
		//LogAdd(LOG_RED, "SendInfoResetVIP [%s]", lpObj->Name);
		BYTE send[8192];

		DKRS_COUNTLIST pMsg;

		pMsg.header.set(0xD3, 0x36, 0);

		int size = sizeof(pMsg);

		pMsg.Count = 0;
		//==
		pMsg.ResetView = lpObj->Reset;

		pMsg.ResetDay = lpObj->Reset;
		pMsg.MaxGHRS = gServerInfo.GHRSMax;

		pMsg.AutoResetEnable = lpObj->AutoResetEnable;

		//==
		pMsg.Level = GetInfoReset->LevelReset[lpObj->AccountLevel];
		pMsg.LoaiReset = gServerInfo.m_CommandResetType;
		pMsg.Point = (gServerInfo.m_CommandResetType == 1 ? this->GetResetPoint(lpObj) : this->GetResetPointKeep(lpObj));
		pMsg.Zen = GetInfoReset->Zen;
		pMsg.WC = GetInfoReset->WC;
		pMsg.WP = GetInfoReset->WP;
		pMsg.GP = GetInfoReset->GP;
		pMsg.TypeReset = 2;//RSVIP == 1
		pMsg.CodeReset = GetInfoReset->CheckCode;
		pMsg.OnResetType = (gResetTable.AutoResetThuongEnable ? 1 : 0) + (gResetTableVip.AutoResetVIPEnable ? 2 : 0);
		//==Get point def
		for (int i = 0; i < 5; i++)
		{
			pMsg.DefStat[i] = gDefaultClassInfo.GetCharacterDefaultStat(lpObj->Class, i);
		}
		for (int nItem = 0; nItem < (GetInfoReset->m_ItemCheck.size() > 5 ? 5 : GetInfoReset->m_ItemCheck.size()); nItem++)
		{
			if (GetInfoReset->m_ItemCheck[nItem].IndexItem < 0) continue;
			CItem m_ItemCache;
			m_ItemCache.m_Level = GetInfoReset->m_ItemCheck[nItem].Level;
			m_ItemCache.m_Durability = (float)(gItemManager.GetItemDurability(GetInfoReset->m_ItemCheck[nItem].IndexItem, GetInfoReset->m_ItemCheck[nItem].Level, GetInfoReset->m_ItemCheck[nItem].ExlOp, 0));
			m_ItemCache.Convert(GetInfoReset->m_ItemCheck[nItem].IndexItem, (BYTE)GetInfoReset->m_ItemCheck[nItem].Skill, (BYTE)GetInfoReset->m_ItemCheck[nItem].Luck, (BYTE)GetInfoReset->m_ItemCheck[nItem].Option, (BYTE)GetInfoReset->m_ItemCheck[nItem].ExlOp, 0, 0, 0, 0, 0xFF);

			DKRS_ITEMINFO info = { 0 };
			info.SL = GetInfoReset->m_ItemCheck[nItem].SL;
			int CountCheck = (int)GetItemCountInInventory(lpObj->Index, m_ItemCache.m_Index, m_ItemCache.m_Level, m_ItemCache.m_NewOption, m_ItemCache.m_Option1, m_ItemCache.m_Option2, m_ItemCache.m_Option3);

			gItemManager.ItemByteConvert(info.Item, m_ItemCache); // Set Info Item
			if (CountCheck >= info.SL)
			{

				info.Status = true;

			}
			else
			{

				info.Status = false;
			}
			if (size + sizeof(info) > 8192) break;

			pMsg.Count++;
			memcpy(&send[size], &info, sizeof(info));
			size += sizeof(info);
		}

		pMsg.header.size[0] = SET_NUMBERHB(size);
		pMsg.header.size[1] = SET_NUMBERLB(size);
		// ---
		memcpy(send, &pMsg, sizeof(pMsg));

		DataSend(lpObj->Index, send, size);
	}

}
bool CResetTableVIP::KiemTraDieuKienReset(LPOBJ lpObj, bool ClearNguyenLieu) // OK
{
	for (std::vector<AutoResetVIPInfo>::iterator it = this->m_AutoResetVIPInfo.begin(); it != this->m_AutoResetVIPInfo.end(); it++)
	{
		if ((lpObj->Reset + 1) >= it->MinReset && (lpObj->Reset + 1) <= it->MaxReset)
		{
			int TruCoin1 = 0;
			int TruCoin2 = 0;
			int TruCoin3 = 0;
			TruCoin1 = it->WC;
			TruCoin2 = it->WP;
			TruCoin3 = it->GP;
			//==Check Coin

			if (lpObj->Money < ((DWORD)this->GetResetMoney(lpObj)))
			{
				gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, this->GetMessage(0), it->Zen, "Zen");
				//LogAdd(LOG_RED, "[RS] Check 6");
				return false;
			}
			if (it->WC > 0 && it->WC > lpObj->Coin1) //WC
			{

				gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, this->GetMessage(0), it->WC, "WC");
				return false;
			}
			if (it->WP > 0 && it->WP > lpObj->Coin2) //WC
			{

				gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, this->GetMessage(0), it->WP, "WP");
				return false;
			}
			if (it->GP > 0 && it->GP > lpObj->Coin3) //WC
			{

				gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, this->GetMessage(0), it->GP, "GP");
				return false;
			}
			//=============================
			//==Check Item Custom
			if (!this->CheckItemNguyenLieu(lpObj, it->m_ItemCheck, ClearNguyenLieu)) // ItemCustom
			{
				return false;
			}
			if (ClearNguyenLieu)
			{
				lpObj->Money -= ((DWORD)this->GetResetMoney(lpObj));

				GCMoneySend(lpObj->Index, lpObj->Money);

				GDSetCoinSend(lpObj->Index, -(TruCoin1), -(TruCoin2), -(TruCoin3), "TruCoinResetVIP");
				//GDSetCoinSend(lpObj->Index, -(TruCoin1), -(TruCoin2), -(TruCoin3),0, "TruCoinResetVIP");
				if (this->AutoResetVIPNotice)
				{
					gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, this->GetMessage(3), lpObj->Name, (int)(lpObj->Reset));

				}
			}
			return true;
		}

	}
	return false;
}
#endif