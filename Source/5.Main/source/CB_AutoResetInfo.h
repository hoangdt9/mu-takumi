#pragma once
#include "CBInterface.h"

#if(CB_AUTORESETINFO)
class CBAutoResetInfo
{
public:
	//==New Resset
	struct DKRS_COUNTLIST
	{
		PSWMSG_HEAD header;
		WORD Level;
		int Point;
		int Zen;
		int WC;
		int WP;
		int GP;
		short ResetView;
		short ResetDay;
		short MaxGHRS;
		BYTE OnResetType;
		BYTE LoaiReset;
		BYTE TypeReset;
		BYTE CodeReset;
		BYTE DefStat[5];
		BYTE AutoResetEnable;
		WORD Count;
	};

	struct DKRS_ITEMINFO
	{
		int SL;
		BYTE Item[16];
		bool Status;
	};

	struct VIEWINFO_RESETLIST
	{
		WORD Level;
		int Point;
		int Zen;
		int WC;
		int WP;
		int GP;
		short ResetView;
		short ResetDay;
		short MaxGHRS;
		BYTE OnResetType;
		BYTE LoaiReset;
		BYTE TypeReset;
		BYTE CodeReset;
		BYTE DefStat[5];
		BYTE AutoResetEnable;
		std::vector<DKRS_ITEMINFO> m_ItemCheck;
		void Clear()
		{
			Level = 0;
			Zen = 0;
			WC = 0;
			WP = 0;
			GP = 0;
			Point = 0;
			ResetView = 0;
			ResetDay = 0;
			MaxGHRS = 0;
			OnResetType = 0;
			LoaiReset = 0;
			TypeReset = 0;
			CodeReset = 0;
			AutoResetEnable = 0;
			DefStat[0] = 0;
			DefStat[1] = 0;
			DefStat[2] = 0;
			DefStat[3] = 0;
			DefStat[4] = 0;
			m_ItemCheck.clear();
		}

	};
	struct RESETCODE_SEND
	{
		PSBMSG_HEAD header; // C3:F3:03
		int TypeReset;
		int AutoResetStats[5];
		char CodeReset[9];
	};
	//=====AutoResetinfo====//
	struct PMSG_AUTORESET_INFO_RECV
	{
		PSBMSG_HEAD header; // C3:F3:03
		DWORD LevelRelife;
		DWORD RelifeCong;
		DWORD MaxRelife;
		DWORD LevelSauKhiRL;
		DWORD ResetSauKhiRL;
		DWORD PointRelife;
	};
	CBAutoResetInfo();
	~CBAutoResetInfo();
	void DrawWindow();
	void OpenWindow();
	void RecvProtocol(BYTE* Recv);
	VIEWINFO_RESETLIST ViewInfoResetWindow;
	void GCCharacterAutoResetRecv(BYTE* Recv);
	//====AutoReset Info========//
	BYTE TypeReset;
	DWORD GHRSReset;
	DWORD GHRSRelife;
	DWORD LvlReset;
	//==========//
	char* GetItemName(int ItemType, int Level);

	bool LoadingInfo;
	void DrawWindowRelife();
	DWORD m_LevelRelife;
	DWORD m_RelifeCong;
	DWORD m_MaxRelife;
	DWORD m_LevelSauKhiRL;
	DWORD m_ResetSauKhiRL;
	DWORD m_PointRelife;
};

extern CBAutoResetInfo* gCBAutoResetInfo;
#endif