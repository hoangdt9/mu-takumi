#include "stdafx.h"
#include "CBChoTroi.h"
#include "NewUISystem.h"
#include "CBInterface.h"
#include "CUIController.h"
#include "CharacterManager.h"
#include "Util.h"
#include "Protocol.h"
#include "NewUIBase.h"
#include "Other.h"
#if CUSTOM_BCHOTROI

CBChotroi gCBChotroi;
CUIPhotoViewer m_PhotoViewChotroi;
CNewUIScrollBar* WindowChoTroiScrollBar = NULL;
CUITextInputBox* CoinRaoBanChoTroi = NULL;
CUITextInputBox* NgayRaoBanChoTroi = NULL; //HSD Them
CUITextInputBox* PassChoTroi = NULL;
char szNgayRaoBan[2 + 1] = { 0, };
char szGiaTriCoin[8 + 1] = { 0, };
char szPassChoTroi[5 + 1] = { 0, };

int SelectBarChoTroiNumber = 0;
const char* NameGiaCoin[7] = { "NULL", "", "", "", "B", "S", "C" };
const char* SelectBarListChoTroi[13] = { "", "", "", "", "", "", "", "", "", "", "", "", "" };
bool SelectBarChoTroi = false;
CUITextInputBox* m_TimTuKhoaChoTroi = NULL;
CUITextInputBox* m_TimTuKhoaChoTroi_Clear = NULL;
CUITextInputBox* NhapPassChoTroi = NULL;
char szNhapPassChoTroi[5 + 1] = { 0, };

int MaxPerPageChoTroi = 3;
int mPageChoTroi = 0;

DWORD CacheTimeFind = 0;
int CacheSizeInputTimKiem = -1;
int CacheItemRaoBan = -1;
int TypeCoinBan = 1;
int ShowInfoItem = -1;
bool UpdateMaxPosChoTroi = false;
int LocItemTypCoin = 63;
int LocItemLvMin = 0;
int LocItemLvMax = 15;
bool LocItemCoSkill = false;
bool LocItemCoLuck = false;
bool LocItemCoOpt = false;
bool LocItemCoExc = false;
bool HoverItemSell = false;

bool ShowWindowChoOK = false;

int BuyID = -1;
static void SyncChoTroiTextCache()
{
	NameGiaCoin[1] = gOther.TextVN_CHOTROI[1];
	NameGiaCoin[2] = gOther.TextVN_CHOTROI[2];
	NameGiaCoin[3] = gOther.TextVN_CHOTROI[3];

	SelectBarListChoTroi[0] = gOther.TextVN_CHOTROI[26];
	SelectBarListChoTroi[1] = gOther.TextVN_CHOTROI[27];
	SelectBarListChoTroi[2] = gOther.TextVN_CHOTROI[28];
	SelectBarListChoTroi[3] = gOther.TextVN_CHOTROI[29];
	SelectBarListChoTroi[4] = gOther.TextVN_CHOTROI[30];
	SelectBarListChoTroi[5] = gOther.TextVN_CHOTROI[31];
	SelectBarListChoTroi[6] = gOther.TextVN_CHOTROI[32];
	SelectBarListChoTroi[7] = gOther.TextVN_CHOTROI[33];
	SelectBarListChoTroi[8] = gOther.TextVN_CHOTROI[34];
	SelectBarListChoTroi[9] = gOther.TextVN_CHOTROI[35];
	SelectBarListChoTroi[10] = gOther.TextVN_CHOTROI[36];
	SelectBarListChoTroi[11] = gOther.TextVN_CHOTROI[37];
	SelectBarListChoTroi[12] = gOther.TextVN_CHOTROI[38];
}

CBChotroi::CBChotroi()
{
}

CBChotroi::~CBChotroi()
{

}
void DrawInfoPhai(int X, int Y)
{
	SyncChoTroiTextCache();

	switch (gCBChotroi.m_iNumCurOpenTab)
	{
	case 0:
	{
		//=================== Khung Char
		float WindowHoatAnhW = 160;
		float ViewCharX = X;
		float ViewCharY = Y;
		m_PhotoViewChotroi.SetPosition(ViewCharX, ViewCharY-35);
		m_PhotoViewChotroi.Render();
		m_PhotoViewChotroi.DoMouseAction();
		if (g_pBCustomMenuInfo->DrawButton(ViewCharX+48, ViewCharY+95, 100, 12, gOther.TextVN_CHOTROI[40], 55) )
		{
			m_PhotoViewChotroi.CopyPlayer();
		}
		//=================== Khung Loc List
		float LocListItemX = X;
		float LocListItemY = Y+120;
		float LocListH = 100;
		g_pBCustomMenuInfo->DrawInfoBox(LocListItemX + 7, LocListItemY, (WindowHoatAnhW - 25), LocListH, 0x00000096, 0);
		TextDraw((HFONT)g_hFontBold, LocListItemX + 9, LocListItemY + 2, 0xFFFFFFFF, 0x00BFFF96, WindowHoatAnhW - 21, 0, 3, gOther.TextVN_CHOTROI[21]);//
		float CheckBoxX = LocListItemX + 12;
		float CheckBoxY = LocListItemY + 12;
		//==============
		if (g_pBCustomMenuInfo->RenderCheckBox(CheckBoxX, CheckBoxY, 0xFFCC00C8, (LocItemTypCoin & 1) == 1 ? TRUE : FALSE, gOther.TextVN_CHOTROI[1]))
		{
			if ((LocItemTypCoin & 1) == 1)
			{
				LocItemTypCoin -= 1;
			}
			else {
				LocItemTypCoin += 1;
			}
		}

		if (g_pBCustomMenuInfo->RenderCheckBox(CheckBoxX + 50, CheckBoxY, 0xFFCC00C8, (LocItemTypCoin & 2) == 2 ? TRUE : FALSE, gOther.TextVN_CHOTROI[2]))
		{
			if ((LocItemTypCoin & 2) == 2)
			{
				LocItemTypCoin -= 2;
			}
			else {
				LocItemTypCoin += 2;
			}
		}
		if (g_pBCustomMenuInfo->RenderCheckBox(CheckBoxX + (50 * 2), CheckBoxY, 0xFFCC00C8, (LocItemTypCoin & 4) == 4 ? TRUE : FALSE, gOther.TextVN_CHOTROI[3]))
		{
			if ((LocItemTypCoin & 4) == 4)
			{
				LocItemTypCoin -= 4;
			}
			else {
				LocItemTypCoin += 4;
			}
		}

		if (g_pBCustomMenuInfo->RenderCheckBox(CheckBoxX, CheckBoxY + 15, 0xFFCC00C8, (LocItemTypCoin & 8) == 8 ? TRUE : FALSE, "B"))
		{
			if ((LocItemTypCoin & 8) == 8)
			{
				LocItemTypCoin -= 8;
			}
			else {
				LocItemTypCoin += 8;
			}
		}

		if (g_pBCustomMenuInfo->RenderCheckBox(CheckBoxX + 50, CheckBoxY + 15, 0xFFCC00C8, (LocItemTypCoin & 16) == 16 ? TRUE : FALSE, "S"))
		{
			if ((LocItemTypCoin & 16) == 16)
			{
				LocItemTypCoin -= 16;
			}
			else {
				LocItemTypCoin += 16;
			}
		}
		if (g_pBCustomMenuInfo->RenderCheckBox(CheckBoxX + (50 * 2), CheckBoxY + 15, 0xFFCC00C8, (LocItemTypCoin & 32) == 32 ? TRUE : FALSE, "C"))
		{
			if ((LocItemTypCoin & 32) == 32)
			{
				LocItemTypCoin -= 32;
			}
			else {
				LocItemTypCoin += 32;
			}
		}

		//	gInterface.DrawMessage(1, "LocItemTypCoin %d",LocItemTypCoin);
		CheckBoxY += 18;
		//===========
		if (g_pBCustomMenuInfo->RenderCheckBox(CheckBoxX, CheckBoxY + (15 * 1), 0xFFCC00C8, LocItemCoSkill == 1 ? TRUE : FALSE, gOther.TextVN_CHOTROI[22]))
		{
			LocItemCoSkill ^= 1;
		}
		if (g_pBCustomMenuInfo->RenderCheckBox(CheckBoxX, CheckBoxY + (15 * 2), 0xFFCC00C8, LocItemCoLuck == 1 ? TRUE : FALSE, gOther.TextVN_CHOTROI[23]))
		{
			LocItemCoLuck ^= 1;
		}
		if (g_pBCustomMenuInfo->RenderCheckBox(CheckBoxX, CheckBoxY + (15 * 3), 0xFFCC00C8, LocItemCoOpt == 1 ? TRUE : FALSE, gOther.TextVN_CHOTROI[24]))
		{
			LocItemCoOpt ^= 1;
		}
		if (g_pBCustomMenuInfo->RenderCheckBox(CheckBoxX, CheckBoxY + (15 * 4), 0xFFCC00C8, LocItemCoExc == 1 ? TRUE : FALSE, gOther.TextVN_CHOTROI[25]))
		{
			LocItemCoExc ^= 1;
		}
	}
	break;
	case 1: //Rao Ban
	{
		//==============
		float StartX = X;
		float StartY = Y;
		float WindowW = 160;
		float WindowH = 250;
		int GetInfoLoadItem = 0;


		//===Draw Button Buy
		float ButtonW = 100;
		g_pBCustomMenuInfo->DrawInfoBox(StartX + (WindowW / 2) - (30), StartY + 40, 55, 55, (HoverItemSell? 0x66646496 :0x00000096), 0);//Khung Item
		float CheckBoxX = StartX + (WindowW / 2) - 70;

		if (gCBChotroi.ItemCacheSelect != NULL && gCBChotroi.ItemCacheShow)
		{
			if (g_pBCustomMenuInfo->RenderCheckBox(CheckBoxX, StartY + 110, 0xFFCC00C8, TypeCoinBan == 1 ? TRUE : FALSE, gOther.TextVN_CHOTROI[1]))
			{
				if (gCBChotroi.OnCointType & 1) TypeCoinBan = 1;
			}

			if (g_pBCustomMenuInfo->RenderCheckBox(CheckBoxX + 50, StartY + 110, 0xFFCC00C8, TypeCoinBan == 2 ? TRUE : FALSE, gOther.TextVN_CHOTROI[2]))
			{
				if (gCBChotroi.OnCointType & 2)TypeCoinBan = 2;
			}
			if (g_pBCustomMenuInfo->RenderCheckBox(CheckBoxX + (50 * 2), StartY + 110, 0xFFCC00C8, TypeCoinBan == 3 ? TRUE : FALSE, gOther.TextVN_CHOTROI[3]))
			{
				if (gCBChotroi.OnCointType & 4)TypeCoinBan = 3;
			}

			if (g_pBCustomMenuInfo->RenderCheckBox(CheckBoxX, StartY + 130, 0xFFCC00C8, TypeCoinBan == 4 ? TRUE : FALSE, "B"))
			{
				if (gCBChotroi.OnCointType & 8)TypeCoinBan = 4;

			}

			if (g_pBCustomMenuInfo->RenderCheckBox(CheckBoxX + (50 * 1), StartY + 130, 0xFFCC00C8, TypeCoinBan == 5 ? TRUE : FALSE, "S"))
			{
				if (gCBChotroi.OnCointType & 16)TypeCoinBan = 5;

			}

			if (g_pBCustomMenuInfo->RenderCheckBox(CheckBoxX + (50 * 2), StartY + 130, 0xFFCC00C8, TypeCoinBan == 6 ? TRUE : FALSE, "C"))
			{
				if (gCBChotroi.OnCointType & 32)TypeCoinBan = 6;

			}

			RenderBitmap(CNewUIGuildMakeWindow::IMAGE_GUILDMAKE_EDITBOX, StartX + (WindowW / 2) - 70, StartY + 150, 140, 20, 0.0, 0.0, 0.82, 0.71, 1, 1, 0.0);
			TextDraw((HFONT)g_hFont, StartX + (WindowW / 2) - 70, StartY + 154, 0xFFFFFFC8, 0x0, 70, 0, 3, gOther.TextVN_CHOTROI[4]);
			float TCoinX = StartX + (WindowW / 2);
			float TCoinY = StartY + 154;
			if (!CoinRaoBanChoTroi)
			{
				CoinRaoBanChoTroi = new CUITextInputBox;
				CoinRaoBanChoTroi->Init(pGameWindow, TCoinX, TCoinY, 8);
				CoinRaoBanChoTroi->SetBackColor(255, 0, 0, 0);
				CoinRaoBanChoTroi->SetTextColor(255, 255, 157, 0);
				CoinRaoBanChoTroi->SetFont((HFONT)g_hFont);
				CoinRaoBanChoTroi->SetState(UISTATE_NORMAL);
				CoinRaoBanChoTroi->SetOption(UIOPTION_NUMBERONLY);
				CoinRaoBanChoTroi->SetPosition(TCoinX, TCoinY);
				CoinRaoBanChoTroi->SetText("0");
			}
			else
			{

				CoinRaoBanChoTroi->Render();

				CoinRaoBanChoTroi->GetText(szGiaTriCoin, 8 + 1);
				CoinRaoBanChoTroi->SetPosition(TCoinX, TCoinY);


				if (SEASON3B::CheckMouseIn(TCoinX - 5, TCoinY - 5, 100, 30) == 1)
				{

					if (GetKeyState(VK_LBUTTON) & 0x8000)
					{
						CoinRaoBanChoTroi->GiveFocus(1);
						PlayBuffer(25, 0, 0);
					}
				}
			}
			//==//HSD Them
			RenderBitmap(CNewUIGuildMakeWindow::IMAGE_GUILDMAKE_EDITBOX, StartX + (WindowW / 2) - 70, StartY + 170, 140, 20, 0.0, 0.0, 0.82, 0.71, 1, 1, 0.0);
			TextDraw((HFONT)g_hFont, StartX + (WindowW / 2) - 70, StartY + 174, 0xFFFFFFC8, 0x0, 70, 0, 3, gOther.TextVN_CHOTROI[43]);
			float NTCoinX = StartX + (WindowW / 2);
			float NTCoinY = StartY + 174;
			if (!NgayRaoBanChoTroi)
			{
				NgayRaoBanChoTroi = new CUITextInputBox;
				NgayRaoBanChoTroi->Init(pGameWindow, NTCoinX, NTCoinY, 2);
				NgayRaoBanChoTroi->SetBackColor(255, 0, 0, 0);
				NgayRaoBanChoTroi->SetTextColor(255, 255, 157, 0);
				NgayRaoBanChoTroi->SetFont((HFONT)g_hFont);
				NgayRaoBanChoTroi->SetState(UISTATE_NORMAL);
				NgayRaoBanChoTroi->SetOption(UIOPTION_NUMBERONLY);
				NgayRaoBanChoTroi->SetPosition(NTCoinX, NTCoinY);
				NgayRaoBanChoTroi->SetText("1");
			}
			else
			{

				NgayRaoBanChoTroi->Render();

				NgayRaoBanChoTroi->GetText(szNgayRaoBan, 2 + 1);
				NgayRaoBanChoTroi->SetPosition(NTCoinX, NTCoinY);


				if (SEASON3B::CheckMouseIn(NTCoinX - 5, NTCoinY - 5, 100, 30) == 1)
				{

					if (GetKeyState(VK_LBUTTON) & 0x8000)
					{
						NgayRaoBanChoTroi->GiveFocus(1);
						PlayBuffer(25, 0, 0);
					}
				}
				if (atoi(szNgayRaoBan) > 30)
				{
					NgayRaoBanChoTroi->SetText("30");
				}
				else if (atoi(szNgayRaoBan) < 1)
				{
					NgayRaoBanChoTroi->SetText("1");
				}
			}

			RenderBitmap(CNewUIGuildMakeWindow::IMAGE_GUILDMAKE_EDITBOX, StartX + (WindowW / 2) - 70, StartY + 190, 140, 20, 0.0, 0.0, 0.82, 0.71, 1, 1, 0.0);
			TextDraw((HFONT)g_hFont, StartX + (WindowW / 2) - 70, StartY + 194, 0xFFFFFFC8, 0x0, 70, 0, 3, "Pass");
			float NTCoinX2 = StartX + (WindowW / 2);
			float NTCoinY2 = StartY + 194;
			if (!PassChoTroi)
			{
				PassChoTroi = new CUITextInputBox;
				PassChoTroi->Init(pGameWindow, NTCoinX2, NTCoinY2, 6);
				PassChoTroi->SetBackColor(255, 0, 0, 0);
				PassChoTroi->SetTextColor(255, 255, 157, 0);
				PassChoTroi->SetFont((HFONT)g_hFont);
				PassChoTroi->SetState(UISTATE_NORMAL);
				PassChoTroi->SetOption(UIOPTION_NUMBERONLY);
				PassChoTroi->SetPosition(NTCoinX2, NTCoinY2);
				PassChoTroi->SetText("0");
			}
			else
			{

				PassChoTroi->Render();

				PassChoTroi->GetText(szPassChoTroi, 5 + 1);
				PassChoTroi->SetPosition(NTCoinX2, NTCoinY2);


				if (SEASON3B::CheckMouseIn(NTCoinX2 - 5, NTCoinY2 - 5, 100, 30) == 1)
				{

					if (GetKeyState(VK_LBUTTON) & 0x8000)
					{
						PassChoTroi->GiveFocus(1);
						PlayBuffer(25, 0, 0);
					}
				}
			}

			if (g_pBCustomMenuInfo->DrawButton(StartX + (WindowW / 2) - (ButtonW / 2), StartY + WindowH - 30, 110, 12, gOther.TextVN_CHOTROI[5]))
			{
				if (atoi(szGiaTriCoin) == 0) { return; }
				if (TypeCoinBan < 1 || TypeCoinBan > 6) { return; }
				//gInterface.DrawMessage(1, "Coint %d", atoi(szGiaTriCoin));
				//
				//gInterface.DrawMessage(1, "Coint %d", TypeCoinBan);
				CBChotroi::PMSG_REQ_MARKET_SELL pMsg;
				pMsg.h.set(0xD3, 0x14, sizeof(pMsg));
				pMsg.Result = 1;
				pMsg.ItemPos = 0;
				pMsg.ItemPriceType = TypeCoinBan;
				pMsg.ItemPrice = atoi(szGiaTriCoin);
				pMsg.ItemDay = atoi(szNgayRaoBan);//HSD Them
				pMsg.Pass = atoi(szPassChoTroi);//HSD Them
				memset(&szGiaTriCoin, 0, sizeof(szGiaTriCoin));
				memset(&szNgayRaoBan, 0, sizeof(szNgayRaoBan)); //HSD Them
				memset(&szPassChoTroi, 0, sizeof(szPassChoTroi)); //HSD Them
				//gInterface.DrawMessage(1, "%d",pMsg.Pass);
				CoinRaoBanChoTroi->SetText("0");
				NgayRaoBanChoTroi->SetText("1");//HSD Them
				PassChoTroi->SetText("0");
				DataSend((LPBYTE)& pMsg, pMsg.h.size);

			}

			//======SHow INFO ITEM Rao Ban

			if (CacheItemRaoBan == -1)
			{
				CacheItemRaoBan = 1;
			}

			g_pNewUISystem->RenderItem3DFree(StartX + (WindowW / 2) - (30), StartY+10, 55, 55, gCBChotroi.ItemCacheSelect->Type, gCBChotroi.ItemCacheSelect->Level, gCBChotroi.ItemCacheSelect->Option1, gCBChotroi.ItemCacheSelect->ExtOption, 0, 1.4);//BMD MOdel
		}
		else
		{
			TextDraw((HFONT)g_hFontBold, StartX, StartY + 120, 0x0, 0x000000CC, 50, 0, 3, "");// Name Item
			TextDraw((HFONT)g_hFontBold, StartX, StartY + 120, 0xFFFFFFFF, 0xFFA20096, WindowW, 0, 3, gOther.TextVN_CHOTROI[6]);// Name Item
			TextDraw((HFONT)g_hFont, StartX, StartY + 120 + 15, 0xFFA200FF, 0x0, WindowW, 0, 3, gOther.TextVN_CHOTROI[7]);// Name Item
			TextDraw((HFONT)g_hFont, StartX, StartY + 120 + 30, 0xA2FF00FF, 0x0, WindowW, 0, 3, gOther.TextVN_CHOTROI[8]);// Name Item
		}
		//===
		if (SEASON3B::CheckMouseIn(StartX + (WindowW / 2) - (30), StartY + 40, 55, 55))
		{
			HoverItemSell = true;
			//===Show
			if (gCBChotroi.ItemCacheSelect != NULL)
			{
				RenderItemInfo(MouseX + 75, MouseY, gCBChotroi.ItemCacheSelect, 0, 0, false, false);
				if (SEASON3B::IsRelease(VK_RBUTTON))
				{
					gCBChotroi.ItemCacheSelect = NULL;
					gCBChotroi.ItemCacheShow = false;

					PlayBuffer(25, 0, 0);

					XULY_CGPACKET pMsg;
					pMsg.header.set(0xD3, 0x12, sizeof(pMsg));
					pMsg.ThaoTac = 0; //RollBack Item
					DataSend((LPBYTE)& pMsg, pMsg.header.size);
				}
			}
			else
			{
				if (SEASON3B::IsRelease(VK_LBUTTON))
				{
					CNewUIPickedItem* pPickedItem = g_pMyInventory->GetInventoryCtrl()->GetPickedItem();
					if (!pPickedItem) return;
					ITEM* ItemSell = pPickedItem->GetItem();
					int Slot = pPickedItem->GetSourceLinealPos();
					if (gCBChotroi.SendItemRaoBan(ItemSell, Slot, TRUE)) pPickedItem->HidePickedItem();
				}
			}
		}
		else
		{
			HoverItemSell = false;
		}
	}
	break;
	default:
		break;
	}
}

void DrawWindowChoTroi()
{
	SyncChoTroiTextCache();

	if (gInterface.CheckWindow(Interface::ObjWindow::MoveList) || gInterface.CheckWindow(Interface::ObjWindow::CashShop) || gInterface.CheckWindow(Interface::ObjWindow::SkillTree) || gInterface.CheckWindow(Interface::ObjWindow::FullMap)
		|| (gInterface.CheckWindow(Interface::Inventory)
			&& gInterface.CheckWindow(Interface::ExpandInventory)
			&& gInterface.CheckWindow(Interface::Store))
		|| (gInterface.CheckWindow(Interface::Inventory)
			&& gInterface.CheckWindow(Interface::Warehouse)
			&& gInterface.CheckWindow(Interface::ExpandWarehouse)))
	{
		gInterface.Data[eWindowChotroi].OnShow = false;
		return;
	}

	if (!gInterface.Data[eWindowChotroi].OnShow)
	{
		if (ShowWindowChoOK)
		{
			ShowWindowChoOK = false;
			gInterface.Data[eWindowNhapPass].OnShow = 0;
			SelectBarChoTroiNumber = 0;
			SelectBarChoTroi = false;
			WindowChoTroiScrollBar = NULL;
			m_TimTuKhoaChoTroi = NULL;
			m_TimTuKhoaChoTroi_Clear = NULL;
			m_PhotoViewChotroi.CopyPlayer();
			BuyID = -1;
			gCBChotroi.m_iNumCurOpenTab = 0;
			gCBChotroi.m_TabBtn.ChangeFrame(gCBChotroi.m_iNumCurOpenTab);
			if (NhapPassChoTroi) NhapPassChoTroi = NULL;
		}
		if (CacheItemRaoBan == 1)
		{
			//gInterface.ItemCacheSelect = NULL;
			//gInterface.ItemCacheShow = false;
			CacheItemRaoBan = -1;

			XULY_CGPACKET pMsg;
			pMsg.header.set(0xD3, 0x12, sizeof(pMsg));
			pMsg.ThaoTac = 0; //RollBack Item
			DataSend((LPBYTE)& pMsg, pMsg.header.size);
		}

		return;
	}
	if (!g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INVENTORY))
	{
		g_pNewUISystem->Show(SEASON3B::INTERFACE_INVENTORY);
	}
	if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_CHARACTER))
	{
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_CHARACTER);
	}

	float StartX = 0;
	float StartY = 20;
	float WindowW = 380;
	float WindowH = 320;



	g_pBCustomMenuInfo->gDrawWindowCustom(&StartX, &StartY, WindowW, WindowH, eWindowChotroi, gOther.TextVN_CHOTROI[10]);

	
	//=== List Item 
	float ListItemX = StartX+10;
	float ListItemY = StartY + 60;
	float ListItemWH = 65;
	float ListItemCachX = 205;
	float ListItemCachY = 15;
	int countx = 0;
	int county = 0;

	int DataListItem = gCBChotroi.m_DataChoTroi.size();
	//int DataListItem = 6;

	////===ScrollBar
	if (WindowChoTroiScrollBar == NULL)
	{
		WindowChoTroiScrollBar = new CNewUIScrollBar();
		WindowChoTroiScrollBar->Create(StartX+ ListItemCachX, StartY+25, WindowH - 100);
		WindowChoTroiScrollBar->SetMaxPos(DataListItem > 3 ? (DataListItem / MaxPerPageChoTroi) : 0);
		WindowChoTroiScrollBar->SetPos(StartX + (ListItemCachX ), StartY + 65);
	}
	//
	else
	{
		if (UpdateMaxPosChoTroi)
		{
			WindowChoTroiScrollBar->SetMaxPos(DataListItem > 3 ? (DataListItem / MaxPerPageChoTroi) : 0);
			UpdateMaxPosChoTroi = false;
		}
		if (gInterface.Data[eWindowChotroi].OnClick)
		{
			WindowChoTroiScrollBar->SetPos(StartX + (ListItemCachX), StartY + 65);
			WindowChoTroiScrollBar->SetCurPos(0);
			
		}
		WindowChoTroiScrollBar->MouseWheelWindow = SEASON3B::CheckMouseIn(ListItemX - 5, StartY + 37, WindowW - 185, WindowH - 100);

		WindowChoTroiScrollBar->Render();
		WindowChoTroiScrollBar->UpdateMouseEvent();
		WindowChoTroiScrollBar->Update();
	}

	//==List Item
	ShowInfoItem = -1;
	mPageChoTroi = WindowChoTroiScrollBar->GetCurPos();
	int CountList = 0;
	for (int n = (mPageChoTroi * MaxPerPageChoTroi); n < gCBChotroi.m_DataChoTroi.size(); n++)
	{
		float KhungInfoX = ListItemX + ((ListItemWH + ListItemCachX) * countx);
		float KhungInfoY = ListItemY + ((ListItemWH + ListItemCachY) * county);
		DWORD BGList = 0x00000096;
		//int GetInfoLoadItem = BPConvertByteItem(*(DWORD*)(GetInstance() + 36), gInterface.m_DataChoTroi[n].Item);
		ITEM* CTItem = g_pNewItemMng->CreateItem(gCBChotroi.m_DataChoTroi[n].Item);
		if (!CTItem) continue;
		int CTItemIndex = CTItem->Type;
		
		BYTE m_Skill = (gCBChotroi.m_DataChoTroi[n].Item[1] / 128) & 1;

		BYTE m_Luck = (gCBChotroi.m_DataChoTroi[n].Item[1] / 4) & 1;

		BYTE m_Opt = (gCBChotroi.m_DataChoTroi[n].Item[1] & 3) + ((gCBChotroi.m_DataChoTroi[n].Item[7] & 64) / 16);
		//==SHow Info Item

		if (SEASON3B::CheckMouseIn(KhungInfoX, KhungInfoY, ListItemWH, ListItemWH) && !SelectBarChoTroi)
		{
			ShowInfoItem = n;
			BGList = 0x66646496;
			if (GetTickCount() - gInterface.Data[eTIME].EventTick > 500) //Click
			{
				if (GetKeyState(VK_LBUTTON) & 0x8000)
				{
					gInterface.Data[eTIME].EventTick = GetTickCount();
					//if (m_TimTuKhoaChoTroi)m_TimTuKhoaChoTroi_Clear->GiveFocus(1);
				}
			}
		}
		else {
			BGList = 0x00000096;
		}
		std::map<int, char*>::iterator it = gCBChotroi.mListItemFind.find(CTItemIndex);
		//=== Lọc KQ Tìm Kiếm
		if (it == gCBChotroi.mListItemFind.end() && CacheSizeInputTimKiem != -1)
		{
			continue;
		}
		//--Loc Coin
		if (LocItemTypCoin != 63)
		{
			if (!(LocItemTypCoin & 1) && gCBChotroi.m_DataChoTroi[n].PriceType == 1)
			{
				continue;
			}
			else if (!(LocItemTypCoin & 2) && gCBChotroi.m_DataChoTroi[n].PriceType == 2)
			{
				continue;
			}
			else if (!(LocItemTypCoin & 4) && gCBChotroi.m_DataChoTroi[n].PriceType == 3)
			{
				continue;
			}
			if (!(LocItemTypCoin & 8) && gCBChotroi.m_DataChoTroi[n].PriceType == 4)
			{
				continue;
			}
			else if (!(LocItemTypCoin & 16) && gCBChotroi.m_DataChoTroi[n].PriceType == 5)
			{
				continue;
			}
			else if (!(LocItemTypCoin & 32) && gCBChotroi.m_DataChoTroi[n].PriceType == 6)
			{
				continue;
			}
		}


		//=== Loc Opt Item
		if (LocItemCoSkill && !m_Skill)
		{
			continue;
		}
		if (LocItemCoLuck && !m_Luck)
		{
			continue;
		}
		//g_Console.AddMessage(1, "%x", m_Opt);
		if (LocItemCoOpt && m_Opt < 5)
		{
			continue;
		}
		if (LocItemCoExc && !CTItem->Option1)
		{
			continue;
		}
		//==========

		gInterface.DrawBarForm(KhungInfoX + ListItemWH + 10, KhungInfoY, (ListItemWH + 50), ListItemWH + 9, 0.0, 0.0, 0.0, 0.8);
		g_pBCustomMenuInfo->DrawInfoBox(KhungInfoX, KhungInfoY, ListItemWH, ListItemWH, BGList, 0);
		g_pNewUISystem->RenderItem3DFree(KhungInfoX , KhungInfoY -30, ListItemWH, ListItemWH, CTItem->Type, CTItem->Level, CTItem->Option1, CTItem->ExtOption, 0, 1.4);//BMD MOdel
		//RenderItemInfo(350, 100, CTItem, 0, 0, false, true);
		//============
			//==Draw Time
		if (gCBChotroi.m_DataChoTroi[n].TimeItemRaoBan > 86400)
		{
			TextDraw((HFONT)g_hFontMini, KhungInfoX + 5, KhungInfoY + 58, 0xFFE50096, 0x0, ListItemWH, 0, 3, gOther.TextVN_CHOTROI[41], ((gCBChotroi.m_DataChoTroi[n].TimeItemRaoBan) / 86400));//
		}
		else
		{
			TextDraw((HFONT)g_hFontMini, KhungInfoX + 5, KhungInfoY + 58, 0xFFE50096, 0x0, ListItemWH, 0, 3, gOther.TextVN_CHOTROI[42], ((gCBChotroi.m_DataChoTroi[n].TimeItemRaoBan) / 3600));//
		}

		if (gCBChotroi.m_DataChoTroi[n].Pass != 0)
		{
			TextDraw((HFONT)g_hFontMini, KhungInfoX + 5, KhungInfoY + 8, 0xFFFFFFFF, 0x3091FF96, ListItemWH, 0, 3, "Pass");//
		}

		//===Info Item
		TextDraw((HFONT)g_hFontBold, KhungInfoX + ListItemWH + 10, KhungInfoY, 0xFFFFFFFF, 0x3091FF96, (ListItemWH + 49), 0, 3, BGetItemName(CTItemIndex, CTItem->Level));// Name Item
		TextDraw((HFONT)g_hFont, KhungInfoX + ListItemWH + 10, KhungInfoY + 12, 0xFF833096, 0x0, (ListItemWH + 49), 0, 3, gOther.TextVN_CHOTROI[11], gCBChotroi.m_DataChoTroi[n].Name);// Name Seller
		TextDraw((HFONT)g_hFontBold, KhungInfoX + ListItemWH + 10, KhungInfoY + 12 * 2, 0xFFDF2B96, 0x0, (ListItemWH + 49), 0, 3, gOther.TextVN_CHOTROI[12], gInterface.NumberFormat(gCBChotroi.m_DataChoTroi[n].Price), NameGiaCoin[gCBChotroi.m_DataChoTroi[n].PriceType]);// Gia Ban
		TextDraw((HFONT)g_hFont, KhungInfoX + ListItemWH + 10, KhungInfoY + 12 * 3, 0xFAFAFA96, 0x0, (ListItemWH + 49), 0, 3, gOther.TextVN_CHOTROI[13], SelectBarListChoTroi[gCBChotroi.m_DataChoTroi[n].TypeItem == 255 ? 11 : gCBChotroi.m_DataChoTroi[n].TypeItem]);// Slot Type
		
		

		if (SelectBarChoTroiNumber == 12)
		{
			//===Draw Button Thu Hoi
			if (g_pBCustomMenuInfo->DrawButton(KhungInfoX + ListItemWH + 10, KhungInfoY + (12 * 3) + 15, 100, 12, gOther.TextVN_CHOTROI[14], 115) && !SelectBarChoTroi && !gInterface.Data[eWindowNhapPass].OnShow)
			{
				//pDrawMessage("Thu Hoi Item", 1);
				CBChotroi::PMSG_REQ_MARKET_BUY pMsg;
				pMsg.h.set(0xD3, 0x21, sizeof(pMsg));
				pMsg.Result = 2;
				pMsg.ID = gCBChotroi.m_DataChoTroi[n].ID;
				DataSend((LPBYTE)& pMsg, pMsg.h.size);
			}
		}
		else
		{
			float ButtonWBuy = 55;
			const ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[CTItem->Type];
			if (pItemAttr->RequireClass[gCharacterManager.GetBaseClass(Hero->Class)] &&
				gCBChotroi.m_DataChoTroi[n].TypeItem >= 1 &&
				gCBChotroi.m_DataChoTroi[n].TypeItem <= 8)
			{
				//===Draw Button Thu Do
				if (g_pBCustomMenuInfo->DrawButton(KhungInfoX + ListItemWH + 10 + 58, KhungInfoY + (12 * 3) + 15, 100, 12, gOther.TextVN_CHOTROI[15], 55) && !SelectBarChoTroi && !gInterface.Data[eWindowNhapPass].OnShow)
				{
					m_PhotoViewChotroi.UpdateItemChar(pItemAttr->m_byItemSlot, CTItem->Type + 1171, CTItem->Level, CTItem->Option1, CTItem->ExtOption);
				}
			}
			else
			{
				ButtonWBuy = 115;
			}
			//===Draw Button Buy
			if (g_pBCustomMenuInfo->DrawButton(KhungInfoX + ListItemWH + 10, KhungInfoY + (12 * 3) + 15, 100, 12, gOther.TextVN_CHOTROI[16], ButtonWBuy) && !SelectBarChoTroi && !gInterface.Data[eWindowNhapPass].OnShow)
			{
				//PMSG_REQ_MARKET_BUY pMsg;
				//pMsg.h.set(0xD3, 0x21, sizeof(pMsg));
				//pMsg.Result = 1;
				//pMsg.ID = gInterface.m_DataChoTroi[n].ID;
				//DataSend((LPBYTE)& pMsg, pMsg.h.size);
				BuyID = gCBChotroi.m_DataChoTroi[n].ID;
				gInterface.Data[eWindowNhapPass].OnShow ^= 1;
				//if (hienpass == true)
				//{
				//	hienpass = false;
				//}
				//else
				//	hienpass = true;

			}
		}
		//===Break;
		CountList++;
		if (CountList >= MaxPerPageChoTroi) { break; }
		if (CountList % 1 == 0 && CountList != 0)
		{
			countx = 0;
			county++;
		}
		else
		{
			countx++;
		}
	}
	//==Select Form
	EnableAlphaTest(true);
	glColor3f(1.0, 1.0, 1.0);
	gCBChotroi.m_TabBtn.ChangeRadioButtonInfo(true, StartX + 230, StartY + 35, 56, 22);
	gCBChotroi.m_TabBtn.Render();
	int iNumCurOpenTab = gCBChotroi.m_TabBtn.UpdateMouseEvent();
	if (iNumCurOpenTab != RADIOGROUPEVENT_NONE) gCBChotroi.m_iNumCurOpenTab = iNumCurOpenTab;
	DrawInfoPhai(StartX + 215, StartY + 65);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	EnableAlphaTest(0);
	

	//===Window Passs
	float WindowWP = 200;
	float WindowHP = 120;

	float StartYP = ((MAX_WIN_HEIGHT - 151) / 2) - (WindowHP / 2) + 50;
	float StartXP = (MAX_WIN_WIDTH / 2) - (WindowWP / 2);

	if (gInterface.Data[eWindowNhapPass].OnShow)
	{
	
		g_pBCustomMenuInfo->gDrawWindowCustom(&StartXP, &StartYP, WindowWP, WindowHP, eWindowNhapPass, "Pass");

		RenderBitmap(CNewUIGuildMakeWindow::IMAGE_GUILDMAKE_EDITBOX, StartXP + (WindowWP / 2) - 70, StartYP + 50, 140, 20, 0.0, 0.0, 0.82, 0.71, 1, 1, 0.0);
		TextDraw((HFONT)g_hFontBold, StartXP + (WindowWP / 2) - 70, StartYP + 54, 0xFFFFFFC8, 0x0, 70, 0, 3, "Pass");

		float NTCoinX2 = StartXP + (WindowWP / 2);
		float NTCoinY2 = StartYP + 54;
		if (!NhapPassChoTroi)
		{
			NhapPassChoTroi = new CUITextInputBox;
			NhapPassChoTroi->Init(pGameWindow, NTCoinX2, NTCoinY2, 6);
			NhapPassChoTroi->SetBackColor(255, 0, 0, 0);
			NhapPassChoTroi->SetTextColor(255, 255, 157, 0);
			NhapPassChoTroi->SetFont((HFONT)g_hFont);
			NhapPassChoTroi->SetState(UISTATE_NORMAL);
			NhapPassChoTroi->SetOption(UIOPTION_NUMBERONLY);
			NhapPassChoTroi->SetPosition(NTCoinX2, NTCoinY2);
			NhapPassChoTroi->SetText("0");
		}
		else
		{

			NhapPassChoTroi->Render();

			NhapPassChoTroi->GetText(szNhapPassChoTroi, 5 + 1);
			NhapPassChoTroi->SetPosition(NTCoinX2, NTCoinY2);


			if (SEASON3B::CheckMouseIn(NTCoinX2 - 5, NTCoinY2 - 5, 100, 30) == 1)
			{

				if (GetKeyState(VK_LBUTTON) & 0x8000)
				{
					NhapPassChoTroi->GiveFocus(1);
					PlayBuffer(25, 0, 0);
				}
			}
		}

		if (g_pBCustomMenuInfo->DrawButton(StartXP + (WindowWP / 2) - 70, StartYP + 80, 100, 12, gOther.TextVN_CHOTROI[16], 55))
		{
			CBChotroi::PMSG_REQ_MARKET_BUY pMsg;
			pMsg.h.set(0xD3, 0x21, sizeof(pMsg));
			pMsg.Result = 1;
			pMsg.ID = BuyID;
			pMsg.Pass = atoi(szNhapPassChoTroi);
			memset(&szNhapPassChoTroi, 0, sizeof(szNhapPassChoTroi)); //HSD Them
			NhapPassChoTroi->SetText("0");
			//gInterface.DrawMessage(1, "%d", pMsg.Pass);
			DataSend((LPBYTE)& pMsg, pMsg.h.size);
			gInterface.Data[eWindowNhapPass].OnShow = 0;
		}

		if (g_pBCustomMenuInfo->DrawButton(StartXP + (WindowWP / 2) + 10, StartYP + 80, 100, 12, "Đóng", 55))
		{
			//hienpass = false;
			gInterface.Data[eWindowNhapPass].OnShow = 0;
			memset(&szNhapPassChoTroi, 0, sizeof(szNhapPassChoTroi)); //HSD Them
			NhapPassChoTroi->SetText("0");
		}
	}
	//==31422 Select Bar
	DWORD SelectBarHoverBG = 0x0;
	float SelectBarY = StartY;
	int SelectBarHover = 0;
	RenderBitmap(CNewUIGuildMakeWindow::IMAGE_GUILDMAKE_EDITBOX, ListItemX - 2, SelectBarY + 37, 110, 20, 0.0, 0.0, 0.82, 0.71, 1, 1, 0.0);
	if (SEASON3B::CheckMouseIn(ListItemX - 2, SelectBarY + 37, 110, 20)/* && !gInterface.Data[eWindowChotroiRaoBan].Hover*/)
	{

		SelectBarHover = 1;
		SelectBarHoverBG = 0x66646450;
		if (GetTickCount() - gInterface.Data[eTIME].EventTick > 500) //Click
		{
			if (GetKeyState(VK_LBUTTON) & 0x8000)
			{
				SelectBarHover = 2;
				gInterface.Data[eTIME].EventTick = GetTickCount();
				PlayBuffer(25, 0, 0);
				SelectBarChoTroi ^= 1;
			}
		}
	}
	RenderBitmap(CNewUICastleWindow::IMAGE_CASTLEWINDOW_SCROLL_DOWN_BTN, ListItemX + 2, SelectBarY + 39.5, 13.5, 13.5, 0.0, 0.21* SelectBarHover, 0.91, 0.21, 1, 1, 0.0);
	TextDraw((HFONT)g_hFontBold, ListItemX + 15, SelectBarY + 41, 0xFFFFFFFF, SelectBarHoverBG, 85, 0, 3, gOther.TextVN_CHOTROI[19], SelectBarListChoTroi[SelectBarChoTroiNumber]);// 
	if (SelectBarChoTroi)
	{
		int CountSelectBar = 1;
		gInterface.DrawBarForm(ListItemX + 15, SelectBarY + 41 + (15), 85, 15 * 12, 0.0, 0.0, 0.0, 0.8);
		for (int i = 0; i < 13; i++)
		{
			if (SelectBarChoTroiNumber == i) continue;
			DWORD SelectHover = 0x0;
			if (SEASON3B::CheckMouseIn(ListItemX + 15, SelectBarY + 41 + (15 * CountSelectBar), 130, 15))
			{
				SelectHover = 0x66646450;
				if (GetTickCount() - gInterface.Data[eTIME].EventTick > 500) //Click
				{
					if (GetKeyState(VK_LBUTTON) & 0x8000)
					{
						SelectBarChoTroiNumber = i;
						gInterface.Data[eTIME].EventTick = GetTickCount();
						PlayBuffer(25, 0, 0);
						SelectBarChoTroi ^= 1;

						CBChotroi::PMSG_REQ_MARKET_ITEM pMsg;
						pMsg.h.set(0xD3, 0x20, sizeof(pMsg));
						pMsg.Result = 1;
						pMsg.PriceType = -1;
						pMsg.PageNumber = 1;
						pMsg.GetTypeItem = i;

						DataSend((LPBYTE)& pMsg, pMsg.h.size);
						WindowChoTroiScrollBar->SetCurPos(0);
						UpdateMaxPosChoTroi = true;
					}
				}
			}
			TextDraw((HFONT)g_hFontBold, ListItemX + 15, SelectBarY + 41 + (15 * CountSelectBar), 0xFFFFFFFF, SelectHover, 85, 0, 3, "%s", SelectBarListChoTroi[i]);// 
			CountSelectBar++;
		}
	}

	//===Show Info
	if (ShowInfoItem != -1 /*&& !gInterface.Data[eWindowChotroiRaoBan].Hover*/)
	{
		//int BGetInfoLoadItem = BPConvertByteItem(*(DWORD*)(GetInstance() + 36), gInterface.m_DataChoTroi[ShowInfoItem].Item);
		//JCCoord B;
		//B.X = (int)pCursorX + 75;
		//B.Y = (int)pCursorY;
		//BPDrawInfoItem(*(DWORD*)(GetInstance() + 308), B, BGetInfoLoadItem, 0, 0, 0);
		ITEM* CTItem = g_pNewItemMng->CreateItem(gCBChotroi.m_DataChoTroi[ShowInfoItem].Item);
		RenderItemInfo(MouseX + 75, MouseY, CTItem, 0, 0, false, false);
	}
	ShowWindowChoOK = true;
}
bool CacheForm = false;
void CBChotroi::BDrawWindowChoTroi()
{
	if (!CacheForm)
	{
		std::list<unicode::t_string> ltext;
		ltext.push_back(gOther.TextVN_CHOTROI[45]); //MUA
		ltext.push_back(gOther.TextVN_CHOTROI[46]);//Ban

		gCBChotroi.m_TabBtn.CreateRadioGroup(2, CNewUIGuildInfoWindow::IMAGE_GUILDINFO_TAB_BUTTON, TRUE);
		gCBChotroi.m_TabBtn.ChangeRadioText(ltext);
		gCBChotroi.m_TabBtn.ChangeRadioButtonInfo(true, 0, 0, 76, 22);
		gCBChotroi.m_TabBtn.ChangeFrame(gCBChotroi.m_iNumCurOpenTab);

		m_PhotoViewChotroi.Init(0);
		m_PhotoViewChotroi.SetSize(150, 150);
		m_PhotoViewChotroi.CopyPlayer();
		m_PhotoViewChotroi.SetAutoupdatePlayer(TRUE);
		m_PhotoViewChotroi.SetAnimation(AT_STAND1);
		m_PhotoViewChotroi.SetAngle(120);
		m_PhotoViewChotroi.SetZoom(0.8);
		m_PhotoViewChotroi.SetPosition(0, 0);
		m_PhotoViewChotroi.SetOption(UIPHOTOVIEWER_CANCONTROL);
		CacheForm = true;
	}

	DrawWindowChoTroi();

}

void CBChotroi::GetOpenChoTroiWinDow()
{
	if (GetTickCount() - gInterface.Data[eWindowChotroi].EventTick < 500)
	{
		return;
	}
	gInterface.Data[eWindowChotroi].EventTick = GetTickCount();

	if (gInterface.Data[eWindowChotroi].OnShow)
	{
		gInterface.Data[eWindowChotroi].OnShow = false;
		return;
	}
	else
	{
		XULY_CGPACKET pMsg;
		pMsg.header.set(0xD3, 0x10, sizeof(pMsg));
		pMsg.ThaoTac = 2; //
		DataSend((LPBYTE)& pMsg, pMsg.header.size);
	}
}
bool CBChotroi::SendItemRaoBan(ITEM* ItemSell, int Slot, bool KeyClick)
{
	if (KeyClick && gInterface.Data[eWindowChotroi].OnShow
		&& m_iNumCurOpenTab == 1)// Send Item Rao Ban
	{
		if (ItemSell->Type < 0) { return 0; }
		if (gCBChotroi.ItemCacheSelect != NULL && gCBChotroi.ItemCacheShow || (GetTickCount() - gCBChotroi.ItemCacheTime) < 300)
		{
			//gInterface.DrawMessage(1, "Item Khong Hop Le" );
			return 0;
		}
		if (!gCBChotroi.ItemCacheShow)
		{
			gCBChotroi.ItemCacheSelect = NULL;
		}
		//memcpy(&gCBChotroi.ItemCacheSelect,item)
		gCBChotroi.ItemCacheSelect = g_pNewItemMng->CreateItem(ItemSell);;
		gCBChotroi.ItemCacheTime = GetTickCount();

		PMSG_ITEM_MOVE_RECV pMsg = { 0 };

		pMsg.h.set(0xD3, 0x13, sizeof(pMsg));
		pMsg.Target = -1;
		pMsg.sFlag = 0;
		pMsg.tFlag = 0;
		pMsg.Target = 0;
		pMsg.Source = Slot;
		::PlayBuffer(SOUND_GET_ITEM01);
		DataSend((BYTE*)& pMsg, pMsg.h.size);
		return 1;
	}

	return 0;
}
void CBChotroi::SetShowItemCache(BYTE* Recv)
{
	XULY_CGPACKET* lpMsg = (XULY_CGPACKET*)Recv;
	if (lpMsg->ThaoTac == 1)
	{
		gCBChotroi.ItemCacheShow = 1;
	}
	else {
		gCBChotroi.ItemCacheShow = 0;
		gCBChotroi.ItemCacheSelect = NULL;
		gCBChotroi.ItemCacheTime = 0;
	}
	return;
}
void CBChotroi::GCSetListChoTroi(BYTE* Recv)
{
	BCSEV_COUNTLIST* lpMsg = (BCSEV_COUNTLIST*)Recv;

	gCBChotroi.OnCointType = 0;
	gCBChotroi.m_DataChoTroi.clear();

	gCBChotroi.OnCointType = lpMsg->Type;

	for (int n = 0; n < lpMsg->Count; n++)
	{
		MARKET_DATA* lpInfo = (MARKET_DATA*)(((BYTE*)lpMsg) + sizeof(BCSEV_COUNTLIST) + (sizeof(MARKET_DATA) * n));
		//gInterface.DrawMessage(1, "GCSetListChoTroi %d %s %x %d", lpInfo->ID, lpInfo->Name, lpInfo->Item, lpInfo->Pass);
		//g_Console.AddMessage(1, "GCSetListChoTroi %d %s %x", lpInfo->ID, lpInfo->Name, lpInfo->Item);
		gCBChotroi.m_DataChoTroi.push_back(*lpInfo);
	}

	if (!gInterface.Data[eWindowChotroi].OnShow)
	{
		gInterface.Data[eWindowChotroi].OnShow = 1;
	}
}
#endif
