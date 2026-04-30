#include "StdAfx.h"
#include "CB_BossGuild.h"
#include "NewUISystem.h"
#include "CBInterface.h"
#include "CUIController.h"
#include "CharacterManager.h"
#include "Util.h"
#include "Protocol.h"
#include "NewUIBase.h"
#include "Other.h"
#include "MapManager.h"

CBBossGuild* gCBBossGuild = nullptr;
CBBossGuild::CBBossGuild()
{
	this->BossGuild_TimeConLai = 0;
	this->CacheForm = false;

	this->CheDo1 = 0;
	this->CheDo2 = 0;
	this->KillBoss = 0;
	this->Score = 0;
	this->WCoinC = 0;
	this->PhuThuyWCoinC = 0;
	this->ChienBinhWCoinC = 0;
	this->TienNuWCoinC = 0;
	this->ThuatSiWCoinC = 0;
	this->ThietBinhWCoinC = 0;
	this->DauSiWCoinC = 0;
	this->ChuaTeWCoinC = 0;
}

CBBossGuild::~CBBossGuild()
{

}
void CGMoveBossGuild(int Type)
{
	if (Type > 0)
	{
		CBBossGuild::BOSSGUILD_CGPACKET pMsg;
		pMsg.header.set(0xF3, 0x40, sizeof(pMsg));
		pMsg.CongVao = Type - 1;
		DataSend((LPBYTE)& pMsg, pMsg.header.size);
	}
}
void CBBossGuild::OpenOnOff()
{
	if (GetTickCount() - gInterface.Data[eBossGuildWindow].EventTick > 300)
	{
		gInterface.Data[eBossGuildWindow].EventTick = GetTickCount();
		if (gInterface.Data[eBossGuildWindow].OnShow)
		{
			gInterface.Data[eBossGuildWindow].OnShow = 0;
			return;
		}
		//==Show hoac send packet open
		gInterface.Data[eBossGuildWindow].OnShow = 1;


	}
}

void CBBossGuild::DrawWindow()
{
	if (!gCBBossGuild) return;

	if (gInterface.CheckWindow(Interface::ObjWindow::MoveList) || gInterface.CheckWindow(Interface::ObjWindow::CashShop) || gInterface.CheckWindow(Interface::ObjWindow::SkillTree) || gInterface.CheckWindow(Interface::ObjWindow::FullMap)
		|| (gInterface.CheckWindow(Interface::Inventory)
			&& gInterface.CheckWindow(Interface::ExpandInventory)
			&& gInterface.CheckWindow(Interface::Store))
		|| (gInterface.CheckWindow(Interface::Inventory)
			&& gInterface.CheckWindow(Interface::Warehouse)
			&& gInterface.CheckWindow(Interface::ExpandWarehouse)))
	{
		gInterface.Data[eBossGuildWindow].OnShow = false;
		return;
	}

	if (!gInterface.Data[eBossGuildWindow].OnShow)
	{
		return;
	}


	float MainWidth = 340;
	float MainHeight = 200.0;

	float StartY = 80.0;
	float StartX = (MAX_WIN_WIDTH / 2) - (MainWidth / 2);
	g_pBCustomMenuInfo->gDrawWindowCustom(&StartX, &StartY, MainWidth, MainHeight, eBossGuildWindow, gOther.Text_Chung[22]);
	float StartBody = StartY;
	float MainCenter = StartX + (MainWidth / 3);
	float ButtonX = MainCenter - (float)(29.0 / 2);

	DWORD Color = eGray100;
	float CuaSoW = MainWidth;
	//====================================
	float TextStartY = StartY;

	int ct = 4;

	if (gCBBossGuild->CheDo1 == 1)
	{
		TextDraw(g_hFont, (StartX + (CuaSoW / 10)) - 30, TextStartY + (12 * ct++), 0xEBFFFBFF, eExcellentS15, CuaSoW, 0, 1, "• Tiêu diệt các Boss Phù Thủy, Chiến Binh, Tiên Nữ"); //
		TextDraw(g_hFont, (StartX + (CuaSoW / 10)) - 30, TextStartY + (12 * ct++), 0xEBFFFBFF, eExcellentS15, CuaSoW, 0, 1, "• Mỗi thành viên sẽ đem lại %d điểm Guild chiến thắng", gCBBossGuild->Score); //
		TextDraw(g_hFont, (StartX + (CuaSoW / 10)) - 30, TextStartY + (12 * ct++), 0xEBFFFBFF, eExcellentS15, CuaSoW, 0, 1, "• Mỗi thành viên sẽ nhận được %d Wcoi khi Guild chiến thắng", gCBBossGuild->WCoinC); //

		TextDraw(g_hFont, (StartX + (CuaSoW / 10)) - 30, TextStartY + (12 * ct++), 0xEBFFFBFF, eExcellentS15, CuaSoW, 0, 1, "• Nhân vật tiêu diệt Boss Phù Thủy sẽ nhận được %d Wcoi", gCBBossGuild->PhuThuyWCoinC); //
		TextDraw(g_hFont, (StartX + (CuaSoW / 10)) - 30, TextStartY + (12 * ct++), 0xEBFFFBFF, eExcellentS15, CuaSoW, 0, 1, "• Nhân vật tiêu diệt Boss Chiến Binh sẽ nhận được %d Wcoi", gCBBossGuild->ChienBinhWCoinC); //
		TextDraw(g_hFont, (StartX + (CuaSoW / 10)) - 30, TextStartY + (12 * ct++), 0xEBFFFBFF, eExcellentS15, CuaSoW, 0, 1, "• Nhân vật tiêu diệt Boss Tiên Nữ sẽ nhận được %d Wcoi", gCBBossGuild->TienNuWCoinC); //
		TextDraw(g_hFont, (StartX + (CuaSoW / 10)) - 30, TextStartY + (12 * ct++), 0xEBFFFBFF, 0x0, CuaSoW, 0, 1, ""); //
		TextDraw(g_hFont, (StartX + (CuaSoW / 10)) - 30, TextStartY + (12 * ct++), 0xEBFFFBFF, eRed, CuaSoW, 0, 1, "=> LƯU Ý HÃY ĐI CŨNG GUILD ĐỂ TRÁNH ĂN HÀNH NGẬP MỒM <="); //
	}

	if (gCBBossGuild->CheDo2 == 1)
	{
		TextDraw(g_hFont, (StartX + (CuaSoW / 10)) - 30, TextStartY + (12 * ct++), 0xEBFFFBFF, eExcellentS15, CuaSoW, 0, 1, "• Tiêu diệt các Boss Thuật Sĩ, Thiết Binh, Đấu Sĩ, Chúa Tể"); //
		TextDraw(g_hFont, (StartX + (CuaSoW / 10)) - 30, TextStartY + (12 * ct++), 0xEBFFFBFF, eExcellentS15, CuaSoW, 0, 1, "• Mỗi thành viên sẽ đem lại %d điểm Guild chiến thắng", gCBBossGuild->Score); //
		TextDraw(g_hFont, (StartX + (CuaSoW / 10)) - 30, TextStartY + (12 * ct++), 0xEBFFFBFF, eExcellentS15, CuaSoW, 0, 1, "• Mỗi thành viên sẽ nhận được %d Wcoi khi Guild chiến thắng", gCBBossGuild->WCoinC); //

		TextDraw(g_hFont, (StartX + (CuaSoW / 10)) - 30, TextStartY + (12 * ct++), 0xEBFFFBFF, eExcellentS15, CuaSoW, 0, 1, "• Nhân vật tiêu diệt Boss Thuật Sĩ sẽ nhận được %d Wcoi", gCBBossGuild->ThuatSiWCoinC); //
		TextDraw(g_hFont, (StartX + (CuaSoW / 10)) - 30, TextStartY + (12 * ct++), 0xEBFFFBFF, eExcellentS15, CuaSoW, 0, 1, "• Nhân vật tiêu diệt Boss Thiết Binh sẽ nhận được %d Wcoi", gCBBossGuild->ThietBinhWCoinC); //
		TextDraw(g_hFont, (StartX + (CuaSoW / 10)) - 30, TextStartY + (12 * ct++), 0xEBFFFBFF, eExcellentS15, CuaSoW, 0, 1, "• Nhân vật tiêu diệt Boss Đấu Sĩ sẽ nhận được %d Wcoi", gCBBossGuild->DauSiWCoinC); //
		TextDraw(g_hFont, (StartX + (CuaSoW / 10)) - 30, TextStartY + (12 * ct++), 0xEBFFFBFF, eExcellentS15, CuaSoW, 0, 1, "• Nhân vật tiêu diệt Boss Chúa Tể sẽ nhận được %d Wcoi", gCBBossGuild->ChuaTeWCoinC); //
		TextDraw(g_hFont, (StartX + (CuaSoW / 10)) - 30, TextStartY + (12 * ct++), 0xEBFFFBFF, 0x0, CuaSoW, 0, 1, ""); //
		TextDraw(g_hFont, (StartX + (CuaSoW / 10)) - 30, TextStartY + (12 * ct++), 0xEBFFFBFF, eRed, CuaSoW, 0, 1, "=> LƯU Ý HÃY ĐI CŨNG GUILD ĐỂ TRÁNH ĂN HÀNH NGẬP MỒM <="); //
	}

	//============Button
	float SizeButtonW = 95.0;
	float SizeButtonH = 19.0;
	float BStartX = StartX + (MainWidth / 2);
	//=====================Button 1
	if (SEASON3B::CheckMouseIn(BStartX - 50, (StartY + MainHeight) - 30, SizeButtonW, SizeButtonH) == 1)
	{
		glColor3f(1.0f, 0.6742f, 0.15f);
		if (GetTickCount() - gInterface.Data[eBossGuildWindow].EventTick > 300) //Click
		{
			if (GetKeyState(1) & 0x8000)
			{

				glColor3f(0.72f, 0.438f, 0.0432f);
				CGMoveBossGuild(1);
				gInterface.Data[eBossGuildWindow].OnShow = 0;
				gInterface.Data[eBossGuildWindow].EventTick = GetTickCount();
			}
		}
	}
	else {
		glColor3f(1.0f, 1.0f, 1.0f);

	}
	RenderBitmap(CNewUITrade::IMAGE_TRADE_NICK_BACK, BStartX - 50, (StartY + MainHeight) - 30, SizeButtonW, SizeButtonH, 0.0, 0.0, 0.64999998, 0.80000001, 1, 1, 0);
	TextDraw((HFONT)g_hFont, BStartX - 50, (StartY + MainHeight) - 25, 0xE6FCF7FF, 0x0, SizeButtonW, 0, 3, gOther.Text_Chung[28]); //Buoc Vao
	//=====================================
	glColor3f(1.0f, 1.0f, 1.0f); //Xoa mau
}
void CBBossGuild::DrawBossGuildMap()
{
	if (SceneFlag < MAIN_SCENE)
	{
		return;
	}

	float InfoW = 100;
	float InfoH = 40;
	float InfoX = MAX_WIN_WIDTH - (InfoW + 20) + 8;
	float InfoY = MAX_WIN_HEIGHT - (InfoH + 60);

	if (gMapManager.WorldActive == 105)
	{
		//pDrawGUI(31650, 412.0f, 362.0f, 227.0f, 87.0f);
		//================ Tinh Toan Time
		if ((GetTickCount() - gInterface.Data[eTIME].EventTick) > 1000)
		{
			if (BossGuild_TimeConLai > 0)
			{
				BossGuild_TimeConLai = BossGuild_TimeConLai - 1;
			}
			gInterface.Data[eTIME].EventTick = GetTickCount();
		}

		int hours = BossGuild_TimeConLai / 3600;
		int minutes = (BossGuild_TimeConLai / 60) % 60;
		int seconds = BossGuild_TimeConLai % 60;


		g_pBCustomMenuInfo->DrawInfoBox(InfoX, InfoY, InfoW, InfoH, 0x00000096, 0, 0);

		TextDraw(g_hFontBold, InfoX, InfoY + 3, 0xE6FCF7FF, 0xFFBB0064, InfoW + 7, 0, 3, "Boss Guild");
		TextDraw(g_hFontBold, InfoX, InfoY + 20, 0xE6FCF7FF, 0x00FFE564, InfoW + 7, 0, 3, "Thời gian còn lại");
		TextDraw(g_hFontBold, InfoX, (InfoY + InfoH) - 7, eWhite, 0x0, InfoW + 10, 0, 3, " %02d:%02d:%02d", hours, minutes, seconds);

		//=================================

		//SetByte(0x0059AF53 + 2, 0);
		//SetByte(0x0059B248 + 2, 0);
	}
}
void CBBossGuild::RecvProtocol(BYTE* Recv)
{
	if (!Recv) return;

	INFOBOSSGUILD_SENDCLIENT* lpMsg = (INFOBOSSGUILD_SENDCLIENT*)Recv;
	if ((GetTickCount() - gInterface.Data[eBossGuildWindow].EventTick) > 300)
	{
		gInterface.Data[eBossGuildWindow].EventTick = GetTickCount();
		gInterface.Data[eBossGuildWindow].OnShow ^= 1;
	}
}
void CBBossGuild::BossGuildRecv(BYTE* Recv)
{
	if (!Recv) return;

	BOSSGUILD_UPDATE* lpMsg = (BOSSGUILD_UPDATE*)Recv;

	this->CheDo1 = lpMsg->CheDo1;
	this->CheDo2 = lpMsg->CheDo2;
	this->KillBoss = lpMsg->KillBoss;
	this->Score = lpMsg->Score;
	this->WCoinC = lpMsg->WCoinC;
	this->PhuThuyWCoinC = lpMsg->PhuThuyWCoinC;
	this->ChienBinhWCoinC = lpMsg->ChienBinhWCoinC;
	this->TienNuWCoinC = lpMsg->TienNuWCoinC;
	this->ThuatSiWCoinC = lpMsg->ThuatSiWCoinC;
	this->ThietBinhWCoinC = lpMsg->ThietBinhWCoinC;
	this->DauSiWCoinC = lpMsg->DauSiWCoinC;
	this->ChuaTeWCoinC = lpMsg->ChuaTeWCoinC;

	this->BossGuild_TimeConLai = 0;
}