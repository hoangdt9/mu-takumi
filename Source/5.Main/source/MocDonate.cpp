#include "StdAfx.h"
#include "MocDonate.h"
#include "NewUISystem.h"
#include "CBInterface.h"
#include "CUIController.h"
#include "CharacterManager.h"
#include "Util.h"
#include "Protocol.h"
#include "NewUIBase.h"
#include "Other.h"
#include "MapManager.h"
#include "ZzzInterface.h"

CNewUIScrollBar* ListMocNap = nullptr;
CNewUIScrollBar* ListItemNhan = nullptr;
int MaxListMocNapInPage = 5;
int MaxListItemInPage = 9;
int SelectTypeMocNap = -1;

MocDonate gMocDonate;

MocDonate::MocDonate()
{

}

MocDonate::~MocDonate()
{

}

void MocDonate::OpenWindowMocNap()
{
	if (GetTickCount() < gInterface.Data[eWindowMocNap].EventTick + 300)
	{
		return;
	}

	gInterface.Data[eWindowMocNap].EventTick = GetTickCount();
	if (!gInterface.Data[eWindowMocNap].OnShow)
	{
		XULY_CGPACKET pMsg;
		pMsg.header.set(0xD3, 0x9C, sizeof(pMsg));
		pMsg.ThaoTac = 1;
		DataSend((LPBYTE)&pMsg, pMsg.header.size);
	}
	gInterface.Data[eWindowMocNap].OnShow ^= 1;
}

void MocDonate::RecvListPhanThuong(BYTE* lpMsg)
{
	if (!lpMsg) return;

	mDataListItemMocNapClient.Clear();
	PMSG_CBLISTTHUONG_SEND* mRecv = (PMSG_CBLISTTHUONG_SEND*)lpMsg;
	mDataListItemMocNapClient.WC = mRecv->WC;
	mDataListItemMocNapClient.WP = mRecv->WP;
	mDataListItemMocNapClient.GP = mRecv->GP;
	mDataListItemMocNapClient.Ruud = mRecv->Ruud;
	for (int i = 0; i < mRecv->count; i++)
	{
		LISTITEMMOCNAP_SENDINFO lpInfo = *(LISTITEMMOCNAP_SENDINFO*)(((BYTE*)lpMsg) + sizeof(PMSG_CBLISTTHUONG_SEND) + (sizeof(LISTITEMMOCNAP_SENDINFO) * i));
		//==SetINfoItem
		INFO_LOCAL_ITEM infoItemLocal = { 0 };
		infoItemLocal.Count = lpInfo.Count;
		infoItemLocal.SizeBMD = lpInfo.SizeBMD;
		infoItemLocal.Index = lpInfo.Index;
		memcpy(infoItemLocal.Item, lpInfo.Item, sizeof(infoItemLocal.Item));
		mDataListItemMocNapClient.ListItemMocNap.push_back(infoItemLocal);

	}
}
void MocDonate::GetListDonate(BYTE* Recv)
{
	if (!Recv) return;

	mDataMocNapClient.Clear();
	PMSG_CBMONAP_SEND* mRecv = (PMSG_CBMONAP_SEND*)Recv;
	mDataMocNapClient.NhanMocNap = mRecv->NhanMocNap;
	mDataMocNapClient.TongNap = mRecv->TongNap;
	for (int i = 0; i < mRecv->count; i++)
	{
		ListMocNapSend lpInfo = *(ListMocNapSend*)(((BYTE*)Recv) + sizeof(PMSG_CBMONAP_SEND) + (sizeof(ListMocNapSend) * i));
		mDataMocNapClient.DanhSachMocNap.push_back(lpInfo);
	}
}

void MocDonate::DrawXemMocNap()
{
	if (!gInterface.Data[eWindowMocNapList].OnShow)
	{
		return;
	}

	float WindowW = 185;
	float WindowH = 270;

	float StartX = (MAX_WIN_WIDTH / 2) - (WindowW / 2);
	float StartY = 25.0;
	if (g_pBCustomMenuInfo->gDrawWindowCustom(&StartX, &StartY, WindowW, WindowH, eWindowMocNapList, "Mốc Nạp"))
	{

		StartX = StartX + 10;
		StartY = StartY - 5;
		float WInfo = (WindowW - 20);
		float HInfo = WindowH - 100;

		TextDraw((HFONT)g_hFont, StartX + 15, StartY + 35, 0xFF00EEDFF, 0x0, WindowW, 0, 1, "Danh Sách Item Nhận");

		//Scroll Bar
		int DataListItem = mDataListItemMocNapClient.ListItemMocNap.size();
		if (!ListItemNhan)
		{
			ListItemNhan = new CNewUIScrollBar();
			ListItemNhan->Create((StartX + WindowW) - 25, StartY + 35 + 15, 130);
			if (ListItemNhan)
			{
				ListItemNhan->SetMaxPos((DataListItem <= MaxListItemInPage) ? 0 : (DataListItem / MaxListItemInPage) - 1);
				ListItemNhan->SetPos((StartX + WindowW) - 25, StartY + 35 + 15);
			}
		}
		if (ListItemNhan)
		{
			if (gInterface.Data[eWindowMocNapList].OnClick)
			{
				ListItemNhan->SetPos((StartX + WindowW) - 25, StartY + 35 + 15);
				ListItemNhan->SetCurPos(0);
			}
			ListItemNhan->MouseWheelWindow = SEASON3B::CheckMouseIn(StartX + 15, StartY + 35 + 15, WindowW, 130);
			ListItemNhan->Render();
			ListItemNhan->UpdateMouseEvent();
			ListItemNhan->Update();
		}

		//==List Box Item 
		float PosXBoxItemGoc = StartX + 25;
		float PosXBoxItem = StartX + 25;
		float PosYBoxItem = StartY + 55;
		float WBox = 30;
		float KhoangCach = 45;
		int CountNgang = 0;
		int CountDoc = 0;
		int ItemListPage = ListItemNhan->GetCurPos();
		int BBShowInfoItem = -1;

		//
		int currentRow = 0;
		int currentCol = 0;

		for (int n = (ItemListPage * MaxListItemInPage); n < DataListItem; n++)
		{
			ITEM* pItem = NULL;
			pItem = g_pNewItemMng->CreateItem(mDataListItemMocNapClient.ListItemMocNap[n].Item);

			if (NULL != pItem)
			{
				const ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pItem->Type];

				int wmodel = pItemAttr->Width * 20;
				int hmodel = pItemAttr->Height * 20;
				int posx = StartX + (currentCol * KhoangCach) - (wmodel / 2) + 45;
				int posy = StartY + (currentRow * KhoangCach) - (hmodel / 2) + 45;

				g_pBCustomMenuInfo->DrawInfoBox(PosXBoxItem, PosYBoxItem, WBox, WBox, 0x00000096, 0, 0);
				g_pNewUISystem->RenderItem3DFree(posx, posy, wmodel, hmodel, pItem->Type, pItem->Level, pItem->Option1, pItem->ExtOption, 0, 1.0);

				TextDraw((HFONT)g_hFont, PosXBoxItem + 5, PosYBoxItem + 25, 0xE0FF14A5, 0x0, WBox, 0, 4, "x%d", mDataListItemMocNapClient.ListItemMocNap[n].Count);
				if (SEASON3B::CheckMouseIn(PosXBoxItem, PosYBoxItem, WBox, WBox))
				{
					BBShowInfoItem = n;
				}
				PosXBoxItem += KhoangCach;
				CountDoc++;
				CountNgang++;
				if (CountNgang >= 3)
				{
					PosXBoxItem = PosXBoxItemGoc;
					PosYBoxItem += KhoangCach;
					CountNgang = 0;
				}
				if (CountDoc >= MaxListItemInPage) break;

				++currentCol;
				if (currentCol >= 3)
				{
					currentCol = 0;
					++currentRow;
				}
			}
		}

		//===Coin
		float PosYCoinNhan = StartY + 185;
		TextDraw((HFONT)g_hFont, StartX + 15, PosYCoinNhan, 0xFF00EEDFF, 0x0, WindowW, 0, 1, "*Coin Nhận");
		PosYCoinNhan += 5;
		TextDraw((HFONT)g_hFont, StartX + 25, PosYCoinNhan + (10 * 1), 0xFF8214FF, 0x0, WindowW, 0, 1, "+WCoin : %s", gInterface.NumberFormat(mDataListItemMocNapClient.WC));			//Text3 = "+ WCoin : %s
		TextDraw((HFONT)g_hFont, StartX + 25, PosYCoinNhan + (10 * 2), 0xFF8214FF, 0x0, WindowW, 0, 1, "+WCoinP :%s", gInterface.NumberFormat(mDataListItemMocNapClient.WP));		  //Text4 = "+ WCoinP : %
		TextDraw((HFONT)g_hFont, StartX + 25, PosYCoinNhan + (10 * 3), 0xFF8214FF, 0x0, WindowW, 0, 1, "+GobinP :%s", gInterface.NumberFormat(mDataListItemMocNapClient.GP));		  //Text5 = "+ GobinP : %
		TextDraw((HFONT)g_hFont, StartX + 25, PosYCoinNhan + (10 * 4), 0xFF8214FF, 0x0, WindowW, 0, 1, "+Ruud : %s", gInterface.NumberFormat(mDataListItemMocNapClient.Ruud));		  //Text6 = " + Ruud : % s"

		TextDraw((HFONT)g_hFont, StartX, PosYCoinNhan + (10 * 5) + 5, 0x14FFC0FF, 0x0, WindowW, 0, 3, "*Phần thưởng bao gồm các Item \n và Coin Nhận !!");
		//===Show Info
		if (BBShowInfoItem != -1)
		{
			ITEM* CTItem = g_pNewItemMng->CreateItem(mDataListItemMocNapClient.ListItemMocNap[BBShowInfoItem].Item);
			RenderItemInfo(MouseX + 75, MouseY, CTItem, 0, 0, false, false);
		}
	}
}

void MocDonate::DrawWindow()
{
	if (gInterface.CheckWindow(Interface::ObjWindow::MoveList) || gInterface.CheckWindow(Interface::ObjWindow::CashShop) || gInterface.CheckWindow(Interface::ObjWindow::SkillTree) || gInterface.CheckWindow(Interface::ObjWindow::FullMap)
		|| (gInterface.CheckWindow(Interface::Inventory)
			&& gInterface.CheckWindow(Interface::ExpandInventory)
			&& gInterface.CheckWindow(Interface::Store))
		|| (gInterface.CheckWindow(Interface::Inventory)
			&& gInterface.CheckWindow(Interface::Warehouse)
			&& gInterface.CheckWindow(Interface::ExpandWarehouse)))
	{
		return;
	}

	if (!gInterface.Data[eWindowMocNap].OnShow)
	{
		if (ListMocNap) ListMocNap = nullptr;
		if (SelectTypeMocNap != -1) SelectTypeMocNap = -1;
		return;
	}

	float WindowW = 220;
	float WindowH = 270;
	gInterface.Data[eWindowMocNap].Width = WindowW;
	float StartX = (MAX_WIN_WIDTH / 2) - (WindowW / 2);
	float StartY = ((MAX_WIN_HEIGHT - 51) / 2) - (WindowH / 2);

#if MaNap
	if (g_pBCustomMenuInfo->gDrawWindowCustom(&StartX, &StartY, WindowW + 160, WindowH, eWindowMocNap, "Mốc Nạp"))
	{
		int TongSoDaNap = mDataMocNapClient.TongNap;
		int MocDaNhan = mDataMocNapClient.NhanMocNap;
		TextDraw((HFONT)g_hFont, StartX, StartY + 25, 0x00FFDDFF, 0x0, WindowW, 0, 3, "*Hệ thống nhận thưởng theo Mốc Nạp");
		TextDraw((HFONT)g_hFont, StartX, StartY + 35, 0xFF9100FF, 0x0, WindowW, 0, 3, "Quét Mã QR Bên Dưới Để Nạp Tiền Nhanh Nhất !!");
		TextDraw((HFONT)g_hFont, StartX, StartY + 50, 0x3CFF00FF, 0x0, WindowW, 0, 3, "[Bạn Đã Nạp : %s]", gInterface.NumberFormat(TongSoDaNap));

		//TextDraw((HFONT)g_hFont, StartX + 225, StartY + 30, 0xffffffff, 0x00FBFF69, 140, 0, 3, "★THÔNG TIN NHẬN DONATE SERVER★");
		//TextDraw((HFONT)g_hFont, StartX + 235, StartY + 45, 0xffffffff, 0xff222299, 120, 0, 3, "CTK : NGUYEN NGOC MINH");
		//TextDraw((HFONT)g_hFont, StartX + 235, StartY + 60, 0xffffffff, 0xff222299, 120, 0, 3, "STK : 17882701");
		//TextDraw((HFONT)g_hFont, StartX + 235, StartY + 75, 0xffffffff, 0xff222299, 120, 0, 3, "Ngân Hàng : ACB BANK");
		//RenderBitmap(IMG_4046, StartX + 225, StartY + 93, 180, 180, 0, 0, 1, 1, 1, 1, 0);
#else
	if (g_pBCustomMenuInfo->gDrawWindowCustom(&StartX, &StartY, WindowW, WindowH, eWindowMocNap, "Mốc Nạp"))
	{
		int TongSoDaNap = mDataMocNapClient.TongNap;
		int MocDaNhan = mDataMocNapClient.NhanMocNap;
		TextDraw((HFONT)g_hFont, StartX, StartY + 25, 0x00FFDDFF, 0x0, WindowW, 0, 3, "*Hệ thống nhận thưởng theo Mốc Nạp");
		TextDraw((HFONT)g_hFont, StartX, StartY + 35, 0xFF9100FF, 0x0, WindowW, 0, 3, "Quét Mã QR Bên Dưới Để Nạp Tiền Nhanh Nhất !!");
		TextDraw((HFONT)g_hFont, StartX, StartY + 50, 0x3CFF00FF, 0x0, WindowW, 0, 3, "[Bạn Đã Nạp : %s]", gInterface.NumberFormat(TongSoDaNap));
#endif
		//===Info Yeu Cau Moc Nap
		float InfoMocNapX = (StartX + 10) + 3;
		float InfoMocNapY = (StartY + 65);
		float TyleInfoYeuCau = 7.5f;
		float WInfo = (WindowW - 20) / 10;
		float WProcess = (WInfo * (TyleInfoYeuCau - 2.7));
		float WButton = 38;
		float HInfo = WindowH - 100;
		TextDraw((HFONT)g_hFont, InfoMocNapX, InfoMocNapY, 0xFFDE26FF, 0x3a4b3978, WInfo * TyleInfoYeuCau + 40, 0, 3, "Yêu Cầu"); //"Yêu Cầu");
		//Scroll Bar
		int DataListMocNap = mDataMocNapClient.DanhSachMocNap.size();
		if (!ListMocNap)
		{
			ListMocNap = new CNewUIScrollBar();
			ListMocNap->Create((StartX + WindowW) - 15, InfoMocNapY + 10, HInfo);
			if (ListMocNap)
			{
				ListMocNap->SetMaxPos((DataListMocNap <= MaxListMocNapInPage) ? 0 : (DataListMocNap / MaxListMocNapInPage) - 1);
				ListMocNap->SetPos((StartX + WindowW) - 15, InfoMocNapY + 10);
			}
		}
		if (ListMocNap)
		{
			if (gInterface.Data[eWindowMocNap].OnClick)
			{
				ListMocNap->SetPos((StartX + WindowW) - 15, InfoMocNapY + 10);
				ListMocNap->SetCurPos(0);
			}
			ListMocNap->MouseWheelWindow = SEASON3B::CheckMouseIn(InfoMocNapX, InfoMocNapY, WInfo * TyleInfoYeuCau, HInfo + 10);
			ListMocNap->Render();
			ListMocNap->UpdateMouseEvent();
			ListMocNap->Update();
		}

		float KhoangCachYMocNap = 34;
		int MixItemListPage = ListMocNap->GetCurPos();
		int MaxList = 0;
		for (int n = (MixItemListPage * MaxListMocNapInPage); n < DataListMocNap; n++)
		{
			//==List MocNap
			gInterface.DrawBarForm(InfoMocNapX, InfoMocNapY + 15, WInfo * TyleInfoYeuCau + 40, 30, 0.0, 0.0, 0.0, 0.5);
			TextDraw((HFONT)g_hFont, InfoMocNapX + 5, InfoMocNapY + 20, 0xFFDE26FF, 0x0, WProcess, 0, 3, "Mốc(%d) %s", mDataMocNapClient.DanhSachMocNap[n].IndexMocNap, gInterface.NumberFormat(mDataMocNapClient.DanhSachMocNap[n].GiaTriNap)); //"Mốc(%d) %s"
			//==Process
			gInterface.DrawBarForm(InfoMocNapX + 5, InfoMocNapY + 20 + 11, WProcess, 6, 0.29, 0.2767, 0.2581, 0.6); //Process BG
			//=Calc Tyle 
			float PhanTramMocNap = (TongSoDaNap) * 100 / (mDataMocNapClient.DanhSachMocNap[n].GiaTriNap); // Tinh Ty le da nap
			float TyLeTGA = (WProcess * PhanTramMocNap) / 100;
			if (TyLeTGA > WProcess) { TyLeTGA = WProcess; }
			gInterface.DrawBarForm(InfoMocNapX + 5, InfoMocNapY + 20 + 11, TyLeTGA, 6, 0.7833, 0.0, 1.0, 1.0); //Process
			//==Xem
			if (g_pBCustomMenuInfo->DrawButton(InfoMocNapX + 5 + WProcess + 3, InfoMocNapY + 20, 110, 11, "Xem", WButton) && (GetTickCount() - gInterface.Data[eWindowMocNapList].EventTick) > 300) //"Xem"
			{

				SelectTypeMocNap = mDataMocNapClient.DanhSachMocNap[n].IndexMocNap;
				gInterface.Data[eWindowMocNapList].OnShow = true;
				gInterface.Data[eWindowMocNapList].EventTick = GetTickCount();
				XULY_CGPACKET pMsg;
				pMsg.header.set(0xD3, 0x9B, sizeof(pMsg));
				pMsg.ThaoTac = SelectTypeMocNap; //
				DataSend((LPBYTE)&pMsg, pMsg.header.size);
			}
			if (MocDaNhan < mDataMocNapClient.DanhSachMocNap[n].IndexMocNap) //Kiem tra neu da nhan roi thi khong cho nhan nua
			{
				//==Nhận
				if( g_pBCustomMenuInfo->DrawButton(InfoMocNapX + 5 + WProcess + 3 + WButton + 3, InfoMocNapY + 20, 110, 11, "Nhận", WButton) && (GetTickCount() - gInterface.Data[eWindowMocNapList].EventTick) > 300) //"Nhận"
				{
					XULY_CGPACKET pMsg;
					pMsg.header.set(0xD3, 0x9A, sizeof(pMsg));
					pMsg.ThaoTac = mDataMocNapClient.DanhSachMocNap[n].IndexMocNap; //
					DataSend((LPBYTE)&pMsg, pMsg.header.size);

				}

			}
			InfoMocNapY = InfoMocNapY + (KhoangCachYMocNap);
			MaxList++;
			if (MaxList >= MaxListMocNapInPage) break;
		}

	}
	
}
