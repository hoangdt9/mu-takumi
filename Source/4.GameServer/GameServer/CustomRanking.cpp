#include "stdafx.h"
#include "DSProtocol.h"
#include "GameMain.h"
#include "Util.h"
#include "Message.h"
#include "User.h"
#include "Path.h"
#include "ServerInfo.h"
#include "MemScript.h"
#include "ItemManager.h"
#include "CustomRanking.h"
#include "Notice.h"

CCustomRanking gCustomRanking;

void CCustomRanking::Load(char* path)
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

	for (int n = 0; n < MAX_RANK; n++)
	{
		this->r_Data[n];
	}

	this->m_count = 0;

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

					int index = lpMemScript->GetNumber();

					strcpy_s(this->r_Data[index].Name, lpMemScript->GetAsString());

					strcpy_s(this->r_Data[index].Col1, lpMemScript->GetAsString());

					strcpy_s(this->r_Data[index].Col2, lpMemScript->GetAsString());

					strcpy_s(this->r_Data[index].Col3, lpMemScript->GetAsString());

					this->m_count++;
				}
			}
		}
	}
	catch (...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;

	//LogAdd(LOG_BLUE, "[ServerInfo] CustomRanking Info loaded successfully");
}

void CCustomRanking::GCReqRankingCount(int Index, PMSG_CUSTOM_RANKING_COUNT_RECV* lpMsg)
{


	if (gServerInfo.m_CustomRankingSwitch == 0)
	{
		return;
	}

	PMSG_CUSTOM_RANKING_COUNT_SEND pMsg;

	pMsg.header.set(0xF3, 0xE7, sizeof(pMsg));

	pMsg.count = (this->m_count > MAX_RANK) ? MAX_RANK : this->m_count;

	DataSend(Index, (LPBYTE)& pMsg, pMsg.header.size);



}

void CCustomRanking::SendMain(LPOBJ lpObj)
{
	PMSG_RANK_INFO pMsg;
	memset(&pMsg, 0, sizeof(pMsg));
	pMsg.h.set((LPBYTE)& pMsg, 0xFA, 0x11, sizeof(pMsg));
	for (int n = 0; n < this->m_count; n++)
	{
		CUSTOM_RANKING* lpInfo = &this->m_RankInfo[n];

		memcpy(pMsg.Info[n].NameRank, this->r_Data[n].Name, 128);
	}

	DataSend(lpObj->Index, (LPBYTE)& pMsg, sizeof(pMsg));
	//LogAdd(LOG_RED, "SendMain [%s]", lpObj->Name);
}



void CCustomRanking::GCReqRanking(int Index, PMSG_CUSTOM_RANKING_RECV* lpMsg)
{


	if (gServerInfo.m_CustomRankingSwitch == 0)
	{
		return;
	}

	if (gObjIsConnected(Index) == false)
	{
		return;
	}

	if (this->m_count <= 0)
	{
		return;
	}

	if (lpMsg->type < 0 || lpMsg->type >= this->m_count)
	{
		return;
	}

	if (this->m_count == 0)
	{
		return;
	}

	SDHP_CUSTOM_RANKING_SEND pMsg;

	pMsg.header.set(0xF4, sizeof(pMsg));

	pMsg.index = Index;

	pMsg.type = lpMsg->type;

	gDataServerConnection.DataSend((BYTE*)& pMsg, pMsg.header.size);


}

void CCustomRanking::GDCustomRankingRecv(SDHP_CUSTOM_RANKING_RECV* lpMsg)
{

	if (lpMsg == NULL)
	{
		return;
	}

	BYTE send[4096];

	PMSG_CUSTOM_RANKING_SEND pMsg;

	pMsg.header.set(0xF3, 0xE6, 0);

	int size = sizeof(pMsg);

	memcpy(pMsg.rankname, this->r_Data[lpMsg->type].Name, sizeof(pMsg.rankname));
	memcpy(pMsg.col1, this->r_Data[lpMsg->type].Col1, sizeof(pMsg.col1));
	memcpy(pMsg.col2, this->r_Data[lpMsg->type].Col2, sizeof(pMsg.col2));
	memcpy(pMsg.col3, this->r_Data[lpMsg->type].Col3, sizeof(pMsg.col3));

	pMsg.count = 0;

	CUSTOM_RANKING_DATA info;

	for (int n = 0; n < lpMsg->count; n++)
	{
		CUSTOM_RANKING_DATA* lpInfo = (CUSTOM_RANKING_DATA*)(((BYTE*)lpMsg) + sizeof(SDHP_CUSTOM_RANKING_RECV) + (sizeof(CUSTOM_RANKING_DATA) * n));
		memcpy(info.szName, lpInfo->szName, sizeof(info.szName));
		memcpy(info.value2, lpInfo->value2, sizeof(info.value2));
#if(CUSTOM_BRANKINGNEW)
		info.CheckOnline = lpInfo->CheckOnline;
#endif
		// time reset
		memcpy(info.value3, lpInfo->value3, sizeof(info.value3));
		//		LogAdd(LOG_RED, "%s",info.value3);
		pMsg.count++;
		memcpy(&send[size], &info, sizeof(info));
		size += sizeof(info);
	}

	pMsg.header.size[0] = SET_NUMBERHB(size);
	pMsg.header.size[1] = SET_NUMBERLB(size);
	// ---
	memcpy(send, &pMsg, sizeof(pMsg));

	DataSend(lpMsg->index, send, size);


}

#if(CUSTOM_BRANKINGNEW)
void CCustomRanking::CGetInfoCharTop(REQUESTINFO_CHARTOP * lpMsg, int aIndex)
{
	if (lpMsg == NULL)
	{
		return;
	}
	if (gObjIsConnected(aIndex) == false)
	{
		return;
	}
	if (GetTickCount() < gObj[aIndex].ClickClientSend + 1000)
	{
		return;
	}
	gObj[aIndex].ClickClientSend = GetTickCount();
	REQUESTINFO_CHARTOP pMsg;
	pMsg.header.set(0xD3, 0x40, sizeof(pMsg));
	pMsg.aIndex = aIndex;
	memcpy(pMsg.NameChar, lpMsg->NameChar, sizeof(pMsg.NameChar) - 1);
	gDataServerConnection.DataSend((BYTE*)& pMsg, pMsg.header.size);
}

void CCustomRanking::RecvInfoCharTop(DATA_VIEWTOPRANKING * lpMsg)
{
	if (lpMsg == NULL)
	{
		return;
	}
	int aIndex = lpMsg->aIndex;

	if (gObjIsConnected(aIndex) == false)
	{
		return;
	}
	BYTE send[4096];

	DATA_VIEWTOPRANKING pMsg;

	pMsg.header.set(0xD3, 0x40, 0);

	int size = sizeof(pMsg);

	memcpy(pMsg.NameChar, lpMsg->NameChar, sizeof(pMsg.NameChar));
	pMsg.TongPoint = lpMsg->TongPoint;
	pMsg.Reset = lpMsg->Reset;
	pMsg.Level = lpMsg->Level;
	pMsg.aIndex = lpMsg->aIndex;
	pMsg.MasterLevel = lpMsg->MasterLevel;
	pMsg.aIndex = lpMsg->aIndex;
	pMsg.Class = lpMsg->Class;
	CItem lpItemTest[12];

	for (int n = 0; n < 12; n++)
	{
		lpItemTest[n].Clear();
		gItemManager.ConvertItemByte(&lpItemTest[n], lpMsg->Item[n]);
		if (lpItemTest[n].IsItem())
		{
			gItemManager.ItemByteConvert(pMsg.Item[n], lpItemTest[n]);
		}
		else
		{
			memset(&pMsg.Item[n], 0xFF, sizeof(pMsg.Item[n]));
		}
	}
	memcpy(pMsg.GuildName, lpMsg->GuildName, sizeof(pMsg.GuildName));
	memcpy(pMsg.GuildMark, lpMsg->GuildMark, sizeof(pMsg.GuildMark));
	pMsg.header.size[0] = SET_NUMBERHB(size);
	pMsg.header.size[1] = SET_NUMBERLB(size);
	// ---
	memcpy(send, &pMsg, sizeof(pMsg));

	DataSend(aIndex, send, size);
	//LogAdd(LOG_RED, "RecvInfoCharTop Send Info Item %s", gObj[aIndex].Name);
}
#endif