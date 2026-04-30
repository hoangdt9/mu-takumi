#pragma once
#include "Protocol.h"
class CB_StatsAdvance
{

public:

	struct STATS_ADVANCE_DATA
	{
		int Level;
		int Reset;
		int GrandReset;
		int TotalDamageReflect;
		int FullDamageReflectRate;
		//--
		int CriticalDamageRate;
		int CriticalDamagePVM;
		int CriticalDamagePVP;
		//--
		int ExellentDamageRate;
		int ExellentDamagePVM;
		int ExellentDamagePVP;
		//--
		int DoubleDamageRate;
		int TripleDamageRate;
		int DamageReductionRate;
		//--
		int ShieldSkillDamageReductionRate;
		int SDDamageReductionRate;
		int SDDecreaseDamageRate;
		//--
		int IgnoreDefenceRate;
		int IgnoreSDRate;
		//--
		int IncreaseDamagePvP;
		int IncreaseDefencePvP;
		//--
		int ResistDoubleDamageRate;
		int ResistIgnoreDefenceRate;
		int ResistIgnoreSDRate;
		int ResistCriticalDamageRate;
		int ResistExellentDamageRate;
		int ResistStumRate;
		//--
		int IncreaseIce;
		int IncreasePoison;
		int IncreaseLighting;
		int IncreaseFire;
		int IncreaseEarth;
		int IncreaseWind;
		int IncreaseWater;
		//--
		int ResistIce;
		int ResistPoison;
		int ResistLighting;
		int ResistFire;
		int ResistEarth;
		int ResistWind;
		int ResistWater;
		//--
		int FullHPRestoreRateDef;
		int FullMPRestoreRateDef;
		int FullSDRestoreRateDef;
		int FullHPRestoreRate;
		int FullMPRestoreRate;
		int FullSDRestoreRate;
		//--
		int AttackPhysispeed;
		int AttackMagicspeed;
		//--
		int PhysiDamageMin;
		int PhysiDamageMax;
		int AttackSuccessRate;
	};


	struct PMSG_STATS_ADVANCE
	{
		PBMSG_HEAD2 h;
		BYTE Result;
		STATS_ADVANCE_DATA StatInfo;
	};
	CB_StatsAdvance();
	virtual ~CB_StatsAdvance();
	void Recv(BYTE* Recv);
	STATS_ADVANCE_DATA m_Data;
	void RenderInfo(int StartX, int StartY);
	bool m_Loaded;
	int Page;
	void DrawLine(float PosX, float PosY, int Width, LPCSTR Text, ...);
	float DrawLineR(float PosX, float PosY, int Width, DWORD dwColor, LPCSTR Text, ...);

};
extern CB_StatsAdvance* gCB_StatsAdvance;