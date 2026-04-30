#pragma once
#include "Protocol.h"
#include "UIWindows.h"


class CB_DrawCustomRank
{

public:
	enum TypeDataSend
	{
		eSendInfoUser = 0x02,
		eSendBXHKill = 0x03,
	};
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

	CB_DrawCustomRank();
	~CB_DrawCustomRank();

	void Draw();
	void DrawBXH();
	void DrawPoint();
	void RecvPacketGS(BYTE* Data);
	std::vector<DATA_INFOUSER_SEND> mDataInfoUserKill;
	std::vector<DATA_BXH_KILL> mDataBXHKiller;
	int CacheMap;

};

extern CB_DrawCustomRank* gCB_DrawCustomRank;