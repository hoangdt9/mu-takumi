#include "StdAfx.h"
#include "CB_NewQuest.h"
#include "user.h"
#include "GameMain.h"
#include "DSProtocol.h"
#include "log.h"
#include "Monster.h"
#include "GameMain.h"
#include "SocketManager.h"
#include "Notice.h"
#include "ItemManager.h"
#include "MapServerManager.h"
#include "CashShop.h"
#include "Message.h"
#include "ItemBagManager.h"
#include "MapServerManager.h"
#include "Party.h"
#include "ServerInfo.h"
#include "Path.h"
#include "MemScript.h"
#include "Util.h"
#include "MonsterSetBase.h"

#if(CB_NEWQUESTFF)

CB_NewQuest gCB_NewQuest;

CB_NewQuest::CB_NewQuest()
{
}


CB_NewQuest::~CB_NewQuest()
{
}
void CB_NewQuest::LoadData(char* path)
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

	this->m_DataNPCList.clear();
	this->m_DataNQuestReq.clear();
	this->m_DataNQestReward.clear();
	this->MaxLevelNQuest = 0;
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

					CB_NewQuest::DATA_NPC infoNPC;

					memset(&infoNPC, 0, sizeof(infoNPC));

					infoNPC.Class = lpMemScript->GetNumber();
					infoNPC.Map = lpMemScript->GetAsNumber();
					infoNPC.X= lpMemScript->GetAsNumber();
					infoNPC.Y= lpMemScript->GetAsNumber();
					infoNPC.Dir = lpMemScript->GetAsNumber();
					this->m_DataNPCList.push_back(infoNPC);

				}
				else if (section == 1)
				{
					if (strcmp("end", lpMemScript->GetAsString()) == 0)
					{
						break;
					}
					CB_NewQuest::DATA_NQUEST_REQ infoReq;

					memset(&infoReq, 0, sizeof(infoReq));
					infoReq.NQuest_LV = lpMemScript->GetNumber();
					strcpy_s(infoReq.NameQuest, lpMemScript->GetAsString());
					for (int i = 0; i < 5; i++)
					{
						infoReq.MonterClass[i] = lpMemScript->GetAsNumber();
						infoReq.NQuest_C[i] = lpMemScript->GetAsNumber();
					}

					infoReq.Map = lpMemScript->GetAsNumber();
					infoReq.ReqLevel = lpMemScript->GetAsNumber();
					infoReq.ReqBless = lpMemScript->GetAsNumber();
					infoReq.ReqSoul = lpMemScript->GetAsNumber();
					infoReq.ReqLife = lpMemScript->GetAsNumber();
					infoReq.ReqChaos = lpMemScript->GetAsNumber();
					infoReq.ReqCre = lpMemScript->GetAsNumber();
					infoReq.ReqPoint = lpMemScript->GetAsNumber();

					this->m_DataNQuestReq.insert(std::pair<int, CB_NewQuest::DATA_NQUEST_REQ>(infoReq.NQuest_LV, infoReq));
					this->MaxLevelNQuest = infoReq.NQuest_LV;

				}
				else if (section == 2)
				{
					if (strcmp("end", lpMemScript->GetAsString()) == 0)
					{
						break;
					}
					CB_NewQuest::DATA_NQUEST_REWARD infoReward;

					memset(&infoReward, 0, sizeof(infoReward));
					infoReward.NQuest_LV = lpMemScript->GetNumber();
					infoReward.ItemIndex = SafeGetItem(GET_ITEM(lpMemScript->GetAsNumber(), lpMemScript->GetAsNumber()));
					infoReward.Level= lpMemScript->GetAsNumber();
					infoReward.Skill= lpMemScript->GetAsNumber();
					infoReward.Luck = lpMemScript->GetAsNumber();
					infoReward.Option = lpMemScript->GetAsNumber();
					infoReward.Exc = lpMemScript->GetAsNumber();
					infoReward.Set= lpMemScript->GetAsNumber();
					infoReward.Dur= lpMemScript->GetAsNumber();
					infoReward.Count = lpMemScript->GetAsNumber();
					this->m_DataNQestReward.push_back(infoReward);
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
	LogAdd(LOG_BLUE, "[NewQuest] Load OK Max [%d] NPC %d, Quest %d, Reward %d", this->MaxLevelNQuest, this->m_DataNPCList.size(), this->m_DataNQuestReq.size(), this->m_DataNQestReward.size());
}
 
void CB_NewQuest::SetNPCList()
{
	for (std::vector<CB_NewQuest::DATA_NPC>::iterator iter = m_DataNPCList.begin(); iter != m_DataNPCList.end(); ++iter)
	{
		if (iter->Class != 0)
		{
			MONSTER_SET_BASE_INFO info;
			memset(&info, 0, sizeof(info));

			info.Type = 0;
			info.MonsterClass = iter->Class;
			info.Map = iter->Map;
			info.Dis = 0;
			info.Dir = iter->Dir;
			info.X = iter->X;
			info.Y = iter->Y;
			gMonsterSetBase.SetInfo(info);
		}		
	}
}
void CB_NewQuest::RecvProtocol(int aIndex,BYTE* Recv)
{
	if (gObj[aIndex].Type != OBJECT_USER)
	{
		return;
	}

	if (gObj[aIndex].Map == 107 || gObj[aIndex].Map == 109)
	{
		return;
	}

	if (gObjIsConnected(aIndex) == false)
	{
		return;
	}
	if (gObj[aIndex].IsBot >= 1 || gObj[aIndex].m_OfflineMode != 0 || gObj[aIndex].IsFakeOnline != 0)
	{
		return;
	}
	if (!Recv) return;
	XULY_CGPACKET* lpMsg = (XULY_CGPACKET*)Recv;
	if (lpMsg->ThaoTac != 1154) return;
	//LogAdd(LOG_RED, "RecvProtocol");
	LPOBJ lpObj = &gObj[aIndex];

	//== 0 = Nhan NV == 1 Hoan thanh NV
	if (lpObj->NQuest_Status == eDaDuYeuCau)
	{
		//== Nhan Thuong
		int LevelQuest = lpObj->NQuest_LV;
		//==Clear
		lpObj->NQuest_Status = eNone;
		lpObj->NQuest_C[0] = 0;
		lpObj->NQuest_C[1] = 0;
		lpObj->NQuest_C[2] = 0;
		lpObj->NQuest_C[3] = 0;
		lpObj->NQuest_C[4] = 0;

		for (int i = 0; i < this->m_DataNQestReward.size(); i++)
		{
			if (this->m_DataNQestReward[i].NQuest_LV != LevelQuest) continue;

			for (int n = 0; n < this->m_DataNQestReward[i].Count; n++)
			{
				GDCreateItemSend(lpObj->Index, 235, 0, 0, this->m_DataNQestReward[i].ItemIndex, (BYTE)this->m_DataNQestReward[i].Level, (BYTE)this->m_DataNQestReward[i].Dur, (BYTE)this->m_DataNQestReward[i].Skill, (BYTE)this->m_DataNQestReward[i].Luck, this->m_DataNQestReward[i].Option, -1, this->m_DataNQestReward[i].Exc, this->m_DataNQestReward[i].Set, 0, 0, 0, 0xFF, 0);
			}
			
		}

		this->SendInfoNQuest(lpObj->Index, lpObj->NQuest_LV + 1);
		gNotice.GCNoticeSend(lpObj->Index, eMessageBox, 0, 0, 0, 0, 0, "Chúc mừng bạn đã nhận thưởng Nhiệm Vụ, hãy cố gắng nhé !");

	}
	else if (lpObj->NQuest_Status == eNone)
	{
		//==== Nhan Nhiem Vu
		int LevelQuest = lpObj->NQuest_LV + 1;
		std::map<int, CB_NewQuest::DATA_NQUEST_REQ>::iterator it = this->m_DataNQuestReq.find(LevelQuest);

		if (it == this->m_DataNQuestReq.end())
		{
			LogAdd(LOG_RED, "Khong co du lieu LevelQuest %d", LevelQuest);
			return;
		}
		lpObj->NQuest_LV = LevelQuest;
		lpObj->NQuest_Status = 1;

		this->SendInfoNQuest(lpObj->Index, LevelQuest, TRUE);
		LogAdd(LOG_RED, "%s Nhan NV LevelQuest %d", lpObj->Name, LevelQuest);
	}
	else
	{
		gNotice.GCNoticeSend(lpObj->Index, eMessageBox, 0, 0, 0, 0, 0, "Vui lòng hoàn thành yêu cầu của Nhiệm Vụ !!");
	}

	if (lpObj->NQuest_Status != eNone) this->SendInfoMonterProc(aIndex);
}

bool CB_NewQuest::CheckNpcQuest(LPOBJ lpObj, LPOBJ lpNpc) // OK
{

	for (std::vector<CB_NewQuest::DATA_NPC>::iterator it = this->m_DataNPCList.begin(); it != this->m_DataNPCList.end(); it++)
	{
		if (it->Class == lpNpc->Class && it->Map == lpNpc->Map && it->X == lpNpc->X && it->Y == lpNpc->Y)
		{
			//--OpenNPCQuest
			if (lpObj->NQuest_Status !=  eNone)
			{
				this->SendInfoNQuest(lpObj->Index, lpObj->NQuest_LV, TRUE);
			}
			else
			{
				this->SendInfoNQuest(lpObj->Index, lpObj->NQuest_LV + 1, TRUE);
			}

			if (lpObj->NQuest_Status != eNone) this->SendInfoMonterProc(lpObj->Index);
		}
	}

	return 0;
}
void CB_NewQuest::MonsterDieProc(LPOBJ Monter, LPOBJ lpObj) // OK
{
	if (lpObj->Type != OBJECT_USER)
	{
		return;
	}

	if (lpObj->Map == 107 || lpObj->Map == 109)
	{
		return;
	}

	if (gObjIsConnected(lpObj->Index) == false)
	{
		return;
	}
	if (lpObj->IsBot >= 1 || lpObj->m_OfflineMode != 0 || lpObj->IsFakeOnline != 0)
	{
		return;
	}

	if (lpObj->NQuest_Status != eDangLam)
	{
		return;
	}
	std::map<int, CB_NewQuest::DATA_NQUEST_REQ>::iterator it = this->m_DataNQuestReq.find(lpObj->NQuest_LV);

	if (it == this->m_DataNQuestReq.end())
	{
		//LogAdd(LOG_RED, "Khong co du lieu LevelQuest %d", lpObj->NQuest_LV);
		return;
	}
	//LogAdd(LOG_RED, "MonsterDieProc NQuest %d (%d)", Monter->Class, Monter->Map);
	if (it->second.Map != Monter->Map)
	{
		return;
	}
	int DoneQuest = 0;
	for (int i = 0; i < 5; i++)
	{
		if (lpObj->NQuest_C[i] >= it->second.NQuest_C[i] || it->second.NQuest_C[i] == -1)
		{
			DoneQuest++;
			continue;
		}
		//LogAdd(LOG_BLUE, "[%d] MonterClass %d ~ %d /%d",i, Monter->Class, it->second.MonterClass[i], it->second.NQuest_C[i]);
		if (it->second.MonterClass[i] == Monter->Class)
		{
			lpObj->NQuest_C[i]++;
			break;
		}
	}
	if (DoneQuest > 4)
	{
		lpObj->NQuest_Status = eDaDuYeuCau;
	}
	//LogAdd(LOG_RED, "MonsterDieProc NQuest %d (%d) DoneQuest %d", Monter->Class, Monter->Map, DoneQuest);
	this->SendInfoMonterProc(lpObj->Index);
	this->SendInfoNQuest(lpObj->Index, lpObj->NQuest_LV);
	//lpObj Mon
}
void CB_NewQuest::SendInfoMonterProc(int aIndex)
{
	if (gObj[aIndex].Type != OBJECT_USER)
	{
		return;
	}

	if (gObj[aIndex].Map == 107 || gObj[aIndex].Map == 109)
	{
		return;
	}

	if (gObjIsConnected(aIndex) == false)
	{
		return;
	}
	if (gObj[aIndex].IsBot >= 1 || gObj[aIndex].m_OfflineMode != 0 || gObj[aIndex].IsFakeOnline != 0)
	{
		return;
	}


	BYTE send[128];

	PMSG_NQUEST_PROCSEND pMsg = { 0 };
	// ---
	pMsg.header.set(0xD3, 0x77, 0);

	int size = sizeof(pMsg);

	LPOBJ lpObj = &gObj[aIndex];


	std::map<int, CB_NewQuest::DATA_NQUEST_REQ>::iterator it = this->m_DataNQuestReq.find(lpObj->NQuest_LV);

	if (it == this->m_DataNQuestReq.end())
	{
		LogAdd(LOG_RED, "Khong co du lieu LevelQuest %d", lpObj->NQuest_LV);
		return;
	}
	for (int i = 0; i < 5; i++)
	{
		pMsg.MonterKill[i] = gObj[aIndex].NQuest_C[i];
	}


	pMsg.header.size[0] = SET_NUMBERHB(size);
	pMsg.header.size[1] = SET_NUMBERLB(size);
	// ---
	memcpy(send, &pMsg, sizeof(pMsg));

	DataSend(aIndex, send, size);


}
void CB_NewQuest::SendInfoNQuest(int aIndex, int LevelQuest, bool OpenNPC)
{
	if (gObj[aIndex].Type != OBJECT_USER)
	{
		return;
	}

	if (gObj[aIndex].Map == 107 || gObj[aIndex].Map == 109)
	{
		return;
	}

	if (gObjIsConnected(aIndex) == false)
	{
		return;
	}
	if (gObj[aIndex].IsBot >= 1 || gObj[aIndex].m_OfflineMode != 0 || gObj[aIndex].IsFakeOnline != 0)
	{
		return;
	}

	BYTE send[512];

	PMSG_NQUEST_SEND pMsg = { 0 };
	// ---
	pMsg.header.set(0xD3, 0x75, 0);

	int size = sizeof(pMsg);

	pMsg.count = 0;
	pMsg.Status = gObj[aIndex].NQuest_Status;
	pMsg.MaxLevelQuest = this->MaxLevelNQuest;
	pMsg.NPCOpen = OpenNPC;

	std::map<int, CB_NewQuest::DATA_NQUEST_REQ>::iterator it = this->m_DataNQuestReq.find(LevelQuest);

	if (it == this->m_DataNQuestReq.end())
	{
		LogAdd(LOG_RED, "Khong co du lieu LevelQuest %d", LevelQuest);
		return;
	}

	pMsg.count++;
	memcpy(&send[size], &it->second, sizeof(it->second));
	size += sizeof(it->second);

	pMsg.header.size[0] = SET_NUMBERHB(size);
	pMsg.header.size[1] = SET_NUMBERLB(size);
	// ---
	memcpy(send, &pMsg, sizeof(pMsg));

	DataSend(aIndex, send, size);

	//===SendListReward
	this->SendInfoNQuestReward(aIndex, LevelQuest);

}

void CB_NewQuest::SendInfoNQuestReward(int aIndex, int LevelQuest)
{
	if (gObj[aIndex].Type != OBJECT_USER)
	{
		return;
	}

	if (gObj[aIndex].Map == 107 || gObj[aIndex].Map == 109)
	{
		return;
	}

	if (gObjIsConnected(aIndex) == false)
	{
		return;
	}
	if (gObj[aIndex].IsBot >= 1 || gObj[aIndex].m_OfflineMode != 0 || gObj[aIndex].IsFakeOnline != 0)
	{
		return;
	}

	BYTE send[512];

	PMSG_NQUEST_SEND pMsg = { 0 };
	// ---
	pMsg.header.set(0xD3, 0x76, 0);

	int size = sizeof(pMsg);

	pMsg.count = 0;
	pMsg.Status = gObj[aIndex].NQuest_Status;
	pMsg.MaxLevelQuest = this->MaxLevelNQuest;



	for (std::vector<DATA_NQUEST_REWARD>::iterator iter = m_DataNQestReward.begin(); iter != m_DataNQestReward.end(); ++iter)
	{
		if (iter->NQuest_LV != LevelQuest) continue;

		DATA_NQUEST_REWARD_LOCAL info = { 0 };

		info.Count = iter->Count;

		CItem m_ItemCache;
		m_ItemCache.m_Level = iter->Level;
		m_ItemCache.m_Durability = (float)(gItemManager.GetItemDurability(iter->ItemIndex, iter->Level, iter->Exc, iter->Set));
		m_ItemCache.Convert(iter->ItemIndex, (BYTE)iter->Skill, (BYTE)iter->Luck, (BYTE)iter->Option, (BYTE)iter->Exc, iter->Set, 0, 0, 0, 0xFF);
		gItemManager.ItemByteConvert(info.Item, m_ItemCache); // Set Info Item

		if ((size + sizeof(info)) > 512) break;

		pMsg.count++;
		memcpy(&send[size], &info, sizeof(info));
		size += sizeof(info);
	}




	pMsg.header.size[0] = SET_NUMBERHB(size);
	pMsg.header.size[1] = SET_NUMBERLB(size);
	// ---
	memcpy(send, &pMsg, sizeof(pMsg));

	DataSend(aIndex, send, size);
	//LogAdd(LOG_RED, "SendInfoNQuestReward %s", gObj[aIndex].Name);

}
#endif