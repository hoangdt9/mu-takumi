#include "StdAfx.h"
#include "UIMapName.h"
#include "MemScript.h"

CUIMapName g_pUIMapName;

CUIMapName::CUIMapName(void)
{
	memset(this->m_MapInfo, 0, sizeof(this->m_MapInfo));
}

CUIMapName::~CUIMapName(void)
{
}

void CUIMapName::OpenScritp(char* path)
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

	int Index = 0;
	memset(this->m_MapInfo, 0, sizeof(this->m_MapInfo));

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

			PathMap info;

			memset(&info, 0, sizeof(PathMap));

			info.world = lpMemScript->GetNumber();

			strcpy_s(info.MapName, lpMemScript->GetAsString());

			strcpy_s(info.TgaMapName, lpMemScript->GetAsString());

			strcpy_s(info.lpszMp3, lpMemScript->GetAsString());

			if (Index > -1 && Index < MAX_MAP && info.world > WD_81KARUTAN2)
			{
				this->m_MapInfo[Index] = info;
			}
			Index++;
		}
	}
	catch (...)
	{
		printf(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}