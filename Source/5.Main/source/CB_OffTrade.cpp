#include "StdAfx.h"
#include "CB_OffTrade.h"
#include "NewUISystem.h"
#include "CBInterface.h"
#include "CUIController.h"
#include "CharacterManager.h"
#include "Util.h"
#include "Protocol.h"
#include "NewUIBase.h"
#include "Other.h"
#include "ZzzInterface.h"


CB_OffTrade* gCB_OffTrade;

CB_OffTrade::CB_OffTrade()
{
	this->ShopActive = 0;
	this->TypeShop = -1;
	this->ClickTickCount = GetTickCount();
}


CB_OffTrade::~CB_OffTrade()
{
}



void CB_OffTrade::DrawButton(float X, float Y)
{
	//gInterface.DrawMessage(1, "Shop Active %d, Type %d", this->ShopActive, this->TypeShop);
	const BYTE state[3] = { 0, 1, 2 };
	X = X + 10;
	float bStartX = X;
	float bStartY = Y;
	if (this->ShopActive == 1 && this->TypeShop != -1)
	{
		if (g_pBCustomMenuInfo->DrawButton(bStartX + 30, bStartY + 320, 140, 12, "Close Store", 120))
		{

			CGSendOffTrade(7);
		}
		if (g_pBCustomMenuInfo->DrawButton(bStartX + 30, bStartY + 320 + 40, 140, 12, "Offline Store", 120))
		{
			gInterface.m_Disconnect = true;
			CGSendOffTrade(6);
			
		}
	}
	else
	{

		if (UIController.BButton(gInterface.Data[IMG_31617].ModelID, bStartX + 30, bStartY + 320, 38, 23, 38, 23, 3, state))
		{
			CGSendOffTrade(0);
		}
		TextDraw((HFONT)g_hFontBold, bStartX + 30, bStartY + 320 + 5, 0xFFC738FF, 0x0, 39, 0, 3, "Bless");

		bStartX += 40;
		if (UIController.BButton(gInterface.Data[IMG_31617].ModelID, bStartX + 30, bStartY + 320, 38, 23, 38, 23, 3, state))
		{
			CGSendOffTrade(1);
		}
		TextDraw((HFONT)g_hFontBold, bStartX + 30, bStartY + 320 + 5, 0xFFC738FF, 0x0, 39, 0, 3, "Soul");

		bStartX += 40;
		if (UIController.BButton(gInterface.Data[IMG_31617].ModelID, bStartX + 30, bStartY + 320, 38, 23, 38, 23, 3, state))
		{
			CGSendOffTrade(2);
		}
		TextDraw((HFONT)g_hFontBold, bStartX + 30, bStartY + 320 + 5, 0xFFC738FF, 0x0, 39, 0, 3, "Chaos");

		//=======================
		bStartX = X;
		bStartY += 27;
		if (UIController.BButton(gInterface.Data[IMG_31617].ModelID, bStartX + 30, bStartY + 320, 38, 23, 38, 23, 3, state))
		{
			CGSendOffTrade(3);
		}
		TextDraw((HFONT)g_hFontBold, bStartX + 30, bStartY + 320 + 5, 0xFFC738FF, 0x0, 39, 0, 3, "WC");


		bStartX += 40;
		if (UIController.BButton(gInterface.Data[IMG_31617].ModelID, bStartX + 30, bStartY + 320, 38, 23, 38, 23, 3, state))
		{
			CGSendOffTrade(4);
		}
		TextDraw((HFONT)g_hFontBold, bStartX + 30, bStartY + 320 + 5, 0xFFC738FF, 0x0, 39, 0, 3, "WP");


		bStartX += 40;
		if (UIController.BButton(gInterface.Data[IMG_31617].ModelID, bStartX + 30, bStartY + 320, 38, 23, 38, 23, 3, state))
		{
			CGSendOffTrade(5);
		}
		TextDraw((HFONT)g_hFontBold, bStartX + 30, bStartY + 320 + 5, 0xFFC738FF, 0x0, 39, 0, 3, "GP");
	}
}

void CB_OffTrade::CGSendOffTrade(int Type)
{
	if (GetTickCount() > this->ClickTickCount)
	{
		PMSG_OFFTRADE_SEND pMsg;
		pMsg.header.set(0xF3, 0xEB, sizeof(pMsg));
		pMsg.Type = Type;
		DataSend((BYTE*)& pMsg, pMsg.header.size);

		this->ClickTickCount = GetTickCount() + 1000;
		//gInterface.DrawMessage(1, "CGSendOffTrade %d", Type);
	}
}

void CB_OffTrade::RecvPShop(PMSG_OFFTRADE_RECV* Data)
{
	this->ShopList = 1;
	this->TypeShop = Data->Type;
}

void CB_OffTrade::PShopActiveRecv(PMSG_SHOPACTIVE_RECV* Data)
{
	this->ShopActive = Data->Active;
	this->TypeShop = Data->Type;
}


void CB_OffTrade::ShowPrice(int TextNum, char* Price)
{
	if (this->ShopActive != 0 || this->ShopList != 0)
	{
		if (this->TypeShop == 0)
		{
			sprintf(TextList[TextNum], gCustomMessage.GetMessage(61), Price);
		}
		else if (this->TypeShop == 1)
		{
			sprintf(TextList[TextNum], gCustomMessage.GetMessage(62), Price);
		}
		else if (this->TypeShop == 2)
		{
			sprintf(TextList[TextNum], gCustomMessage.GetMessage(63), Price);
		}
		else if (this->TypeShop == 3)
		{
			sprintf(TextList[TextNum], gCustomMessage.GetMessage(55), Price);
		}
		else if (this->TypeShop == 4)
		{
			sprintf(TextList[TextNum], gCustomMessage.GetMessage(56), Price);
		}
		else if (this->TypeShop == 5)
		{
			sprintf(TextList[TextNum], gCustomMessage.GetMessage(57), Price);
		}
		else
		{
			sprintf(TextList[TextNum], gCustomMessage.GetMessage(49), Price);
		}
	}
	else
	{
		sprintf(TextList[TextNum], GlobalText[63], Price);
	}
	
}

void CB_OffTrade::ShowMessNotice()
{
	if (this->ShopActive != 0 || this->ShopList != 0)
	{
		if (this->TypeShop == 0)
		{
			g_pChatListBox->AddText("", "Bạn không đủ Bless", SEASON3B::TYPE_ERROR_MESSAGE);
		}
		else if (this->TypeShop == 1)
		{
			g_pChatListBox->AddText("", "Bạn không đủ Soul", SEASON3B::TYPE_ERROR_MESSAGE);
		}
		else if (this->TypeShop == 2)
		{
			g_pChatListBox->AddText("", "Bạn không đủ WCoin", SEASON3B::TYPE_ERROR_MESSAGE);
		}
		else if (this->TypeShop == 3)
		{
			g_pChatListBox->AddText("", "Bạn không đủ WCoinP", SEASON3B::TYPE_ERROR_MESSAGE);
		}
		else if (this->TypeShop == 4)
		{
			g_pChatListBox->AddText("", "Bạn không đủ GobinPoint", SEASON3B::TYPE_ERROR_MESSAGE);
		}
		else if (this->TypeShop == 5)
		{
			g_pChatListBox->AddText("", "Bạn không đủ Zen", SEASON3B::TYPE_ERROR_MESSAGE);
		}
		else
		{
			g_pChatListBox->AddText("", GlobalText[423], SEASON3B::TYPE_ERROR_MESSAGE);
		}
	}
	else
	{
		g_pChatListBox->AddText("", GlobalText[423], SEASON3B::TYPE_ERROR_MESSAGE);
	}
}

void CB_OffTrade::RenderTextNotice(float X, float Y)
{
	unicode::t_char Text[100];
	memset(&Text, 0, sizeof(unicode::t_char) * 100);

	if (this->ShopActive != 0 || this->ShopList != 0)
	{
		if (this->TypeShop == 0)
		{
			sprintf(Text, "Chỉ có thể giao dịch bằng Bless");
		}
		else if (this->TypeShop == 1)
		{
			sprintf(Text, "Chỉ có thể giao dịch bằng Soul");
		}
		else if (this->TypeShop == 2)
		{
			sprintf(Text, "Chỉ có thể giao dịch bằng Chaos");
		}
		else if (this->TypeShop == 3)
		{
			sprintf(Text, "Chỉ có thể giao dịch bằng WCoin");
		}
		else if (this->TypeShop == 4)
		{
			sprintf(Text, "Chỉ có thể giao dịch bằng WCoinP");
		}
		else if (this->TypeShop == 5)
		{
			sprintf(Text, "Chỉ có thể giao dịch bằng GobinP");
		}
		else
		{
			sprintf(Text, GlobalText[1135]);
		}
	}
	else
	{
		sprintf(Text, GlobalText[1135]);
	}

	//sprintf(Text, GlobalText[1135]); 


	TextDraw((HFONT)g_hFontBold, X + 30, Y + 332, RGBA(255, 45, 47, 255), 0x0, 0, 0, 3, Text);
}