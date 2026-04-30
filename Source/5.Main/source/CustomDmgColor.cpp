#include "stdafx.h"
#include "CustomDmgColor.h"
#include "MemScript.h"

CustomDmgColor gCustomDmgColor;

CustomDmgColor::CustomDmgColor()
{
	this->Init();
}

CustomDmgColor::~CustomDmgColor()
{

}

void CustomDmgColor::Init()
{
	for (int n = 0; n < MAX_CUSTOM_DMG; n++)
	{
		this->m_CustomDmgColor[n].Index = -1;

		this->m_CustomDmgColor[n].TypeIndex = -1;

		this->m_CustomDmgColor[n].Red = -1;

		this->m_CustomDmgColor[n].Green = -1;

		this->m_CustomDmgColor[n].Blue = -1;
	}
}

void CustomDmgColor::Load(CUSTOM_DMG_COLOR* info)
{
	this->Init();

	for (int n = 0; n < MAX_CUSTOM_DMG; n++)
	{
		this->SetInfo(info[n]);
	}
}

void CustomDmgColor::SetInfo(CUSTOM_DMG_COLOR info) // OK
{
	if (info.Index < 0 || info.Index >= MAX_CUSTOM_DMG)
	{
		return;
	}

	this->m_CustomDmgColor[info.Index] = info;
}

CUSTOM_DMG_COLOR* CustomDmgColor::GetInfo(int index) // OK
{
	if (index < 0 || index >= MAX_CUSTOM_DMG)
	{
		return 0;
	}

	if (this->m_CustomDmgColor[index].Index != index)
	{
		return 0;
	}

	return &this->m_CustomDmgColor[index];
}
CUSTOM_DMG_COLOR* CustomDmgColor::GetInfoTypeIndex(int TypeIndex) // OK
{
	for (int n = 0; n < MAX_CUSTOM_DMG; n++)
	{
		CUSTOM_DMG_COLOR* lpInfo = this->GetInfo(n);

		if (lpInfo == 0)
		{
			continue;
		}

		if (lpInfo->TypeIndex == TypeIndex && lpInfo->Index != -1)
		{
			return lpInfo;
		}
	}

	return 0;
}