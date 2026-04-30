#pragma once
#include "Protocol.h"
class CCustomRankUser
{
	struct PMSG_CUSTOM_RANKUSER
	{
		PSBMSG_HEAD h;
		char szName[20]; //Rank
		int iIndex;
		int iLevel;
		int iReset;
		int iMReset;
		int iKills;
		int iDeads;
		int iFor;
		int iEne;
		int iAgi;
		int iVit;
		int iCom;
		int iClasse;
		int iRank;
		char iVip[20];
#if DANH_HIEU_NEW
		DWORD m_rDanhHieu;
#endif
#if (CB_VIP_CHAR)
		WORD mLvVIPChar;
#endif

	};
	struct CUSTOM_RANKUSER_DATA
	{
		WORD m_Index;
		WORD m_Resets;
		WORD m_MResets;
		WORD m_Level;
		WORD m_Kills;
		WORD m_Deads;
		WORD m_For;
		WORD m_Vit;
		WORD m_Ene;
		WORD m_Agi;
		WORD m_Com;
		WORD m_Classe;
		WORD m_Rank;
		char m_Vip[20];
		char szTitleRank[20];
#if DANH_HIEU_NEW
		DWORD m_rDanhHieu;
#endif
#if (CB_VIP_CHAR)
		WORD mLvVIPChar;
#endif

	};
public:
	CCustomRankUser();
	virtual ~CCustomRankUser();
	void GCReqRankLevelUser(SEND_COUNTLIST* aRecv);
	std::vector< CUSTOM_RANKUSER_DATA> m_DataRank;
	int GetNumberData(int KeyIndex);

	void DrawInfo(int aIndex,float X, float Y, float W, float H);

};

extern CCustomRankUser gCustomRankUser;
