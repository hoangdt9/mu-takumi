#pragma once
#include "User.h"
#include "Protocol.h"
#include "ItemManager.h"
#if(CB_NEWQUESTFF)
class CB_NewQuest
{
public:
	struct DATA_NPC
	{
		int Class;
		int Map;
		int X;
		int Y;
		int Dir;
	};

	struct DATA_NQUEST_REQ
	{
		int NQuest_LV;
		char NameQuest[90];
		int MonterClass[5];
		int NQuest_C[5];
		int Map;
		int ReqLevel;
		int ReqBless;
		int ReqSoul;
		int ReqLife;
		int ReqChaos;
		int ReqCre;
		int ReqPoint;
	};

	struct DATA_NQUEST_REWARD
	{
		int NQuest_LV;
		int ItemIndex;
		int Level;
		int Skill;
		int Luck;
		int Option;
		int Exc;
		int Set;
		int Dur;
		int Count;
	};
	struct DATA_NQUEST_REWARD_LOCAL
	{
		BYTE Item[12];
		int Count;
	};
	struct PMSG_NQUEST_SEND
	{
		PSWMSG_HEAD header; // C2:F3:E2
		BYTE count;
		BYTE Status;
		int MaxLevelQuest;
		BYTE NPCOpen;
	};
	struct PMSG_NQUEST_PROCSEND
	{
		PSWMSG_HEAD header; // C2:F3:E2
		DWORD MonterKill[5];
	};
	enum STATUS_NQUEST
	{
		eNone,
		eDangLam,
		eDaDuYeuCau,
	};

	CB_NewQuest();
	~CB_NewQuest();
	void CB_NewQuest::LoadData(char* path);
	void CB_NewQuest::SetNPCList();
	bool CB_NewQuest::CheckNpcQuest(LPOBJ lpObj, LPOBJ lpNpc);
	void CB_NewQuest::SendInfoNQuest(int aIndex, int LevelQuest, bool OpenNPC = false);
	void CB_NewQuest::SendInfoNQuestReward(int aIndex, int LevelQuest);
	void CB_NewQuest::SendInfoMonterProc(int aIndex);
	void CB_NewQuest::RecvProtocol(int aIndex, BYTE* Recv);


	std::vector<CB_NewQuest::DATA_NPC> m_DataNPCList;
	std::map<int, CB_NewQuest::DATA_NQUEST_REQ > m_DataNQuestReq;
	std::vector<CB_NewQuest::DATA_NQUEST_REWARD> m_DataNQestReward;
	int MaxLevelNQuest;
	void MonsterDieProc(LPOBJ Monter, LPOBJ lpObj);

};
extern CB_NewQuest gCB_NewQuest;
#endif