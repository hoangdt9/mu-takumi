#pragma once
#include "Protocol.h"
#if(CongHuongV2)
class CB_CongHuong
{
public:
	struct PMSG_GSSendInfoClient
	{
		PSBMSG_HEAD header;
		int mCH_TyLeThanhCong;
		int mCH_ResetMix;
		int mCH_TypeMix;
		int mCH_CheckLvItem;
		int mCH_CoinMix[3];
		int mCH_IndexItem[3];
		BYTE mCH_InfoItem[3][12];
	};

	CB_CongHuong();
	~CB_CongHuong();

	bool SendItem(ITEM* ItemSell, int Slot, bool KeyClick);
	void XoaDataCongHuong();
	void DrawWindowCongHuong();

	void CH_RecvInfoGSSend(PMSG_GSSendInfoClient* pMsg);
	int GetCountPhoiCongHuong(int mIndexItem, int Type);
private:
	int  m_ScrollPos;
	void EventScrollPos(MOUSEHOOKSTRUCTEX* Mouse, WPARAM wParam);

	BOOL CH_EffectMixOn;
	int CH_IndexItem[3];
	int CH_CoinMix[3];
	int CH_TyLeThanhCong;
	int CH_ResetMix;
	int CH_TypeMix;
	int CH_CheckLvItem;
	BYTE CH_InfoItem[3][12];

	
};

extern CB_CongHuong* gCB_CongHuong;
#endif