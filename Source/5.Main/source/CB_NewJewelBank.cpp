#include "StdAfx.h"
#include "CB_NewJewelBank.h"
#include "NewUISystem.h"
#include "CBInterface.h"
#include "CUIController.h"
#include "CharacterManager.h"
#include "Util.h"
#include "Protocol.h"
#include "NewUIBase.h"
#include "Other.h"
#include "ZzzInterface.h"

CB_NewJewelBank* gCB_NewJewelBank;

CB_NewJewelBank::CB_NewJewelBank()
{
	this->mCListItemBank.clear();
}


CB_NewJewelBank::~CB_NewJewelBank()
{
}

void CB_NewJewelBank::OpenOnOff()
{
	if (GetTickCount() - gInterface.Data[eWindowJewelBank].EventTick > 300)
	{
		gInterface.Data[eWindowJewelBank].EventTick = GetTickCount();
		if (gInterface.Data[eWindowJewelBank].OnShow)
		{
			gInterface.Data[eWindowJewelBank].OnShow = 0;
			return;
		}
		//==Show hoac send packet open
		gInterface.Data[eWindowJewelBank].OnShow = 1;


	}
}

bool CB_NewJewelBank::SendItem(ITEM* ItemSell, int Slot, bool KeyClick)
{
	if (KeyClick && gInterface.Data[eWindowJewelBank].OnShow && GetKeyState(VK_SHIFT) & 0x8000)// Send Item
	{
		if (ItemSell->Type < 0) { return 0; }

		PMSG_ITEM_MOVE_RECV pMsg = { 0 };

		pMsg.h.set(0xFA, 0x22, sizeof(pMsg));
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
int PageBank = 0;
int MaxPerPageBank = 8;
static void BPagePrevAction(LPVOID pClass)
{
	if (GetTickCount() < gInterface.Data[eWindowJewelBank].EventTick + 300) return;
	gInterface.Data[eWindowJewelBank].EventTick = GetTickCount();

	if (PageBank > 0)
	{
		PageBank--;
	}

	//pDrawMessage("-",1);
}
static void BPageNextAction(LPVOID pClass)
{
	if (GetTickCount() < gInterface.Data[eWindowJewelBank].EventTick + 300) return;
	gInterface.Data[eWindowJewelBank].EventTick = GetTickCount();

	if (PageBank < GetPageMax(gCB_NewJewelBank->mCListItemBank.size(), MaxPerPageBank) - 1)
	{
		PageBank++;
	}
	//	pDrawMessage("+", 1);
}

static void BPagePrevAction1(LPVOID pClass)
{
	if (GetTickCount() < gInterface.Data[eWindowJewelBank].EventTick + 300) return;
	gInterface.Data[eWindowJewelBank].EventTick = GetTickCount();

	if (PageBank > 0)
	{
		PageBank--;
	}

	//pDrawMessage("-",1);
}
static void BPageNextAction1(LPVOID pClass)
{
	if (GetTickCount() < gInterface.Data[eWindowJewelBank].EventTick + 300) return;
	gInterface.Data[eWindowJewelBank].EventTick = GetTickCount();

	if (PageBank < (gCB_NewJewelBank->mCListItemBank.size() / MaxPerPageBank))
	{
		PageBank++;
	}
	//	pDrawMessage("+", 1);
}

float FixYItemBMD(int ItemIndex)
{

	switch (ItemIndex)
	{
	case 6159:
	{
		return 6.f;
	}
	break;
	case 7199:
	{
		return 9.5f;
	}
	break;
	default:
		break;
	}
	return 0;
}
int IndexNgoc = -1;

void RutBankSoLuong(int SoLuong, int SelectIndex)
{
	//if ((GetTickCount() - gInterface.Data[NewBankSau].EventTick) < 300)
	//{
	//	return;
	//}
	//==Tham Gia Than Ma
	REQUEST_RUTGUIBANK pMsg;
	pMsg.header.set(0xFA, 0x21, sizeof(pMsg));
	pMsg.TypeSend = SoLuong;
	pMsg.IndexList = SelectIndex;

	//gInterface.DrawMessage(1, "RUTGUIBANK  %d / %d - %d / %d", pMsg.TypeSend, pMsg.IndexList, SoLuong, SelectIndex);

	DataSend((LPBYTE)& pMsg, pMsg.header.size);

	//gInterface.Data[NewBankSau].EventTick = GetTickCount();
}
CUITextInputBox* InputSoLuongNgoc = NULL;
char GetSoLuongNgoc[9];
void CB_NewJewelBank::DrawNhapSoNgoc()
{
	if (!gInterface.Data[eWindowJewelBankRut].OnShow)
	{
		IndexNgoc = -1;
		if (InputSoLuongNgoc)
		{
			InputSoLuongNgoc = nullptr;
		}
		return;
	}

	float WindowW = 150;
	float WindowH = 80;
	float StartX = (MAX_WIN_WIDTH / 2) - (WindowW / 2);
	float StartY = ((MAX_WIN_HEIGHT - 51) / 2) - (WindowH / 2);
	g_pBCustomMenuInfo->gDrawWindowCustom(&StartX, &StartY, WindowW, WindowH, eWindowJewelBankRut, " ");

	TextDraw((HFONT)g_hFontBold, StartX + 5, StartY + 10, 0xffffffff, 0x3a4b3978, WindowW - 10, 0, 3, "Nhập só lượng rút: "); //"Yêu Cầu");

	float NTCoinX = StartX + 45;
	float NTCoinY = StartY + 35;
	float NTW = 70;
	gInterface.DrawBarForm(NTCoinX - 3, NTCoinY - 2.5, NTW, 12, 1.0, 1.0, 1.0, 0.8);
	if (!InputSoLuongNgoc)
	{
		InputSoLuongNgoc = new CUITextInputBox;
		InputSoLuongNgoc->Init(pGameWindow, NTCoinX, NTCoinY - 2, 8);
		InputSoLuongNgoc->SetBackColor(0, 0, 0, 0);
		InputSoLuongNgoc->SetTextColor(255, 0, 0, 0);
		InputSoLuongNgoc->SetFont((HFONT)g_hFont);
		InputSoLuongNgoc->SetState(UISTATE_NORMAL);
		InputSoLuongNgoc->SetOption(UIOPTION_NUMBERONLY);
		InputSoLuongNgoc->SetPosition(NTCoinX, NTCoinY - 2);
		InputSoLuongNgoc->SetText("0");
	}
	else
	{
		InputSoLuongNgoc->SetPosition(NTCoinX, NTCoinY - 2);
		InputSoLuongNgoc->Render();
		InputSoLuongNgoc->GetText(GetSoLuongNgoc, sizeof(GetSoLuongNgoc));

		if (SEASON3B::CheckMouseIn(NTCoinX - 3, NTCoinY - 4.5, NTW, 20) == 1)
		{

			if (GetKeyState(VK_LBUTTON) & 0x8000)
			{
				InputSoLuongNgoc->GiveFocus(1);
				PlayBuffer(25, 0, 0);
			}

		}
	}

	if (g_pBCustomMenuInfo->DrawButton(StartX + 50, StartY + 50, 100, 12, "Rút", 50))
	{
		RutBankSoLuong(atoi(GetSoLuongNgoc), IndexNgoc);
		InputSoLuongNgoc->SetText("0");
		gInterface.Data[eWindowJewelBankRut].OnShow ^= 1;
	}

}

void NewBankSendGS(int Type, int SelectIndex)
{

	if ((GetTickCount() - gInterface.Data[eWindowJewelBank].EventTick) < 300)
	{
		return;
	}
	////==Tham Gia Than Ma
	REQUEST_RUTGUIBANK pMsg;
	pMsg.header.set(0xFA, 0x20, sizeof(pMsg));
	pMsg.TypeSend = Type;
	pMsg.IndexList = SelectIndex;
	DataSend((LPBYTE)& pMsg, pMsg.header.size);

	gInterface.Data[eWindowJewelBank].EventTick = GetTickCount();
}

void CB_NewJewelBank::DrawWindow()
{

	if (gInterface.CheckWindow(Interface::MoveList) || gInterface.CheckWindow(Interface::ObjWindow::CashShop) || gInterface.CheckWindow(Interface::ObjWindow::SkillTree) || gInterface.CheckWindow(Interface::ObjWindow::FullMap)
		|| (gInterface.CheckWindow(Interface::Inventory)
			&& gInterface.CheckWindow(Interface::ExpandInventory)
			&& gInterface.CheckWindow(Interface::Store))
		|| (gInterface.CheckWindow(Interface::Inventory)
			&& gInterface.CheckWindow(Interface::Warehouse)
			&& gInterface.CheckWindow(Interface::ExpandWarehouse)))
	{
		gInterface.Data[eWindowJewelBank].OnShow = false;
		return;
	}

	if (!gInterface.Data[eWindowJewelBank].OnShow || !gCB_NewJewelBank)
	{
		return;
	}
	float CuaSoW = 440;
	float CuaSoH = 340;
	//float StartY = ((MAX_WIN_HEIGHT - 51) / 2) - (WindowH / 2);
	//float StartX = (MAX_WIN_WIDTH / 2) - (WindowW / 2);
	const BYTE state[3] = { 0, 1, 2 };
	float StartX = 10;
	float StartY = 30;


	g_pBCustomMenuInfo->gDrawWindowCustom(&StartX, &StartY, CuaSoW, CuaSoH, eWindowJewelBank, gCustomMessage.GetMessage(73));
	float CenterX = StartX + (CuaSoW / 2);
	float MaxY = StartY + CuaSoH;

	TextDraw((HFONT)g_hFont, StartX, StartY + 45, 0xFF6600FF, 0x0, CuaSoW, 0, 3, "Bấm vào nút (Gửi) để gửi ngọc vào ngân hàng hoặc Shift + chuột phải để gửi nhanh");
	TextDraw((HFONT)g_hFont, StartX, StartY + 45 + 15, 0xFF6600FF, 0x0, CuaSoW, 0, 3, "* Auto Pick tự động gửi vào ngân hàng khi nhặt");

	float StartXItem = StartX + 10;
	float StartYItem = StartY + 75;

	TextDraw((HFONT)g_hFont, StartXItem + 36 + 100 + 70, StartYItem + 15, 0xFFFFFFFF, 0xD69015A9, 138, 0, 3, "Rút Ngọc");
	TextDraw((HFONT)g_hFont, StartXItem + 36 + 100 + 70 + 138, StartYItem + 15, 0xFFFFFFFF, 0x15D6C9A9, 40, 0, 3, "Gửi Ngọc");
	TextDraw((HFONT)g_hFont, StartXItem + 36 + 100 + 70 + 138 + 40, StartYItem + 15, 0xFFFFFFFF, 0x9C15D6A9, 40, 0, 3, "Auto Pick");
	float KhoangCach = 25;

	for (int n = 0, r = (PageBank * MaxPerPageBank); n < MaxPerPageBank && r < gCB_NewJewelBank->mCListItemBank.size(); r++)
	{
		int ItemIndex = gCB_NewJewelBank->mCListItemBank[r].ItemIndex;
		int ItemLevel = gCB_NewJewelBank->mCListItemBank[r].ItemLevel;


		float itemScale = 1.0;
		float addY = FixYItemBMD(ItemIndex);
		int nCount = 0;

		if (n % 2)
		{
			gInterface.DrawBarForm(StartXItem, (StartYItem + (n * KhoangCach)) + 25, CuaSoW - 20, 20, 0.0, 0.0, 0.0, 0.3);//bg
		}
		else
		{
			gInterface.DrawBarForm(StartXItem, (StartYItem + (n * KhoangCach)) + 25, CuaSoW - 20, 20, 0.0, 0.0, 0.0, 0.6);//bg
		}
		g_pNewUISystem->RenderItem3DFree(StartXItem, (StartYItem + (n * KhoangCach) + addY), 45, 45, ItemIndex, SET_ITEMOPT_LEVEL(ItemLevel), 0, 0, 0, itemScale, false);//bmd
		TextDraw((HFONT)g_hFont, StartXItem + 36, (StartYItem + (n * KhoangCach)) + 28, 0xFFFFFFA9, 0x0, 100, 0, 3, BGetItemName(ItemIndex, SET_ITEMOPT_LEVEL(ItemLevel))); //Name
		TextDraw((HFONT)g_hFont, StartXItem + 36 + 100, (StartYItem + (n * KhoangCach)) + 28, 0xFFDD00A9, 0x0, 60, 0, 3, "x%d", gCB_NewJewelBank->mCListItemBank[r].ItemCount); //so luong
		//==ButtonRut 1

		if (UIController.BButton(gInterface.Data[IMG_31618].ModelID, StartXItem + 36 + 100 + 70 + (28 * nCount), (StartYItem + (n * KhoangCach)) + 25, 25, 22, 25, 22, 3, state) && !gInterface.Data[eWindowJewelBankRut].OnShow)
		{
			//NewBankSendGS(eRut1, r);
			IndexNgoc = r;

			if ((GetTickCount() - gInterface.Data[eWindowJewelBankRut].EventTick) > 300)
			{
				gInterface.Data[eWindowJewelBankRut].EventTick = GetTickCount();

				if (gInterface.Data[eWindowJewelBankRut].OnShow)
				{
					gInterface.Data[eWindowJewelBankRut].OnShow = 0;

					return;
				}
				gInterface.Data[eWindowJewelBankRut].OnShow ^= 1;
			}
		}
		TextDraw((HFONT)g_hFont, StartXItem + 36 + 100 + 70 + (28 * nCount++), (StartYItem + (n * KhoangCach)) + 30, 0xFFFFFFBE, 0x0, 26, 0, 3, "Rút");

		//==ButtonRut 10
		if (UIController.BButton(gInterface.Data[IMG_31618].ModelID, StartXItem + 36 + 100 + 70 + (28 * nCount), (StartYItem + (n * KhoangCach)) + 25, 25, 22, 25, 22, 3, state) && !gInterface.Data[eWindowJewelBankRut].OnShow)
		{
			NewBankSendGS(eRut10, r);
		}
		TextDraw((HFONT)g_hFont, StartXItem + 36 + 100 + 70 + (28 * nCount++), (StartYItem + (n * KhoangCach)) + 30, 0xFFFFFFBE, 0x0, 26, 0, 3, "10");
		//==ButtonRut 20
		if (UIController.BButton(gInterface.Data[IMG_31618].ModelID, StartXItem + 36 + 100 + 70 + (28 * nCount), (StartYItem + (n * KhoangCach)) + 25, 25, 22, 25, 22, 3, state) && !gInterface.Data[eWindowJewelBankRut].OnShow)
		{
			NewBankSendGS(eRut20, r);
		}
		TextDraw((HFONT)g_hFont, StartXItem + 36 + 100 + 70 + (28 * nCount++), (StartYItem + (n * KhoangCach)) + 30, 0xFFFFFFBE, 0x0, 26, 0, 3, "20");
		//==ButtonRut 30
		if (UIController.BButton(gInterface.Data[IMG_31618].ModelID, StartXItem + 36 + 100 + 70 + (28 * nCount), (StartYItem + (n * KhoangCach)) + 25, 25, 22, 25, 22, 3, state) && !gInterface.Data[eWindowJewelBankRut].OnShow)
		{
			NewBankSendGS(eRut30, r);
		}
		TextDraw((HFONT)g_hFont, StartXItem + 36 + 100 + 70 + (28 * nCount++), (StartYItem + (n * KhoangCach)) + 30, 0xFFFFFFBE, 0x0, 26, 0, 3, "30");
		//==ButtonRut All
		if (UIController.BButton(gInterface.Data[IMG_31618].ModelID, StartXItem + 36 + 100 + 70 + (28 * nCount), (StartYItem + (n * KhoangCach)) + 25, 25, 22, 25, 22, 3, state) && !gInterface.Data[eWindowJewelBankRut].OnShow)
		{
			NewBankSendGS(eRutAll, r);
		}
		TextDraw((HFONT)g_hFont, StartXItem + 36 + 100 + 70 + (28 * nCount++), (StartYItem + (n * KhoangCach)) + 30, 0xFFFFFFBE, 0x0, 26, 0, 3, "***");
		//==ButtonGui All
		if (UIController.BButton(gInterface.Data[IMG_31617].ModelID, StartXItem + 36 + 100 + 70 + (28 * nCount), (StartYItem + (n * KhoangCach)) + 25, 38, 23, 38, 23, 3, state) && !gInterface.Data[eWindowJewelBankRut].OnShow)
		{
			NewBankSendGS(eGuiAll, r);
		}
		TextDraw((HFONT)g_hFont, StartXItem + 36 + 100 + 70 + (28 * nCount++), (StartYItem + (n * KhoangCach)) + 30, 0xFFFFFFBE, 0x0, 38, 0, 3, "Gửi");
		//==Auto Pick
		if (g_pBCustomMenuInfo->RenderCheckBox(StartXItem + 36 + 100 + 70 + (28 * nCount) + 20, (StartYItem + (n * KhoangCach)) + 28, 0xFFCC00C8, gCB_NewJewelBank->mCListItemBank[r].AutoPick, "") && !gInterface.Data[eWindowJewelBankRut].OnShow)
		{
			NewBankSendGS(eAutoPick, r);
		}
		n++;
	}

	//Next Page
	RenderBitmap(CNewUIInGameShop::IMAGE_IGS_STORAGE_PAGE, CenterX - (60 / 2), MaxY - 40, 60.f, 22.f, 0, 0, 80.f / 128.f, 30.f / 34.f, 1, 1, 0.0);

	TextDraw((HFONT)g_hFontBold, CenterX - (60 / 2), MaxY - 40 + 5, 0xffffffff, 0x0, 60, 0, 3, "%d / %d", PageBank + 1, GetPageMax(gCB_NewJewelBank->mCListItemBank.size(), MaxPerPageBank));
	//

	//
	UIController.Button(CNewUIInGameShop::IMAGE_IGS_STORAGE_PAGE_LEFT, CenterX - (60 / 2) - 26, MaxY - 40, 20, 22, 20, 22, 3, state, BPagePrevAction);
	UIController.Button(CNewUIInGameShop::IMAGE_IGS_STORAGE_PAGE_RIGHT, CenterX + (60 / 2) + 6, MaxY - 40, 20, 22, 20, 22, 3, state, BPageNextAction);

}
