#include "StdAfx.h"
#include "CB_OffMuHelper.h"
#include "NewUISystem.h"
#include "CBInterface.h"
#include "CUIController.h"
#include "CharacterManager.h"
#include "Util.h"
#include "Protocol.h"
#include "NewUIBase.h"
#include "Other.h"
#include "ZzzInterface.h"

#if(CB_OFFMUHELPER)
CB_OffMuHelper* gCB_OffMuHelper;

CB_OffMuHelper::CB_OffMuHelper()
{
	this->ActiveOfflineMode = false;
}


CB_OffMuHelper::~CB_OffMuHelper()
{
}
void CB_OffMuHelper::Render(float X, float Y)
{
	if (g_pBCustomMenuInfo->RenderCheckBox(X + 8, Y+5, 0xFFFFFFFF, this->ActiveOfflineMode == 1 ? TRUE : FALSE, "Treo Máy Offline"))
	{
		this->ActiveOfflineMode ^= 1;
	}
	g_pRenderText->SetTextColor(0xFF00B4FF);
	g_pRenderText->RenderText(X + 8, Y + 25, "*Kích hoạt chế độ Offline", 0, 0, RT3_SORT_CENTER);
}
void CB_OffMuHelper::RunningOffHelper()
{

	if (!this->ActiveOfflineMode)
	{
		return;
	}

	gInterface.m_Disconnect = true;

	CG_OFFMODE_RESULT pRequest;
	pRequest.h.set((LPBYTE)& pRequest, 0xFB, 0x3D, sizeof(pRequest));
	//-- Range Attack
	pRequest.HuntingRange = g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Range[0];
	//-- Auto Heling, Drain and Potion
	pRequest.RecoveryPotionOn = g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.AutoPotion;
	pRequest.RecoveryPotionPercent = g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.PotPercent;
	pRequest.RecoveryHealOn = g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.AutoHeal;
	pRequest.RecoveryHealPercent = g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.HealPercent;
	pRequest.RecoveryDrainOn = g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.AutoDrainLife;
	pRequest.RecoveryDrainPercent = g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.DrainPercent;
	//-- Attack
	pRequest.DistanceLongOn = g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.LongDistance;
	pRequest.DistanceReturnOn = g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.OriginalPosition;
	pRequest.DistanceMin = (g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.MoveTime);
	pRequest.SkillBasicID = g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Skill[0];
	pRequest.ComboOn = g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Combo;
	//-- Party Settings
	pRequest.PartyModeOn = g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Party;
	pRequest.PartyModeHealOn = g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.PartyAutoHeal;
	pRequest.PartyModeHealPercent = g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.PartyHealPercent;
	pRequest.PartyModeBuffOn = g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.PartyAutoBuff;
	//-- AutoBuffo
	pRequest.BuffOn = g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.AutoBuff;
	pRequest.BuffSkill[0] = g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Buff[0];
	pRequest.BuffSkill[1] = g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Buff[0];
	pRequest.BuffSkill[2] = g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Buff[0];
	//-- PickObtain
	pRequest.ObtainRange = g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.Range[1];
	pRequest.ObtainRepairOn = g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.RepairItem;
	pRequest.ObtainPickNear = g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.PickAllItem;
	pRequest.ObtainPickSelected = g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.PickSelected;
	pRequest.ObtainPickMoney = g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.PickMoney;
	pRequest.ObtainPickJewels = g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.PickJewel;
	pRequest.ObtainPickExcellent = g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.PickExc;
	pRequest.ObtainPickAncient = g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.PickSet;
	pRequest.ObtainPickExtra = g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.PickExtra;

	pRequest.ObtainPickExtraCount = 0;

	for (int i = 0; i < 12; i++)
	{
		ZeroMemory(pRequest.ObtainPickItemList[i], sizeof(g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.ItemList[i]));
		wsprintf(pRequest.ObtainPickItemList[i], g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.ItemList[i]);

		if ((strcmp(g_pNewUISystem->Get_pNewUIMuHelper()->DataAutoMu.ItemList[i], "") != 0))
		{
			pRequest.ObtainPickExtraCount++;
		}
	}

	DataSend((LPBYTE)& pRequest, pRequest.h.size);
}
#endif