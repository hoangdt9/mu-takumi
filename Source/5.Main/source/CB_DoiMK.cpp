#include "StdAfx.h"
#include "CB_DoiMK.h"
#include "NewUISystem.h"
#include "CBInterface.h"
#include "CUIController.h"
#include "CharacterManager.h"
#include "Util.h"
#include "Protocol.h"
#include "NewUIBase.h"
#include "Other.h"
#include "ZzzInterface.h"

#if(DOIMK)

CB_DoiMK* gCB_DoiMK;

CB_DoiMK::CB_DoiMK()
{
	this->TimeSend = GetTickCount();
}

CB_DoiMK::~CB_DoiMK()
{
}
void CB_DoiMK::OpenWindow()
{


	if ((GetTickCount() - gInterface.Data[eWindowDoiMK].EventTick) > 300)
	{
		gInterface.Data[eWindowDoiMK].EventTick = GetTickCount();

		if (gInterface.Data[eWindowDoiMK].OnShow)
		{
			gInterface.Data[eWindowDoiMK].OnShow = 0;
			return;
		}

		//CGPACKET_LOCKWINDOW pMsg;
		//pMsg.header.set(0xD3, 0x1A, sizeof(pMsg));
		//pMsg.Status = 1; //Open Recv
		//pMsg.InPass = 1234; //
		//DataSend((LPBYTE)& pMsg, pMsg.header.size);
		gInterface.Data[eWindowDoiMK].OnShow ^= 1; //On/Off
	}

}

CUITextInputBox* InputMatKhauCu = NULL;
CUITextInputBox* InputMatKhauMoi = NULL;
char GetTextMatKhauCu[11] = { 0, };
char GetTextMatKhauMoi[11] = { 0, };
void CB_DoiMK::DrawWindow()
{
	if (gInterface.CheckWindow(Interface::MoveList) || gInterface.CheckWindow(Interface::ObjWindow::CashShop) || gInterface.CheckWindow(Interface::ObjWindow::SkillTree) || gInterface.CheckWindow(Interface::ObjWindow::FullMap)
		|| (gInterface.CheckWindow(Interface::Inventory)
			&& gInterface.CheckWindow(Interface::ExpandInventory)
			&& gInterface.CheckWindow(Interface::Store))
		|| (gInterface.CheckWindow(Interface::Inventory)
			&& gInterface.CheckWindow(Interface::Warehouse)
			&& gInterface.CheckWindow(Interface::ExpandWarehouse)))
	{
		gInterface.Data[eWindowDoiMK].OnShow = false;
		return;
	}

	if (!gInterface.Data[eWindowDoiMK].OnShow)
	{
		if (InputMatKhauCu)
		{
			InputMatKhauCu = nullptr;
			InputMatKhauMoi = nullptr;

		}
		return;
	}
	float WindowW = 230;
	float WindowH = 240;
	float StartX = (MAX_WIN_WIDTH / 2) - (WindowW / 2);
	float StartY = 30;

	g_pBCustomMenuInfo->gDrawWindowCustom(&StartX, &StartY, WindowW, WindowH, eWindowDoiMK, "Đổi Mật Khẩu Game");
	//===Info Text
	float TextX = StartX + 15;
	float TextY = StartY + 55;
	int CountText = 0;
	TextDraw((HFONT)g_hFont, TextX, TextY + (11 * CountText++), 0xFFFFFFFF, 0x0, 0, 0, 1, "**Thay đổi Mật Khẩu Tài Khoản");
	TextDraw((HFONT)g_hFont, TextX, TextY + (11 * CountText++), 0xFFAE00FF, 0x0, 0, 0, 1, "● Lưu Ý: kiểm tra và ghi nhớ MK trước khi đổi !");
	TextDraw((HFONT)g_hFont, TextX, TextY + (11 * CountText++), 0x00FFB3FF, 0x0, 0, 0, 1, "- Mật Khẩu chỉ dùng để Đăng Nhập Game");


	TextDraw((HFONT)g_hFont, TextX, (TextY + (11 * CountText)) + 10, 0xFFFFFFFF, 0x0, 0, 0, 1, "Mật Khẩu Cũ: ");

	float TCoinX = TextX + 70;
	float TCoinY = (TextY + (11 * CountText)) + 11;
	gInterface.DrawBarForm(TCoinX - 3, TCoinY - 2.5, 100, 13, 0.0, 0.0, 0.0, 0.8);

	if (!InputMatKhauCu)
	{
		InputMatKhauCu = new CUITextInputBox;
		InputMatKhauCu->Init(pGameWindow, TCoinX, TCoinY, 11, TRUE);
		InputMatKhauCu->SetBackColor(0, 0, 0, 0);
		InputMatKhauCu->SetTextColor(255, 255, 157, 0);
		InputMatKhauCu->SetFont((HFONT)g_hFont);
		InputMatKhauCu->SetState(UISTATE_NORMAL);
		InputMatKhauCu->SetOption(UIOPTION_NOLOCALIZEDCHARACTERS);
		InputMatKhauCu->SetPosition(TCoinX, TCoinY);
	}
	else
	{
		InputMatKhauCu->SetTabTarget(InputMatKhauMoi);
		if (gInterface.Data[eWindowLockItem].OnClick)
		{
			InputMatKhauCu->SetPosition(TCoinX, TCoinY);
		}
		InputMatKhauCu->Render();

		InputMatKhauCu->GetText(GetTextMatKhauCu, 20);

		if (SEASON3B::CheckMouseIn(TCoinX - 5, TCoinY - 5, 100, 30) == 1)
		{

			if (GetKeyState(VK_LBUTTON) & 0x8000)
			{
				InputMatKhauCu->GiveFocus(1);
				PlayBuffer(25, 0, 0);
			}
		}

	}
	TextY += 20;
	TextDraw((HFONT)g_hFont, TextX, (TextY + (11 * CountText)) + 10, 0xFFFFFFFF, 0x0, 0, 0, 1, "Mật Khẩu Mới: ");

	TCoinX = TextX + 70;
	TCoinY = (TextY + (11 * CountText)) + 11;
	gInterface.DrawBarForm(TCoinX - 3, TCoinY - 2.5, 100, 13, 0.0, 0.0, 0.0, 0.8);

	if (!InputMatKhauMoi)
	{
		InputMatKhauMoi = new CUITextInputBox;
		InputMatKhauMoi->Init(pGameWindow, TCoinX, TCoinY, 11, TRUE);
		InputMatKhauMoi->SetBackColor(0, 0, 0, 0);
		InputMatKhauMoi->SetTextColor(255, 255, 157, 0);
		InputMatKhauMoi->SetFont((HFONT)g_hFont);
		InputMatKhauMoi->SetState(UISTATE_NORMAL);
		InputMatKhauMoi->SetOption(UIOPTION_NOLOCALIZEDCHARACTERS);
		InputMatKhauMoi->SetPosition(TCoinX, TCoinY);
	}
	else
	{
		InputMatKhauMoi->SetTabTarget(InputMatKhauCu);
		if (gInterface.Data[eWindowLockItem].OnClick)
		{
			InputMatKhauMoi->SetPosition(TCoinX, TCoinY);
		}
		InputMatKhauMoi->Render();

		InputMatKhauMoi->GetText(GetTextMatKhauMoi, 20);

		if (SEASON3B::CheckMouseIn(TCoinX - 5, TCoinY - 5, 100, 30) == 1)
		{

			if (GetKeyState(VK_LBUTTON) & 0x8000)
			{
				InputMatKhauMoi->GiveFocus(1);
				PlayBuffer(25, 0, 0);
			}
		}

	}
	if (g_pBCustomMenuInfo->DrawButton(StartX + (WindowW / 2) - (90 / 2), StartY + (WindowH - 20 - 30), 100, 12, "Xác Nhận Đổi MK", 80))
	{
		if (this->TimeSend > GetTickCount())
		{
			gInterface.OpenMessageBox("Warning", "Vui Lòng Thao Tác Chậm Lại !!");
			return ;
		}
		//=== SEND GS
		CSENDGS_DOIMK pMsg;
		pMsg.TYPE = 0x01;
		memcpy(pMsg.PASS_OLD, GetTextMatKhauCu, sizeof(pMsg.PASS_OLD));
		memcpy(pMsg.PASS_NEW, GetTextMatKhauMoi, sizeof(pMsg.PASS_NEW));
		pMsg.Head.set(0xFF, 0x91, sizeof(pMsg));
		DataSend((BYTE*)& pMsg, pMsg.Head.size);
		this->TimeSend = GetTickCount() + 5000;
		//===========

	}

}
#endif