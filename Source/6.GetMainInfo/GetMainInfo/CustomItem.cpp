// CustomItem.cpp: implementation of the CCustomItem class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CustomItem.h"
#include "MemScript.h"

CCustomItem gCustomItem;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCustomItem::CCustomItem() // OK
{
	this->Init();
}

CCustomItem::~CCustomItem() // OK
{

}

void CCustomItem::Init() // OK
{
	for (int n = 0; n < MAX_CUSTOM_ITEM; n++)
	{
		this->m_CustomItemInfo[n].Index = -1;
		this->m_CustomItemInfo[n].ItemIndex = -1;

		this->m_CustomRingPenInfo[n].Index = -1;
		this->m_CustomRingPenInfo[n].ItemIndex = -1;
		this->m_CustomRingPenInfo[n].Type = -1;
	}
}

void CCustomItem::LoadRingPen(char* path) // OK
{
	CMemScript* lpMemScript = new CMemScript;

	if (lpMemScript == 0)
	{
		printf(MEM_SCRIPT_ALLOC_ERROR, path);
		return;
	}

	if (lpMemScript->SetBuffer(path) == 0)
	{
		printf(lpMemScript->GetLastError());
		delete lpMemScript;
		return;
	}

	int CountRingPen = 0;

	try
	{
		while (true)
		{
			if (lpMemScript->GetToken() == TOKEN_END)
			{
				break;
			}

			int section = lpMemScript->GetNumber();

			while (true)
			{
				if (section == 0)
				{
					if (strcmp("end", lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					CUSTOM_RINGPEN_INFO info;

					memset(&info, 0, sizeof(info));

					info.Index = CountRingPen++;

					info.ItemIndex = lpMemScript->GetNumber();

					info.Type = 0;


					this->SetInfoRingPen(info);
				}
				else if (section == 1)
				{
					if (strcmp("end", lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					CUSTOM_RINGPEN_INFO info;

					memset(&info, 0, sizeof(info));

					info.Index = CountRingPen++;

					info.ItemIndex = lpMemScript->GetNumber();

					info.Type = 1;

					this->SetInfoRingPen(info);
				}
				else
				{
					break;
				}
			}
		}
	}
	catch (...)
	{
		printf(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}
void CCustomItem::SetInfoRingPen(CUSTOM_RINGPEN_INFO info) // OK
{
	if (info.Index < 0 || info.Index >= MAX_CUSTOM_ITEM)
	{
		return;
	}

	this->m_CustomRingPenInfo[info.Index] = info;
}

void CCustomItem::Load(char* path) // OK
{
	CMemScript* lpMemScript = new CMemScript;

	if (lpMemScript == 0)
	{
		printf(MEM_SCRIPT_ALLOC_ERROR, path);
		return;
	}

	if (lpMemScript->SetBuffer(path) == 0)
	{
		printf(lpMemScript->GetLastError());
		delete lpMemScript;
		return;
	}

	this->Init();
	int CustomItemIndexCount = 0;
	try
	{
		while (true)
		{
			if (lpMemScript->GetToken() == TOKEN_END)
			{
				break;
			}

			if (strcmp("end", lpMemScript->GetString()) == 0)
			{
				break;
			}

			CUSTOM_ITEM_INFO info;

			memset(&info, 0, sizeof(info));

			info.Index = CustomItemIndexCount++;

			info.ItemIndex = lpMemScript->GetNumber();

			info.ColorR = lpMemScript->GetAsNumber();

			info.ColorG = lpMemScript->GetAsNumber();

			info.ColorB = lpMemScript->GetAsNumber();

			strcpy_s(info.ModelName, lpMemScript->GetAsString());

			this->SetInfo(info);
		}
	}
	catch (...)
	{
		printf(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

void CCustomItem::SetInfo(CUSTOM_ITEM_INFO info) // OK
{
	if (info.Index < 0 || info.Index >= MAX_CUSTOM_ITEM)
	{
		return;
	}

	this->m_CustomItemInfo[info.Index] = info;
}
