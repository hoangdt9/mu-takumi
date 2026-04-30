#include "StdAfx.h"
#include "CB_LockItem.h"
#include "NewUISystem.h"
#include "CBInterface.h"
#include "CUIController.h"
#include "CharacterManager.h"
#include "Util.h"
#include "Protocol.h"
#include "NewUIBase.h"
#include "Other.h"
#include "ZzzInterface.h"

#if(CUSTOM_WINDOWLOCKITEM)
CUITextInputBox* InputMatKhau = NULL;
CUITextInputBox* InputCaptCha = NULL;
char GetTextMatKhau[8 + 1] = { 0, };
char GetTextCaptCha[4 + 1] = { 0, };

CB_LockItem* gCB_LockItem;

CB_LockItem::CB_LockItem()
{
	this->StatusLock = 0;
}

CB_LockItem::~CB_LockItem()
{
}

void CB_LockItem::RecvGSSatusLockWindow(CGPACKET_LOCKWINDOW* lpMsg)
{
	StatusLock = lpMsg->Status; //
}
void CB_LockItem::OpenWindowLock()
{


	if ((GetTickCount() - gInterface.Data[eWindowLockItem].EventTick) > 300)
	{
		gInterface.Data[eWindowLockItem].EventTick = GetTickCount();

		if (gInterface.Data[eWindowLockItem].OnShow)
		{
			gInterface.Data[eWindowLockItem].OnShow = 0;
			return;
		}

		CGPACKET_LOCKWINDOW pMsg;
		pMsg.header.set(0xD3, 0x1A, sizeof(pMsg));
		pMsg.Status = 1; //Open Recv
		pMsg.InPass = 1234; //
		DataSend((LPBYTE)& pMsg, pMsg.header.size);
		gInterface.Data[eWindowLockItem].OnShow ^= 1; //On/Off
	}

}

void CB_LockItem::SendLockUnLock(int Pass)
{
	//===SendUnLock
	CGPACKET_LOCKWINDOW pMsg;
	pMsg.header.set(0xD3, 0x1B, sizeof(pMsg));
	pMsg.Status = StatusLock; //Open Recv
	pMsg.InPass = Pass; //
	DataSend((LPBYTE)& pMsg, pMsg.header.size);
}
void CB_LockItem::DrawWindow()
{
	if (gInterface.CheckWindow(Interface::MoveList) || gInterface.CheckWindow(Interface::ObjWindow::CashShop) || gInterface.CheckWindow(Interface::ObjWindow::SkillTree) || gInterface.CheckWindow(Interface::ObjWindow::FullMap)
		|| (gInterface.CheckWindow(Interface::Inventory)
			&& gInterface.CheckWindow(Interface::ExpandInventory)
			&& gInterface.CheckWindow(Interface::Store))
		|| (gInterface.CheckWindow(Interface::Inventory)
			&& gInterface.CheckWindow(Interface::Warehouse)
			&& gInterface.CheckWindow(Interface::ExpandWarehouse)))
	{
		gInterface.Data[eWindowLockItem].OnShow = false;
		return;
	}

	if (!gInterface.Data[eWindowLockItem].OnShow)
	{
		return;
	}
	float WindowW = 230;
	float WindowH = 260;
	float StartX = (MAX_WIN_WIDTH/2)-(WindowW/2);
	float StartY = 30;

	g_pBCustomMenuInfo->gDrawWindowCustom(&StartX, &StartY, WindowW, WindowH, eWindowLockItem, "Khoá Giao Dịch");
	//===Info Text
	float TextX = StartX + 15;
	float TextY = StartY + 55;
	int CountText = 0;

	if (!StatusLock)
	{
		TextDraw((HFONT)g_hFont, TextX + 60, TextY, 0xFF6600FF, 0x0, 0, 0, 1, "Chưa Khoá Giao Dịch");
	}
	else
	{
		TextDraw((HFONT)g_hFont, TextX + 60, TextY, 0x0077FFFF, 0x0, 0, 0, 1, "Đã Khoá Giao Dịch");
	}
	TextDraw((HFONT)g_hFont, TextX, TextY + (11 * CountText++), 0xFFFFFFFF, 0x0, 0, 0, 1, "**Trạng Thái:");
	TextDraw((HFONT)g_hFont, TextX, TextY + (11 * CountText++), 0xFFAE00FF, 0x0, 0, 0, 1, "● Lưu Ý: Nhập mật khẩu để khoá  trạng thái !");
	TextDraw((HFONT)g_hFont, TextX, TextY + (11 * CountText++), 0x00FFB3FF, 0x0, 0, 0, 1, "- Giao Dịch - Cửa Hàng - Phi Shop - Xoá Nhân Vật");

	TextDraw((HFONT)g_hFont, TextX, (TextY + (11 * CountText)) + 10, 0xFFFFFFFF, 0x0, 0, 0, 1, "Mật Khẩu Khoá");
	
	float TCoinX = TextX + 70;
	float TCoinY = (TextY + (11 * CountText)) + 11;
	gInterface.DrawBarForm(TCoinX - 3, TCoinY - 2.5, 70, 12, 0.0, 0.0, 0.0, 0.8);

	if (!InputMatKhau)
	{
		InputMatKhau = new CUITextInputBox;
		InputMatKhau->Init(pGameWindow, TCoinX, TCoinY, 8, TRUE);
		InputMatKhau->SetBackColor(0, 0, 0, 0);
		InputMatKhau->SetTextColor(255, 255, 157, 0);
		InputMatKhau->SetFont((HFONT)g_hFont);
		InputMatKhau->SetState(UISTATE_NORMAL);
		InputMatKhau->SetOption(UIOPTION_NUMBERONLY);
		InputMatKhau->SetPosition(TCoinX, TCoinY);
	}
	else
	{
		InputMatKhau->SetTabTarget(InputCaptCha);
		if (gInterface.Data[eWindowLockItem].OnClick)
		{
			InputMatKhau->SetPosition(TCoinX, TCoinY);
		}
		InputMatKhau->Render();

		InputMatKhau->GetText(GetTextMatKhau, 8 + 1);

		if (SEASON3B::CheckMouseIn(TCoinX - 5, TCoinY - 5, 100, 30) == 1)
		{

			if (GetKeyState(VK_LBUTTON) & 0x8000)
			{
				InputMatKhau->GiveFocus(1);
				PlayBuffer(25, 0, 0);
			}
		}

	}
	TextDraw((HFONT)g_hFontBold, TextX, TCoinY + 25, 0xFF009DFF, 0x0, WindowW - 30, 0, 3, "Chú Ý : Vui lòng ghi nhớ mật khẩu ## nếu quên thì liên hệ Admin !");
	//===Captcha
	TextDraw((HFONT)g_hFontBold, TextX, TCoinY + 55, 0x62FF00FF, 0x0, WindowW - 30, 0, 3, "Mã Xác Nhận");
	float CaptChaX = TextX + 40;
	float CaptChaY = TCoinY + 70;
	if (!InputCaptCha)
	{
		InputCaptCha = new CUITextInputBox;
		InputCaptCha->Init(pGameWindow, CaptChaX, CaptChaY, 5);
		InputCaptCha->SetBackColor(0, 0, 0, 0);
		InputCaptCha->SetTextColor(255, 255, 157, 0);
		InputCaptCha->SetFont((HFONT)g_hFont);
		InputCaptCha->SetState(UISTATE_NORMAL);
		InputCaptCha->SetOption(UIOPTION_NUMBERONLY);
		InputCaptCha->SetPosition(CaptChaX, CaptChaY);
	}
	else
	{
		InputCaptCha->SetTabTarget(InputMatKhau);

		gInterface.RenderCaptchaNumber(CaptChaX, CaptChaY, InputCaptCha, gInterface.vCaptcha.c_str());
		InputCaptCha->GetText(GetTextCaptCha, 4 + 1);
	}
	std::string CaptchaInput(GetTextCaptCha);
	//==Lock // UnLock
	//if (g_pBCustomMenuInfo->DrawButton(StartX + (WindowW / 2) - (90 / 2), StartY - 20 - 30, 90, 30, FALSE, (char*)((char*)((!StatusLock) ? "Khoá Giao Dịch" : "Mở Khoá Giao Dịch"))))
	if (g_pBCustomMenuInfo->DrawButton(StartX + (WindowW / 2) - (90 / 2), StartY+(WindowH - 20 - 30), 100, 12, (char*)((char*)((!StatusLock) ? "Khoá Giao Dịch" : "Mở Khoá Giao Dịch")), 80))
	{

		if (!gInterface.check_Captcha(gInterface.vCaptcha, CaptchaInput))
		{
			gInterface.DrawMessage(1, "[Lỗi] Sai mã Captcha !!");
		}
		else
		{
			//gInterface.DrawMessage(1, "[OK] Mat Khau %d",atoi(GetTextMatKhau));
			this->SendLockUnLock(atoi(GetTextMatKhau));
			//StatusLock ^= 1;
		}
		gInterface.vCaptcha = gInterface.generateCaptcha(4); //tao lai Captcha
	}
}
#endif