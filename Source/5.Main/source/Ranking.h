#pragma once
#include "stdafx.h"
#include "Protocol.h"
#if(RANKING_NEW == 1)

#define MAXTOP	150

struct CharTop
{
	char Name[11];
	BYTE Class;
	int level;
	int Reset;
	int Relifes;
	int Time;
	int Map;
	char Guild[9];
};

struct DGCharTop
{
	PWMSG_HEAD h;
	CharTop	tp[MAXTOP];
};
struct SEND_GS_REQ
{
	PSBMSG_HEAD h;
};
class cRanking
{
public:
	cRanking();
	~cRanking();

	void RecvInfo(DGCharTop* lpMsg);
	void Draw();
	int StartX;
	int StartY;
	int Page;
	DWORD ClickTime;
	void OpenOnOff();
	CharTop	RankingChar[MAXTOP];
	int GetNumberTop(int KeyIndex);
};
extern cRanking gRanking;
#endif