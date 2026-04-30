#pragma once
#include "CBInterface.h"
#include "Protocol.h"
class CBHuyDongExc
{
public:
	enum T
	{
		Damage = 80,		//-> Additional Damage +<N>
		WizardryDamage = 81,
		CurseSpell = 82,
		Defense = 84,		//-> Additional Defense +<N>
		Luck = 85,		//-> Default: 0
		HPRecovery = 86,
		// ----
		LifeIncrease = 87,		//-> Increase Max HP +4%%
		ManaIncrease = 88,		//-> Increase Max Mana +4%%
		DamageDecrease = 89,		//-> Damage Decrease +4%%
		Reflect = 90,		//-> Reflect Damage +5%%
		DefenseSuccess = 91,		//-> Defense success rate +10%%
		IncreaseZen = 92,		//-> Increases acquisition rate of Zen after hunting monsters +30%%
		// ----
		ExcellentDamage = 93,		//-> Excellent Damage rate +10%%
		DamageByLevel = 94,		//-> Increase Damage +level/20
		IncreaseDamage = 95,		//-> Increase Damage +%d%%
		IncreaseWizardry = 96,		//-> Increase Wizardry Dmg +level/20
		IncreaseSpeed = 98,		//-> Increase Attacking(Wizardry)speed +%d
		IgnorOpDefense = 103,
		ReturnAttack = 106,		//-> Return's the enemy's attack power in <N>%
		CompleteLife = 107,		//-> Complete recovery of life in <N>% rate
		CompleteMana = 108,		//-> Complete recovery of Mana in <N>% rate
		//================
		IncreaseWizardryDmg = 97,    //-> Increase Wizardry Dmg +2%
		LifePerMob = 99,        //-> Increases acquisition rate of Life after hunting monsters +life/8
		ManaPerMob = 100,        //-> Increases acquisition rate of Mana after hunting monsters +Mana/8
	};
	struct HUYEXC_GSSENDINFO
	{
		PSBMSG_HEAD header;
		int mWC[6];
		int mWP[6];
		int mGP[6];
		int mRate[6];
		BYTE Request;
	};
	struct PMSG_HDCLIENTSENDGS
	{
		PSBMSG_HEAD h;
		BYTE Source;
	};

	CBHuyDongExc();
	virtual ~CBHuyDongExc();
	void OpenWindow();
	void DrawWindowHuyExc();
	ITEM* ItemCacheSelect;
	DWORD ItemCacheTime;
	bool ItemCacheShow;
	bool SendItem(ITEM* ItemSell, int Slot, bool KeyClick);
	void RecvProtocol(BYTE* Recv);
};

extern CBHuyDongExc* gCBHuyDongExc;