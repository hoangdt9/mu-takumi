#include "StdAfx.h"
#include "CustomNoticeDrop.h"
#include "MemScript.h"
#include "Message.h"
#include "Notice.h"
#include "Util.h"
#include "ItemManager.h"
#include "ServerInfo.h"
#include "ItemLevel.h"
#include "MapManager.h"

CustomNoticeDrop gCustomNoticeDrop;

CustomNoticeDrop::CustomNoticeDrop(void)
{
}

CustomNoticeDrop::~CustomNoticeDrop(void)
{
}


void CustomNoticeDrop::Load(char* path) // OK
{
	CMemScript* lpMemScript = new CMemScript;

	if (lpMemScript == 0)
	{
		ErrorMessageBox(MEM_SCRIPT_ALLOC_ERROR, path);
		return;
	}

	if (lpMemScript->SetBuffer(path) == 0)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
		delete lpMemScript;
		return;
	}

	this->m_ItemNotice.clear();

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

					this->Enable = lpMemScript->GetNumber();

					this->AllItemExc = lpMemScript->GetAsNumber();

					this->TypeShow = lpMemScript->GetAsNumber();

				}
				else if (section == 1)
				{
					if (strcmp("end", lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					ITEM_NOTICE_DROP info;

					info.index = SafeGetItem(GET_ITEM(lpMemScript->GetNumber(), lpMemScript->GetAsNumber()));

					info.level = lpMemScript->GetAsNumber();

					this->m_ItemNotice.push_back(info);
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
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

void CustomNoticeDrop::NoticeItem(LPOBJ lpObj, CItem* lpItem)
{
	if (!this->Enable)
	{
		return;
	}

	char buff[256], buff1[256], buff2[256];
	if (lpItem->IsItem() != 0)
	{
		int ItemType = lpItem->m_Index / 512;

		wsprintf(buff, gMessage.GetMessage(1112), lpObj->Name, gItemLevel.GetItemName(lpItem->m_Index, lpItem->m_Level), gMapManager.GetMapName(lpObj->Map));
		wsprintf(buff1, gMessage.GetMessage(1113), lpObj->Name, gItemLevel.GetItemName(lpItem->m_Index, lpItem->m_Level), gMapManager.GetMapName(lpObj->Map));
		wsprintf(buff2, gMessage.GetMessage(1114), lpObj->Name, gItemLevel.GetItemName(lpItem->m_Index, lpItem->m_Level), gMapManager.GetMapName(lpObj->Map));

		if (this->AllItemExc == 1 && ItemType <= 11 && lpItem->m_NewOption > 0)
		{
			if (this->TypeShow == 1)
			{
				PostMessageItemNotice(lpObj->Name, buff);
			}
			else
			{
				gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, buff1);
			}

		}
		else
		{
			for (std::vector<ITEM_NOTICE_DROP>::iterator it = this->m_ItemNotice.begin(); it != this->m_ItemNotice.end(); it++)
			{
				if (it->index == lpItem->m_Index && lpItem->m_Level >= it->level)
				{
					if (this->TypeShow == 1)
					{
						PostMessageItemNotice(lpObj->Name, buff);
					}
					else
					{
						gNotice.GCNoticeSendToAll(0, 0, 0, 0, 0, 0, buff2);
					}
					return;
				}
			}
		}

	}

}
