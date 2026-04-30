#pragma once
#include "StdAfx.h"
#include "DSProtocol.h"
#include "User.h"

struct CUSTOM_VIPCHAR_MESSAGE
{
	int Index;
	char Message[256];
};
//==COnfig
struct DATA_VIP_CHAR
{
	int LevelVIP;
	int PointVIP;
	int Coin[3];
	int TypeOp[6];
	int ValueOp[6];
};
struct DATA_ITEMVIPPOINT
{
	WORD IndexItem;
	BYTE Level;
	BYTE ExlOp;
	DWORD PointVIP;
};

//===Send List Cho Client
struct BPMSG_CUSTOM_VIPCHAR_SEND
{
	PSWMSG_HEAD header;
	WORD TypeSend;
	DWORD UserPointVIP;
	//===
	int LevelVIP;
	int PointVIP;
	int Coin[3];
	int TypeOp[6];
	int ValueOp[6];
};
typedef struct VCSEV_COUNTLIST
{
	PSWMSG_HEAD header;
	int Count;

};
class BCustomVIPChar
{
public:
	void Load(char* FilePath);
	std::map<int, DATA_VIP_CHAR> m_DataVIPChar;
	std::vector<DATA_ITEMVIPPOINT> m_ItemPoint;
	DATA_VIP_CHAR* GetInfoVIPChar(int LvVIPChar);

	void SendInfoToClient(int Index, int Type = 0);
	void SendListItemPoint(int Index);
	void CheckItemPointVIP(LPOBJ lpObj, int SlotItem);
	void UpLevelVIPChar(LPOBJ lpObj);
	void ActiveBonus(LPOBJ lpObj, bool flag);

private:
	int Enable;
	int		Firework;
	int			Notice;
	char		Msg[90];
	//===Mess
	std::map<int, CUSTOM_VIPCHAR_MESSAGE> m_MessageInfoBP;
	char* GetMessage(int index);

};

extern BCustomVIPChar gBCustomVIPChar;