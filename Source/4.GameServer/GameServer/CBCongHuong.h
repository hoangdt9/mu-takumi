#pragma once
#include "Protocol.h"
#include "ItemManager.h"
#if (CongHuongV2)
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
struct MESSAGE_CH
{
	int Index;
	char Message[256];
};

struct TYPEMIX_CH
{

	int LvMin;
	int LvMax;
	int ExcMin;
	int ExcMax;
	int CHLevelMax;
	int CH_WC;
	int CH_WCP;
	int CH_GBP;
	int CHRate;
};
class CBCongHuong
{
public:
	CBCongHuong();
	virtual ~CBCongHuong();
	//=========
	int CH_TyLeThanhCong;
	int CH_ResetMix;
	int CH_TypeMix;
	int CH_CoinMix[3];
	int CH_MaxExcCount;
	//==========
	void CH_LoadConfigXML(char* FilePath);
	void CH_ClientSetItemGS(int aIndex, PMSG_MOVEITEM* lpMsg);
	void CH_ClearDB(int aIndex, int TypeClear, bool RollBackItem = 1);
	void CH_ClearConfig();
	void CH_GSSendInfoClient(int aIndex);
	void CH_MixItemGS(int aIndex, XULY_CGPACKET* lpMsg);
	//==============
	int mCH_Enabled;
	int mCH_NPCClass;
	int mCH_NPCMap;
	int mCH_NPCX;
	int mCH_NPCY;
	int mCH_NPCDir;

	int CHLuck;
	int CHSkill;
	int CHLevel;
	int CHOptLife;
	int CHLevelPlus;
	int CHCheckPhoiType;
	int CHKeepItem;
	int CHAllowAnc;

	bool TalkNPC(int aIndex, int aNpcIndex);
	BOOL CHCheckItemCH(CItem ItemCheck);
	std::vector<TYPEMIX_CH> m_TypeMixCH;
	//===Mess
	char* GetMessage(int index);
	std::map<int, MESSAGE_CH> m_MessageInfo;
	bool RecvOpenWindowCH(int aIndex, XULY_CGPACKET* lpMsg);
	void CBCongHuong::SetNPC();
};

extern CBCongHuong gCBCongHuong;
#endif