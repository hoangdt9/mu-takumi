#include "StdAfx.h"
#include "CB_BotTrader.h"
#include "NewUISystem.h"
#include "CBInterface.h"
#include "CUIController.h"
#include "CharacterManager.h"
#include "Util.h"
#include "Protocol.h"
#include "NewUIBase.h"
#include "Other.h"
#include "ZzzInterface.h"


CB_BotTrader* gCB_BotTrader;

CB_BotTrader::CB_BotTrader()
{
	ListTraderMixScrollBar = nullptr;
	ListItemScrollBar = nullptr;
	MaxListItemInPage = 8;
	StartXWindow = 0;
	CB_BotTrader::ClearData();
}

CB_BotTrader::~CB_BotTrader()
{
}
void CB_BotTrader::ClearData()
{
	ListItemMix.Clear();
	m_DataListMix.clear();
	gInterface.Data[eWindowListItemTrader].OnShow = 0;
}
void CB_BotTrader::DrawXemItemMix()
{
	if (!gInterface.Data[eWindowListItemTrader].OnShow)
	{
		return;
	}

	float WindowW = 225;
	float WindowH = 270;

	float StartX = StartXWindow- WindowW;
	float StartY = 55.0;

	char* NameCoin[] = { "Zen","WCoin","WCoinP", "GobinPoint","NULL" };
	int CountTextInfo = 0;
	gInterface.Data[eWindowListItemTrader].AllowMove = false;
	if (g_pBCustomMenuInfo->gDrawWindowCustom(&StartX, &StartY, WindowW, WindowH, eWindowListItemTrader, "Công Thức Trader Mix"))
	{

		StartX = StartX + 10;
		StartY = StartY +5;
		float WInfo = (WindowW - 20);
		float HInfo = WindowH - 100;

		if (!ListItemMix.IsLoad)
		{
			char buf[25];
			int step = GetTickCount() / 1000 % 4;
			sprintf(buf, "Loading data%%.%ds", step);
			TextDraw((HFONT)g_hFont, StartX + 1, StartY + 55, 0xffffffff, 0x0, WInfo, 0, 3, buf, "...");
			return;
		}

		TextDraw((HFONT)g_hFont, StartX + 15, StartY + 35, 0xFF00EEDFF, 0x0, WindowW, 0, 1, "Danh Sách Item Cần");

		//Scroll Bar
		int DataListItem = ListItemMix.ListItem.size();
		if (!ListItemScrollBar)
		{
			ListItemScrollBar = new CNewUIScrollBar();
			ListItemScrollBar->Create((StartX + WindowW) - 25, StartY + 35 + 15, 130);
			if (ListItemScrollBar)
			{
				ListItemScrollBar->SetMaxPos((DataListItem <= MaxListItemInPage) ? 0 : (DataListItem / MaxListItemInPage) - 1);
				ListItemScrollBar->SetPos((StartX + WindowW) - 25, StartY + 35 + 15);
			}
		}
		if (ListItemScrollBar)
		{
			if (gInterface.Data[eWindowListItemTrader].OnClick)
			{
				ListItemScrollBar->SetPos((StartX + WindowW) - 25, StartY + 35 + 15);
				ListItemScrollBar->SetCurPos(0);
			}
			ListItemScrollBar->MouseWheelWindow = SEASON3B::CheckMouseIn(StartX + 15, StartY + 35 + 15, WindowW, 130);
			ListItemScrollBar->Render();
			ListItemScrollBar->UpdateMouseEvent();
			ListItemScrollBar->Update();
		}

		//==List Box Item 
		float PosXBoxItemGoc = StartX + 25;
		float PosXBoxItem = StartX + 25;
		float PosYBoxItem = StartY + 55;
		float WBox = 30;
		float KhoangCach = 45;
		int CountNgang = 0;
		int CountDoc = 0;
		int ItemListPage = ListItemScrollBar->GetCurPos();
		int BBShowInfoItem = -1;

		//
		int currentRow = 0;
		int currentCol = 0;
		int CountText = 0;
		int CountList = 0;
		
		for (int n = (ItemListPage * MaxListItemInPage); n < DataListItem; n++)
		{
			if (CountList > MaxListItemInPage) break;
			std::ostringstream ossNameItem;
			ossNameItem << BGetItemName(ListItemMix.ListItem[n].IndexItem, SET_ITEMOPT_LEVEL(ListItemMix.ListItem[n].MinLevel));
			if(ListItemMix.ListItem[n].MaxLevel > ListItemMix.ListItem[n].MinLevel && ListItemMix.ListItem[n].MinLevel > 0) ossNameItem << "Lv "<< ListItemMix.ListItem[n].MinLevel << "~" << ListItemMix.ListItem[n].MaxLevel;
			if (ListItemMix.ListItem[n].MaxOpt > ListItemMix.ListItem[n].MinOpt && ListItemMix.ListItem[n].MinOpt > 0) ossNameItem << ", Opt " << ListItemMix.ListItem[n].MinOpt << "~" << ListItemMix.ListItem[n].MaxOpt;
			if (ListItemMix.ListItem[n].Luck) ossNameItem << ", +Luck";
			if (ListItemMix.ListItem[n].Skill) ossNameItem << ", +Skill";
			if (ListItemMix.ListItem[n].NeedExc) ossNameItem << ", +Exl";

			TextDraw(g_hFont, StartX + 5, PosYBoxItem + (14 * CountText++), 0xFFFFFFFF, 0x0, WindowH, 0, 1, "x1 %s", strdup(ossNameItem.str().c_str()));
			CountList++;

		}

		//===Coin
		float PosYCoinNhan = StartY + 175;
		CountTextInfo++;
		TextDraw((HFONT)g_hFont, StartX, PosYCoinNhan + (10 * CountTextInfo), 0xFF8214FF, 0x0, WindowW - 40, 0, 1, "*Cần :");
		TextDraw((HFONT)g_hFont, StartX, PosYCoinNhan + (10 * CountTextInfo++), 0xFFFFFFFF, 0x0, WindowW-40, 0, 4, "%s : %s", NameCoin[ListItemMix.TypeCoin], gInterface.NumberFormat(ListItemMix.Coin));
	
		if (ListItemMix.KeepOption || ListItemMix.KeepLevel)
		{
			TextDraw((HFONT)g_hFont, StartX, PosYCoinNhan + (10 * CountTextInfo), 0xFF8214FF, 0x0, WindowW - 40, 0, 1, "*Thành Công :");
			std::ostringstream ossTextKeep;
			if (ListItemMix.KeepLevel) ossTextKeep << "Giữ Cấp Độ ";
			if (ListItemMix.KeepOption) ossTextKeep << "Giữ Tính Năng";
			TextDraw((HFONT)g_hFont, StartX, PosYCoinNhan + (10 * CountTextInfo++), 0xFFFFFFFF, 0x0, WindowW - 40, 0, 4, strdup(ossTextKeep.str().c_str()));
		}
		if (ListItemMix.KeepItemMixFail)
		{
			TextDraw((HFONT)g_hFont, StartX, PosYCoinNhan + (10 * CountTextInfo), 0xFF8214FF, 0x0, WindowW - 40, 0, 1, "*Thất Bại :");
			TextDraw((HFONT)g_hFont, StartX, PosYCoinNhan + (10 * CountTextInfo++), 0xFFFFFFFF, 0x0, WindowW - 40, 0, 4, "Giữ lại Item Chính");
		}
		TextDraw((HFONT)g_hFont, StartX, PosYCoinNhan + (10 * CountTextInfo), 0xFF8214FF, 0x0, WindowW - 40, 0, 1, "*Tỷ lệ :");
		TextDraw((HFONT)g_hFont, StartX, PosYCoinNhan + (10 * CountTextInfo++), 0xFFFFFFFF, 0x0, WindowW - 40, 0, 4, "%d%%", ListItemMix.Rate);
	}
}
bool CB_BotTrader::DrawWindow(int X, int Y)
{
	StartXWindow = X;
	if (this->m_DataListMix.empty()) return 0;

	float StartX = X + 9;
	float StartY = Y + 35;
	float W = 172;
	float H = 190;
	static int ClickCheck = 0;
	static int TimeClickCheck = 0;

	RenderImage(CNewUITrade::IMAGE_TRADE_BACK, StartX, StartY, W, H);

	TextDraw(g_hFontBold, StartX, StartY, 0xFFFFFFFF, 0x0, W, 0, 3, "**Danh Sách Trader Mix**");
	g_pBCustomMenuInfo->DrawInfoBox(StartX + 3, StartY + 15, W-13, 130, 0x00000096, 0, 0);

	////===ScrollBar
	float ScrollBarX =StartX + 3;
	float ScrollBarY =StartY + 17;
	float ScrollBarW = 160;
	float ScrollBarH = 130;
	float ScrollBarMaxPos = (this->m_DataListMix.size()/ MaxListItemInPage);
	if (!ListTraderMixScrollBar)
	{
		ListTraderMixScrollBar = new CNewUIScrollBar();
		ListTraderMixScrollBar->Create(ScrollBarX, ScrollBarY, ScrollBarH);
		ListTraderMixScrollBar->SetMaxPos(ScrollBarMaxPos);
		ListTraderMixScrollBar->SetPos(ScrollBarX+(ScrollBarW-5), ScrollBarY);
	}
	//
	else
	{
		ListTraderMixScrollBar->MouseWheelWindow = SEASON3B::CheckMouseIn(ScrollBarX, ScrollBarY, ScrollBarW, ScrollBarH);

		ListTraderMixScrollBar->Render();
		ListTraderMixScrollBar->UpdateMouseEvent();
		ListTraderMixScrollBar->Update();
	}
	int CountText = 0;
	DWORD Color = 0x0;
	int mPageGet = ListTraderMixScrollBar->GetCurPos();
	int CountList = 0;
	int MaxListInPage = 8;
	for (int n = (mPageGet * MaxListInPage); n < this->m_DataListMix.size(); n++)
	{
		if (CountList > MaxListInPage) break;
		if (SEASON3B::CheckMouseIn(StartX + 5, StartY + 20 + (14 * CountText),140,14))
		{
			Color = 0x87CEFAA5;
			if (SEASON3B::IsPress(VK_LBUTTON))
			{
				ClickCheck++;
				TimeClickCheck = GetTickCount() + 500;
				if (ClickCheck > 1)
				{
					//gInterface.DrawMessage(1, "Dupe Click %d", ClickCheck);
					ClickCheck = 0;
					this->OpenListMix(this->m_DataListMix[n].MixIndex);
				}
			}
		}
		else
		{
			Color = 0x0;
		}
		TextDraw(g_hFont, StartX+5, StartY+20+(14* CountText++), 0xFFFFFFFF, Color, 140, 0, 1, "%s [%d]", this->m_DataListMix[n].NameMix, this->m_DataListMix[n].MixIndex);
		CountList++;
	}
	TextDraw((HFONT)g_hFont, StartX, StartY+160, 0x14FFC0FF, 0x0, W, 0, 1, "*Click 2 lần vào tên Mix để hiển thị công thức Mix!");
	TextDraw((HFONT)g_hFont, StartX, StartY+175, 0xA9FFC0FF, 0x0, W, 0, 1, "*Mỗi BotTrader sẽ có danh sách mix riêng");
	//gInterface.DrawMessage(1, "Dupe Click %d", ClickCheck);
	if (GetTickCount() > TimeClickCheck) ClickCheck =0;
	//===
	CB_BotTrader::DrawXemItemMix();
	return true;
}
void CB_BotTrader::OpenListMix(int MixIndex)
{
	gInterface.Data[eWindowListItemTrader].OnShow = 1;
	XULY_CGPACKET pMsg;
	pMsg.header.set(0xD3, 0x2E, sizeof(pMsg));
	pMsg.ThaoTac = MixIndex; //
	DataSend((LPBYTE)& pMsg, pMsg.header.size);
}
void CB_BotTrader::RecvProtocol(BYTE* lpMsg)
{
	if (!lpMsg) return;
	PMSG_CUSTOM_LIST* mRecv = (PMSG_CUSTOM_LIST*)lpMsg;

	//gInterface.DrawMessage(1, "Recv Type %d", mRecv->TypeSend);
	switch (mRecv->TypeSend)
	{
	case CB_BotTrader::eSendRecvListDataMix:
		{
			this->m_DataListMix.clear();
			for (int i = 0; i < mRecv->count; i++)
			{
				LIST_MIX_TRADER lpInfo = *(LIST_MIX_TRADER*)(((BYTE*)lpMsg) + sizeof(PMSG_CUSTOM_LIST) + (sizeof(LIST_MIX_TRADER) * i));
				this->m_DataListMix.push_back(lpInfo);
			}
		}
		break;
	case CB_BotTrader::eSendRecvListItemMix:
		{
			this->ListItemMix.Clear();
			this->ListItemMix.TypeCoin = mRecv->TypeCoin;
			this->ListItemMix.Coin = mRecv->Coin;
			this->ListItemMix.Rate = mRecv->Rate;
			this->ListItemMix.KeepLevel = mRecv->KeepLevel;
			this->ListItemMix.KeepOption = mRecv->KeepOption;
			this->ListItemMix.KeepItemMixFail = mRecv->KeepItemMixFail;
			for (int i = 0; i < mRecv->count; i++)
			{
				DATA_ITEMSEND lpInfo = *(DATA_ITEMSEND*)(((BYTE*)lpMsg) + sizeof(PMSG_CUSTOM_LIST) + (sizeof(DATA_ITEMSEND) * i));
				this->ListItemMix.ListItem.push_back(lpInfo);
			}
			this->ListItemMix.IsLoad = true;
		}
		break;
	default:
		break;
	}

}