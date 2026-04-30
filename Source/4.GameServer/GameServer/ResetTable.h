// ResetTable.h: interface for the CResetTable class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "User.h"
#if(CB_AUTORESETINFO)

struct NGUYENLIEUITEM_DATA
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

struct MESSAGE_INFO_RS
{
	int Index;
	char Message[256];
};



struct AutoResetThuongInfo
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
	std::vector<NGUYENLIEUITEM_DATA> m_ItemCheck;
};
class CResetTable
{
public:
	CResetTable();
	virtual ~CResetTable();
	void Load(char* path);
	int GetResetLevel(LPOBJ lpObj);
	int GetResetMoney(LPOBJ lpObj);
	int GetResetPoint(LPOBJ lpObj, bool GetPointMasterReset = 0);
	int GetResetPointKeep(LPOBJ lpObj);
	//
	int GetStatusCheckCode(LPOBJ lpObj);
	void SendInfoReset(LPOBJ lpObj);
	//
	std::map<int, MESSAGE_INFO_RS> m_MessageInfoBP;
	char* GetMessage(int index);
	bool CheckItemNguyenLieu(LPOBJ lpObj, std::vector<NGUYENLIEUITEM_DATA> DataNhomNguyenLieu, bool ClearItem = 0);
	bool AutoResetThuongEnable;
	bool AutoResetThuongNotice;
	bool AutoResetThuongType;
	AutoResetThuongInfo* GetInfoAutoReset(LPOBJ lpObj);
	std::vector<AutoResetThuongInfo> m_AutoResetThuongInfo;
	bool KiemTraDieuKienReset(LPOBJ lpObj, bool ClearNguyenLieu = 0);
};

extern CResetTable gResetTable;
#else
struct RESET_TABLE_INFO
{
	int MinReset;
	int MaxReset;
	int Level[MAX_ACCOUNT_LEVEL];
	int Money[MAX_ACCOUNT_LEVEL];
	int Point[MAX_ACCOUNT_LEVEL];
};

class CResetTable
{
public:
	CResetTable();
	virtual ~CResetTable();
	void Load(char* path);
	int GetResetLevel(LPOBJ lpObj);
	int GetResetMoney(LPOBJ lpObj);
	int GetResetPoint(LPOBJ lpObj);
	int GetResetPointTable(LPOBJ lpObj);
private:
	std::vector<RESET_TABLE_INFO> m_ResetTableInfo;
};

extern CResetTable gResetTable;
#endif