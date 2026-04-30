// Move.h: interface for the CMove class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Protocol.h"
#include "User.h"

struct CUSTOMCOMBO_INFO
{
	int Index;
	int Skill;
};
struct CUSTOMCOMBO_ONESKILL
{
	int Index;
	int Skill;
	int Rate;
};
class CCustomCombo
{
public:
	CCustomCombo();
	virtual ~CCustomCombo();
	void Load(char* path);
	bool CheckSkillPrimary(int Skill);
	bool CheckSkillSecundary(int Skill);
	bool CheckOneSkillCombo(int Skill);
private:
	std::map<int,CUSTOMCOMBO_INFO> m_CustomComboSkillPrimary;
	std::map<int,CUSTOMCOMBO_INFO> m_CustomComboSkillSecundary;
	std::map<int, CUSTOMCOMBO_ONESKILL> m_CustomComboOneSkill;
};

extern CCustomCombo gCustomCombo;
