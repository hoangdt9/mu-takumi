// CharacterManager.h: interface for the CCharacterManager class.
//////////////////////////////////////////////////////////////////////
#pragma once


class CCharacterManager
{
	enum DB_CLASS_CODES {
		DB_DARK_WIZARD = 0,
		DB_SOUL_MASTER = 1,
		DB_GRAND_MASTER = 2,

		DB_DARK_KNIGHT = 16,
		DB_BLADE_KNIGHT = 17,
		DB_BLADE_MASTER = 18,

		DB_FAIRY_ELF = 32,
		DB_MUSE_ELF = 33,
		DB_HIGH_ELF = 34,

		DB_MAGIC_GLADIATOR = 48,
		DB_DUEL_MASTER = 49,

		DB_DARK_LORD = 64,
		DB_LORD_EMPEROR = 65,

		DB_SUMMONER = 80,
		DB_BLOODY_SUMMONER = 81,
		DB_DIMENSION_MASTER = 82,

		DB_RAGEFIGHER = 96,
		DB_RAGEFIGHER_EVO = 97,
	};

	enum PROTO_CLASS_CODES {
		PR_DARK_WIZARD = 0x0,
		PR_SOUL_MASTER = 0x10,
		PR_GRAND_MASTER = 0x18,

		PR_DARK_KNIGHT = 0x20,
		PR_BLADE_KNIGHT = 0x30,
		PR_BLADE_MASTER = 0x38,

		PR_FAIRY_ELF = 0x40,
		PR_MUSE_ELF = 0x50,
		PR_HIGH_ELF = 0x58,

		PR_MAGIC_GLADIATOR = 0x60,
		PR_DUEL_MASTER = 0x78,

		PR_DARK_LORD = 0x80,
		PR_LORD_EMPEROR = 0x98,

		PR_SUMMONER = 0x0A0,
		PR_BLOODY_SUMMONER = 0x0B0,
		PR_DIMENSION_MASTER = 0x0B8,

		PR_RAGEFIGHER = 0xC0,
		PR_RAGEFIGHER_EVO = 0xD8,
	};
public:
	CCharacterManager();
	virtual ~CCharacterManager();
	BYTE ChangeServerClassTypeToClientClassType(const BYTE byServerClassType);
	BYTE GetCharacterClass(const BYTE byClass);
	BYTE GetCharacterClassCode(const BYTE DbClass);
	bool IsSecondClass(const BYTE byClass);
	bool IsThirdClass(const BYTE byClass);
	bool IsMasterLevel(const BYTE byClass);
	const char* GetCharacterClassText(const BYTE byClass);
	int GetBaseClass( int iClass)	{ return ( 0x7 & iClass); }
	int IsFemale(int iClass) { return (this->GetBaseClass(iClass) == CLASS_ELF || this->GetBaseClass(iClass) == CLASS_SUMMONER); }
	BYTE GetSkinModelIndex(const BYTE byClass);
	BYTE GetStepClass(const BYTE byClass);
	int GetEquipedBowType(CHARACTER *pChar);
	int GetEquipedBowType();
	int GetEquipedBowType(ITEM* pItem);
	int GetEquipedBowType_Skill( );
	bool IsEquipedWing();
	void GetMagicSkillDamage( int iType, int *piMinDamage, int *piMaxDamage);
	void GetCurseSkillDamage(int iType, int *piMinDamage, int *piMaxDamage);
    void GetSkillDamage( int iType, int *piMinDamage, int *piMaxDamage);
public:

};

extern CCharacterManager gCharacterManager;
