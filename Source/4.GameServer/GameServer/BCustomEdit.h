#pragma once
#if (FIXTELE ==1)

struct SKILKTELEFIX_DATA {
	int Index;
	int IDSkill;
};

struct DATA_SKILL_GIATLUI
{
	int IDSkill;
	int Rate;
	int Type;
};
struct DATA_SKILL_CHOANGCHAM
{
	int IDSkill;
	int Rate;
	int Delay;
};
class BCustomEdit
{
public:
	BCustomEdit();
	~BCustomEdit();
	// ----
	void	Load();
	//====
	void	SkillFixData(char* FilePath);
	BOOL	CheckSkillOK(int Skill);
	//===
	// ----Mapping
	std::map<int, SKILKTELEFIX_DATA> m_SetSkillTeleFix;




	void	FixSkillGiatLui(int aIndex, int bIndex, int Skill);
	std::map<int, DATA_SKILL_GIATLUI> m_DataSkillGiatLui;
	std::map<int, DATA_SKILL_CHOANGCHAM> m_DataSkillChoang;
	std::map<int, DATA_SKILL_CHOANGCHAM> m_DataSkillCham;
private:

	// ----
}; extern BCustomEdit gBCustomEdit;
#endif