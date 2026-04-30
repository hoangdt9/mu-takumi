#pragma once

struct CUSTOM_BOW_INFO
{
	int Index;
	int ItemIndex;
	char ModelName[32];
	int ItemType;
	int SkillShot;
	int ColorR;
	int ColorG;
	int ColorB;
};

class CCustomBow
{
public:
	CCustomBow();
	virtual ~CCustomBow();
	void Load(CUSTOM_BOW_INFO* info);
	CUSTOM_BOW_INFO* GetInfoByItem(int ItemIndex);
	bool CheckCustomBow(int ItemIndex);
	bool CheckCustomCross(int ItemIndex);
	int GetCustomSkill(int ItemIndex); // OK
public:
	std::map<int,CUSTOM_BOW_INFO> m_CustomBowInfo;
};

extern CCustomBow gCustomBow;