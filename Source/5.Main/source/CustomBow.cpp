#include "stdafx.h"
#include "CustomBow.h"
#include "Util.h"
#include "Defined_Global.h"


CCustomBow gCustomBow;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCustomBow::CCustomBow() // OK
{
	this->m_CustomBowInfo.clear();
}

CCustomBow::~CCustomBow() // OK
{

}

DWORD SkillBow(DWORD BowItem)
{
	return gCustomBow.GetCustomSkill(BowItem);
}


void CCustomBow::Load(CUSTOM_BOW_INFO* info) // OK
{
	for(int n=0;n < MAX_CUSTOM_BOW;n++)
	{
		if(info[n].Index < 0 || info[n].Index >= MAX_CUSTOM_BOW)
		{
			return;
		}
		this->m_CustomBowInfo.insert(std::pair<int,CUSTOM_BOW_INFO>(info[n].ItemIndex,info[n]));
	}
}

CUSTOM_BOW_INFO* CCustomBow::GetInfoByItem(int ItemIndex) // OK
{
	CUSTOM_BOW_INFO* value;

	std::map<int,CUSTOM_BOW_INFO>::iterator it = this->m_CustomBowInfo.find(ItemIndex);

	if(it != this->m_CustomBowInfo.end())
	{
		(*value) = it->second;

		return value;
	}

	return 0;
}

bool CCustomBow::CheckCustomBow(int ItemIndex) // OK
{
	std::map<int,CUSTOM_BOW_INFO>::iterator it = this->m_CustomBowInfo.find(ItemIndex);
	
	if(it != this->m_CustomBowInfo.end())
	{
		return it->second.ItemType == 0;
	}

	return false;
}

bool CCustomBow::CheckCustomCross(int ItemIndex) // OK
{
	std::map<int,CUSTOM_BOW_INFO>::iterator it = this->m_CustomBowInfo.find(ItemIndex);
	
	if(it != this->m_CustomBowInfo.end())
	{
		return it->second.ItemType == 1;
	}

	return false;
}

int CCustomBow::GetCustomSkill(int ItemIndex) // OK
{
	std::map<int, CUSTOM_BOW_INFO>::iterator it = this->m_CustomBowInfo.find(ItemIndex);
	
	if(it != this->m_CustomBowInfo.end())
	{
		return it->second.SkillShot;
	}
	return -1;
}
