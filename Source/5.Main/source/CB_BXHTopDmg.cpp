#include "StdAfx.h"
#include "CB_BXHTopDmg.h"
#include "NewUISystem.h"
#include "CBInterface.h"
#include "CUIController.h"
#include "CharacterManager.h"
#include "Util.h"
#include "Protocol.h"
#include "NewUIBase.h"
#include "Other.h"
#include "ZzzInterface.h"

CB_BXHTopDmg* gCB_BXHTopDmg;

CB_BXHTopDmg::CB_BXHTopDmg()
{
	this->m_DataTopBXHDMG.clear();
}

CB_BXHTopDmg::~CB_BXHTopDmg()
{
}

void CB_BXHTopDmg::RecvProtocol(BYTE* Recv)
{
	if (!Recv) return;
	this->m_DataTopBXHDMG.clear();
	this->ClassMonter = -1;
	PMSG_COUNT_SEND_DATA* lpMsg = (PMSG_COUNT_SEND_DATA*)Recv;
	this->ClassMonter = lpMsg->ClassMonter;
	for (int n = 0; n < lpMsg->count; n++)
	{
		DATA_BXH_SEND* lpInfo = (CB_BXHTopDmg::DATA_BXH_SEND*)(((BYTE*)lpMsg) + sizeof(PMSG_COUNT_SEND_DATA) + (sizeof(DATA_BXH_SEND) * n));
		this->m_DataTopBXHDMG.push_back(*lpInfo);
	}
	//gInterface.DrawMessage(1, "BXHDmg Recv Size %d %s %I64u", this->m_DataTopBXHDMG.size(), this->m_DataTopBXHDMG[0].Name, this->m_DataTopBXHDMG[0].damage);

	if (!this->m_DataTopBXHDMG.empty()) gInterface.Data[eWindowBXHDmg].OnShow = 1;
}

void CB_BXHTopDmg::DrawWindowMini()
{
	if (gInterface.CheckWindow(Interface::MoveList)
		|| gInterface.CheckWindow(Interface::ObjWindow::CreateGuild)
		|| gInterface.CheckWindow(Interface::ObjWindow::BloodCastle)
		|| gInterface.CheckWindow(Interface::ObjWindow::DevilSquare)
		|| gInterface.CheckWindow(Interface::ObjWindow::Character)
		|| gInterface.CheckWindow(Interface::ObjWindow::FastMenu)
		|| gInterface.CheckWindow(Interface::ObjWindow::CashShop)
		|| gInterface.CheckWindow(Interface::ObjWindow::SkillTree) || gInterface.CheckWindow(Interface::ObjWindow::FullMap)
		|| (gInterface.CheckWindow(Interface::Inventory)
			&& gInterface.CheckWindow(Interface::ExpandInventory)
			&& gInterface.CheckWindow(Interface::Store))
		|| (gInterface.CheckWindow(Interface::Inventory)
			&& gInterface.CheckWindow(Interface::Warehouse)
			&& gInterface.CheckWindow(Interface::ExpandWarehouse)) || gInterface.CheckWindow(Interface::Inventory))
	{

		return;
	}

	if (!gInterface.Data[eWindowBXHDmg].OnShow || this->m_DataTopBXHDMG.empty())
	{
		gInterface.Data[eWindowBXHDmg].OnShow = false;
		return;
	}
	float WindowW = 150;
	float WindowH = 120;
	float StartX = (MAX_WIN_WIDTH - WindowW) - (10);
	float StartY = (MAX_WIN_HEIGHT - 350);

	g_pBCustomMenuInfo->DrawWindowCustomMini(&StartX, &StartY, WindowW, WindowH, eWindowBXHDmg, "%s", getMonsterName(this->ClassMonter));

	if (gInterface.Data[eWindowBXHDmg].BActiveHiden) return;

	TextDraw((HFONT)g_hFontBold, StartX + 3, StartY + 25, 0xFFC421FF, 0x0, 60, 0, 3, "Top");
	TextDraw((HFONT)g_hFontBold, StartX + 5 + 50, StartY + 25, 0xFFC421FF, 0x0, 85, 0, 3, "Sát Thương");
	int TextY = StartY + 25 + 13;
	for (int i = 0; i < this->m_DataTopBXHDMG.size(); i++)
	{
		TextDraw((HFONT)g_hFont, StartX + 3, TextY + (11*i), 0xFFFFFFFF, 0x0, 60, 0, 3, this->m_DataTopBXHDMG[i].Name);
		TextDraw((HFONT)g_hFont, StartX + 5 + 50, TextY +(11 * i), 0xFFFFFFFF, 0x0, 85, 0, 3, "%s",gInterface.QNumberFormat(this->m_DataTopBXHDMG[i].damage));
	}

}