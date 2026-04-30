#pragma once
#include "user.h"
#include "stdafx.h"
#include "DSProtocol.h"
#include "Protocol.h"

struct MESSAGE_HUYDONGEXC
{
	int Index;
	char Message[256];
};

struct HUYEXC_COIN
{
	int Index;
	int Coin[3];
	int Rate;
};
struct HUYEXC_GSSENDINFO
{
	PSBMSG_HEAD header;
	int mWC[6];
	int mWP[6];
	int mGP[6];
	int mRate[6];
	BYTE Request;
};

struct PMSG_HDCLIENTSENDGS
{
	PSBMSG_HEAD h;
	BYTE Source;
};



class CBHuyDongExc
{
public:
	CBHuyDongExc();
	virtual ~CBHuyDongExc();
	void LoadConfigXML(char* FilePath);
	//=== 
	int mHuyExc_Enabled;
	std::map<int, HUYEXC_COIN> m_HuyExcInfo;
	int HExcRS;
	int HExcLvMin;
	int HExcLvMax;
	int HExcSkill;
	int HExcLuck;
	void HuyExc_ClientSetItemGS(int aIndex, PMSG_HDCLIENTSENDGS* lpMsg);
	void HuyExc_GSSendInfo(int aIndex);
	void ActionHuyExc(int aIndex, XULY_CGPACKET* lpMsg);
	void CBHuyDongExc::ClearCache(int aIndex, int Clear = 0);
private:
	//===Mess
	char* GetMessage(int index);
	std::map<int, MESSAGE_HUYDONGEXC> m_MessageInfo;
};

extern CBHuyDongExc gBHuyDongExc;