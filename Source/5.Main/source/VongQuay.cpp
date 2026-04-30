

#include "stdafx.h"
#include "VongQuay.h"
#include "Util.h"
#include "Interface.h"
#include "CBInterface.h"
#include "Other.h"
#include "CBNewUiEx.h"
#include <cmath>
#include "NewUISystem.h"

CVongQuay gVongQuay;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CVongQuay::CVongQuay()
{
	this->Init();
}


CVongQuay::~CVongQuay()
{
}

void CVongQuay::Init()
{
	this->ListItemVongQuay.clear();
	this->DanhSachVongQuay.clear();
}


void CVongQuay::OpenVongQuay()
{
	if ((GetTickCount() - gInterface.Data[eWindowVongQuay].EventTick) > 300)
	{
		gInterface.Data[eWindowVongQuay].EventTick = GetTickCount();

		if (gInterface.Data[eWindowVongQuay].OnShow)
		{
			gInterface.Data[eWindowVongQuay].OnShow = 0;

			return;
		}

		XULY_CGPACKET pMsg;
		pMsg.header.set(0xD3, 0x8C, sizeof(pMsg));
		pMsg.ThaoTac = 1;
		DataSend((LPBYTE)&pMsg, pMsg.header.size);

		pMsg.header.set(0xD3, 0x8B, sizeof(pMsg));
		pMsg.ThaoTac = 1; //
		DataSend((LPBYTE)&pMsg, pMsg.header.size);
		
	}
}

CNewUIScrollBar* ListVongQuay = nullptr;

int SelectTypeVQ = 1;
int Chay = -1;
float CountVong = 0.5;
bool KickHoatQuay = false;
CUITextInputBox* InputSoVong = NULL;
char GetSoVong[5];
int PageQuay = 0;
void quay10lan()
{
	XULY_CGPACKET pMsg;
	pMsg.header.set(0xD3, 0x8A, sizeof(pMsg));
	pMsg.ThaoTac = SelectTypeVQ; //
	//gInterface.DrawMessage(1, "%d", pMsg.ThaoTac);
	DataSend((LPBYTE)&pMsg, pMsg.header.size);
	gInterface.Data[eTickCount].EventTick = GetTickCount();
}
bool UpdateMaxPosSVQ= false;

void CVongQuay::DrawWindowVQ()
{
	if (gInterface.CheckWindow(Interface::ObjWindow::MoveList) || gInterface.CheckWindow(Interface::ObjWindow::CashShop) || gInterface.CheckWindow(Interface::ObjWindow::SkillTree) || gInterface.CheckWindow(Interface::ObjWindow::FullMap)
		|| (gInterface.CheckWindow(Interface::Inventory)
			&& gInterface.CheckWindow(Interface::ExpandInventory)
			&& gInterface.CheckWindow(Interface::Store))
		|| (gInterface.CheckWindow(Interface::Inventory)
			&& gInterface.CheckWindow(Interface::Warehouse)
			&& gInterface.CheckWindow(Interface::ExpandWarehouse))
		|| gInterface.CheckWindow(Interface::ChaosBox)
		)
	{
		gInterface.Data[eWindowVongQuay].OnShow = 0;

		return;
	}
	if (!gInterface.Data[eWindowVongQuay].OnShow)
	{
		if (ListVongQuay) ListVongQuay = nullptr;
		if (SelectTypeVQ != 1) SelectTypeVQ = 1;
		gVongQuay.StartRollSau = -1;
		gVongQuay.IndexItemSau = -1;
		this->ListItemVongQuay.clear();
	
		if (!InputSoVong)
		{
			InputSoVong = nullptr;
		}

		return;
	}
	int MaxListVQInPage = 5;
	int MaxListItemVQInPage = 9;
	float WindowW = 380;
	float WindowH = 310;
	gInterface.Data[eWindowVongQuay].Width = WindowW;
	float StartX = (MAX_WIN_WIDTH / 2) - (WindowW / 2);
	float StartY = ((MAX_WIN_HEIGHT - 51) / 2) - (WindowH / 2);
	if (g_pBCustomMenuInfo->gDrawWindowCustom( &StartX, &StartY, WindowW, WindowH, eWindowVongQuay, "Vòng Quay New"))
	{

		EnableAlphaBlend();
		glColor3f(1.0, 1.0, 1.0);
		//===Info Yeu Cau Moc Nap
		float InfoMocNapX = (StartX + 10) + 3;
		float InfoMocNapY = (StartY + 15);
		float TyleInfoYeuCau = 7.5f;
		float WInfo = (WindowW - 20) / 10;
		float WProcess = (WInfo * (TyleInfoYeuCau - 2.7));
		float WButton = 88;
		float HInfo = WindowH - 160;

		g_pBCustomMenuInfo->DrawInfoBox((StartX + WindowW) - 110, InfoMocNapY + 15, 85, 125, 0x00000096, 0, 0);

		g_pBCustomMenuInfo->DrawInfoBox((StartX + WindowW) - 110, InfoMocNapY + 160, 85, 105, 0x00000096, 0, 0);


		//Scroll Bar

		int DataListVQ = gVongQuay.DanhSachVongQuay.size();
		

		//if (ListVongQuay == NULL)
		//{
		//	ListVongQuay = new CNewUIScrollBar();
		//	ListVongQuay->Create((StartX + WindowW) - 15, InfoMocNapY + 10, HInfo);
		//	//ListVongQuay->SetMaxPos(DataListVQ > 3 ? (DataListVQ / MaxListVQInPage) : 0);
		//	ListVongQuay->SetPos((StartX + WindowW) - 15, InfoMocNapY + 10);
		//
		//}
		//else
		//{
		//	if (UpdateMaxPosSVQ)
		//	{
		//		ListVongQuay->SetMaxPos(DataListVQ > 3 ? (DataListVQ / MaxListVQInPage) : 0);
		//		UpdateMaxPosSVQ = false;
		//	}
		//	if (gInterface.Data[eWindowVongQuay].OnClick)
		//	{
		//		ListVongQuay->SetPos((StartX + WindowW) - 15, InfoMocNapY + 10);
		//		ListVongQuay->SetCurPos(0);
		//	}
		//	ListVongQuay->MouseWheelWindow = SEASON3B::CheckMouseIn(InfoMocNapX, InfoMocNapY, WindowW, WindowH);
		//	ListVongQuay->Render();
		//	ListVongQuay->UpdateMouseEvent();
		//	ListVongQuay->Update();
		//}
		int sotrang = DataListVQ / MaxListVQInPage;

		if (DataListVQ % MaxListVQInPage > 0)
		{
			sotrang += 1;
		}
		if (SEASON3B::CheckMouseIn(InfoMocNapX, InfoMocNapY, WindowW, WindowH))
		{
			if (MouseWheel != 0)
			{
				if ((MouseWheel < 0))
					if (PageQuay < sotrang - 1)
					{
						PageQuay++;
					}
				if ((MouseWheel > 0))
					if (PageQuay > 0)
					{
						PageQuay--;
					};
				MouseWheel = 0;
			}
		}

		float KhoangCachYMocNap = 24;
		//int MixItemListPage = ListVongQuay->GetCurPos();
		int MaxList = 0;

		for (int n = (PageQuay * MaxListVQInPage); n < DataListVQ; n++)
		{
			//==Xem
			if (g_pBCustomMenuInfo->DrawButton((StartX + WindowW) - 105, InfoMocNapY + 20, 110, 11, gVongQuay.DanhSachVongQuay[n].Name, WButton) && (GetTickCount() - gInterface.Data[eWindowVongQuay].EventTick) > 300 && gVongQuay.StartRollSau < 1) //"Xem"
			{

				SelectTypeVQ = gVongQuay.DanhSachVongQuay[n].IndexVongQuay;
				Chay = -1;
				gInterface.Data[eWindowVongQuay].OnShow = true;
				gInterface.Data[eWindowVongQuay].EventTick = GetTickCount();
				XULY_CGPACKET pMsg;
				pMsg.header.set(0xD3, 0x8B, sizeof(pMsg));
				pMsg.ThaoTac = SelectTypeVQ; //
				DataSend((LPBYTE)&pMsg, pMsg.header.size);
				gVongQuay.IndexItemSau = -1;
			}
			
			InfoMocNapY = InfoMocNapY + (KhoangCachYMocNap);
			MaxList++;
		
			if (MaxList >= MaxListVQInPage) break;
		}

		if ((GetTickCount() - gInterface.Data[eTickCount].EventTick) > 6000)
		{

			if (g_pBCustomMenuInfo->DrawButton(StartX + 115, StartY + 200, 100, 11, "Quay", 60) && (GetTickCount() - gInterface.Data[eTickCount].EventTick) > 6000) //"Nhận"
			{
				XULY_CGPACKET_SOLAN pMsg;
				pMsg.header.set(0xD3, 0x8A, sizeof(pMsg));
				pMsg.ThaoTac = SelectTypeVQ; //
				pMsg.SoLan = atoi(GetSoVong);
				//gInterface.DrawMessage(1, "%d", pMsg.ThaoTac);
				DataSend((LPBYTE)&pMsg, pMsg.header.size);
				KickHoatQuay = true;
				//InputSoVong->SetText("1");
				gInterface.Data[eTickCount].EventTick = GetTickCount();
			}
		}


		float NTCoinX = (StartX + WindowW) - 105;
		float NTCoinY = StartY + 185 + 90;
		float NTW = 70;
		gInterface.DrawBarForm(NTCoinX, NTCoinY - 5, 70, 15, 0.0, 0.0, 0.0, 0.5);
		if (!InputSoVong)
		{
			InputSoVong = new CUITextInputBox;
			InputSoVong->Init(pGameWindow, NTCoinX, NTCoinY - 2, 2);
			InputSoVong->SetBackColor(0, 0, 0, 0);
			InputSoVong->SetTextColor(255, 255, 157, 0);
			InputSoVong->SetFont(g_hFont);
			InputSoVong->SetState(UISTATE_NORMAL);
			InputSoVong->SetOption(UIOPTION_NUMBERONLY);
			InputSoVong->SetPosition(NTCoinX, NTCoinY - 2);
			InputSoVong->SetText("1");
		}
		else
		{
			InputSoVong->SetPosition(NTCoinX, NTCoinY - 2);
			InputSoVong->Render();
			InputSoVong->GetText(GetSoVong, sizeof(GetSoVong));

			if (SEASON3B::CheckMouseIn(NTCoinX - 3, NTCoinY - 4.5, NTW, 20) == 1)
			{

				if (GetKeyState(VK_LBUTTON) & 0x8000)
				{
					InputSoVong->GiveFocus(1);
					PlayBuffer(25, 0, 0);
				}
			}
		}
		int solan = atoi(GetSoVong);
		static int V10 = 0;
		if (KickHoatQuay == true)
		{

			for (int x = 0; x < solan - 1; x++)
			{
				if ((GetTickCount() - gInterface.Data[eTickCount].EventTick) > 500)
				{
					gInterface.Data[eTickCount].EventTick = GetTickCount();
					quay10lan();
					V10++;
				}

			}
			if (V10 == solan - 1)
			{
				KickHoatQuay = false;
			}
			//gInterface.DrawMessage(1, "%d ~[%d] ~%d", V10, solan,KickHoatQuay);
		}
		else
		{
			V10 = 0;
		}

		float PosYCoinNhan = StartY + 185;
		TextDraw(g_hFont, (StartX + WindowW) - 105, PosYCoinNhan - 5, 0xFFDE26FF, 0x3a4b3978, 90, 0, 3, "Yêu Cầu"); //);
		TextDraw(g_hFont, (StartX + WindowW) - 105, PosYCoinNhan + (10 * 1), 0xFF8214FF, 0x0, WindowW, 0, 1, "+WCoin : %s", gInterface.NumberFormat(gVongQuay.WCYC));			//Text3 = "+ WCoin : %s
		TextDraw(g_hFont, (StartX + WindowW) - 105, PosYCoinNhan + (10 * 2), 0xFF8214FF, 0x0, WindowW, 0, 1, "+WCoinP :%s", gInterface.NumberFormat(gVongQuay.WPYC));		  //Text4 = "+ WCoinP : %
		TextDraw(g_hFont, (StartX + WindowW) - 105, PosYCoinNhan + (10 * 3), 0xFF8214FF, 0x0, WindowW, 0, 1, "+GobinP :%s", gInterface.NumberFormat(gVongQuay.GPYC));		  //Text5 = "+ GobinP : %
		if (gVongQuay.CountItem > 0)
		{
			TextDraw(g_hFont, (StartX + WindowW) - 105, PosYCoinNhan + (10 * 4), 0xFF8214FF, 0x0, WindowW, 0, 1, "+Cần : %s", gInterface.NumberFormat(gVongQuay.CountItem));		  //Text6 = " + Ruud : % s"
			TextDraw(g_hFont, (StartX + WindowW) - 105, PosYCoinNhan + (10 * 5) + 5, 0x26FFD7A4, 0x0, WindowW, 0, 1, "%s", BGetItemName(gVongQuay.IndexYC, 0));//
		}
		TextDraw(g_hFont, (StartX + WindowW) - 105, PosYCoinNhan + (10 * 7), 0xFF8214FF, 0x0, WindowW, 0, 1, "Nhập Số Vòng");		  //Text5 = "+ GobinP : %


		int DataListItem = gVongQuay.ListItemVongQuay.size();
		float Radius = 100.0f; // Set the radius of the circular layout
		float AngleStep = 2 * 3.14159265358979323846 / 12.0f; // Divide the circle into 12 equal angles
		float WBox = 30;
		int BBShowInfoItem = -1;
		DWORD boxColor = 0x00000096;
		unsigned int timeInterval2 = 10; // 5 seconds
		unsigned int currentTime2 = GetTickCount();
		unsigned int colorIndex2 = (currentTime2 / timeInterval2) % 2;



		if (gVongQuay.StartRollSau >= 1)
		{
			if (Chay >= 12)
			{
				Chay = 0;
				CountVong += 0.1;

			}

			if (colorIndex2 == 0 && CountVong <= 1.3)
			{
				Chay += CountVong;
			}
			if (CountVong >= 1.3)
			{
				Chay += CountVong;
			}

		}
		else
		{
			CountVong = 1;
		}
		for (int n = 0; n < DataListItem; n++)
		{
			const ITEM_ATTRIBUTE* is = &ItemAttribute[gVongQuay.ListItemVongQuay[n].Index];
		
			int size = max(is->Width, is->Height);

			float itemScale = 1.0;
			float addY = 0;
			switch (size)
			{
			case 4:
				addY = -5;
				break;
			case 3:
				addY = -3;
				break;
			case 2:
				addY = -1;
				break;
			}

			// Calculate polar coordinates for circular layout
			float angle = n * AngleStep;
			float PosXBoxItem = StartX + 125 + Radius * cos(angle);
			float PosYBoxItem = StartY + 140 + Radius * sin(angle);

			if (n == Chay)  // Replace yourNewVariable with the variable you want to compare with
			{
				boxColor = 0xD4966396;
			}
			else
			{
				boxColor = 0x00000096;
			}
			if (solan != 1 && KickHoatQuay == true)
			{
				int roll = rand() % 2;
				switch (roll)
				{
				case 0:
					g_pBCustomMenuInfo->DrawInfoBox(PosXBoxItem, PosYBoxItem, WBox, WBox, 0x00000096, 0, 0);
					break;
				case 1:
					g_pBCustomMenuInfo->DrawInfoBox(PosXBoxItem, PosYBoxItem, WBox, WBox, 0xD4966396, 0, 0);
					break;
				case 2:
					g_pBCustomMenuInfo->DrawInfoBox(PosXBoxItem, PosYBoxItem, WBox, WBox, 0x26E0A596, 0, 0);
					break;
				}
			}


			g_pBCustomMenuInfo->DrawInfoBox(PosXBoxItem, PosYBoxItem, WBox, WBox, boxColor, 0, 0);
			g_pNewUISystem->RenderItem3DFree(PosXBoxItem + gVongQuay.ListItemVongQuay[n].PosX, PosYBoxItem + gVongQuay.ListItemVongQuay[n].PosY, WBox, WBox, gVongQuay.ListItemVongQuay[n].Index, gVongQuay.ListItemVongQuay[n].Item->Level, gVongQuay.ListItemVongQuay[n].Item->Option1, gVongQuay.ListItemVongQuay[n].Item->ExtOption, 0, 1.0 / gVongQuay.ListItemVongQuay[n].SizeBMD);

			if (gVongQuay.IndexItemSau != -1)
			{
				if (solan == 1)
				{
					Chay = gVongQuay.IndexItemSau;
				}
				else
				{
					Chay = -1;
				}
				g_pBCustomMenuInfo->DrawInfoBox(InfoMocNapX + 110, StartY + 150, WBox, WBox, 0xD4966396, 0, 0);
				g_pNewUISystem->RenderItem3DFree(InfoMocNapX + 115, StartY + 125, WBox, WBox, gVongQuay.ListItemVongQuay[gVongQuay.IndexItemSau].Index, gVongQuay.ListItemVongQuay[gVongQuay.IndexItemSau].Item->Level, gVongQuay.ListItemVongQuay[gVongQuay.IndexItemSau].Item->Option1, gVongQuay.ListItemVongQuay[gVongQuay.IndexItemSau].Item->ExtOption, 0, 1.0 / gVongQuay.ListItemVongQuay[gVongQuay.IndexItemSau].SizeBMD);
				if (SEASON3B::CheckMouseIn(InfoMocNapX + 110, StartY + 150, WBox, WBox))
				{
					BBShowInfoItem = gVongQuay.IndexItemSau;
				}
			}

			if (SEASON3B::CheckMouseIn(PosXBoxItem, PosYBoxItem, WBox, WBox))
			{
				BBShowInfoItem = n;
			}
		}

		//===Coin


		if (BBShowInfoItem != -1)
		{
			RenderItemInfo(MouseX + 75, MouseY, this->ListItemVongQuay[BBShowInfoItem].Item, 0, 0, false, false);
		}
		float CenterX = StartX + (WindowW / 3)+20;
		float CenterY = (StartY + WindowH) -5;
		const BYTE state[3] = { 0, 1, 2 };
		::EnableAlphaTest();
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		RenderBitmap(32344, CenterX - (60 / 2), CenterY - 22, 60.f, 22.f, 0, 0, 80.f / 128.f, 30.f / 34.f, 1, 1, 0.0);
		TextDraw(g_hFont, CenterX - (60 / 2), CenterY - 22 + 5, 0xffffffff, 0x0, 60, 0, 3, "%d / %d", PageQuay + 1, sotrang);

		DisableAlphaBlend();
	}
}

void CVongQuay::GetListVQ(BYTE* Recv)
{
	if (!Recv) return;

	gVongQuay.DanhSachVongQuay.clear();
	PMSG_VONGQUAY_SEND* mRecv = (PMSG_VONGQUAY_SEND*)Recv;

	for (int i = 0; i < mRecv->count; i++)
	{
		ListVongQuaySend lpInfo = *(ListVongQuaySend*)(((BYTE*)Recv) + sizeof(PMSG_VONGQUAY_SEND) + (sizeof(ListVongQuaySend) * i));
		gVongQuay.DanhSachVongQuay.push_back(lpInfo);
	}

}


void CVongQuay::RecvListItemVQ(BYTE* Recv)
{
	if (!Recv) return;

	gVongQuay.ListItemVongQuay.clear();

	PMSG_YCVONGQUAY_SEND* mRecv = (PMSG_YCVONGQUAY_SEND*)Recv;
	gVongQuay.IndexYC = mRecv->IndexYC;
	gVongQuay.CountItem = mRecv->CountItem;
	gVongQuay.WCYC = mRecv->WCYC;
	gVongQuay.WPYC = mRecv->WPYC;
	gVongQuay.GPYC = mRecv->GPYC;

	for (int i = 0; i < mRecv->count; i++)
	{
		LISTITEMVONGQUAY_SENDINFO lpInfo = *(LISTITEMVONGQUAY_SENDINFO*)(((BYTE*)Recv) + sizeof(PMSG_YCVONGQUAY_SEND) + (sizeof(LISTITEMVONGQUAY_SENDINFO) * i));
		//==SetINfoItem
		INFO_VONGQUAY_LOCAL_ITEM infoItemLocal = { 0 };
		infoItemLocal.SizeBMD = lpInfo.SizeBMD;
		infoItemLocal.PosX = lpInfo.PosX;
		infoItemLocal.PosY = lpInfo.PosY;
		infoItemLocal.Index = lpInfo.Index;
		infoItemLocal.Item = g_pNewItemMng->CreateItem(lpInfo.Item);
		infoItemLocal.Item->Durability = lpInfo.Dur;
		if (lpInfo.PeriodTime)
		{
			infoItemLocal.Item->bPeriodItem = 1;
			infoItemLocal.Item->lExpireTime = lpInfo.PeriodTime;
		}
		gVongQuay.ListItemVongQuay.push_back(infoItemLocal);

	}
	gInterface.Data[eWindowVongQuay].OnShow = 1;

}


void CVongQuay::GetInfoVQ(BYTE* Recv)
{
	if (!Recv) return;
	XULY_CGPACKET_VONGQUAY* mRecv = (XULY_CGPACKET_VONGQUAY*)Recv;
	//gInterface.DrawMessage(1, "%d ~ %d", mRecv->StartRoll, mRecv->IndexWin);
	gVongQuay.StartRollSau = mRecv->StartRoll;
	gVongQuay.IndexItemSau = mRecv->IndexWin;
}