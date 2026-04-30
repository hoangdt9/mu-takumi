#pragma once

#include "User.h"
#include "Protocol.h"

//#include "DSProtocol.h"

#define MAX_BSVEVENT_USER 100

enum eBsVEventState
{
	BSV_EVENT_STATE_BLANK = 0,
	BSV_EVENT_STATE_EMPTY = 1,
	BSV_EVENT_STATE_STAND = 2,
	BSV_EVENT_STATE_START = 3,
	BSV_EVENT_STATE_CLEAN = 4,
};

enum eBsVTeams
{
	BSV_BLUETEAM = 1,
	BSV_REDTEAM = 2,
};

struct BSV_EVENT_START_TIME
{
	int Year;
	int Month;
	int Day;
	int DayOfWeek;
	int Hour;
	int Minute;
	int Second;
};

struct BSV_EVENT_USER
{
	void Reset() // OK
	{
		int Rank = 99;
		this->Index = -1;
		this->Available = false;
		this->Team = -1;
		this->Kills = 0;
		this->Deaths = 0;
	}
	int Rank;
	int Index;
	bool Available;
	int Team;
	int Kills;
	int Deaths;
};

struct BSV_TIME
{
	int Year;
	int Month;
	int Day;
	int DayOfWeek;
	int Hour;
	int Minute;
	int Second;
};

//struct HIDE_NAME
//{
//	PSBMSG_HEAD	h;
//	int		HideName;
//};


class CBsVEvent
{
public:
	struct PMSG_COUNTLIST
	{
		PSWMSG_HEAD header;
		int Count;
		BYTE TypeSend;
	};

	struct DATA_BXH_KILL
	{
		char Name[11];
		int Kills;
		int Deaths;
	};
	struct DATA_INFOUSER_SEND
	{
		int Kills;
		int Deaths;
		int Rank;
	};
	enum TypeDataSend
	{
		eSendInfoUser = 0x02,
		eSendBXHKill = 0x03,
	};
	struct DATA_THUONG_TOP
	{
		int Top;
		int WC;
		int WP;
		int GP;
		int ItemBag;
	};
	CBsVEvent();
	virtual ~CBsVEvent();
	void Init();
	void Clear();
	void Load(char* path);
	void MainProc();
	void ProcState_BLANK();
	void ProcState_EMPTY();
	void ProcState_STAND();
	void ProcState_START();
	void ProcState_CLEAN();
	void SetState(int state);
	void SetState_BLANK();
	void SetState_EMPTY();
	void SetState_STAND();
	void SetState_START();
	void SetState_CLEAN();
	void CheckSync();
	int GetState();
	bool Dialog(LPOBJ lpObj, LPOBJ lpNpc);
	bool CheckEnterEnabled(LPOBJ lpObj);
	bool CheckReqItems(LPOBJ lpObj);
	int GetEnterEnabled();
	int GetEnteredUserCount();
	bool CheckEnteredUser(int aIndex);
	bool CheckPlayerTarget(LPOBJ lpObj);
	bool CheckPlayerJoined(LPOBJ lpObj, LPOBJ lpTarget);
	bool CheckSelfTeam(LPOBJ lpObj, LPOBJ lpTarget);
	bool CheckAttack(LPOBJ lpObj, LPOBJ lpTarget);
	bool AddUser(int aIndex);
	bool DelUser(int aIndex);
	BSV_EVENT_USER* GetUser(int aIndex);
	void CleanUser();
	void ClearUser();
	void CheckUser();
	int GetUserCount();
	bool GetUserRespawnLocation(LPOBJ lpObj, int* gate, int* map, int* x, int* y, int* dir, int* level);
	void DivisionTeam();
	int ReturnMaxReset();
	void UserDieProc(LPOBJ lpObj, LPOBJ lpTarget);
	void NoticeSendToAll(int type, char* message, ...);
	void StartBSV();

	int m_State;
	int m_RemainTime;
	int m_StandTime;
	int m_CloseTime;
	int m_TickCount;
	int m_WarningTime;
	int m_EventTime;
	int EnterEnabled;
	int AlarmMinSave;
	int AlarmMinLeft;
	int TargetTime;

	int TotalPlayer;
	int ScoreBlue;
	int ScoreRed;
	int ScoreUser;

	int ReqItemCount;
	int ReqItemIndex;
	int ReqItemLevel;
	int EventMap;
	int WaitingGate;
	int BluTeamGate;
	int RedTeamGate;
	int SetSkin;
	int MinLevel;
	int MaxLevel;
	int MinReset;
	int MaxReset;
	int MinMasterReset;
	int MaxMasterReset;
	int MaxDie;


	int Coin1;
	int Coin2;
	int Coin3;
	int ItemBonus;
	int BagKill;
	//int DropInventory;

	BSV_EVENT_USER User[MAX_BSVEVENT_USER];
	std::vector<BSV_TIME> m_BSVStartTime;

	DATA_BXH_KILL CacheTopBXH[11];
	void ClearCacheBXH();
	void CalcUserRank();
	void SendBXHVeClient();
	void SendInfoKillUser(int aIndex);
	std::map<int, DATA_THUONG_TOP> m_DataThuongTop;
	void CBsVEvent::SetThuongTop(LPOBJ lpObj, int Top);
};
extern CBsVEvent gBsVEvent;