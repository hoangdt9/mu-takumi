#include "stdafx.h"
#include "CustomGloves.h"
#include "Defined_Global.h"
#include "Util.h"

cCustomGloves gCustomBattleGloves;

cCustomGloves::cCustomGloves() // OK
{
	this->m_CustomGlovesInfo.clear();
}

cCustomGloves::~cCustomGloves() // OK
{

}

#pragma optimize("t",off)
void cCustomGloves::Load(CUSTOM_RF_GLOVES* info) // OK
{
	for (int n = 0; n < MAX_CUSTOM_GLOVES; n++)
	{
		if (info[n].Index < 0 || info[n].Index >= MAX_CUSTOM_GLOVES)
		{
			return;
		}

		this->m_CustomGlovesInfo.insert(std::pair<int, CUSTOM_RF_GLOVES>(info[n].ItemIndex,info[n]));
	}
}

int cCustomGloves::GetItemIndex(int ItemIndex)
{
	std::map<int, CUSTOM_RF_GLOVES>::iterator it = this->m_CustomGlovesInfo.find(ItemIndex);

	if(it != this->m_CustomGlovesInfo.end())
	{
		return it->second.ItemIndex;
	}
	return -1;
}

bool cCustomGloves::CheckGloves(int ItemIndex)
{
	std::map<int, CUSTOM_RF_GLOVES>::iterator it = this->m_CustomGlovesInfo.find(ItemIndex);

	if(it != this->m_CustomGlovesInfo.end())
	{
		return true;
	}
	return false;
}

