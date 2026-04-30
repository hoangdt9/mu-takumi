#include "StdAfx.h"
#include "APICB.h"
//#include "PrintPlayer.h"
//#include "HackCheck.h"
#include "Protocol.h"

#define TYPE_MAINSS		0  //1 Là SS6 1.04E, 0 là SRC Main
#if(TYPE_MAINSS==1)
extern DWORD FrameValue;
extern DWORD SpeedValue;
extern DWORD MainTickCount;
extern DWORD SyncTickCount;
extern DWORD CountModifier;
extern DWORD DelayModifier;
extern DWORD HasteModifier;
extern DWORD SleepModifier;
extern DWORD SpeedModifier1;
extern DWORD SpeedModifier2;
extern DWORD ModelModifier1;
extern DWORD ModelModifier2;
extern DWORD ModelModifier3;
extern DWORD ViewPoint;
extern DWORD ViewStrength;
extern DWORD ViewDexterity;
extern DWORD ViewVitality;
extern DWORD ViewEnergy;
extern DWORD ViewLeadership;
extern DWORD ViewAddStrength;
extern DWORD ViewAddDexterity;
extern DWORD ViewAddVitality;
extern DWORD ViewAddEnergy;
extern DWORD ViewAddLeadership;
extern DWORD ViewPhysiSpeed;
extern DWORD ViewMagicSpeed;
#endif

APICB gAPICB;
void CBDataSendCallback(BYTE* lpMsg, DWORD size)
{
	DataSend(lpMsg, size);
}
APICB::APICB()
{

	SetAPIDATA_SEND(CBDataSendCallback);
}


APICB::~APICB()
{
}
bool CacheCallBack = false;
void APICB::Work()
{
	

#if(TYPE_MAINSS==1)
	if (*(DWORD*)(0x00E609E8) == 5)
	{
		lpCharObj lpPlayer = &*(ObjectCharacter*) * (int*)(0x08128AC8);
		lpViewObj lpViewPlayer = &*(ObjectPreview*) * (int*)(0x07BC4F04);

		if (!lpPlayer || !lpViewPlayer) return;

		API_CharLevel = lpPlayer->Level;
		API_CharWeaponFirstSlot = lpViewPlayer->WeaponFirstSlot;
		API_CharWeaponSecondSlot = lpViewPlayer->WeaponSecondSlot;
		API_CharHelmSlot = lpViewPlayer->HelmSlot;
		API_CharArmorSlot = lpViewPlayer->ArmorSlot;
		API_CharPantsSlot = lpViewPlayer->PantsSlot;
		API_CharGlovesSlot = lpViewPlayer->GlovesSlot;
		API_CharBootsSlot = lpViewPlayer->BootsSlot;
		API_CharWingsSlot = lpViewPlayer->WingsSlot;
		API_CharPetSlot = lpViewPlayer->PetSlot;
		API_CharAnimationID = lpViewPlayer->m_Model.AnimationID;
		API_CharActiveMagic = lpPlayer->pMagicList[lpViewPlayer->ActiveMagic];
#else
	if (SceneFlag == 5)
	{
		if (!CharacterAttribute || !Hero) return;
		if (!CacheCallBack)
		{
			SetAPIDATA_SEND(CBDataSendCallback);
			API_GameState = &*(DWORD*)(SceneFlag);
			API_FrameValue = 0;
			API_SpeedValue = 0;
			API_MainTickCount = 0;
			API_SyncTickCount = 0;
			API_CountModifier = 0;
			API_DelayModifier = 0;
			API_HasteModifier = 0;
			API_SleepModifier = 0;
			API_SpeedModifier1 = 0;
			API_SpeedModifier2 = 0;
			API_ModelModifier1 = 0;
			API_ModelModifier2 = 0;
			API_ModelModifier3 = 0;
			API_ViewPoint = &CharacterAttribute->PrintPlayer.ViewPoint;
			API_ViewStrength = &CharacterAttribute->PrintPlayer.ViewStrength;
			API_ViewDexterity = &CharacterAttribute->PrintPlayer.ViewDexterity;
			API_ViewVitality = &CharacterAttribute->PrintPlayer.ViewVitality;
			API_ViewEnergy = &CharacterAttribute->PrintPlayer.ViewEnergy;
			API_ViewLeadership = &CharacterAttribute->PrintPlayer.ViewLeadership;
			API_ViewAddStrength = &CharacterAttribute->PrintPlayer.ViewAddStrength;
			API_ViewAddDexterity = &CharacterAttribute->PrintPlayer.ViewAddDexterity;
			API_ViewAddVitality = &CharacterAttribute->PrintPlayer.ViewAddVitality;
			API_ViewAddEnergy = &CharacterAttribute->PrintPlayer.ViewAddEnergy;
			API_ViewAddLeadership = &CharacterAttribute->PrintPlayer.ViewAddLeadership;
			API_ViewPhysiSpeed = &CharacterAttribute->PrintPlayer.ViewPhysiSpeed;
			API_ViewMagicSpeed = &CharacterAttribute->PrintPlayer.ViewMagicSpeed;
			CacheCallBack = true;

		}
		API_CharLevel = CharacterAttribute->Level;
		API_CharWeaponFirstSlot = Hero->Weapon[0].Type;
		API_CharWeaponSecondSlot = Hero->Weapon[1].Type;
		API_CharHelmSlot = Hero->BodyPart[1].Type;
		API_CharArmorSlot = Hero->BodyPart[2].Type;
		API_CharPantsSlot = Hero->BodyPart[3].Type;
		API_CharGlovesSlot = Hero->BodyPart[4].Type;
		API_CharBootsSlot = Hero->BodyPart[5].Type;
		API_CharWingsSlot = Hero->Wing.Type;
		API_CharPetSlot = Hero->Helper.Type;
		API_CharAnimationID = Hero->Object.m_iAnimation;
		API_CharActiveMagic = CharacterAttribute->Skill[Hero->CurrentSkill];


#endif
		CBAnihack_Work();
		
	}
}
void APICB::Init()
{
	CBAnihack_Init();
	API_TypeMain = TYPE_MAINSS; //1 = Main 1.04E
#if(TYPE_MAINSS==1)
	API_GameState = &*(DWORD*)(0x00E609E8);
	API_FrameValue = &FrameValue;
	API_SpeedValue = &SpeedValue;
	API_MainTickCount = &MainTickCount;
	API_SyncTickCount = &SyncTickCount;
	API_CountModifier = &CountModifier;
	API_DelayModifier = &DelayModifier;
	API_HasteModifier = &HasteModifier;
	API_SleepModifier = &SleepModifier;
	API_SpeedModifier1 = &SpeedModifier1;
	API_SpeedModifier2 = &SpeedModifier2;
	API_ModelModifier1 = &ModelModifier1;
	API_ModelModifier2 = &ModelModifier2;
	API_ModelModifier3 = &ModelModifier3;
	API_ViewPoint = &ViewPoint;
	API_ViewStrength = &ViewStrength;
	API_ViewDexterity = &ViewDexterity;
	API_ViewVitality = &ViewVitality;
	API_ViewEnergy = &ViewEnergy;
	API_ViewLeadership = &ViewLeadership;
	API_ViewAddStrength = &ViewAddStrength;
	API_ViewAddDexterity = &ViewAddDexterity;
	API_ViewAddVitality = &ViewAddVitality;
	API_ViewAddEnergy = &ViewAddEnergy;
	API_ViewAddLeadership = &ViewAddLeadership;
	API_ViewPhysiSpeed = &ViewPhysiSpeed;
	API_ViewMagicSpeed = &ViewMagicSpeed;
#endif
}
void APICB::Recv(BYTE* Recv)
{
	CBAnihack_Recv(Recv);
}

void APICB::Attack()
{
	CBAnihack_Attack();
}