#pragma once
#include "Protocol.h"
#if(CB_VIP_CHAR)


class CBInterfaceVIPChar
{

public:
	struct CUSTOMVIPCHAR_DATA
	{
		int		Index;
		int		TypeRank;
		char	TextDir[256];
		DWORD	ColorText;
		float	SizeW;
		float	H;
		float	X;
		float	Y;
		int		CacheRank;
	};
	struct DATA_VIP_CHAR
	{
		int UserPointVIP;
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


	struct CSEV_COUNTLIST
	{
		PSWMSG_HEAD header;
		int Count;
	};

	//===Set Info VIP Chart GS <> Client
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

	CBInterfaceVIPChar();
	virtual ~CBInterfaceVIPChar();
	void LoadData();
	std::map<int, CUSTOMVIPCHAR_DATA> m_VipCharRank;
	void Draw();
	DATA_VIP_CHAR m_VIPCharInfo;
	std::vector<DATA_ITEMVIPPOINT> m_VIPCharInfoList;
	//std::vector<ITEM_INFO_DATA> m_DungLuyenInfoList;
	void GCSetInfoVIPChar(BPMSG_CUSTOM_VIPCHAR_SEND* lpMsg);
	void GCSetInfoVIPCharList(CSEV_COUNTLIST* lpMsg);
	void CGSendOpenWinwdowVIP();
	bool SendItem(ITEM* ItemSell, int Slot, bool KeyClick);
private:
	void ErrorMessageBox(char* message, ...);
};

extern CBInterfaceVIPChar* gBInterfaceVIPChar;
#endif