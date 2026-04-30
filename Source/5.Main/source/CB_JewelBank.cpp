#include "StdAfx.h"
#include "CB_JewelBank.h"
#include "NewUISystem.h"
#include "CBInterface.h"
#include "CUIController.h"
#include "CharacterManager.h"
#include "Util.h"
#include "Protocol.h"
#include "NewUIBase.h"
#include "Other.h"

CBJewelBank gCBJewelBank;
char* NumberButton[7] = { "01","10","20","30","40","50","..." };
CBJewelBank::CBJewelBank()
{
	this->Pagina = 0;
	this->ItemJewelry[0] = ITEM(12, 15);//-- CHAOS
	this->ItemJewelry[1] = ITEM(14, 13);//-- BLESS
	this->ItemJewelry[2] = ITEM(14, 14);//-- SOUL
	this->ItemJewelry[3] = ITEM(14, 16);//-- LIFE
	this->ItemJewelry[4] = ITEM(14, 22);//-- CREATION

	this->ItemJewelry[5] = ITEM(14, 41);//-- GEMS
	this->ItemJewelry[6] = ITEM(14, 31);//-- GUARDYAN
	this->ItemJewelry[7] = ITEM(14, 42);//-- HARMONY
	this->ItemJewelry[8] = ITEM(14, 43);//-- LOWER
	this->ItemJewelry[9] = ITEM(14, 44);//-- HIGHER

	this->BankJewelry[0] = 0;//-- CHAOS
	this->BankJewelry[1] = 0;//-- BLESS
	this->BankJewelry[2] = 0;//-- SOUL
	this->BankJewelry[3] = 0;//-- LIFE
	this->BankJewelry[4] = 0;//-- CREATION

	this->BankJewelry[5] = 0;//-- GEMS
	this->BankJewelry[6] = 0;//-- GUARDYAN
	this->BankJewelry[7] = 0;//-- HARMONY
	this->BankJewelry[8] = 0;//-- LOWER
	this->BankJewelry[9] = 0;//-- HIGHER
}

CBJewelBank::~CBJewelBank()
{

}
void CBJewelBank::OpenOnOff()
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
void CBJewelBank::DrawWindow()
{

	if (gInterface.CheckWindow(Interface::ObjWindow::MoveList) || gInterface.CheckWindow(Interface::ObjWindow::CashShop) || gInterface.CheckWindow(Interface::ObjWindow::SkillTree) || gInterface.CheckWindow(Interface::ObjWindow::FullMap)
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

	if (!gInterface.Data[eWindowJewelBank].OnShow)
	{
		return;
	}
	float WindowW = 245.0;
	float WindowH = 320;
	float StartY = ((MAX_WIN_HEIGHT - 51) / 2) - (WindowH / 2);
	float StartX = (MAX_WIN_WIDTH / 2) - (WindowW / 2);
	//------------------------------------------------------------------------------
	if (!g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INVENTORY))
	{
		g_pNewUISystem->Show(SEASON3B::INTERFACE_INVENTORY);
	}
	g_pBCustomMenuInfo->gDrawWindowCustom(&StartX, &StartY, WindowW, WindowH, eWindowJewelBank, gCustomMessage.GetMessage(73));

	float LineX = StartX + 10;
	float LineY = StartY + 40;
	float WInfo = 80;
	for (int i = 0; i < 5; i++)
	{
		g_pNewUISystem->RenderItem3DFree(LineX, LineY, 15, 15, this->ItemJewelry[i+5 * this->Pagina], 0, 1, 0, 0, 1.0);//BMD MOdel
		TextDraw((HFONT)g_hFontBold, LineX + 15, LineY + 22, 0xFFDE26FF, 0x3a4b3978, WInfo, 0, 3, BGetItemName(this->ItemJewelry[i+5 * this->Pagina], 0));
		LineY += 10;
		TextDraw((HFONT)g_hFont, LineX + 15, LineY + 22, 0xFFDE26FF, 0x0, WInfo, 0, 3, "Đang có: %d (Viên)", this->BankJewelry[i+5 * this->Pagina]);

		//==Button
		float ButtonX = LineX + WInfo + 20;
		float ButtonY = LineY + 5;
		for (int B = 0; B < 7; B++)
		{
			if (g_pBCustomMenuInfo->DrawButtonGUI(BITMAP_CHECK_BTN, ButtonX + (17 * B), ButtonY, 16, 32, 0) && (GetTickCount() - gInterface.Data[eWindowJewelBank].EventTick > 300))
			{
				PMSG_JEWELBANK_SUB pMsg;
				pMsg.h.set(0xF3, 0xF1, sizeof(pMsg));

				pMsg.Type = (int)(i + 5 * this->Pagina);
				pMsg.Mode = B;

				DataSend((LPBYTE)& pMsg, pMsg.h.size);
			}
			TextDraw((HFONT)g_hFont, ButtonX + 2 + (17 * B), ButtonY + 3, 0xFFDE26FF, 0x0, 16, 0, 3, NumberButton[B]);
		}
		LineY += 30;
	}

	TextDraw((HFONT)g_hFont, StartX, StartY + (WindowH -35), 0x7DF4FFFF, 0x0, WindowW, 0, 3, gCustomMessage.GetMessage(74));

	if (g_pBCustomMenuInfo->DrawButtonGUI(CNewUIGuildInfoWindow::IMAGE_GUILDINFO_TAB_BUTTON, StartX + 40, StartY+(WindowH-40), 20, 23, 3))
	{
		if (this->Pagina > 0) this->Pagina--;
	}
	if (g_pBCustomMenuInfo->DrawButtonGUI(CNewUIInGameShop::IMAGE_IGS_PAGE_RIGHT, StartX + (WindowW - 65), StartY + (WindowH - 40), 20, 23, 3))
	{
		if (this->Pagina < 1) this->Pagina++;
	}
}

void CBJewelBank::RecvProtocol(BYTE* Recv)
{
	if (Recv == NULL)
	{
		return;
	}
	PMSG_JEWELSBANK* aRecv = (PMSG_JEWELSBANK*)Recv;

	this->BankJewelry[0] = aRecv->Chaos;//-- CHAOS
	this->BankJewelry[1] = aRecv->Bless;//-- BLESS
	this->BankJewelry[2] = aRecv->Soul;//-- SOUL
	this->BankJewelry[3] = aRecv->Life;//-- LIFE
	this->BankJewelry[4] = aRecv->CreateonBank;//-- CREATION

	this->BankJewelry[5] = aRecv->GemStoneBank;//-- GEMS
	this->BankJewelry[6] = aRecv->GuardianBank;//-- GUARDYAN
	this->BankJewelry[7] = aRecv->HarmonyBank;//-- HARMONY
	this->BankJewelry[8] = aRecv->LowStoneBank;//-- LOWER
	this->BankJewelry[9] = aRecv->HighStoneBank;//-- HIGHER
}
DWORD CacheTimeClickSendBank = 0;
bool CBJewelBank::SendItem(ITEM* ItemSell, int Slot, bool KeyClick)
{
	if (KeyClick && /*gInterface.Data[eWindowJewelBank].OnShow*/ GetTickCount() > CacheTimeClickSendBank && GetKeyState(VK_SHIFT) & 0x8000)// Send Item
	{
		if (ItemSell->Type < 0) { return 0; }

		PMSG_ITEM_MOVE_RECV pMsg = { 0 };

		pMsg.h.set(0xF3, 0xDF, sizeof(pMsg));
		pMsg.Target = -1;
		pMsg.sFlag = 0;
		pMsg.tFlag = 0;
		pMsg.Target = 0;
		pMsg.Source = Slot;
		::PlayBuffer(SOUND_GET_ITEM01);
		DataSend((BYTE*)& pMsg, pMsg.h.size);
		CacheTimeClickSendBank = GetTickCount()+1000;
		return 1;
	}

	return 0;
}
