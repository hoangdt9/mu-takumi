#pragma once
#include "Protocol.h"

#if(CB_OFFMUHELPER)
class CB_OffMuHelper
{
public:
	struct CG_OFFMODE_RESULT
	{
		PBMSG_HEAD2 h;
		WORD HuntingRange;
		//-- Potion
		BYTE RecoveryPotionOn;
		BYTE RecoveryPotionPercent;
		BYTE RecoveryHealOn; //-> CLASS_ELF
		BYTE RecoveryHealPercent; //-> CLASS_ELF
		BYTE RecoveryDrainOn; //-> CLASS_SUMMONER
		BYTE RecoveryDrainPercent; //-> CLASS_SUMMONER
		//-- Attack
		BYTE DistanceLongOn;
		BYTE DistanceReturnOn;
		WORD DistanceMin;//-- tiempo
		WORD SkillBasicID;
		BYTE ComboOn;				//-> CLASS_KNIGHT
		BYTE PartyModeOn;
		BYTE PartyModeHealOn;		//-> CLASS_ELF
		BYTE PartyModeHealPercent;	//-> CLASS_ELF
		BYTE PartyModeBuffOn;
		BYTE BuffOn;
		WORD BuffSkill[3];
		//-- Obtaing
		WORD ObtainRange;
		BYTE ObtainRepairOn;
		BYTE ObtainPickNear;
		BYTE ObtainPickSelected;
		BYTE ObtainPickMoney;
		BYTE ObtainPickJewels;
		BYTE ObtainPickExcellent;
		BYTE ObtainPickAncient;
		BYTE ObtainPickExtra;
		int ObtainPickExtraCount;
		char ObtainPickItemList[12][16];
	};

	CB_OffMuHelper();
	~CB_OffMuHelper();
	void RunningOffHelper();
	void Render(float X, float Y);
	bool ActiveOfflineMode;
};

extern CB_OffMuHelper* gCB_OffMuHelper;
#endif