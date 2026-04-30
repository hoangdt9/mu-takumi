#include "stdafx.h"
#include "ChangeClass.h"
#include "Notice.h"
#include "MasterSkillTree.h"
#include "ObjectManager.h"
#include "ServerInfo.h"
#include "DSProtocol.h"
#include "Util.h"
#include "CashShop.h"
#include "Message.h"

cChangeClass gChangeClass;

cChangeClass::cChangeClass()
{
	this->Init();
}

cChangeClass::~cChangeClass()
{

}

void cChangeClass::Init()
{
	this->m_WCoinC = 0;
}

void cChangeClass::Load(char* path)
{
	this->Enable = GetPrivateProfileInt("Common", "Enable", 0, path);
	this->m_WCoinC = GetPrivateProfileInt("Common", "WCoinC", 0, path);
	this->LevelStart = GetPrivateProfileInt("Common", "LevelStart", 400, path);
}

void cChangeClass::SendData(int aIndex)
{
	PMSG_CHANGECLASS_DATA pRequest;
	pRequest.Head.set(0xFB, 0x08, sizeof(pRequest));

	pRequest.m_WCoinC = this->m_WCoinC;

	DataSend(aIndex, (LPBYTE)& pRequest, sizeof(pRequest));
}

void cChangeClass::RecvChangeClass(CG_CHANGECLASS_RECV* Data, int aIndex)
{
	if (OBJECT_RANGE(aIndex) == 0)
	{
		return;
	}
	if (!this->Enable)
	{
		gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, gMessage.GetMessage(45));
		return;
	}
	if (gObj[aIndex].Type != OBJECT_USER || (gObj[aIndex].Connected != OBJECT_ONLINE))
	{
		return;
	}
	if (gObj[aIndex].Interface.use != 0 || gObj[aIndex].Teleport != 0 || gObj[aIndex].DieRegen != 0 || gObj[aIndex].PShopOpen != 0)
	{
		gNotice.GCNoticeSend(aIndex, eMessageBox, 0, 0, 0, 0, 0, gMessage.GetMessage(659));
		return;
	}
	LPOBJ lpObj = &gObj[aIndex];
	for (int i = 0; i < 12; i++)
	{
		if (lpObj->Inventory[i].IsItem())
		{
			gNotice.GCNoticeSend(lpObj->Index, eMessageBox, 0, 0, 0, 0, 0, gMessage.GetMessage(2022));
			return;
		}
	}

	gCashShop.GDCashShopRecievePointSend(aIndex, (DWORD)& cChangeClass::ChangeClassCallback, (DWORD)Data->Type, 0);
}

void cChangeClass::ChangeClassCallback(LPOBJ lpObj, int Class, DWORD null, DWORD WCoinC, DWORD WCoinP, DWORD GoblinPoint)
{
	if (lpObj->Coin1 < gChangeClass.m_WCoinC)
	{
		gNotice.GCNoticeSend(lpObj->Index, 1, 0, 0, 0, 0, 0, gMessage.GetMessage(2021), gChangeClass.m_WCoinC);
		return;
	}
	// ----

	gChangeClass.ChangeClass(lpObj, Class);
}

void cChangeClass::ChangeClass(LPOBJ lpObj, int Class)
{
	int NextClass = 0;
	switch (Class)
	{
	case 0:
		NextClass = CLASS_DW;
		break;
	case 16:
		NextClass = CLASS_DK;
		break;
	case 32:
		NextClass = CLASS_FE;
		break;
	case 48:
		NextClass = CLASS_MG;
		break;
	case 64:
		NextClass = CLASS_DL;
		break;
	case 80:
		NextClass = CLASS_SU;
		break;
	case 96:
		NextClass = CLASS_RF;
		break;
	}

	lpObj->Level = this->LevelStart;
	lpObj->Experience = 0;
	lpObj->MasterExperience = 0;
	lpObj->MasterNextExperience = gMasterSkillTree.GetMasterLevelExpTlbInfo(lpObj->MasterExperience + 1);
	lpObj->MasterPoint = lpObj->MasterLevel - 1;

	for (int n = 0; n < MAX_SKILL_LIST; n++)
	{
		lpObj->Skill[n].Clear();
	}

	for (int n = 0; n < MAX_MASTER_SKILL_LIST; n++)
	{
		lpObj->MasterSkill[n].Clear();
	}

	gMasterSkillTree.GCMasterSkillListSend(lpObj->Index);
	gSkillManager.GCSkillListSend(lpObj, 0);
	gMasterSkillTree.GCMasterInfoSend(lpObj);

	gCashShop.GDCashShopSubPointSaveSend(lpObj->Index, 0, gChangeClass.m_WCoinC, 0, 0,0,"ChangeClass");
	gCashShop.CGCashShopPointRecv(lpObj->Index);

	lpObj->DBClass = Class;

	memset(lpObj->Quest, (BYTE)-1, sizeof(lpObj->Quest));

	if (lpObj->PartyNumber >= 0)
	{
		gParty.DelMember(lpObj->PartyNumber, lpObj->Index);
		lpObj->PartyTargetUser = -1;
	}
	//----------------------------------------------------------------------
	int NewPoints = lpObj->Strength + lpObj->Dexterity + lpObj->Vitality + lpObj->Energy;

	if (lpObj->Class == CLASS_DL)
	{
		NewPoints += lpObj->Leadership;
	}

	lpObj->LevelUpPoint += (NewPoints - (gDefaultClassInfo.m_DefaultClassInfo[lpObj->Class].Strength + gDefaultClassInfo.m_DefaultClassInfo[lpObj->Class].Dexterity +
		gDefaultClassInfo.m_DefaultClassInfo[lpObj->Class].Energy + gDefaultClassInfo.m_DefaultClassInfo[lpObj->Class].Vitality + gDefaultClassInfo.m_DefaultClassInfo[lpObj->Class].Leadership));

	lpObj->Strength = gDefaultClassInfo.m_DefaultClassInfo[lpObj->Class].Strength;
	lpObj->Dexterity = gDefaultClassInfo.m_DefaultClassInfo[lpObj->Class].Dexterity;
	lpObj->Energy = gDefaultClassInfo.m_DefaultClassInfo[lpObj->Class].Energy;
	lpObj->Vitality = gDefaultClassInfo.m_DefaultClassInfo[lpObj->Class].Vitality;
	lpObj->Leadership = gDefaultClassInfo.m_DefaultClassInfo[lpObj->Class].Leadership;

	//----------------------------------------------------------------------

	gObjectManager.CharacterCalcAttribute(lpObj->Index);

	GCNewCharacterInfoSend(lpObj);

	GDCharacterInfoSaveSend(lpObj->Index);
	// ----
	switch (Class)
	{
	case CLASS_DW:
		gObjMoveGate(lpObj->Index, 17);
		break;
	case CLASS_DK:
		gObjMoveGate(lpObj->Index, 17);
		break;
	case CLASS_FE:
		gObjMoveGate(lpObj->Index, 27);
		break;
	case CLASS_MG:
		gObjMoveGate(lpObj->Index, 17);
		break;
	case CLASS_DL:
		gObjMoveGate(lpObj->Index, 17);
		break;
	case CLASS_SU:
		gObjMoveGate(lpObj->Index, 267);
		break;
	case CLASS_RF:
		gObjMoveGate(lpObj->Index, 17);
		break;
	}

	gObjectManager.CharacterGameCloseSet(lpObj->Index, 1);
}
