#pragma once
#include "Protocol.h"


class CB_OffTrade
{
public:
	enum eOffTradeType
	{
		eOFF_ZEN,
		eOFF_WCOINP,
		eOFF_WCOINC,
		eOFF_CHAOS,
		eOFF_BLESS,
		eOFF_SOUL,
	};

	struct PMSG_OFFTRADE_SEND
	{
		PSBMSG_HEAD header;
		int Type;
	};

	struct PMSG_OFFTRADE_RECV
	{
		PSBMSG_HEAD header;
		int Type;
	};

	struct PMSG_SHOPACTIVE_RECV
	{
		PSBMSG_HEAD header;
		int Active;
		int Type;
	};


	CB_OffTrade();
	~CB_OffTrade();
	void DrawButton(float X, float Y);
	void RenderTextNotice(float X, float Y);
	void RecvPShop(PMSG_OFFTRADE_RECV* Data);
	void CGSendOffTrade(int Type);
	void PShopActiveRecv(PMSG_SHOPACTIVE_RECV* Data);

	void ShowPrice(int TextNum, char* Price);
	void ShowMessNotice();
	
	int m_OfftradeType;
	int TypeShop;
	int ShopActive;
	int ShopList;

	DWORD ClickTickCount;
};

extern CB_OffTrade* gCB_OffTrade;