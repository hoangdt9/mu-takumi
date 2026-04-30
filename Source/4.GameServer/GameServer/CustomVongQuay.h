// CustomHealthBar.h: interface for the CCustomHealthBar class.
//
//////////////////////////////////////////////////////////////////////

#pragma once
#include "Protocol.h"


struct MESSAGE_INFO_VONGQUAY
{
	int Index;
	char Message[256];
};

struct DATA_VONGQUAYITEM
{
	float SizeBMD;
	float PosX;
	float PosY;
	int IndexItem;
	int LvItem;
	int Dur;
	int Skill;
	int Luck;
	int Opt;
	int Exc;
	int Anc;
	int SK[MAX_SOCKET_OPTION];
	int SKBonus;
	int HSD;
	int Rate;
};

struct DATA_VONGQUAY
{
	int IndexVongQuay;
	int IndexItemYC;
	int WC;
	int WP;
	int GP;
	int Count;
	char NameVongQuay[90];
	std::vector<DATA_VONGQUAYITEM> ListItemNhan;
};

//===List VONG QUAY
struct PMSG_VONGQUAY_SEND
{
	PSWMSG_HEAD header; // C2:F3:E2
	BYTE count;
};

struct ListVongQuaySend
{
	int IndexVongQuay;
	char Name[30];
};


//===List THuowng
struct PMSG_YCVONGQUAY_SEND
{
	PSWMSG_HEAD header; // C2:F3:E2
	BYTE count;
	int IndexYC;
	int CountItem;
	int WCYC;
	int WPYC;
	int GPYC;
};

struct LISTITEMVONGQUAY_SENDINFO
{
	float SizeBMD;
	float PosX;
	float PosY;
	short Index;
	BYTE Dur;
	BYTE Item[12];
	int  PeriodTime;
};

struct XULY_CGPACKET_VONGQUAY
{
	PSBMSG_HEAD header; // C3:F3:03
	DWORD StartRoll;
	DWORD IndexWin;

};

struct XULY_CGPACKET_SOLAN
{
	PSBMSG_HEAD header; // C3:F3:03
	DWORD ThaoTac;
	DWORD SoLan;
};


class CCustomVongQuay
{
public:
	CCustomVongQuay();
	virtual ~CCustomVongQuay();
	void Init();
	void LoadFileXML(char* FilePath);
	void CCustomVongQuay::UserSendClientInfo(int aIndex);
	void CCustomVongQuay::SendListNhanThuong(int aIndex, int VongQuaySo);
	void CCustomVongQuay::ActionVongQuay(int aIndex, int MocNap,int solan);
	void CCustomVongQuay::MakeItem(int aIndex,int type);
	int SoVongQuay;
private:
	int Enable;
	int Firework;
	int Notice;
	std::map<int, MESSAGE_INFO_VONGQUAY> m_MessageInfoBP;
	std::map<int, DATA_VONGQUAY> m_DataVongQuay;
	char* GetMessage(int index);
};

extern CCustomVongQuay gCustomVongQuay;