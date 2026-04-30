#pragma once
#include "DSProtocol.h"
#include "Protocol.h"
#if(CB_AutoBanking)
struct DataLichSuNapThe
{
	int TienNap;
	char Checking[30];
	int Status;
};

//===GS ==> DS
struct LISTCOUNDAUTONAP
{
	PSWMSG_HEAD header;
	int count;
	int aIndex;
};
struct LISTCOUNDAUTONAP_CLIENT
{
	PSWMSG_HEAD header;
	int count;
	int aIndex;
	DWORD TyleNapTien;
	DWORD GiaTriNapThapNhat;
};
struct SDHP_GSGETDATANAPTHE
{
	PSBMSG_HEAD h;
	int aIndex;
	char Account[11];
};

struct RuleAutoNapConfig
{
	PSBMSG_HEAD header;
	int TyleNapTien;
	int GiaTriNapThapNhat;
};

struct SEND_RECV_NAPBANK
{
	PSBMSG_HEAD header; // C3:F3:03
	int aIndex;
	char Account[11];
	char CharName[11];
	int TienNap;
};

class CBAutoNapGame
{
public:
	CBAutoNapGame();
	virtual ~CBAutoNapGame();
	bool Enable;
	int TyleNapTien;
	int GiaTriNapThapNhat;
	void CBAutoNapGame::Init();
	void CBAutoNapGame::RecvConfigNapThe(BYTE* recv);
	void CBAutoNapGame::RecvListThanhToan(BYTE* recv);
	void CBAutoNapGame::GetListThanhToan(int aIndex);
	void CBAutoNapGame::SendNapTienBank(BYTE* recv, int aIndex);
};
extern CBAutoNapGame gCBAutoNapGame;
#endif

