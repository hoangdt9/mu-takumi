#include "stdafx.h"
#include "CustomRankUser.h"
#include "NewUISystem.h"
#include "CBInterface.h"
#include "CUIController.h"
#include "CharacterManager.h"
#include "Util.h"
#include "Protocol.h"
#include "NewUIBase.h"
#include "Other.h"
#include "Ranking.h"
#include "CBInterfaceVIPChar.h"
#include "GOBoid.h"
CCustomRankUser gCustomRankUser;

CCustomRankUser::CCustomRankUser()
{
	this->m_DataRank.clear();
}

CCustomRankUser::~CCustomRankUser()
{

}
int CCustomRankUser::GetNumberData(int KeyIndex)
{
	if (KeyIndex < 0) return -1;
	for (int i = 0; i < this->m_DataRank.size(); i++)
	{
		if (this->m_DataRank[i].m_Index == KeyIndex)
		{
			return i;
		}
	}
	return -1;
}

void CCustomRankUser::GCReqRankLevelUser(SEND_COUNTLIST* lpMsg)
{
	if (lpMsg == NULL)
	{
		return;
	}
	for (int n = 0; n < lpMsg->Count; n++)
	{
		PMSG_CUSTOM_RANKUSER* aRecv = (PMSG_CUSTOM_RANKUSER*)(((BYTE*)lpMsg) + sizeof(SEND_COUNTLIST) + (sizeof(PMSG_CUSTOM_RANKUSER) * n));
		// ---
		CUSTOM_RANKUSER_DATA info = { 0 };

		info.m_Index = aRecv->iIndex;
		info.m_Level = aRecv->iLevel;
		info.m_Resets = aRecv->iReset;
		info.m_MResets = aRecv->iMReset;
		info.m_Kills = aRecv->iKills;
		info.m_Deads = aRecv->iDeads;
		info.m_For = aRecv->iFor;
		info.m_Agi = aRecv->iAgi;
		info.m_Ene = aRecv->iEne;
		info.m_Vit = aRecv->iVit;
		info.m_Com = aRecv->iCom;
		info.m_Classe = aRecv->iClasse;
		info.m_Rank = aRecv->iRank;
#if DANH_HIEU_NEW
		info.m_rDanhHieu = aRecv->m_rDanhHieu;
#endif
#if (CB_VIP_CHAR)
		info.mLvVIPChar = aRecv->mLvVIPChar;
#endif

		memcpy(info.szTitleRank, aRecv->szName, sizeof(info.szTitleRank));
		memcpy(info.m_Vip, aRecv->iVip, sizeof(info.m_Vip));

		int NumberData = this->GetNumberData(info.m_Index);
		if (NumberData == -1)
		{
			this->m_DataRank.push_back(info);
		}
		else
		{
			this->m_DataRank[NumberData] = info;
		}
		auto it = std::find_if(m_DataRank.begin(), m_DataRank.end(), [&](const CUSTOM_RANKUSER_DATA & data) {return data.m_Index == info.m_Index; });
		if (it != m_DataRank.end())
		{
			*it = info;
		}
		else
		{
			this->m_DataRank.push_back(info);
		}
	}
}
int transformValue(int value) {
	return 20 - value;
}
void CCustomRankUser::DrawInfo(int aIndex, float X, float Y, float W, float H)
{
#if DANH_HIEU_NEW
	int NumberData = this->GetNumberData(aIndex);
	if (NumberData == -1) return;
	int v1 = this->m_DataRank[NumberData].m_rDanhHieu % 10;
	int NumberDH = (this->m_DataRank[NumberData].m_rDanhHieu - (v1)) / 10;
#endif
	int FixYRank = 0;
#if(CB_VIP_CHAR)
	//===Show Rank VIP Char
	if (this->m_DataRank[NumberData].mLvVIPChar > 0 && gBInterfaceVIPChar)
	{
		float VipCharPosX = X + (W / 2) - 50;
		float VipCharPosY = Y - (30 + FixYRank);
		int LevelVipChar = this->m_DataRank[NumberData].mLvVIPChar;
		std::map<int, CBInterfaceVIPChar::CUSTOMVIPCHAR_DATA>::iterator it = gBInterfaceVIPChar->m_VipCharRank.find(LevelVipChar);
		if (it != gBInterfaceVIPChar->m_VipCharRank.end())
		{
			//=== Rank Text Or Img
			if (it->second.TypeRank == 1)
			{
				int SizeTextFont = it->second.SizeW;
				HFONT CHFontVIPRank = CreateFontA(SizeTextFont, 0, 0, 0, 400, 0, 0, 0, 0x1, 0, 0, 3u, 0, "Tahoma");
				TextDraw(CHFontVIPRank, (float)VipCharPosX-50 - it->second.X, (float)VipCharPosY - it->second.Y, it->second.ColorText, 0x0, 200, 0, 3, it->second.TextDir);//
				DeleteObject(CHFontVIPRank);
				FixYRank += 20;
			}
			else if (it->second.TypeRank == 2)
			{
				EnableAlphaTest(true);
				glColor3f(1.0, 1.0, 1.0);
				RenderBitmap(150000 + LevelVipChar, (float)VipCharPosX - it->second.X, (float)VipCharPosY - it->second.Y, it->second.SizeW, it->second.H, 0.0, 0.0, 1.0, 1.0, 1, 1, 0.0);
				glColor3f(1.0f, 1.0f, 1.0f); //Xoa mau
				EnableAlphaTest(0);
				FixYRank += it->second.H;
			}
		}
		
	}
#endif

#if DANH_HIEU_NEW
	//===Show Danh Hieu Mua
	float RankStartX = X + (W / 2) - 45; // Gốc 50
	float RankStartY = Y - (50+ FixYRank);
	if (NumberDH > 0 && mShowDanhHieu)
	{
		EnableAlphaTest(true);
		glColor3f(1.0, 1.0, 1.0);
		RenderBitmap(gInterface.Data[eDanhImg + NumberDH - 1].ModelID, RankStartX, RankStartY, 100, 45, 0, 0, 0.7, 1.0, 1, 1, 0.0);
		glColor3f(1.0f, 1.0f, 1.0f); //Xoa mau
		EnableAlphaTest(0);
		FixYRank += 15;
	}
#endif
}