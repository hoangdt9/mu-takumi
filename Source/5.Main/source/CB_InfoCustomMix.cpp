#include "StdAfx.h"
#include "CB_InfoCustomMix.h"
#include "NewUISystem.h"
#include "CBInterface.h"
#include "CUIController.h"
#include "CharacterManager.h"
#include "Util.h"
#include "Protocol.h"
#include "NewUIBase.h"
#include "Other.h"
#include "ZzzInterface.h"
#include "MixMgr.h"

#if(CB_CUSTOMMIXINFO)

CB_InfoCustomMix* gCB_InfoCustomMix;

CB_InfoCustomMix::CB_InfoCustomMix()
{
	this->m_DataInfoCustomMix.clear();
}

CB_InfoCustomMix::~CB_InfoCustomMix()
{
}
void CB_InfoCustomMix::AddItemToMixItemInventory(ITEM* pItem)
{
	//if ((int)g_MixRecipeMgr.GetMostSimilarRecipe() != 0)
	//{
	//	this->m_DataInfoCustomMix.clear();
	//}
	this->m_DataListItemMixInv.push_back(pItem);
	
}

void CB_InfoCustomMix::RecvInfo(PMSG_BCUSTOM_MIX_INFO* lpMsg)
{
	this->m_DataInfoCustomMix.clear();

	if (lpMsg->Index == -1 || g_MixRecipeMgr.GetMixInventoryType() != SEASON3A::MIXTYPE_GOBLIN_NORMAL) return;


	LOCAL_DATA_INFOCUSTOMMIX info;
	info.Index = lpMsg->Index;
	info.MixMoney = lpMsg->MixMoney;
	info.MixRate = lpMsg->MixRate;
	info.Name1= lpMsg->Name1;
	info.Name2= lpMsg->Name2;
	info.Name3= lpMsg->Name3;
	info.Des1= lpMsg->Des1;
	info.Des2= lpMsg->Des2;
	info.Des3= lpMsg->Des3;
	info.Advice1= lpMsg->Advice1;
	info.Advice2= lpMsg->Advice2;
	info.Advice3= lpMsg->Advice3;

	info.m_DataItemCheck.clear();

	for (int n = 0; n < lpMsg->count; n++)
	{
		BCUSTOM_MIX_NGUYENLIEU* lpInfo = (BCUSTOM_MIX_NGUYENLIEU*)(((BYTE*)lpMsg) + sizeof(PMSG_BCUSTOM_MIX_INFO) + (sizeof(BCUSTOM_MIX_NGUYENLIEU) * n));

		//gInterface.DrawMessage(1, "Get %d ~ %d", lpInfo->ItemIndexMin, lpInfo->ItemIndexMax);
		info.m_DataItemCheck.push_back(*lpInfo);
	}

	this->m_DataInfoCustomMix.push_back(info);
}

bool CB_InfoCustomMix::CheckCustomMixOK()
{
	if (!this->m_DataInfoCustomMix.empty())
	{
		if (this->CheckCountMix == this->m_DataInfoCustomMix[0].m_DataItemCheck.size()) return 1;
	}

	return 0;
}
int CB_InfoCustomMix::GetSourceName(int iItemNum, unicode::t_char* pszNameOut)
{
	//gInterface.DrawMessage(1, "m_DataListItemMixInv %d %d ~ %d", iItemNum, this->m_DataListItemMixInv.size(), this->m_DataInfoCustomMix[0].m_DataItemCheck.size());


	
	int GetKQResut = (int)SEASON3A::MIX_SOURCE_NO;
	if (!this->m_DataInfoCustomMix.empty())
	{

		unicode::t_char szTempName[100];
		//gInterface.DrawMessage(1, "Get %d ~ %d", this->m_DataListItemMixInv[0]->Type, this->m_DataListItemMixInv[0]->Level);
		int GetNameIndex = this->m_DataInfoCustomMix[0].m_DataItemCheck[iItemNum].ItemIndexMin;

		for (int i = 0; i < this->m_DataListItemMixInv.size(); i++)
		{
			if (this->m_DataListItemMixInv[i]->Type < this->m_DataInfoCustomMix[0].m_DataItemCheck[iItemNum].ItemIndexMin) continue;
			if (this->m_DataListItemMixInv[i]->Type > this->m_DataInfoCustomMix[0].m_DataItemCheck[iItemNum].ItemIndexMax) continue;
			if (this->m_DataListItemMixInv[i]->Level < this->m_DataInfoCustomMix[0].m_DataItemCheck[iItemNum].Level) continue;
			if (this->m_DataListItemMixInv[i]->option_380 != this->m_DataInfoCustomMix[0].m_DataItemCheck[iItemNum].Item380) continue;
			if ((this->m_DataListItemMixInv[i]->ExtOption % 0x04) != this->m_DataInfoCustomMix[0].m_DataItemCheck[iItemNum].Set) continue;
			if ((getNumberOfExcOptions(this->m_DataListItemMixInv[i]->Option1)) < this->m_DataInfoCustomMix[0].m_DataItemCheck[iItemNum].Exl) continue;

			if (GetNameIndex != this->m_DataListItemMixInv[i]->Type) GetNameIndex = this->m_DataListItemMixInv[i]->Type;

			//gInterface.DrawMessage(1, "GetNameIndex %d ~ %d (%d/%d)", GetNameIndex , this->m_DataListItemMixInv[i]->Type, this->m_DataListItemMixInv[i]->Level , this->m_DataInfoCustomMix[0].m_DataItemCheck[iItemNum].Level);

			GetKQResut = SEASON3A::MIX_SOURCE_YES;
			this->CheckCountMix++;

			break;
		}
		GetItemName(GetNameIndex, this->m_DataInfoCustomMix[0].m_DataItemCheck[iItemNum].Level, szTempName);

		//===So luong
		if (this->m_DataInfoCustomMix[0].m_DataItemCheck[iItemNum].Count > 1)
		{
			unicode::_sprintf(szTempName, "%s %d%s %s", szTempName, this->m_DataInfoCustomMix[0].m_DataItemCheck[iItemNum].Count, GlobalText[2345], GlobalText[2341]);
		}
		else
		{
			unicode::_sprintf(szTempName, "%s %d%s %s", szTempName, this->m_DataInfoCustomMix[0].m_DataItemCheck[iItemNum].Count, GlobalText[2345], GlobalText[2342]);
		}
		//==Tinh nang
		if (this->m_DataInfoCustomMix[0].m_DataItemCheck[iItemNum].Op > 1)
		{
			unicode::_sprintf(szTempName, "%s %d%s %s", szTempName, this->m_DataInfoCustomMix[0].m_DataItemCheck[iItemNum].Op * 4, GlobalText[2343], GlobalText[2341]);
		}

		if (this->m_DataInfoCustomMix[0].m_DataItemCheck[iItemNum].Item380)
		{
			unicode::_sprintf(szTempName, GlobalText[2335]);
		}

		if (this->m_DataInfoCustomMix[0].m_DataItemCheck[iItemNum].Exl)
		{
			unicode::_sprintf(pszNameOut, "%s %s", GlobalText[620], szTempName);

		}
		if (this->m_DataInfoCustomMix[0].m_DataItemCheck[iItemNum].Set)
		{
			unicode::_sprintf(pszNameOut, "%s %s", GlobalText[1089], szTempName);

		}
		if (this->m_DataInfoCustomMix[0].m_DataItemCheck[iItemNum].Harmory)
		{
			unicode::_sprintf(pszNameOut, "%s %s", GlobalText[1550], szTempName);

		}
		if (this->m_DataInfoCustomMix[0].m_DataItemCheck[iItemNum].Socket)
		{
			unicode::_sprintf(pszNameOut, "%s %s", GlobalText[2650], szTempName);
		}

		unicode::_strcpy(pszNameOut, szTempName);

	}
	//if (this->m_DataListItemMixInv.empty() || iItemNum >= this->m_DataListItemMixInv.size()) return SEASON3A::MIX_SOURCE_NO;
	//gInterface.DrawMessage(1, "CheckCustomMixOK() %d", this->CheckCustomMixOK());
	return GetKQResut;
}
BOOL CB_InfoCustomMix::GetCurRecipeDesc(unicode::t_char* pszDescOut, int iDescLine)
{
	if (!gCB_InfoCustomMix->m_DataInfoCustomMix.empty())
	{
		switch (iDescLine)
		{
		case 1: 
		{
			if (gCB_InfoCustomMix->m_DataInfoCustomMix[0].Des1 == 0) return 0;
			unicode::_sprintf(pszDescOut, GlobalText[gCB_InfoCustomMix->m_DataInfoCustomMix[0].Des1]);
		}
		break;
		case 2: 
		{
			if (gCB_InfoCustomMix->m_DataInfoCustomMix[0].Des2 == 0) return 0;
			unicode::_sprintf(pszDescOut, GlobalText[gCB_InfoCustomMix->m_DataInfoCustomMix[0].Des1]);
		}
		break;
		case 3: 
		{
			if (gCB_InfoCustomMix->m_DataInfoCustomMix[0].Des3 == 0) return 0;
			unicode::_sprintf(pszDescOut, GlobalText[gCB_InfoCustomMix->m_DataInfoCustomMix[0].Des1]);
		}
		break;
		default:
			break;
		}
		
	}
	return 0;
}
BOOL CB_InfoCustomMix::GetRecipeAdvice(unicode::t_char* pszDescOut, int iDescLine)
{
	if (!gCB_InfoCustomMix->m_DataInfoCustomMix.empty())
	{
		switch (iDescLine)
		{
		case 1:
		{
			if (gCB_InfoCustomMix->m_DataInfoCustomMix[0].Advice1 == 0) return 0;
			unicode::_sprintf(pszDescOut, GlobalText[gCB_InfoCustomMix->m_DataInfoCustomMix[0].Advice1]);
		}
		break;
		case 2:
		{
			if (gCB_InfoCustomMix->m_DataInfoCustomMix[0].Advice2 == 0) return 0;
			unicode::_sprintf(pszDescOut, GlobalText[gCB_InfoCustomMix->m_DataInfoCustomMix[0].Advice2]);
		}
		break;
		case 3:
		{
			if (gCB_InfoCustomMix->m_DataInfoCustomMix[0].Advice3 == 0) return 0;
			unicode::_sprintf(pszDescOut, GlobalText[gCB_InfoCustomMix->m_DataInfoCustomMix[0].Advice3]);
		}
		break;
		default:
			break;
		}

	}
	return 0;
}
#endif