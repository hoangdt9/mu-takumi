// ---
#include "Protocol.h"
// ---
#define MAX_RANK  20
// ---
struct CUSTOM_RANKING
{
	int index;
	char Name[20];
	char Col1[20];
	char Col2[20];
	char Col3[20];
};
// ---
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
//**********************************************//
//********** GameServer -> DataServer **********//
//**********************************************//
struct SDHP_CUSTOM_RANKING_SEND
{
	PBMSG_HEAD header;
	WORD index;
	WORD type;
};

//**********************************************//
//********** DataServer -> GameServer **********//
//**********************************************//
struct SDHP_CUSTOM_RANKING_RECV
{
	PWMSG_HEAD header;
	int index;
	int type;
	int count;
};

//**********************************************//
//********** GameServer -> Cliente    **********//
//**********************************************//

struct PMSG_CUSTOM_RANKING_SEND
{
	PSWMSG_HEAD header;
	int index;
	char rankname[20];
	char col1[20];
	char col2[20];
	char col3[20];
	int count;
};

struct PMSG_CUSTOM_RANKING_COUNT_SEND
{
	PSBMSG_HEAD header; // C1:BF:51
	int count;
};

//**********************************************//
//********** Cliente -> GameServer    **********//
//**********************************************//

struct PMSG_CUSTOM_RANKING_COUNT_RECV
{
	PSBMSG_HEAD header; // C1:BF:51
};

struct PMSG_CUSTOM_RANKING_RECV
{
	PSBMSG_HEAD header; // C1:BF:51
	BYTE type;
};

struct RANK_INFO_SEND
{
	char NameRank[128];
};

struct PMSG_RANK_INFO
{
	PWMSG_HEAD2 h;
	RANK_INFO_SEND Info[MAX_RANK];
};

// ---
class CCustomRanking
{
public:
	void Load(char* path);
	void GCReqRanking(int Index, PMSG_CUSTOM_RANKING_RECV* pMsg);
	void GCReqRankingCount(int Index, PMSG_CUSTOM_RANKING_COUNT_RECV* lpMsg);
	void SendMain(LPOBJ lpObj);
	// ---
	CUSTOM_RANKING m_RankInfo[MAX_RANK];

	int GetRankIndex(int aIndex);
	void CheckUpdate(LPOBJ lpObj);
	void GDCustomRankingRecv(SDHP_CUSTOM_RANKING_RECV* lpMsg);
#if(CUSTOM_BRANKINGNEW)
	void CGetInfoCharTop(REQUESTINFO_CHARTOP* lpMsg, int aIndex);
	void RecvInfoCharTop(DATA_VIEWTOPRANKING* lpMsg);
#endif
private:
	int m_count;
	// ---
	CUSTOM_RANKING r_Data[MAX_RANK];
};
extern CCustomRanking gCustomRanking;
// ---
