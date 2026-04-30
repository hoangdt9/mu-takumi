#pragma once
#include "CriticalSection.h"
#include "DataServerProtocol.h"
////===cURL
//#pragma comment(lib, "crypt32.lib")
//#pragma comment(lib,"wldap32.lib")
//#pragma comment(lib,"Normaliz.lib")
//#define CURL_STATICLIB
//#ifdef _DEBUG
//#pragma comment(lib,"..\\Util\\curl\\libcurl_a_debug.lib")
//#else
//#pragma comment(lib,"..\\..\\Util\\curl\\libcurl_a.lib")
//#endif
//#include "..\\..\\Util\\curl\\curl.h"
//#include "..\\..\\Util\\curl\\multi.h"
#if(CB_AutoBanking)
#define LIMITSIZELIST   512
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

struct DataGETKQNapTien
{
	int STT;
	char Account[11];
	char Name[11];
	int TienNap;
	char Checking[30];
	int Status;
};
class CBAutoNapGame
{
public:
	CBAutoNapGame();
	virtual ~CBAutoNapGame();
	void CBAutoNapGame::UserGetLichSuNap(BYTE* recv, int gsIndex);
	void CBAutoNapGame::SendConfigToGS(int uIndex);
	void CBAutoNapGame::RecvNapTienBank(BYTE* recv, int gsIndex);
	void CBAutoNapGame::Init();
	void CBAutoNapGame::LoadConfig();
	bool Enable;
	bool GetLogUpdateCoin;
	int TypeDB;
	char APIAutoCheckBank[120];
	int TyleNapTien;
	int GiaTriNapThapNhat;
	int HuyThanhToan;
	bool EnableThongBao;
	char MsgThongBao[255];
	bool StatusGetAPI;
	std::vector<DataGETKQNapTien> mDataGETKQNapTien;
	void CBAutoNapGame::SendListThanhToan(char* Account, int aIndex, int gsIndex);
	void CBAutoNapGame::GetKQNapTien(int Type = 0);
	std::string GetData;
private:
	CCriticalSection m_critical;

};

extern CBAutoNapGame gCBAutoNapGame;
#endif