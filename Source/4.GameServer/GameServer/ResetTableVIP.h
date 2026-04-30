// ResetTableVIP.h: interface for the CResetTableVIP class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "User.h"
#if(CB_AUTORESETINFO)

struct VNGUYENLIEUITEM_DATA
{
	int Nhom;
	int SL;
	int Type;
	int IndexItem;
	int Level;
	int Skill;
	int Luck;
	int Option;
	int ExlOp;
};

struct MESSAGE_INFO_RSVIP
{
	int Index;
	char Message[256];
};



struct AutoResetVIPInfo
{
	int MinReset;
	int MaxReset;
	int LevelReset[MAX_ACCOUNT_LEVEL];
	int PointReset[MAX_ACCOUNT_LEVEL];
	int Zen;
	int WC;
	int WP;
	int GP;
	int CheckCode;
	std::vector<VNGUYENLIEUITEM_DATA> m_ItemCheck;
};
class CResetTableVIP
{
public:
	CResetTableVIP();
	virtual ~CResetTableVIP();
	void Load(char* path);
	int GetResetLevel(LPOBJ lpObj);
	int GetResetMoney(LPOBJ lpObj);
	int GetResetPoint(LPOBJ lpObj, bool GetPointMasterReset = 0);
	int GetResetPointKeep(LPOBJ lpObj);
	//
	int GetStatusCheckCode(LPOBJ lpObj);
	void SendInfoReset(LPOBJ lpObj);
	//
	std::map<int, MESSAGE_INFO_RSVIP> m_MessageInfoBP;
	char* GetMessage(int index);
	bool CheckItemNguyenLieu(LPOBJ lpObj, std::vector<VNGUYENLIEUITEM_DATA> DataNhomNguyenLieu, bool ClearItem = 0);
	bool AutoResetVIPEnable;
	bool AutoResetVIPNotice;
	bool AutoResetVIPType;
	AutoResetVIPInfo* GetInfoAutoReset(LPOBJ lpObj);
	std::vector<AutoResetVIPInfo> m_AutoResetVIPInfo;
	bool KiemTraDieuKienReset(LPOBJ lpObj, bool ClearNguyenLieu = 0);
};

extern CResetTableVIP gResetTableVip;
#endif