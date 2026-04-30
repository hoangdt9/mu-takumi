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
	}
}

void CustomDmgColor::Load(char* path)
{
	CMemScript* lpMemScript = new CMemScript;

	if(lpMemScript == 0)
	{
		printf(MEM_SCRIPT_ALLOC_ERROR,path);
		return;
	}

	if(lpMemScript->SetBuffer(path) == 0)
	{
		printf(lpMemScript->GetLastError());
		delete lpMemScript;
		return;
	}

	this->Init();

	try
	{
		while(true)
		{
			if(lpMemScript->GetToken() == TOKEN_END)
			{
				break;
			}

			if(strcmp("end",lpMemScript->GetString()) == 0)
			{
				break;
			}

			CUSTOM_DMG_COLOR info;

			memset(&info,0,sizeof(info));

			static int CustomItemIndexCount = 0;

			info.Index = CustomItemIndexCount++;

			info.TypeIndex = lpMemScript->GetNumber();
				
			info.Red = lpMemScript->GetAsNumber();

			info.Green = lpMemScript->GetAsNumber();

			info.Blue = lpMemScript->GetAsNumber();

			this->SetInfo(info);
		}
	}
	catch(...)
	{
		printf(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

void CustomDmgColor::SetInfo(CUSTOM_DMG_COLOR info) // OK
{
	if(info.Index < 0 || info.Index >= MAX_CUSTOM_DMG)
	{
		return;
	}
	this->m_CustomDmgColor[info.Index] = info;
}
