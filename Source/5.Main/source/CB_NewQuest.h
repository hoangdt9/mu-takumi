#pragma once
#include "Protocol.h"
#if(CB_NEWQUESTFF)
class CB_NewQuest
{
public:
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
		int ItemSection;
		int ItemType;
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
	struct DATA_NQUEST_REWARD_LOCAL_C
	{
		ITEM* Item;
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
	void RecvInfoMonterKill(BYTE* Recv);
	void RecvInfo(BYTE* Recv);
	void RecvRewardList(BYTE* Recv);
	void DrawWindow();
	void DrawWindowMini();
	BYTE Status;
	int MaxLevelQuest;

	std::vector<DATA_NQUEST_REWARD_LOCAL_C> m_DataRewardList;
	std::vector<DATA_NQUEST_REQ> m_DataNQuest;

	int Page;
	int MaxPerPage;

	DWORD MonterKill[5];
};

extern CB_NewQuest* gCB_NewQuest;
#endif