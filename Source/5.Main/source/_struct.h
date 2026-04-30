#ifndef ___STRUCT_H__
#define ___STRUCT_H__

#pragma once
#include "./ExternalObject/Leaf/stdleaf.h" //QWORD
#include "w_Buff.h"

#include "w_ObjectInfo.h"
class OBJECT;
#include "w_CharacterInfo.h"
class CHARACTER;

namespace SEASON3B
{
	class CNewUIItemMng;
}
#if defined(__ANDROID__) || defined(MU_IOS)
typedef unsigned long long BQWORD;
#else
typedef unsigned __int64 BQWORD;
#endif
#define MAX_MONSTERSKILL_NUM	10

namespace info
{
	struct Script_Item
	{
		char Name[30];
		bool TwoHand;
		WORD Level;
		BYTE m_byItemSlot;
		BYTE m_bySkillIndex;
		BYTE Width;
		BYTE Height;
		BYTE DamageMin;
		BYTE DamageMax;
		BYTE SuccessfulBlocking;
		BYTE Defense;
		BYTE MagicDefense;
		BYTE WeaponSpeed;
		BYTE WalkSpeed;
		BYTE Durability;
		BYTE MagicDur;
		BYTE MagicPower;
		WORD RequireStrength;
		WORD RequireDexterity;
		WORD RequireEnergy;
		WORD  RequireVitality;
		WORD RequireCharisma;
		WORD RequireLevel;
		BYTE Value;
		int  iZen;
		BYTE  AttType;
		BYTE RequireClass[MAX_CLASS];
		BYTE Resistance[MAX_RESISTANCE];
	};
	
	struct Script_Silde 
	{
		typedef std::vector<char*>     SildeVECTOR;
		SildeVECTOR					   Sildelist;
	};

	struct Script_Dialog
	{
		char m_lpszText[MAX_LENGTH_DIALOG];
		int m_iNumAnswer;
		int m_iLinkForAnswer[MAX_ANSWER_FOR_DIALOG];
		int m_iReturnForAnswer[MAX_ANSWER_FOR_DIALOG];
		char m_lpszAnswer[MAX_ANSWER_FOR_DIALOG][MAX_LENGTH_ANSWER];
	};
	
	struct Script_Credit
	{
		BYTE	byClass;
		char	szName[32];
		//char	szName[CRW_NAME_MAX];
	};

	struct Script_Movereq
	{
		int index;
		char szMainMapName[32];			//. Main map name
		char szSubMapName[32];			//. Substitute map name
		int	iReqLevel;					//. required level
		int iReqZen;					//. required zen
		int iGateNum;					//. Gate number
	};

	struct Script_Quest_Class_Act
	{
		BYTE    chLive;
		BYTE    byQuestType;
		WORD    wItemType;
		BYTE    byItemSubType;
		BYTE	byItemLevel;
		BYTE    byItemNum;
		BYTE    byRequestType;
		BYTE    byRequestClass[MAX_CLASS];
		short   shQuestStartText[4];
	};

	struct Script_Quest_Class_Request
	{
		BYTE    byLive;
		BYTE    byType;
		WORD    wCompleteQuestIndex;
		WORD    wLevelMin;
		WORD    wLevelMax;
		WORD	wRequestStrength;
		DWORD   dwZen;
		short   shErrorText;
	};

	struct Script_Quest
	{
		short   shQuestConditionNum;
		short   shQuestRequestNum;
		WORD    wNpcType;
		char    strQuestName[32];

		Script_Quest_Class_Act     QuestAct[MAX_QUEST_CONDITION];
		Script_Quest_Class_Request QuestRequest[MAX_QUEST_REQUEST];
	};
};

//infomation start
typedef struct
{
	BYTE Flag;
	BYTE Map;
	BYTE x1;
	BYTE y1;
	BYTE x2;
	BYTE y2;
	WORD Target;
	BYTE Angle;
	WORD Level;
	WORD m_wMaxLevel;
} GATE_ATTRIBUTE;

typedef struct
{
	char m_lpszText[MAX_LENGTH_DIALOG];
	int m_iNumAnswer;
	int m_iLinkForAnswer[MAX_ANSWER_FOR_DIALOG];
	int m_iReturnForAnswer[MAX_ANSWER_FOR_DIALOG];
	char m_lpszAnswer[MAX_ANSWER_FOR_DIALOG][MAX_LENGTH_ANSWER];
} DIALOG_SCRIPT;//Script_Dialog


typedef struct
{
	char Name[MAX_ITEM_NAME];
	WORD TwoHand;
	WORD Level;
	WORD m_byItemSlot;
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
	WORD m_wSkillIndex;
#else //PBG_ADD_NEWCHAR_MONK_SKILL
	WORD m_bySkillIndex;
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
	BYTE Width;
	BYTE Height;
	BYTE DamageMin;
	BYTE DamageMax;
	BYTE SuccessfulBlocking;
	BYTE Defense;
	BYTE MagicDefense;
	BYTE WeaponSpeed;
	BYTE WalkSpeed;
	BYTE Durability;
	BYTE MagicDur;
	BYTE MagicPower;
	WORD RequireStrength;
	WORD RequireDexterity;
	WORD RequireEnergy;
	WORD  RequireVitality;
	WORD RequireCharisma;
	WORD RequireLevel;
	WORD Value;
	int  iZen;
	BYTE  AttType;
	BYTE RequireClass[MAX_CLASS];
	BYTE Resistance[MAX_RESISTANCE+1];
} ITEM_ATTRIBUTE;
//===Item Tool tip season 6

struct ITEM_TOOL_TIP_TEXT_DATA
{
	short text_index;
	BYTE color;
	BYTE is_bold;
};
struct _ITEM_TOOLTIP
{
	WORD type;
	WORD index;
	char name[64];
	BYTE color;
	BYTE is_bold;
	BYTE ShowNameInShop;
	WORD ShowItemLevel;
	__int16 ItemLevelToolTipIndex; // 76
	ITEM_TOOL_TIP_TEXT_DATA tooltip_text[12];
};

struct _ITEM_LEVEL_TOOLTIP
{
	WORD index;
	char name[64]; // 2
	BYTE color; // 66
	BYTE is_bold; // 67
	BYTE unk3; // 68
	BYTE unk4; // 69
	ITEM_TOOL_TIP_TEXT_DATA tooltip_text[8];  // 70
};
struct _ITEM_TOOLTIP_TEXT
{
	WORD index;
	char text[256];
	short type;
};
typedef struct _MASTER_LEVEL_DATA
{
	BYTE Width;
	int Ability[8][4];
}MASTER_LEVEL_DATA;

typedef struct _MINI_MAP
{
	BYTE Kind;
	int	 Location[2];
	int  Rotation;
	char Name[100];
}MINI_MAP;

#pragma pack(push, 1)
typedef struct tagITEM
{
	short Type;
	int   Level;
	BYTE  Part;
	BYTE  Class;
	bool  TwoHand;
	WORD  DamageMin;
	WORD  DamageMax;
	BYTE  SuccessfulBlocking;
	WORD  Defense;
	WORD  MagicDefense;
	BYTE  MagicPower;
	BYTE  WeaponSpeed;
	WORD  WalkSpeed;
	BYTE  Durability;
	BYTE  Option1;
	BYTE  ExtOption;
	WORD  RequireStrength;
	WORD  RequireDexterity;
	WORD  RequireEnergy;
	WORD  RequireVitality;
	WORD  RequireCharisma;
	WORD  RequireLevel;
	BYTE  SpecialNum;
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
	WORD  Special     [MAX_ITEM_SPECIAL];
#else //PBG_ADD_NEWCHAR_MONK_SKILL
	BYTE  Special     [MAX_ITEM_SPECIAL];
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
	BYTE  SpecialValue[MAX_ITEM_SPECIAL];
	DWORD Key;	//client olny
	
	BYTE bySelectedSlotIndex;

	union 
	{
		BYTE  x;    //client olny
		BYTE  lineal_pos;
	};
	union
	{
		BYTE  y;    //client olny
		BYTE  ex_src_type;
	};
	WORD  Jewel_Of_Harmony_Option;
	WORD  Jewel_Of_Harmony_OptionLevel;
	bool option_380;
	BYTE bySocketOption[MAX_SOCKETS];
	BYTE SocketCount;
	BYTE SocketSeedID[MAX_SOCKETS];
	BYTE SocketSphereLv[MAX_SOCKETS];
	BYTE SocketSeedSetOption;

	int   Number;
	BYTE  Color;
	
	BYTE byColorState;
	
#ifdef KJH_ADD_PERIOD_ITEM_SYSTEM
	bool	bPeriodItem;
	bool	bExpiredPeriod;
	//DWORD	dwLeftTime;
	long	lExpireTime;
	//DWORD	dwPastSystemTime;
#endif // KJH_ADD_PERIOD_ITEM_SYSTEM

private:
	friend class SEASON3B::CNewUIItemMng;
	int   RefCount;
} ITEM;
#pragma pack(pop)

typedef struct
{
	WORD Life;
	WORD MoveSpeed;
	WORD AttackSpeed;
	WORD AttackDamageMin;
	WORD AttackDamageMax;
	WORD Defense;
	WORD MagicDefense;
	WORD AttackRating;
	WORD SuccessfulBlocking;
} MONSTER_ATTRIBUTE;

typedef struct
{
	WORD				Type;
	char				Name[MAX_MONSTER_NAME];
	WORD				Level;
	MONSTER_ATTRIBUTE	Attribute;
} MONSTER_SCRIPT;

typedef struct
{
	short             Type;
	BYTE              Level;
	int               Experince;
	MONSTER_ATTRIBUTE Attribute;
} MONSTER;

typedef struct
{
	char Name[32];
	WORD Level;//32
	WORD Damage;//34
	WORD Mana;//36
	WORD AbilityGuage;//38
	DWORD Distance;//40
	int  Delay;//44
	int Energy;//48
	WORD Charisma;//52
	BYTE MasteryType;//54
	BYTE SkillUseType;//56
	DWORD SkillBrand;//58
	BYTE KillCount;//62
	BYTE RequireDutyClass[MAX_DUTY_CLASS];//63+3= 66
	BYTE RequireClass[MAX_CLASS];//67+7
	BYTE SkillRank;//72
	WORD SkillGroup;//73
	BYTE TypeSkill;//75
	int Strength;//77
	int Dexterity;//81
	BYTE ItemSkill;//85
	BYTE IsDamage;//86
	WORD Magic_Icon;//87

} SKILL_ATTRIBUTE;

typedef struct
{
	float CalcX;
	float CalcY;
} JCCoord;


typedef struct
{
	WORD Strength;
	WORD Dexterity;
	WORD Vitality;
	WORD Energy;
	WORD Life;
	WORD Mana;
	WORD Shield;
	BYTE LevelLife;
	BYTE LevelMana;
	BYTE VitalityToLife;
	BYTE EnergyToMana;
	BYTE ClassSkill[MAX_SKILLS];
} CLASS_ATTRIBUTE;

typedef struct
{
	DWORD ViewReset;
	DWORD ViewMasterReset;
	DWORD ViewValue;
	DWORD ViewPoint;
	DWORD ViewCurHP;
	DWORD ViewMaxHP;
	DWORD ViewCurMP;
	DWORD ViewMaxMP;
	DWORD ViewCurBP;
	DWORD ViewMaxBP;
	DWORD ViewCurSD;
	DWORD ViewMaxSD;
	DWORD ViewDamageTYPE;
	DWORD GetViewIndex;
	QWORD ViewDamageHP;
	QWORD ViewDamageSD;
	QWORD ViewDamageTable[4];
	DWORD ViewDamageCount;
	DWORD ViewDamageValue;
	DWORD ViewExperience;
	DWORD ViewNextExperience;
	BQWORD ViewMasterExperience;
	BQWORD ViewMasterNextExperience;
	DWORD ViewStrength;
	DWORD ViewDexterity;
	DWORD ViewVitality;
	DWORD ViewEnergy;
	DWORD ViewLeadership;
	DWORD ViewAddStrength;
	DWORD ViewAddDexterity;
	DWORD ViewAddVitality;
	DWORD ViewAddEnergy;
	DWORD ViewAddLeadership;
	DWORD ViewPhysiDamageMin;
	DWORD ViewPhysiDamageMax;
	DWORD ViewMagicDamageMin;
	DWORD ViewMagicDamageMax;
	DWORD ViewCurseDamageMin;
	DWORD ViewCurseDamageMax;
	DWORD ViewMulPhysiDamage;
	DWORD ViewDivPhysiDamage;
	DWORD ViewMulMagicDamage;
	DWORD ViewDivMagicDamage;
	DWORD ViewMulCurseDamage;
	DWORD ViewDivCurseDamage;
	DWORD ViewMagicDamageRate;
	DWORD ViewCurseDamageRate;
	DWORD ViewPhysiSpeed;
	DWORD ViewMagicSpeed;
	DWORD ViewAttackSuccessRate;
	DWORD ViewAttackSuccessRatePvP;
	DWORD ViewDefense;
	DWORD ViewDefenseSuccessRate;
	DWORD ViewDefenseSuccessRatePvP;
	DWORD ViewDamageMultiplier;
	DWORD ViewRFDamageMultiplierA;
	DWORD ViewRFDamageMultiplierB;
	DWORD ViewRFDamageMultiplierC;
	DWORD ViewDarkSpiritAttackDamageMin;
	DWORD ViewDarkSpiritAttackDamageMax;
	DWORD ViewDarkSpiritAttackSpeed;
	DWORD ViewDarkSpiritAttackSuccessRate;
	DWORD ViewSkillMagicDamage;
	DWORD ViewSkillPhysiDamage;
	DWORD ViewIndex;
	//==Coin
	DWORD Coin1;
	DWORD Coin2;
	DWORD Coin3;
	void Clear()
	{
		ViewReset = 0;
		ViewMasterReset = 0;
		ViewValue = 0;
		ViewPoint = 0;
		ViewCurHP = 0;
		ViewMaxHP = 0;
		ViewCurMP = 0;
		ViewMaxMP = 0;
		ViewCurBP = 0;
		ViewMaxBP = 0;
		ViewCurSD = 0;
		ViewMaxSD = 0;
		ViewDamageTYPE = 0;
		GetViewIndex = 0;
		ViewDamageHP = 0;
		ViewDamageSD = 0;
		FillMemory(ViewDamageTable,0,sizeof(ViewDamageTable));
		ViewDamageCount = 0;
		ViewDamageValue = 0;
		ViewExperience = 0;
		ViewNextExperience = 0;
		ViewMasterExperience = 0;
		ViewMasterNextExperience = 0;
		ViewStrength = 0;
		ViewDexterity = 0;
		ViewVitality = 0;
		ViewEnergy = 0;
		ViewLeadership = 0;
		ViewAddStrength = 0;
		ViewAddDexterity = 0;
		ViewAddVitality = 0;
		ViewAddEnergy = 0;
		ViewAddLeadership = 0;
		ViewPhysiDamageMin = 0;
		ViewPhysiDamageMax = 0;
		ViewMagicDamageMin = 0;
		ViewMagicDamageMax = 0;
		ViewCurseDamageMin = 0;
		ViewCurseDamageMax = 0;
		ViewMulPhysiDamage = 0;
		ViewDivPhysiDamage = 0;
		ViewMulMagicDamage = 0;
		ViewDivMagicDamage = 0;
		ViewMulCurseDamage = 0;
		ViewDivCurseDamage = 0;
		ViewMagicDamageRate = 0;
		ViewCurseDamageRate = 0;
		ViewPhysiSpeed = 0;
		ViewMagicSpeed = 0;
		ViewAttackSuccessRate = 0;
		ViewAttackSuccessRatePvP = 0;
		ViewDefense = 0;
		ViewDefenseSuccessRate = 0;
		ViewDefenseSuccessRatePvP = 0;
		ViewDamageMultiplier = 0;
		ViewRFDamageMultiplierA = 0;
		ViewRFDamageMultiplierB = 0;
		ViewRFDamageMultiplierC = 0;
		ViewDarkSpiritAttackDamageMin = 0;
		ViewDarkSpiritAttackDamageMax = 0;
		ViewDarkSpiritAttackSpeed = 0;
		ViewDarkSpiritAttackSuccessRate = 0;
		ViewSkillMagicDamage = 0;
		ViewSkillPhysiDamage = 0;
		ViewIndex = 0;
		Coin1 = 0;
		Coin2 = 0;
		Coin3 = 0;
	}

} PRINT_PLAYER_S6;

typedef struct
{
	char Name[MAX_ID_SIZE+1];
	BYTE Class;
	BYTE Skin;
	BYTE InventoryExtensions;
	BYTE IsVaultExtended;
	WORD Level;
	DWORD Experience;
	DWORD NextExperince;

	WORD Strength;
	WORD Dexterity;
	WORD Vitality;
	WORD Energy;
	WORD Charisma;
	WORD Life;
	WORD Mana;
	WORD LifeMax;
	WORD ManaMax;
	WORD Shield;
	WORD ShieldMax;
	WORD AttackRatingPK;
	WORD SuccessfulBlockingPK;
	WORD AddStrength;
	WORD AddDexterity;
	WORD AddVitality;
	WORD AddEnergy;
	WORD AddLifeMax;
	WORD AddManaMax;
	WORD AddCharisma;
	DWORD SkillMana;
	DWORD SkillManaMax;
	BYTE Ability;
	//WORD AbilityTime[3];
	float AbilityTime[3]; // Number of reference frames

	short AddPoint;
	short MaxAddPoint;
	WORD wMinusPoint;
	WORD wMaxMinusPoint;
	WORD AttackSpeed;
	WORD AttackRating;
	WORD AttackDamageMinRight;
	WORD AttackDamageMaxRight;
	WORD AttackDamageMinLeft;
	WORD AttackDamageMaxLeft;
	WORD MagicSpeed;
	WORD MagicDamageMin;
	WORD MagicDamageMax;
	WORD CurseDamageMin;
	WORD CurseDamageMax;
	WORD CriticalDamage;
	WORD SuccessfulBlocking;
	WORD Defense;
	WORD MagicDefense;
	WORD WalkSpeed;
	DWORD LevelUpPoint;
	BYTE SkillNumber;
	BYTE SkillMasterNumber;
	WORD Skill[MAX_SKILLS];
	BYTE SkillLevel[MAX_SKILLS];
	int  SkillDelay[MAX_SKILLS];
	DWORD btDamageMultiplier;
	//=== NEW
	PRINT_PLAYER_S6 PrintPlayer;


} CHARACTER_ATTRIBUTE;

typedef struct _MASTER_LEVEL_VALUE
{
	short		 nMLevel;
	__int64		 lMasterLevel_Experince;
	__int64		 lNext_MasterLevel_Experince;

	short		nAddMPoint;
	short      	nMLevelUpMPoint;
	short		nTotalMPoint;
	short		nMaxPoint;

	WORD		wMaxLife;
	WORD		wMaxMana;
	WORD		wMaxShield;
	WORD		wMaxBP;

} MASTER_LEVEL_VALUE;

//infomation end

typedef struct _OBJECT_BLOCK
{
	unsigned char Index;
	OBJECT        *Head;
	OBJECT        *Tail;
	bool          Visible;

	_OBJECT_BLOCK()
	{
		Index = 0;
		Head = NULL;
		Tail = NULL;
		Visible = false;
	}

} OBJECT_BLOCK;

typedef struct
{
	bool   Live;
	QWORD    Type;
	int    TexType;
	int    SubType;
	float  Scale;
	vec3_t Position;
	vec3_t Angle;
	vec3_t Light;
	float  Alpha;
	float    LifeTime;
	OBJECT *Target;
	float  Rotation;
	int    Frame;

	bool   bEnableMove;
	float  Gravity;
	vec3_t Velocity;
	vec3_t TurningForce;
	vec3_t StartPosition; 
	int iNumBone;
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
	bool bRepeatedly;
	float fRepeatedlyHeight;
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
} PARTICLE;

typedef struct
{
	bool	 Live;
	int	     Type;
	OBJECT   *Owner;
} OPERATE;

typedef struct
{
	short  Key;
	ITEM   Item;
	OBJECT Object;
} ITEM_t;

//object end

typedef struct Script_Skill
{
	int Skill_Num[MAX_MONSTERSKILL_NUM];
	int Slot;
} Script_Skill;


typedef struct
{
	bool		Live;
	int			Type;
	int         TexType;
	int			SubType;
	BYTE        RenderType;
	BYTE        RenderFace;
	float		Scale;
	vec3_t		Position;
	vec3_t      StartPosition;
	vec3_t		Angle;
	vec3_t		HeadAngle;
	vec3_t		Light;
	OBJECT      *Target;
	vec3_t      TargetPosition;
	BYTE        byOnlyOneRender;
	int			NumTails;
	int			MaxTails;
	vec3_t		Tails[MAX_TAILS][4];
	float  		LifeTime;
	bool        Collision;
	float		Velocity;
	vec3_t		Direction;
	short       PKKey;
	WORD		Skill;
	BYTE		Weapon;
	int			MultiUse;
	bool        bTileMapping;
	BYTE        m_byReverseUV;
	bool        m_bCreateTails;
	int			TargetIndex[5];
	BYTE		m_bySkillSerialNum;
	int			m_iChaIndex;
	short int	m_sTargetIndex;
} JOINT;
//character end

//inventory start
typedef struct
{
	char Name[MAX_ID_SIZE+1];
	BYTE Number;
	BYTE Server;
	BYTE GuildStatus;
} GUILD_LIST_t;

typedef struct
{
	int  Key;
	char UnionName[8+1];
	char GuildName[8+1];
	BYTE Mark[64];
} MARK_t;

typedef struct
{
	char    Name[MAX_ID_SIZE+1];
	BYTE    Number;
	BYTE    Map;
	BYTE    x;
	BYTE    y;
	int     currHP;
	int     maxHP;
	BYTE    stepHP;
	int     index;
} PARTY_t;
//inventory end

//interface start
typedef struct
{
	char      ID[MAX_ID_SIZE+1];
	char      Text[256];
	int       Type;
	int       LifeTime;
	int       Width;
} WHISPER;

typedef struct
{
	bool m_bMagic;
	int m_iSkill;
	int m_iTarget;
} MovementSkill;
//interface end

//matchevent start
typedef struct
{
	char	m_lpID[MAX_ID_SIZE];
	int		m_iScore;
	DWORD	m_dwExp;
	int		m_iZen;
} DevilSquareRank;
//matchevent end

//gmhellas start
typedef struct 
{
	char    m_strName[64];
	vec3_t  m_vPos;
}ObjectDescript;
//gmhellas end

//csquest start
typedef struct 
{
	BYTE    chLive;
	BYTE    byQuestType;
	WORD    wItemType;
	BYTE    byItemSubType;
	BYTE	byItemLevel;
	BYTE    byItemNum;
	BYTE    byRequestType;
	BYTE    byRequestClass[MAX_CLASS];
	short   shQuestStartText[4];
} QUEST_CLASS_ACT;

typedef struct 
{
	BYTE    byLive;
	BYTE    byType;
	WORD    wCompleteQuestIndex;
	WORD    wLevelMin;
	WORD    wLevelMax;
	WORD	wRequestStrength;
	DWORD   dwZen;
	short   shErrorText;
} QUEST_CLASS_REQUEST;

typedef struct 
{
	short   shQuestConditionNum;
	short   shQuestRequestNum;
	WORD	wNpcType;

	unicode::t_char strQuestName[32];

	QUEST_CLASS_ACT     QuestAct[MAX_QUEST_CONDITION];
	QUEST_CLASS_REQUEST QuestRequest[MAX_QUEST_REQUEST];
} QUEST_ATTRIBUTE;

typedef struct 
{
	BYTE    byFlag;
	BYTE    byCount;
} QUEST_FLAG_BUFFER;

typedef struct 
{
	int iCrastGold;
	int iCrastSilver;
	int iCrastBronze;
} QUEST_CRAST;
//csquest end

//csmapinterface start 
typedef struct
{
	BYTE    bIndex;
	BYTE    x;
	BYTE    y;
}VisibleUnitLocation;

typedef struct 
{
	BYTE    byTeam;
	BYTE    byX;
	BYTE    byY;
	BYTE    byCmd;
	BYTE    byLifeTime;
}GuildCommander;
//csmapinterface end

//csitemsetoption start
typedef	struct 
{
	BYTE	byOption[2];
	BYTE	byMixItemLevel[2];
}ITEM_SET_TYPE;


typedef struct 
{
	char	strSetName[64];
	BYTE	byStandardOption[6][2];
	BYTE	byStandardOptionValue[6][2];
	BYTE	byExtOption[2];
	BYTE	byExtOptionValue[2];
	BYTE	byOptionCount;
	BYTE	byFullOption[5];
	BYTE	byFullOptionValue[5];
	BYTE	byRequireClass[MAX_CLASS];
}ITEM_SET_OPTION;
//csitemsetoption end

//cseventmatch start
typedef struct
{
	char	m_lpID[MAX_ID_SIZE];
	int		m_iScore;
	DWORD	m_dwExp;
	int		m_iZen;
}MatchResult;
//cseventmatch end

//cdirection start
struct DirectionMonster
{
	int		m_Index;
	int		m_iActionCheck;
	bool	m_bAngleCheck;
};

#define MAX_HELPER_ITEM		12
struct HELPER_INFO
{
	BYTE PickFlag;
	BYTE Range;
	WORD MovementTime;
	WORD SkillAttack1;
	WORD SkillAttack2;
	WORD SkillDelay1;
	WORD SkillAttack3;
	WORD SkillDelay2;
	WORD BuffPartyTime;
	WORD SkillBuff[3];
	WORD PercentFlag;
	DWORD OptionFlag;
	BYTE OtherData[36];
	char ItemList[MAX_HELPER_ITEM][16];
};
struct HELPER_STRUCT
{
	bool Started;
	bool Offline;
	int Stage;
	short StartX;
	short StartY;
	int Range[2];
	int MoveTime;
	int Skill[3];
	int Delay[2];
	int Buff[3];
	int PotPercent;
	int HealPercent;
	int DrainPercent;
	int PartyHealPercent;
	int PartyBuffTime;
	int PickJewel;
	int PickSet;
	int PickExc;
	int PickMoney;
	int PickExtra;
	char ItemList[12][16];
	int AutoPotion;
	int AutoHeal; //=hoi mau
	int AutoDrainLife; //hut mau
	int LongDistance;
	int OriginalPosition;
	int Combo;
	int Party;
	int PartyAutoHeal;
	int PartyAutoBuff;
	int DarkSpirit;
	int AutoBuff;
	int SkillDelay[2];
	int SkillCondition[2];
	int SkillPreCon[2];
	int SkillSubCon[2];
	int RepairItem;
	int PickAllItem;
	int PickSelected;
	int DarkSpiritAuto;
	int DarkSpiritAttack;
	int AutoAcceptFriend;
	int AutoAcceptGuild;
	int PotionElite;
	int ShortDistance;
	int RegularAttack;
	int SkillCache;
	bool ReBuff[2];
	int SelectedItem;
	DWORD TimerMoney;
	DWORD TimerStage;
	DWORD TimerOffline;
	DWORD TimerBuffParty;
	DWORD TimerMove;
	DWORD TimerSkill;
	DWORD TimerRepair;
	DWORD TimerDelay[3];
	void Clear()
	{
		SkillCache = 0;
		Started = 0;
		StartX = 0;
		StartY = 0;
		Stage = 0;
		TimerBuffParty = 0;
		TimerStage = 0;
		TimerOffline = 0;
		ReBuff[0] = 0;
		ReBuff[1] = 0;
		ReBuff[2] = 0;
		TimerBuffParty = 0;
		TimerMove = 0;
		TimerSkill = 0;
		TimerRepair = 0;
		TimerDelay[0] = 0;
		TimerDelay[1] = 0;
		TimerDelay[2] = 0;
		SelectedItem = -1;
	};
};
//cdirection end
#pragma pack(push, 1)
typedef struct
{
	BYTE gap05[44];
	/*+44*/		DWORD	Unknown44;
	/*+48*/		DWORD	Unknown48;
	/*+52*/		DWORD	Unknown52;
	/*+56*/		DWORD	Unknown56;
	/*+60*/		DWORD	Unknown60;
	//BYTE gap06[12];
	/*+64*/		DWORD	Time02;
	/*+68*/		DWORD	MapPosX;
	/*+72*/		DWORD	ObtainRange;
	/*+76*/		DWORD	Time03;
	/*+80*/		DWORD	Unknown80;
	BYTE gap07[76];
	/*+160*/	BYTE	Unknown160;
	BYTE gap08[7];
	/*+168*/	BYTE Unknown168;
	BYTE gap09[3];
	/*+172*/	DWORD	HuntingRange; //43 hutingrange
	/*+176*/	bool	RecoveryPotionOn;
	/*+177*/	BYTE	RecoveryPotionPercent;
	/*+178*/	bool	RecoveryHealOn;			//-> CLASS_ELF
	/*+179*/	BYTE	RecoveryHealPercent;	//-> CLASS_ELF
	/*+180*/	bool	RecoveryDrainOn;		//-> CLASS_SUMMONER
	/*+181*/	BYTE	RecoveryDrainPercent;	//-> CLASS_SUMMONER
	/*+182*/	bool	DistanceLongOn;
	/*+183*/	bool	DistanceReturnOn;
	/*+184*/	DWORD	DistanceMin;
	/*+188*/	DWORD	SkillBasicID;
	///*+192*/	SkillData SkillSecond1;
	///*+216*/	SkillData SkillSecond2;
	/*+192*/	DWORD	SkillSecond1ID;
	/*+196*/	BOOL	SkillSecond1DelayOn;
	/*+200*/	DWORD	SkillSecond1DelayTime;
	/*+204*/	BOOL	SkillSecond1CounterOn;
	/*+208*/	int		SkillSecond1CounterPre;
	/*+212*/	int		SkillSecond1CounterSub;
	/*+216*/	DWORD	SkillSecond2ID;
	/*+220*/	BOOL	SkillSecond2DelayOn;
	/*+224*/	DWORD	SkillSecond2DelayTime;
	/*+228*/	BOOL	SkillSecond2CounterOn;
	/*+232*/	int		SkillSecond2CounterPre;
	/*+236*/	int		SkillSecond2CounterSub;
	/*+240*/	bool	ComboOn;				//-> CLASS_KNIGHT
	/*+241*/	bool	PartyModeOn;
	/*+242*/	bool	PartyModeHealOn;		//-> CLASS_ELF
	/*+243*/	BYTE	PartyModeHealPercent;	//-> CLASS_ELF
	/*+244*/	BOOL	PartyModeBuffOn;
	/*+248*/	DWORD	PartyModeBuffInterval;
	/*+252*/	BOOL	DarkSpiritOn;			//-> CLASS_DARKLORD
	/*+256*/	DWORD	DarkSpiritMode;			//-> CLASS_DARKLORD
	/*+260*/	BOOL	BuffOn;
	/*+264*/	DWORD	BuffSkill[3];
	/*+276*/	DWORD	BuffExtra[3];
	//--
	/*+288*/	DWORD	ObtainRange22;
	/*+292*/	bool	ObtainRepairOn;
	/*+293*/	bool	ObtainPickNear;
	/*+294*/	bool	ObtainPickSelected;
	/*+295*/	bool	ObtainPickJewels;
	/*+296*/	bool	ObtainPickAncient;
	/*+297*/	bool	ObtainPickMoney;
	/*+298*/	bool	ObtainPickExcellent;
	/*+299*/	bool	ObtainPickExtra;
	/*+300*/	BYTE	ObtainPickItemList[28][12];
	/*+636*/	BYTE	Unknown636;
	/*+637*/	BYTE	Unknown637;
	/*+638*/	BYTE	Unknown638;
} MUHelper, * lpMUHelper;


#pragma pack(pop)




#endif ___STRUCT_H__
