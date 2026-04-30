#pragma once
#define MAX_RANK 50
#include "Protocol.h"

class CustomRanking
{
public:

	struct PMSG_CUSTOM_RANKING_RECV
	{
		PSWMSG_HEAD h;
		int index;
		char rankname[20];
		char col1[20];
		char col2[20];
		char col3[20];
		int count;
	};

	struct PMSG_CUSTOM_RANKING_COUNT_RECV
	{
		PSBMSG_HEAD header; // C1:BF:51
		int count;
	};

	struct PMSG_CUSTOM_RANKING_COUNT_SEND
	{
		PSBMSG_HEAD header; // C1:BF:51
	};

	struct PMSG_CUSTOM_RANKING_SEND
	{
		PSBMSG_HEAD header; // C1:BF:51
		BYTE type;
	};

	struct CUSTOM_RANKING_DATA
	{
		char szName[20];
		char value2[25];
		char value3[25];
#if(CUSTOM_BRANKINGNEW)
		bool CheckOnline;
#endif
	};
#if(CUSTOM_BRANKINGNEW)
	struct DATA_VIEWTOPRANKING
	{
		PSWMSG_HEAD header;
		char NameChar[11];
		char GuildName[11];
		BYTE GuildMark[32];
		DWORD TongPoint;
		DWORD Reset;
		DWORD Level;
		DWORD MasterLevel;
		BYTE Item[12][16];
		int aIndex;
		BYTE Class;
	};
	struct REQUESTINFO_CHARTOP
	{
		PSBMSG_HEAD header; // C3:F3:03
		char NameChar[11];
		int aIndex;
	};
#endif

	struct RANK_INFO_SEND
	{
		char NameRank[128];
	};

	struct PMSG_RANK_INFO
	{
		PWMSG_HEAD2 h;
		RANK_INFO_SEND Info[MAX_RANK];
	};
	CustomRanking();
	virtual ~CustomRanking();
	void OpenWindow();
	void ClearCustomRanking();
	void GCReqRanking(PMSG_CUSTOM_RANKING_RECV* lpMsg);
	void GCReqRankingCount(PMSG_CUSTOM_RANKING_COUNT_RECV* lpMsg);
	void DrawRankPanelWindow();

	int RankingType;
	int RankEnable;
	int RankingCount;
	int count;
	std::vector<std::string> mListRanking;
	void RankNameRev(PMSG_RANK_INFO* lpMsg);
	RANK_INFO_SEND m_RankInfo[MAX_RANK];
#if(CUSTOM_BRANKINGNEW)
	DATA_VIEWTOPRANKING CacheDataUserTop;
	void GCReqInfoCharTop(DATA_VIEWTOPRANKING* lpMsg);
	bool CacheUserRank;
	bool SetItemCharTop;
#endif
	int SelectBarRankingNumber;
	int PageBank;
	int MaxPerPageBank;
private:
	char rankname[20];
	char col1[20];
	char col2[20];
	char col3[20];
	CUSTOM_RANKING_DATA gCustomRanking[MAX_RANK];
	int RankLoad;


};
extern CustomRanking* gCustomRanking;